#include "WebSocketClientSession.h"
#include "WebSocketUrl.h"
#include "c74_min.h"
#include "../shared/ohlano_min.h"

using namespace c74::min;


class websocketclient : public object<websocketclient> {
public:

	MIN_DESCRIPTION{ "WebSockets for Max! (Client)" };
	MIN_TAGS{ "net" };
	MIN_AUTHOR{ "Jonas Ohland" };
	MIN_RELATED{ "udpsend, udpreceive" };

	inlet<> main_inlet {this, "(anything) data in"};
	outlet<> data_out{this, "data out"};
	outlet<> status_out{ this, "status out" };


	ohlano::state_relevant_value<long> val{ [this](long val) { cout << "value changed!" << endl; } };

	attribute<long> port { this, "port", 80,
		setter{[this](const c74::min::atoms& args, int inlet) -> c74::min::atoms {
			cout << "got: " << std::string(args[0]) << endl;
			val = static_cast<long>(args[0]);
			return args;
		}},
		description{ "remote port to connect to" },
		range{ 0, 65535 },
	};
	
	websocketclient(const atoms& args = {}) {

		cout << "hello!" << endl;

		for(auto& arg : args) {

			cout << "processing: " << std::string(arg) << endl;

			WebSocketUrl::error_code ec;
			WebSocketUrl url;

			switch (arg.a_type) {

			case c74::max::e_max_atomtypes::A_SYM:

				url = WebSocketUrl::from_string(arg, ec);

				if (ec != WebSocketUrl::error_code::SUCCESS)
					cerr << "symbol argument could not be decoded to an url" << endl;

				cout << "assuming host: " << url.get_host() << ", port: " << url.get_port() << ", path: " << url.get_handshake() << endl;

				break;

			case c74::max::e_max_atomtypes::A_FLOAT:

				cerr << "float not supported as argument" << endl;
				break;

			case c74::max::e_max_atomtypes::A_LONG:

				cout << "int" << endl;

				break;

			default:

				cerr << "unsupported argument type" << endl;
				break;

			}
		}

	}


private:

	ohlano::console_stream_adapter console_adapter{ [this](std::string str) { cout << str << endl; }, true};
	ohlano::console_stream_adapter console_error_adapter{ [this](std::string str) { cerr << str << endl; }, true };
	
 
	ohlano::WebSocketClientSession session{ 
		console_adapter, 
		console_error_adapter 
	};

};

MIN_EXTERNAL(websocketclient);