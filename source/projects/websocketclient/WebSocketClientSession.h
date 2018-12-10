#pragma once

#include "BeastSession.h"
#include "WebSocketUrl.h"
#include "../shared/ohlano.h"
#include "../shared/ohlano_min.h"

#include <boost/asio/basic_waitable_timer.hpp>
#include <chrono>

#include <thread>
#include <functional>


#include "iiwaPosition.pb.h"





using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

namespace ohlano {

	template<typename T>
	struct state_relevant_value {

		explicit state_relevant_value(std::function<void(T)> func) {
			change_handler = func;
		}

		state_relevant_value* operator=(const state_relevant_value& other) {
			value = other.value;
			change_handler = other.change_handler;
			return this;
		}

		state_relevant_value* operator=(const T& value_in) {

			if (value != value_in) {
				value = value_in;

				if (change_handler != nullptr) {
					change_handler(value);
				}
			}
			return this;
		}

		T value;

		std::function<void(T)> change_handler = nullptr;

		void set_if_changed(T& value_in) {

			if (value != value_in) {
				value = value_in;

				if (change_handler != nullptr) {
					change_handler(value);
				}
			}
		}

		void update(T input) {
			if (input != value) {
				value = input;
				if (change_handler != nullptr) {
					change_handler(value);
				}
			}
		}

		void set_change_handler(std::function<void(T)> func) {
			change_handler = func;
		}

		operator T () {
			return value;
		}
	};

    class WebSocketClientSession
    {
    private:


        std::thread client_thread;
        std::shared_ptr<BeastSession> session;

        
        boost::asio::io_context ioc;
        boost::asio::io_context observer_ioc;

		console_stream_adapter post;
		console_stream_adapter error;

        WebSocketUrl url;
        
		de::hsmainz::iiwa::messages::protocolbuffers::States::Frame frame;
        
    public:
        
        
        
        void run_io_context() {
            
            ioc.restart();
            ioc.run();

			DBG("ioc.run() call returned");
            
        }

        void disconnect();
        void connect();
        
        bool setUrl(WebSocketUrl url);
        
        

        void report_status();

        WebSocketClientSession(console_stream_adapter, console_stream_adapter);
        ~WebSocketClientSession();
        
        OHLANO_NOCOPY(WebSocketClientSession)

    };
}



