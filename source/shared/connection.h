#pragma once
#include "net_url.h"
#include "devices/write_queue.h"
#include "messages/generic_max_message.h"
#include "messages/string_message.h"
#include "ohlano.h"
#include <cassert>
#include <atomic>
#include <functional>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/steady_timer.hpp>

namespace ohlano {

    template<typename Stream, typename Message>
	class connection: public std::enable_shared_from_this<connection<Stream, Message>> {
	public:

		typedef std::function<void(boost::system::error_code)> connection_handler_type;
		typedef std::function<void(boost::system::error_code, Message*, size_t)> message_received_handler_type;
		typedef std::function<void(boost::system::error_code)> closed_handler_type;
		typedef std::function<void(boost::system::error_code)> accepted_handler_type;


		typedef enum status_codes {
			OFFLINE, ONLINE, BLOCKED, ABORTED
		} status_t;

        explicit connection(boost::asio::io_context& ctx, typename Message::factory& allocator) : ctx_(ctx), stream_(ctx_), allocator_(allocator)
		{ out_queue_ = std::make_shared<write_queue<Message, Stream>>(&stream_); }

		std::string status_string() {
			switch (status_.load()) {
			case 0:
				return "offline";
			case 1:
				return "online";
			case 2:
				return "blocked";
			case 3:
				return "aborted";
			default:
				return "undefined";
			}

		}

		explicit connection(typename Stream::next_layer_type&& next_layer, boost::asio::io_context& ctx, typename Message::factory& allocator): 
			ctx_(ctx), stream_(std::forward<typename Stream::next_layer_type>(next_layer)), allocator_(allocator)
		{ out_queue_ = std::make_shared<write_queue<Message, Stream>>(&stream_); }

		status_t status() { return status_.load(); }

        std::shared_ptr<write_queue<Message, Stream>>& wq() { return out_queue_; }

		void connect(net_url<> url, connection_handler_type handler) {

			status_.store(status_t::BLOCKED);

			assert(url.valid());
			assert(url.is_resolved());

			

			boost::asio::async_connect(
				stream_.next_layer(),
				url.endpoints(),
				std::bind(
					&connection::connect_handler,
					this->shared_from_this(),
					std::placeholders::_1,
					handler, 
					url
				)
			);
		}

		void accept(accepted_handler_type handler) {
			stream_.async_accept(
				std::bind(
					&connection::accepted_handler,
					shared_from_this(),
					std::placeholders::_1,
					handler
				)
			);
		}

		void begin_read(message_received_handler_type handler) {
			read_handler_ = handler;
			stream_.binary(true);
			perform_read();
		}

		void close(closed_handler_type handler) {

			close_tmt = std::make_unique<boost::asio::steady_timer>(stream_.get_executor().context(), std::chrono::milliseconds(500));

			close_tmt->async_wait([=](boost::system::error_code ec) {

				if (!ec) {
					stream_.next_layer().cancel();


					if (stream_.next_layer().is_open()) {
						stream_.next_layer().close();
					}
					if (stream_.lowest_layer().is_open()) {
						stream_.lowest_layer().close();
					}
				}
			});

			if (status() == status_codes::ONLINE) {
				stream_.async_close(
					boost::beast::websocket::close_code::going_away,
					std::bind(
						&connection::close_handler,
                        std::enable_shared_from_this<connection<Stream, Message>>::shared_from_this(),
						std::placeholders::_1,
						handler
					)
				);
			}
		}



	private:

		boost::asio::io_context& ctx_;
		Stream stream_;
		boost::beast::multi_buffer buffer_;

        std::shared_ptr<write_queue<Message, Stream>> out_queue_;

		typename Message::factory& allocator_;

		std::atomic<status_t> status_;

		message_received_handler_type read_handler_ = nullptr;

		std::unique_ptr<boost::asio::steady_timer> close_tmt;

		void connect_handler(boost::system::error_code ec, connection_handler_type handler, net_url<> url) {

			if (!ec) {
				stream_.async_handshake(url.host(), url.path(),
					std::bind(
						&connection::handshake_handler,
						this->shared_from_this(),
						std::placeholders::_1,
						handler
					)
				);
			}
			else {
				status_.store(status_t::ABORTED);
				handler(ec);
			}
		}

		void handshake_handler(boost::system::error_code ec, connection_handler_type handler) {
			if (ec) {
				status_.store(status_t::ABORTED);
				handler(ec);
			}
			else {
				status_.store(status_t::ONLINE);
				handler(ec);
			}
		}

		void accepted_handler(boost::system::error_code ec, accepted_handler_type handler) {
			if (ec) {
				status_.store(status_t::ABORTED);
			}
			else {
				status_.store(status_t::ONLINE);
			}
			handler(ec);
		}

		void perform_read() {

			if (status() == status_t::ONLINE) {
				stream_.async_read(
					buffer_,
					std::bind(
						&connection::read_handler,
						this->shared_from_this(),
						std::placeholders::_1,
						std::placeholders::_2
					)
				);
			}
		}

		void read_handler(boost::system::error_code ec, size_t bytes_transferred) {

			if (!ec) {

				if (read_handler_) {

					Message* new_msg = static_cast<Message*>(allocator_.allocate());

					Message::from_const_buffers(buffer_.data(), new_msg);

					DBG("received ", bytes_transferred, " bytes");

					read_handler_(ec, new_msg, bytes_transferred);
				}

				buffer_.consume(bytes_transferred);

				perform_read();
			}
			else {
				status_.store(status_t::ABORTED);
				read_handler_(ec, nullptr, bytes_transferred);
			}
		}

		void close_handler(boost::system::error_code ec, closed_handler_type handler) {


			if (close_tmt) {
				close_tmt->cancel();
			}

			status_.store(status_t::OFFLINE);

			handler(ec);
		}
	};
}
