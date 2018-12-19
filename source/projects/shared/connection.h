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
		typedef std::function<void(Message*, size_t)> message_received_handler_type;
		typedef std::function<void(boost::system::error_code)> closed_handler_type;


		typedef enum status_codes {
			OFFLINE, ONLINE, BLOCKED, ABORTED
		} status_t;

		connection() = default;

        explicit connection(net_url<>& url, boost::asio::io_context& ctx, typename Message::factory& allocator) : url_(url), ctx_(ctx), stream_(ctx_), allocator_(allocator)
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

		status_t status() { return status_.load(); }

        std::shared_ptr<write_queue<Message, Stream>>& wq() { return out_queue_; }

		void connect(connection_handler_type handler) {

			status_.store(status_t::BLOCKED);

			assert(url_.valid());
			assert(url_.is_resolved());

			stream_.binary(true);

			boost::asio::async_connect(
				stream_.next_layer(),
				url_.endpoints(),
				std::bind(
					&connection::connect_handler,
					this->shared_from_this(),
					std::placeholders::_1,
					handler
				)
			);
		}

		void begin_read(message_received_handler_type handler) {
			read_handler_ = handler;
			perform_read();
		}

		void close(closed_handler_type handler) {

			close_tmt = std::make_unique<boost::asio::steady_timer>(stream_.get_executor().context(), std::chrono::milliseconds(500));

			close_tmt->async_wait([=](boost::system::error_code ec) {

				if (!ec.failed()) {
					DBG("cancelling socket tasks");
					stream_.next_layer().cancel();

					DBG("closing underlying sockets");

					if (stream_.next_layer().is_open()) {
						stream_.next_layer().close();
					}
					if (stream_.lowest_layer().is_open()) {
						stream_.lowest_layer().close();
					}
				}
			});

			DBG("close tmt set");

			if (status() == status_codes::ONLINE) {
				DBG("closing stream");
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

		net_url<> url_;

		boost::asio::io_context& ctx_;
		Stream stream_;
		boost::beast::multi_buffer buffer_;

        std::shared_ptr<write_queue<Message, Stream>> out_queue_;

		typename Message::factory& allocator_;

		std::atomic<status_t> status_;

		message_received_handler_type read_handler_ = nullptr;

		std::unique_ptr<boost::asio::steady_timer> close_tmt;

		void connect_handler(boost::system::error_code ec, connection_handler_type handler) {

			if (!ec.failed()) {
				stream_.async_handshake(url_.host(), url_.path(),
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
			if (!ec.failed()) {
				status_.store(status_t::ONLINE);
				handler(ec);
			}
			else {
				status_.store(status_t::ABORTED);
				handler(ec);
			}
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

			if (!ec.failed()) {

				if (read_handler_) {

					Message* new_msg = static_cast<Message*>(allocator_.allocate());

					Message::from_const_buffers(buffer_.data(), new_msg);

					DBG("received ", bytes_transferred, " bytes");

					read_handler_(new_msg, bytes_transferred);
				}

				buffer_.consume(bytes_transferred);

				perform_read();
			}
			else {

			}
		}

		void close_handler(boost::system::error_code ec, closed_handler_type handler) {

			DBG("connnection close: ", ec.message());

			if (close_tmt) {
				close_tmt->cancel();
			}

			handler(ec);
		}
	};
}
