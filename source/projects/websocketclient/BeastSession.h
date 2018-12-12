//
//  BeastSession.hpp
//  max-external
//
//  Created by Jonas Ohland on 01.11.18.
//

#ifndef BeastSession_hpp
#define BeastSession_hpp

#include <boost/asio/connect.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/post.hpp>

#include <thread>
#include <functional>
#include <atomic>
#include <deque>

#include "WebSocketUrl.h"
#include "../shared/write_queue.h"
#include "../shared/ohlano.h"
#include "../shared/ohlano_min.h"


using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

namespace ohlano {

    class BeastSession : public std::enable_shared_from_this<BeastSession> {
        
    public:
        
        

        OHLANO_NODEFAULT(BeastSession);
        OHLANO_NOCOPY(BeastSession);
        
        ~BeastSession() {
			DBG("BEAST SESSION DECTRUCTOR");
		}
        
        bool is_online() noexcept { return session_online.load(); }
        bool blocked() noexcept { return is_blocked.load(); }
        void block() noexcept { is_blocked.store(true); DBG("Stream is blocked now"); }
        void unblock() noexcept { is_blocked.store(false); DBG("Stream is free now"); }

		void send_queue(std::string msg);
        
        void cancel_socket();
        
        void clear_output_queue();
        
        explicit BeastSession(boost::asio::io_context& _ioc, console_stream_adapter post_adapter, console_stream_adapter error_adapter);
        
        void set_url(WebSocketUrl _url);

		WebSocketUrl& get_url();
        
        void connect();
        void send(std::string input);
        
        void send_impl(std::string input);
        
        
        
        void disconnect();
        
        void on_resolve(boost::system::error_code ec, tcp::resolver::results_type results);
        
        void on_connect(boost::system::error_code ec);
        
        void on_handshake(boost::system::error_code ec);
        
        void send_next();
        
        void on_write(boost::system::error_code ec, std::size_t bytes);
        
        void on_read(boost::system::error_code ec, std::size_t bytes);
        
        void on_close(boost::system::error_code ec);
        
	private:
        
		// networking objects
        tcp::resolver resolver;
        websocket::stream<tcp::socket> ws;
		boost::beast::multi_buffer buffer;

		write_queue<std::string, websocket::stream<tcp::socket>> out_queue{&ws};

		//executors
		boost::asio::io_context& ioc;
		boost::asio::io_context::strand _send_strand;

		//timers etc
		boost::asio::steady_timer disconnector;
		boost::asio::steady_timer sender_timeout;

		void set_disconnect_tmt();


		//logging
		console_stream_adapter post;
		console_stream_adapter error;
        
        //status
        std::atomic<bool> is_blocked;
        std::atomic<bool> session_online;
        
		//status setters
		void online() noexcept { session_online.store(true); DBG("Stream status: online"); post << "WebSocket online" << endl;  }
		void offline() noexcept { session_online.store(false); DBG("Stream status: offline"); post << "WebSocket offline" << endl; }
        
		//containers
        std::deque<std::string> output_queue;
		
		//internal data
        WebSocketUrl url;

        
    };
} // namespace ohlano



#endif /* BeastSession_hpp */
