#include "WebSocketClientSession.h"
#include "../shared/net_url.h"
#include "c74_min.h"
#include "../shared/ohlano_min.h"
#include "../shared/connection.h"
#include <mutex>

using namespace c74::min;
using namespace std::placeholders;


class websocketclient : public object<websocketclient> {
public:

	MIN_DESCRIPTION{ "WebSockets for Max! (Client)" };
	MIN_TAGS{ "net" };
	MIN_AUTHOR{ "Jonas Ohland" };
	MIN_RELATED{ "udpsend, udpreceive" };

	inlet<> main_inlet {this, "(anything) data in"};
	outlet<> data_out{this, "data out"};
	outlet<> status_out{ this, "status out" };
    
    std::mutex hello_mtx;
    

    atoms set_host(const atoms& args, int inlet){ host_val = static_cast<std::string>(args[0]); return args; }

    atoms set_port(const atoms& args, int inlet){ port_val = static_cast<long>(args[0]); return args; }

    void changed_port(long val){}

    void changed_host(std::string val){}


	ohlano::state_relevant_value<long> port_val { std::bind(&websocketclient::changed_port, this, _1) };
    ohlano::state_relevant_value<std::string> host_val { std::bind(&websocketclient::changed_host, this, _1) };

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

					t_url = net_url<>(arg, ec);

					if (ec != net_url<>::error_code::SUCCESS)
						cerr << "symbol argument could not be decoded to an url" << endl;

					if (url.has_port() && t_url.has_port()) {
						cerr << "Found multiple port arguments!" << endl;
					}

					url = t_url;

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
				//session.setUrl(url);
				//session.connect();
			}
			else {
				cout << "no valid websocket address provided" << endl;
			}
		}
		else {
		
		}
	}

	atoms report_status(const atoms& args, int inlet) {
		session.report_status();
		return args;
	}

	atoms send_hello(const atoms& args, int inlet) {
		session.send("hello");
		return args;
	}


	message<> status { this, "status", "report current status", min_wrap_member(&websocketclient::report_status) };

	message<threadsafe::yes> hello { this, "hello", "send hello message", min_wrap_member(&websocketclient::send_hello) };


private:

	ohlano::console_stream_adapter console_adapter{ [this](std::string str) { cout << str << endl; }, true};
	ohlano::console_stream_adapter console_error_adapter{ [this](std::string str) { cerr << str << endl; }, true };
	
    
 
	ohlano::WebSocketClientSession session{ 
		console_adapter, 
		console_error_adapter 
	};

};

void ext_main(void* r) {
        c74::max::cpost("websockets for max (c) Jonas Ohland 2018");
		c74::min::wrap_as_max_external<websocketclient>("websocketclient", __FILE__, r);
}
