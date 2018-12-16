#include "WebSocketClientSession.h"
#include "../shared/net_url.h"
#include "../shared/devices/write_queue.h"
#include "../shared/devices/multi_resolver.h"
#include "../shared/messages/generic_max_message.h"
#include "../shared/connection.h"
#include "../shared/ohlano_min.h"
#include "../shared/connection.h"
#include "c74_min.h"
#include <mutex>
#include <thread>

using namespace c74::min;
using namespace std::placeholders;


class websocketclient : public object<websocketclient> {
public:

	MIN_DESCRIPTION{ "WebSockets for Max! (Client)" };
	MIN_TAGS{ "net" };
	MIN_AUTHOR{ "Jonas Ohland" };
	MIN_RELATED{ "udpsend, udpreceive" };

	inlet<> main_inlet{ this, "(anything) data in" };
	outlet<> data_out{ this, "data out" };
	outlet<> status_out{ this, "status out" };

    void changed_port(long val){}

    void changed_host(std::string val){}


	ohlano::state_relevant_value<long> port_val { std::bind(&websocketclient::changed_port, this, _1) };
    ohlano::state_relevant_value<std::string> host_val { std::bind(&websocketclient::changed_host, this, _1) };

	attribute<long long> port { this, "port", 80, min_wrap_member(&websocketclient::set_port),
		description{ "remote port to connect to" }, range{ 0, 65535 }};

	attribute<symbol> host { this, "host", "localhost", min_wrap_member(&websocketclient::set_host)};

	ohlano::generic_max_message test_mess;

	
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
				cout << "running io service thread" << endl;

				client_thread_ptr = std::make_shared<std::thread>([this]() {
					io_context_.run();
					cout << "finished running io service thread" << endl;
				});

				make_connection(url);

			}
			else {
				cout << "no valid websocket address provided" << endl;
			}
		}
		else {
		// nop
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
					connection_ = std::make_shared<ohlano::connection<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>>>(_url, io_context_);
					perform_connect();
				}
				else {
					cerr << "resolving failed: " << ec.message() << endl;
				}
			});
		}
		else {
			cout << "connecting..." << endl;
			connection_ = std::make_shared<ohlano::connection<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>>>(url, io_context_);
			io_context_.post([=]() { perform_connect(); });
		}
	}

	void perform_connect() {
		if (connection_) {
			connection_->connect([=](boost::system::error_code ec) {
				if (!ec.failed()) {
					cout << "conection established" << endl;
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
			connection_->begin_read([=](ohlano::string_message mess, size_t bytes_transferred) {
				cout << "received: " << mess.str() << endl;
			});
		}
	}


	~websocketclient(){

		if (connection_) {
			connection_->close();
		}

		if (work.owns_work()) {
			work.reset();
		}

		if (client_thread_ptr) {
			if (client_thread_ptr->joinable()) {
				client_thread_ptr->join();
			}
		}
	}

	atoms report_status(const atoms& args, int inlet) {
		if (connection_) { status_out.send(connection_->status_string()); }
		else { status_out.send("offline"); }
		return args;
	}

	atoms send_hello(const atoms& args, int inlet) {
		if (connection_) {
			auto mess = std::string("hellooooo!!!!!");
			connection_->wq().submit(ohlano::string_message(mess));
		}
		return args;
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


	message<> status { this, "status", "report status", min_wrap_member(&websocketclient::report_status) };
	message<> set_port_cmd{ this, "port", "set port", min_wrap_member(&websocketclient::set_port) };
	message<> set_host_cmd{ this, "host", "set host", min_wrap_member(&websocketclient::set_host) };
	message<> connect_cmd{ this, "connect", "connect websocket", min_wrap_member(&websocketclient::connect) };

	message<threadsafe::yes> hello { this, "hello", "send hello message", min_wrap_member(&websocketclient::send_hello) };


private:

	/** The executor that will provide io functionality */
	boost::asio::io_context io_context_;

	/** This object will keep the io_context alive as long as the object exists */
	boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work{ io_context_.get_executor() };

	/** This object is responsible for resolving hostnames to ip addresses */
	multi_resolver<boost::asio::ip::tcp> resolver{ io_context_ };

	std::shared_ptr<ohlano::connection<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>>> connection_;

	std::shared_ptr<std::thread> client_thread_ptr;

	std::mutex post_mtx;

	ohlano::console_stream_adapter console_adapter{ [this](std::string str) { cout << str << endl; }, true};
	ohlano::console_stream_adapter console_error_adapter{ [this](std::string str) { cerr << str << endl; }, true };
 
	ohlano::WebSocketClientSession session { 
		console_adapter, 
		console_error_adapter 
	};

};

void ext_main(void* r) {
        c74::max::object_post(nullptr, "WebSockets for Max // (c) Jonas Ohland 2018");
		c74::min::wrap_as_max_external<websocketclient>("websocketclient", __FILE__, r);
}
