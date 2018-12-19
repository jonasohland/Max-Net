#include <mutex>
#include <thread>

#include "../shared/net_url.h"
#include "../shared/connection.h"

#include "../shared/messages/generic_max_message.h"
#include "../shared/messages/proto_message_wrapper.h"
#include "../shared/proto_message_base.h"


#include "../shared/devices/devices.h"

#include "../shared/ohlano_min.h"

#include "c74_min.h"



using namespace c74::min;
using namespace std::placeholders;


class websocketclient : public object<websocketclient> {
public:

	using websocket_stream = boost::beast::websocket::stream<boost::asio::ip::tcp::socket>;
	using websocket_connection = ohlano::connection<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>, ohlano::max_message>;

	MIN_DESCRIPTION{ "WebSockets for Max! (Client)" };
	MIN_TAGS{ "net" };
	MIN_AUTHOR{ "Jonas Ohland" };
	MIN_RELATED{ "udpsend, udpreceive" };

	inlet<> main_inlet{ this, "(anything) data in" };
	outlet<thread_check::any, thread_action::fifo> data_out{ this, "data out" };
	outlet<> status_out{ this, "status out" };

    void changed_port(long val){}

    void changed_host(std::string val){}

    attribute<long> port { this, "port", 80, min_wrap_member(&websocketclient::set_port),
		description{ "remote port to connect to" }, range{ 0, 65535 }};

	attribute<symbol> host { this, "host", "localhost", min_wrap_member(&websocketclient::set_host)};

	explicit websocketclient(const atoms& args = {}) {

		net_url<>::error_code ec;
		net_url<> url;
		net_url<> t_url;

		if (args.size() > 0) {

			for (auto& arg : args) {

				switch (arg.a_type) {
				case c74::max::e_max_atomtypes::A_SYM:

					if (!url) {
						t_url = net_url<>(arg, ec);

						if (ec != net_url<>::error_code::SUCCESS)
							cerr << "symbol argument could not be decoded to an url" << endl;

						if (url.has_port() && t_url.has_port()) {
							cerr << "Found multiple port arguments!" << endl;
						}

						url = t_url;
					}

					break;

				case c74::max::e_max_atomtypes::A_FLOAT:
					cerr << "float not supported as argument" << endl;
					break;
				case c74::max::e_max_atomtypes::A_LONG:
					cout << "long arg: " << std::string(arg) << endl;
					if (!url.has_port()) { url.set_port(arg); }
					else { cerr << "Found multiple port arguments!" << endl; }
					break;
				default:
					cerr << "unsupported argument type" << endl;
					break;
				}

			}

			

			if (url) {

				atoms host_at = { url.host() };
				atoms port_at = { url.port_int() };

				host.set(host_at, false);
				port.set(port_at, false);

				//there is work to do
				cout << "running network io worker thread" << endl;

				client_thread_ptr = std::make_shared<std::thread>([this]() {
					try {
						io_context_.run();
					}
					catch (std::exception const&  ex) {
						cerr << "exception in network io worker thread: " << ex.what() << endl;
					}
					cout << "finished running network io worker thread" << endl;
				});

				make_connection(url);

				dec_worker_.run(4);

			}
			else {
				cout << "no valid websocket address provided" << endl;
			}
		}
	}

	

	void make_connection(net_url<> url) {

		if (!url.is_resolved()) {

			cout << "resolving " << url.host() << endl;

			resolver.resolve(url, [this](boost::system::error_code ec, net_url<> _url) {
				if (!ec.failed()) {
					for (auto& endpoint : _url.endpoints()) {
						cout << "result: " << endpoint.address().to_string() << ":" << endpoint.port() << endl;
					}
					cout << "connecting..." << endl;
					connection_ = std::make_shared<websocket_connection>(_url, io_context_, allocator_);
					perform_connect();
				}
				else {
					cerr << "resolving failed: " << ec.message() << endl;
				}
			});
		}
		else {

			cout << "url:" << url.host() << url.port() << endl;

			cout << "connecting..." << endl;
			connection_ = std::make_shared<websocket_connection>(url, io_context_, allocator_);
			io_context_.post([=]() { perform_connect(); });
		}
	}

	void perform_connect() {
		if (connection_) {
			connection_->connect([=](boost::system::error_code ec) {
				if (!ec.failed()) {

					cout << "connection established" << endl;

					connection_->wq()->attach_sent_handler([=](const ohlano::max_message* msg) {

						allocator_.deallocate(msg);
					});
					begin_read();
				}
				else {
					cerr << "connection error: " << ec.message() << endl;
				}
			});
		}
	}

    
	void begin_read() {
		if (connection_) {
			connection_->begin_read([=](ohlano::max_message* mess, size_t bytes_transferred) {
				mess->deserialize();
				data_out.send(mess->get_atoms());
				allocator_.deallocate(mess);
			});
		}
	}


	~websocketclient(){

		if (connection_) {
			connection_->close([=](boost::system::error_code ec) {
				if (!ec.failed())
					cout << "gracefully closed connection" << endl;
			});
		}

		if (work.owns_work()) {
			work.reset();
		}

		dec_worker_.stop();

		if (client_thread_ptr) {
			if (client_thread_ptr->joinable()) {
				client_thread_ptr->join();
			}
		}

	}

	atoms report_status(const atoms& args, int inlet) {
		if (connection_) { status_out.send(connection_->status_string()); }
		else { status_out.send("no_connection"); }
		return args;
	}

	atoms handle_data(const atoms& args, int inlet) {
		if (connection_) {
			if (connection_->status() == websocket_connection::status_t::ONLINE) {
				auto msg = allocator_.allocate();

				msg->push_atoms(args);

				msg->serialize();

				connection_->wq()->submit(msg);
			}
		}

		return args;
	}

	atoms handle_float(const atoms& args) {
		if (connection_) {
			if (connection_->status() == websocket_connection::status_codes::ONLINE) {
				
				auto arg_it = args.cbegin();

				while (arg_it->a_type == c74::max::e_max_atomtypes::A_FLOAT) {

					arg_it++;
				}

				auto msg = allocator_.allocate();

				msg->push_atomarray(args, arg_it, c74::max::e_max_atomtypes::A_FLOAT);

				msg->push_atoms(args);

				msg->serialize();

				connection_->wq()->submit(msg);

			}
		}
	}

	atoms set_port(const atoms& args, int inlet) {
		return args;
	}

	atoms set_host(const atoms& args, int inlet) {
		return args;
	}

	atoms connect(const atoms& args, int inlet) {
		return args;
	}


	message<> status{ this, "status", "report status", min_wrap_member(&websocketclient::report_status) };
	message<> set_port_cmd{ this, "port", "set port", min_wrap_member(&websocketclient::set_port) };
	message<> set_host_cmd{ this, "host", "set host", min_wrap_member(&websocketclient::set_host) };
	message<> connect_cmd{ this, "connect", "connect websocket", min_wrap_member(&websocketclient::connect) };
	
	message<threadsafe::yes> data_input{ this, "anything", "send data", min_wrap_member(&websocketclient::handle_data) };


private:

	/** The executor that will provide io functionality */
	boost::asio::io_context io_context_;

	/** This object will keep the io_context alive as long as the object exists */
	boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work{ io_context_.get_executor() };

	/** This object is responsible for resolving hostnames to ip addresses */
	multi_resolver<boost::asio::ip::tcp> resolver{ io_context_ };

	ohlano::max_message::factory allocator_;

	std::shared_ptr<websocket_connection> connection_;

	std::shared_ptr<std::thread> client_thread_ptr;

	protobuf_decoder decoder_;
	protobuf_decoder_worker dec_worker_{ decoder_ };

	

	std::mutex post_mtx;

	ohlano::console_stream_adapter console_adapter{ [this](std::string str) { cout << str << endl; }, true};
	ohlano::console_stream_adapter console_error_adapter{ [this](std::string str) { cerr << str << endl; }, true };

};

void ext_main(void* r) {

	GOOGLE_PROTOBUF_VERIFY_VERSION;

    c74::max::object_post(nullptr, "WebSockets for Max // (c) Jonas Ohland 2018");
	c74::min::wrap_as_max_external<websocketclient>("websocketclient", __FILE__, r);
}
