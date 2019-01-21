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

			acceptor_ = std::make_unique<boost::asio::ip::tcp::acceptor>(ctx_);

			boost::asio::socket_base::reuse_address option(true);


			boost::system::error_code ec;

			acceptor_->open(endpoint.protocol(), ec);
			if (ec)
			{
				DBG("open error ", ec.message());
				return;
			}

			acceptor_->set_option(option, ec);
			if (ec)
			{
				DBG("addr reuse error ", ec.message());
				return;
			}

			acceptor_->bind(endpoint, ec);
			if (ec)
			{
				DBG("bind error: ", ec.message());
				return;
			}

			acceptor_->listen(boost::asio::socket_base::max_listen_connections, ec);
			if (ec)
			{
				DBG("listen error: ", ec.message());
				return;
			}

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
