#pragma once
#include "net_url.h"
#include "write_queue.h"
#include "ohlano.h"
#include <cassert>
#include <atomic>
#include <functional>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/steady_timer.hpp>

namespace ohlano {

	template<typename StreamType, typename MessageType = std::string>
	class connection {
	public:

		typedef std::function<void(boost::system::error_code)> connection_handler_type;
		typedef std::function<void(std::string, size_t)> message_received_handler_type;


		typedef enum status_codes {
			OFFLINE, ONLINE, BLOCKED, ABORTED
		} status_t;

		connection() = default;

		explicit connection(net_url<>& url, boost::asio::io_context& ctx) : url_(url), ctx_(ctx), stream_(ctx_), out_queue_(&stream_)
		{}

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

		write_queue<std::string, typename StreamType>& wq() { return out_queue_; }

		void connect(connection_handler_type handler) {

			status_.store(status_t::BLOCKED);

			assert(url_.valid());
			assert(url_.is_resolved());

			boost::asio::async_connect(
				stream_.next_layer(),
				url_.endpoints(),
				std::bind(
					&connection::connect_handler,
					this,
					std::placeholders::_1,
					handler
				)
			);
		}

		void begin_read(message_received_handler_type handler) {
			read_handler_ = handler;
			perform_read();
		}

		void close() {

			close_tmt = std::make_unique<boost::asio::steady_timer>(stream_.get_executor().context(), std::chrono::seconds(1));

			close_tmt->async_wait([=](boost::system::error_code ec) {
				if (!ec.failed()) {
					DBG("canceling open socket tasks...")
					stream_.next_layer().cancel();
				}
			});
			DBG("close tmt set");

			if (status() == status_codes::ONLINE) {
				stream_.async_close(
					boost::beast::websocket::close_code::normal,
					std::bind(
						&connection::close_handler,
						this,
						std::placeholders::_1
					)
				);
			}
		}


	private:

		net_url<> url_;

		boost::asio::io_context& ctx_;
		StreamType stream_;
		boost::beast::multi_buffer buffer_;

		write_queue<std::string, typename StreamType> out_queue_;

		std::atomic<status_t> status_;

		message_received_handler_type read_handler_ = nullptr;

		std::unique_ptr<boost::asio::steady_timer> close_tmt;


		void connect_handler(boost::system::error_code ec, connection_handler_type handler) {

			if (!ec.failed()) {
				stream_.async_handshake(url_.host(), url_.path(),
					std::bind(
						&connection::handshake_handler,
						this,
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
						this,
						std::placeholders::_1,
						std::placeholders::_2
					)
				);
			}
		}

		void read_handler(boost::system::error_code ec, size_t bytes_transferred) {

			if (!ec.failed()) {

				if (read_handler_) {
					read_handler_(MessageType(boost::beast::buffers_to_string(buffer_.data())), bytes_transferred);
				}

				buffer_.consume(bytes_transferred);

				perform_read();
			}
			else {

			}
		}

		void close_handler(boost::system::error_code ec) {
			if (close_tmt) {
				close_tmt->cancel();
			}
		}
	};
}