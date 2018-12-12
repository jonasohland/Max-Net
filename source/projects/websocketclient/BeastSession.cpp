//
//  BeastSession.cpp
//  max-external
//
//  Created by Jonas Ohland on 01.11.18.
//

#include "BeastSession.h"

namespace ohlano {

BeastSession::BeastSession(boost::asio::io_context& _ioc, console_stream_adapter post_adapter, console_stream_adapter error_adapter) : 
	resolver(_ioc), ws(_ioc), ioc(_ioc), _send_strand(_ioc), disconnector(_ioc), sender_timeout(_ioc), post(post_adapter), error(error_adapter), is_blocked(false), session_online(false)
{
}

void BeastSession::send_queue(std::string msg) {
	out_queue.submit(msg);
}

void BeastSession::connect() {
    
    //connect if socket is offline and not blocked
    if(url.valid()){
        if (!is_online() && !blocked()) {

			post << "Connecting to:" << url.get_host() << "port:" << url.get_port() << endl;

            DBG("Resolving Host: ", url.get_host(), ", Port: ", url.get_port());
            resolver.async_resolve(
                                   url.get_host(), url.get_port(), std::bind(
                                                            &BeastSession::on_resolve,
                                                            shared_from_this(),
                                                            std::placeholders::_1,
                                                            std::placeholders::_2
                                                            )
                                   );
        } else {
            DBG("did not start resolving");
            DBG("blocked: ", (is_online()) ? "true" : "false", " blocked: ", (is_online()) ? "true" : "false");
        }
    } else {
        DBG("invalid url");
    }
}



void BeastSession::disconnect() {
    
    DBG("starting close process with timeout")
    
    if (!ioc.stopped()) {
        
        block();

		DBG("setting timeout");

		set_disconnect_tmt();
        
        ws.async_close(websocket::close_code::normal,
            boost::asio::bind_executor(_send_strand,
                    std::bind(
                        &BeastSession::on_close,
                        shared_from_this(),
                        std::placeholders::_1
                    )
            )
        );
    }
    else {
        DBG("session not running");
    }
}

void BeastSession::on_resolve(boost::system::error_code ec, tcp::resolver::results_type results) {
    
    if (ec) {

        offline();
        DBG("Resolver error: ", ec.message())

    } else if(!blocked()) {
        
        DBG("connecting... ");

		url.set_resolver_results(results);
        
        boost::asio::async_connect(
                                   ws.next_layer(),
                                   results.begin(),
                                   results.end(),
                                   std::bind(
                                             &BeastSession::on_connect,
                                             shared_from_this(),
                                             std::placeholders::_1
                                             )
                                   );
    }
}

void BeastSession::on_connect(boost::system::error_code ec) {
    
    if (ec) {
        offline();
		DBG("Connection error: ", ec.message());
		error << "Could not connect to:" << url.get_host() << endl;
		ioc.stop();
    }
    else if(!blocked()){
        
        DBG("performing handshake... ");
        
        ws.async_handshake(url.get_host(), url.get_handshake(),
                           std::bind(
                                     &BeastSession::on_handshake,
                                     shared_from_this(),
                                     std::placeholders::_1
                                     )
                           );
    } else {
        DBG("Could not send handshake request because the stream is blocked")
    }
}

void BeastSession::on_handshake(boost::system::error_code ec) {
    
    if (ec) {
        DBG("Handshake error: ", ec.message(), ", disconnecting")
        shared_from_this()->disconnect();
        offline();
		ioc.stop();
    }
    else if(!blocked()){
        
		online();
        ws.async_read(
                      buffer,
                      std::bind(
                                &BeastSession::on_read,
                                shared_from_this(),
                                std::placeholders::_1,
                                std::placeholders::_2
                                )
                      );
    } else {
        DBG("Could not start listening, because the stream is blocked")
    }
}

void BeastSession::on_read(boost::system::error_code ec, std::size_t bytes) {
    
    if (ec == websocket::error::closed) {
        DBG("stopped reading: ", ec.message())
    }
    else if (ec.value() == 995) {
        DBG("stopped reading: ", ec.message())
    }
    else if (ec.failed()) {
        DBG("stopped reading: ", ec.message(), " code: ", ec.value())
        offline();
    }
    else {
        
        buffer.consume(bytes);

		post << "received:" << boost::beast::buffers_to_string(buffer.data()) << "bytes:" << bytes << endl;
        
        ws.async_read(
                      buffer,
                      std::bind(
                                &BeastSession::on_read,
                                shared_from_this(),
                                std::placeholders::_1,
                                std::placeholders::_2
                                )
                      );
    }
}

void BeastSession::on_close(boost::system::error_code ec) {
    
    if (ec) {
        DBG("Error closing the Socket Connection: ")
        
        try {
            ws.next_layer().cancel();
        }
        catch (std::exception ex) {
            DBG("Error canceling open socket Tasks: ", ex.what())
        }
        offline();
    }
    else {
		disconnector.cancel();
        DBG("Connection closed")
        offline();
    }
    unblock();
}

/* connection clonsing, disconnecting, etc */

void BeastSession::cancel_socket() {
	try {
		ws.next_layer().cancel();
	}
	catch (std::exception ex) {
		DBG("Error canceling open socket Tasks: ", ex.what())
	}
}

void BeastSession::set_disconnect_tmt() {

	disconnector.async_wait([=](boost::system::error_code ec) {
		if (!ec) {
			error("operation aborted");
			cancel_socket();
		}
		else {
			DBG("not canceling socket tasks: ", ec.message());
		}
	});

	disconnector.expires_after(std::chrono::seconds(1));

	DBG("timeout set")
}

void BeastSession::clear_output_queue() {
	output_queue.clear();
	output_queue.shrink_to_fit();
}


/* data management */


void BeastSession::set_url(WebSocketUrl _url) {
	url = _url;
}

WebSocketUrl& BeastSession::get_url() {
	return url;
}


} //namespace ohlano


