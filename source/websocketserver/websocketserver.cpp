#include <mutex>
#include <thread>

#include "../shared/net_url.h"
#include "../shared/connection.h"

#include "../shared/messages/generic_max_message.h"
#include "../shared/messages/proto_message_wrapper.h"
#include "../shared/messages/proto_message_base.h"


#include "../shared/devices/devices.h"

#include "../shared/ohlano_min.h"

#include "c74_min.h"


class websocketserver : public c74::min::object<websocketserver> {

	using websocket_connection = ohlano::connection<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>, ohlano::max_message>;

	c74::min::attribute<long long> port{ this, "port", -1, min_wrap_member(&websocketserver::handle_port_change) };
	bool is_set_port() { return port != -1; }

	c74::min::attribute<c74::min::symbol> address{ this, "address", "0.0.0.0", min_wrap_member(&websocketserver::handle_address_change) };

	boost::asio::ip::tcp::endpoint make_endpoint(c74::min::attribute<c74::min::symbol>& addr, c74::min::attribute<long long>& prt) { 
		return boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(std::string(addr.get().c_str())), static_cast<unsigned short>(prt.get())); 
	}

public:

	explicit websocketserver(const c74::min::atoms& args = {}) {

		if (args.size() > 0) {
			for (auto& arg : args) {
				switch (arg.a_type) {
				case c74::max::e_max_atomtypes::A_LONG:
					break;
				case c74::max::e_max_atomtypes::A_FLOAT:
					break;
				case c74::max::e_max_atomtypes::A_SYM:
					break;
				default:
					break;
				}
			}


			if (!is_set_port()) {
				port = 80;
			}

			listener_.start_listen(make_endpoint(address, port),

				[=](boost::system::error_code ec, boost::asio::ip::tcp::socket&& sock) {

				if (ec) {
					return;
				}

				DBG("accepting new connection from: ", sock.remote_endpoint().address().to_string());

				connections_.push_back(std::make_shared<websocket_connection>(std::forward<boost::asio::ip::tcp::socket>(sock), ctx_, alloc_));

				connections_.back()->accept([=](boost::system::error_code ec) {
					if (ec) {
						return;
					}
					
					DBG("accepted, start reading");

					connections_.back()->begin_read([=](boost::system::error_code ec, ohlano::max_message* msg, size_t bytes_transferred) {

						if (ec) {
							cout << "connection to client closed: " << ec.message() << c74::min::endl;
							return;
						}

						msg->deserialize();
						cout << msg->proto()->DebugString() << c74::min::endl;
						alloc_.deallocate(msg);

					});
				});
			});


			ctx_thread_ = std::make_unique<std::thread>([=]() {
				cout << "running io worker thread" << c74::min::endl;
				ctx_.run();
				cout << "finished running network io worker thread" << c74::min::endl;
			});
		}
	}

	~websocketserver() {

		if (ctx_thread_) {

			listener_.stop_listen();

			for (auto& connection : connections_) {
				if (connection->status() == websocket_connection::status_t::ONLINE) {
					connection->close([](boost::system::error_code ec) {
						DBG("Closed connection");
					});
				}
			}

			if (work_.owns_work()) {
				work_.reset();
			}

			if (ctx_thread_->joinable()) {
				ctx_thread_->join();
			}
		}
	}

#define VER(v) #v

	/* ------------- handlers ------------- */

	c74::min::atoms handle_address_change(const c74::min::atoms& args, int inlet) {
		return args;
	}

	c74::min::atoms handle_port_change(const c74::min::atoms& args, int inlet) {
		return args;
	}

	c74::min::message<> version{ this, "anything", "print version number", [=](const c74::min::atoms& args, int inlet) -> c74::min::atoms {

#ifdef VERSION_TAG
			cout << "WebSocket Server for Max " << STR(VERSION_TAG) << "-" << STR(CONFIG_TAG) << "-" << STR(OS_TAG) << c74::min::endl;
#else
			cout << "test build" << c74::min::endl;
#endif
			return args;
		}
	};

private:

	boost::asio::io_context ctx_;
	boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_{ ctx_.get_executor() };
	
	ohlano::listener listener_{ctx_};
	ohlano::max_message::factory alloc_;

	std::unique_ptr<std::thread> ctx_thread_;
	boost::asio::ip::tcp::endpoint local_endpoint_;
	std::vector<std::shared_ptr<websocket_connection>> connections_;

};


void ext_main(void* r) {

	GOOGLE_PROTOBUF_VERIFY_VERSION;

#ifdef VERSION_TAG
	c74::max::object_post(nullptr, "WebSocket Server for Max // (c) Jonas Ohland 2018 -- %s-%s-%s built: %s", STR(VERSION_TAG), STR(CONFIG_TAG), STR(OS_TAG), __DATE__);
#else
	c74::max::object_post(nullptr, "WebSocket Server for Max // (c) Jonas Ohland 2018 -- built %s - test build", __DATE__);
#endif
	
	c74::min::wrap_as_max_external<websocketserver>("websocketserver", __FILE__, r);
}