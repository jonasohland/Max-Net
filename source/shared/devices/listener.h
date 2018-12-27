#include <boost/asio.hpp>
#include "../ohlano.h"

namespace ohlano {

	class listener {
	public:

		listener() = delete;

		using new_connection_handler = std::function<void(boost::system::error_code, boost::asio::ip::tcp::socket&&)>;

		explicit listener(boost::asio::io_context& ctx) : socket_(ctx), ctx_(ctx) {

		}

		void start_listen(boost::asio::ip::tcp::endpoint endpoint, new_connection_handler handler) {

			acceptor_ = std::make_unique<boost::asio::ip::tcp::acceptor>(ctx_, endpoint);

			acceptor_->async_accept(socket_, std::bind(
				&listener::accept_handler,
				this,
				std::placeholders::_1,
				handler
			));
		}

		void stop_listen() {
			acceptor_->close();
		}


	private:

		void accept_handler(boost::system::error_code ec, new_connection_handler handler) {
			if (ec) {
				handler(ec, std::move(socket_));
				return;
			}

			handler(ec, std::move(socket_));

			acceptor_->async_accept(socket_, std::bind(
				&listener::accept_handler,
				this,
				std::placeholders::_1,
				handler
			));
		}

		boost::asio::ip::tcp::socket socket_;
		std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
		boost::asio::io_context& ctx_;

	};
}
