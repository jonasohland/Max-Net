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

	c74::min::attribute<int> port{ this, "port", -1, min_wrap_member(&websocketserver::handle_port_change) };

	bool is_set_port() { return port != -1; }

	c74::min::attribute<c74::min::symbol> address{ this, "address", "0.0.0.0", min_wrap_member(&websocketserver::handle_address_change) };

	boost::asio::ip::tcp::endpoint make_endpoint(c74::min::attribute<c74::min::symbol>& addr, c74::min::attribute<int>& prt) { 
		return boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(std::string(addr.get().c_str())), static_cast<unsigned short>(prt.get())); 
	}

public:

	struct iolet {

		iolet(c74::min::object_base* obj, std::string inlet_desc, std::string outlet_desc) :
			inlet_(obj, inlet_desc),
			outlet_(obj, outlet_desc),
			output_(&outlet_)
		{

		}

		c74::min::inlet<> inlet_;
		c74::min::outlet<c74::min::thread_check::none, c74::min::thread_action::assert> outlet_;
		ohlano::outlet_output_adapter<ohlano::max_message> output_;

	};

	explicit websocketserver(const c74::min::atoms& args = {}) { 

		int num_cnn = -1;
		int temp_port = -1;

		if (args.size() > 0) {
			for (auto& arg : args) {
				switch (arg.a_type) {
				case c74::max::e_max_atomtypes::A_LONG:

					if (temp_port == -1) {
						temp_port = arg;
					}
					else if(num_cnn == -1){
						num_cnn = arg;
					}
					else {
						cerr << "to many ints in my motherfuckin meal" << c74::min::endl;
					}

					break;

				case c74::max::e_max_atomtypes::A_FLOAT:
					break;
				case c74::max::e_max_atomtypes::A_SYM:
					try {
						auto endp = boost::asio::ip::address::from_string(arg);
						address = endp.to_string();
						break;
					}
					catch(std::exception e){
						cerr << "could not parse symbol argument as ip address" << c74::min::endl;
						break;
					}
				default:
					break;
				}
			}
			
			port = (temp_port != -1) ? temp_port : 80;

			if (num_cnn != -1) {

				bundle_output = false;

				for (int i = 0; i < num_cnn; ++i) {
					iolets_.push_back(std::make_unique<iolet>(this, "input", "output"));
					connections_.push_back(std::shared_ptr<websocket_connection>());
				}

			}
			else {

				iolets_.push_back(std::make_unique<iolet>(this, "input", "output"));

				status_outlet_ = std::make_unique<c74::min::outlet<>>(this, "status_outlet");

				for (int i = 0; i < 24; i++) {
					connections_.push_back(std::shared_ptr<websocket_connection>());
				}

				bundle_output = true;
			}

			listener_.start_listen(make_endpoint(address, port),
			[=](boost::system::error_code ec, boost::asio::ip::tcp::socket&& sock) {

				if (ec) {
					return;
				}

				cout << "accepting new connection from: " << sock.remote_endpoint().address().to_string() << c74::min::endl;

				for (auto connections_it = connections_.begin(); connections_it != connections_.end(); ++connections_it) {

					if (*connections_it) {
						if ((*connections_it)->status() == websocket_connection::status_codes::OFFLINE ||
							(*connections_it)->status() == websocket_connection::status_codes::ABORTED) {
							(*connections_it).reset();
						}

					}

					if (!(*connections_it)) {

						(*connections_it) = std::make_shared<websocket_connection>(std::forward<boost::asio::ip::tcp::socket>(sock), ctx_, alloc_);
						
						(*connections_it)->accept([=](boost::system::error_code ec) {

							if (ec) {
								return;
							}

							if (bundle_output) {
								(*connections_it)->begin_read([=](boost::system::error_code ec, ohlano::max_message* msg, size_t bytes) {
									if (ec) {
										return;
									}

									msg->deserialize();

									(*iolets_.begin())->output_.write(msg);

									alloc_.deallocate(msg);

								});
							}
							else {
								(*connections_it)->begin_read([=](boost::system::error_code ec, ohlano::max_message* msg, size_t bytes) {
									if (ec) {
										return;
									}

									msg->deserialize();

									iolets_.at(connections_it - connections_.begin())->output_.write(msg);

									alloc_.deallocate(msg);

								});
							}

							(*connections_it)->wq()->attach_sent_handler([=](const ohlano::max_message* msg) {
								alloc_.deallocate(msg);
							});

						});

						return;
					}
				}
				sock.close();
				cerr << "connection limit reached" << c74::min::endl;
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
				if (connection) {
					if (connection->status() == websocket_connection::status_t::ONLINE) {
						connection->close([](boost::system::error_code ec) {
							DBG("Closed connection");
						});
					}
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

	/* ------------- handlers ------------- */


	c74::min::atoms handle_data(const c74::min::atoms& args, int inlet) {

		auto& connection = connections_.at(inlet);

		if (connection) {
			if (connection->status() == websocket_connection::status_codes::ONLINE) {

				auto msg = alloc_.allocate();

				msg->push_atoms(args);

				if (msg->serialize()) connection->wq()->submit(msg);
			}
		}

		return args;
	}

	c74::min::atoms handle_float(const c74::min::atoms& args, int inlet) {

		auto& connection = connections_.at(inlet);

		if (connection) {
			if (connection->status() == websocket_connection::status_codes::ONLINE) {

				auto msg = alloc_.allocate();

				msg->push_atoms(args);

				if (msg->serialize()) connection->wq()->submit(msg);
			}
		}
		return args;
	}

	c74::min::atoms handle_long(const c74::min::atoms& args, int inlet) {

		auto& connection = connections_.at(inlet);

		if (connection) {
			if (connection->status() == websocket_connection::status_codes::ONLINE) {

				auto msg = alloc_.allocate();

				msg->push_atoms(args);

				if (msg->serialize()) connection->wq()->submit(msg);
			}
		}
		return args;
	}

	c74::min::atoms handle_list(const c74::min::atoms& args, int inlet) {

		auto& connection = connections_.at(inlet);

		if (connection) {
			if (connection->status() == websocket_connection::status_codes::ONLINE) {

				auto msg = alloc_.allocate();

				auto tp = static_cast<c74::max::e_max_atomtypes>(args[0].a_type);

				if (args.size() > 2) {

					auto arg_it = args.cbegin();

					while (arg_it->a_type == tp) {

						arg_it++;
						if (arg_it == args.end()) {
							break;
						}
					}

					if (arg_it - args.begin() > 3) {
						msg->push_atomarray(args.begin(), arg_it, tp);
						for (; arg_it != args.end(); arg_it++) {
							msg->push_atom(*arg_it);
						}
					}
					else {
						msg->push_atoms(args);
					}
				}
				else {
					msg->push_atoms(args);
				}

				if(msg->serialize()) connection->wq()->submit(msg);
				
			}
		}
		return args;
	}

	c74::min::message<c74::min::threadsafe::yes> data_input{ this, "anything", "send data", min_wrap_member(&websocketserver::handle_data) };
	c74::min::message<c74::min::threadsafe::yes> list_input{ this, "list", "send list data", min_wrap_member(&websocketserver::handle_list) };
	c74::min::message<c74::min::threadsafe::yes> long_input{ this, "int", "send data", min_wrap_member(&websocketserver::handle_long) };
	c74::min::message<c74::min::threadsafe::yes> float_input{ this, "float", "send data", min_wrap_member(&websocketserver::handle_float) };

	c74::min::atoms handle_address_change(const c74::min::atoms& args, int inlet) {
		return args;
	}

	c74::min::atoms handle_port_change(const c74::min::atoms& args, int inlet) {
		return args;
	}

	c74::min::message<> version{ this, "version", "print version number", [=](const c74::min::atoms& args, int inlet) -> c74::min::atoms {

#ifdef VERSION_TAG
			cout << "WebSocket Server for Max " << STR(VERSION_TAG) << "-" << STR(CONFIG_TAG) << "-" << STR(OS_TAG) << c74::min::endl;
#else
			cout << "test build" << c74::min::endl;
#endif
			return args;
		}
	};


private:

	bool bundle_output = true;

	boost::asio::io_context ctx_;
	boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_{ ctx_.get_executor() };
	
	ohlano::listener listener_{ctx_};
	ohlano::max_message::factory alloc_;

	std::unique_ptr<std::thread> ctx_thread_;
	boost::asio::ip::tcp::endpoint local_endpoint_;

	std::vector<std::shared_ptr<websocket_connection>> connections_;
	std::vector<std::unique_ptr<iolet>> iolets_;

	std::unique_ptr<c74::min::outlet<>> status_outlet_;

};


void ext_main(void* r) {

#ifdef VERSION_TAG
	c74::max::object_post(nullptr, "WebSocket Server for Max // (c) Jonas Ohland 2018 -- %s-%s-%s built: %s", STR(VERSION_TAG), STR(CONFIG_TAG), STR(OS_TAG), __DATE__);
#else
	c74::max::object_post(nullptr, "WebSocket Server for Max // (c) Jonas Ohland 2018 -- built %s - test build", __DATE__);
#endif
	
	c74::min::wrap_as_max_external<websocketserver>("websocketserver", __FILE__, r);
}
