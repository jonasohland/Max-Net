//
//  BeastSession.cpp
//  max-external
//
//  Created by Jonas Ohland on 01.11.18.
//

#include "BeastSession.h"

namespace ohlano {

BeastSession::BeastSession(boost::asio::io_context& _ioc) : ioc(_ioc), _send_strand(_ioc), resolver(_ioc), ws(_ioc), is_blocked(false), session_online(false)
{

}

void BeastSession::cancel_socket() {
    ws.next_layer().cancel();
}

void BeastSession::clear_output_queue() {
    output_queue.clear();
    output_queue.shrink_to_fit();
}

void BeastSession::set_url(WebSocketUrl _url){
    url = _url;
}
    
WebSocketUrl BeastSession::get_url() {
    return url;
}
    
void BeastSession::connect(std::string host, std::string service, std::string handshake) {
    
    //connect if socket is offline and not blocked
    
    if (!is_online() && !blocked()) {
        DBG("Resolving Host: ", host, ", Port: ", service)
        resolver.async_resolve(
                               host, service, std::bind(
                                                        &BeastSession::on_resolve,
                                                        shared_from_this(),
                                                        std::placeholders::_1,
                                                        std::placeholders::_2,
                                                        handshake, host
                                                        )
                               );
    } else {
        DBG("did not start resolving");
        DBG("blocked: ", (is_online()) ? "true" : "false", " blocked: ", (is_online()) ? "true" : "false");
    }
}
    
void BeastSession::connect() {
    
    //connect if socket is offline and not blocked
    if(url.valid()){
        if (!is_online() && !blocked()) {
            DBG("Resolving Host: ", url.get_host(), ", Port: ", url.get_port());
            resolver.async_resolve(
                                   url.get_host(), url.get_port(), std::bind(
                                                            &BeastSession::on_resolve,
                                                            shared_from_this(),
                                                            std::placeholders::_1,
                                                            std::placeholders::_2,
                                                            url.get_handshake(), url.get_host()
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

void BeastSession::send(std::string input) {
    
    //push to output queue
    output_queue.push_back(input);
    
    //if more than one task is on the queue return
    if (output_queue.size() > 1) {
        DBG("pushing \"", input, "\" to queue")
        return;
    }
    DBG("sending: ", input)
    //else run send command
    boost::asio::post<boost::asio::io_context::strand>(
                                                       _send_strand,
                                                       std::bind(&BeastSession::send_impl, shared_from_this(), input)
                                                       );
}

void BeastSession::send_impl(std::string input) {
    //perform if socket is online and not blocked
    if (is_online() && !blocked()) {
        ws.async_write(
                       boost::asio::buffer(input),
                       boost::asio::bind_executor<boost::asio::io_context::strand>(_send_strand,
                                                                                   std::bind(
                                                                                             &BeastSession::on_write,
                                                                                             shared_from_this(),
                                                                                             std::placeholders::_1,
                                                                                             std::placeholders::_2
                                                                                             )
                                                                                   )
                       );
    }
    else {
        DBG("This websocket client is offline")
    }
}

void BeastSession::disconnect() {
    
    DBG("sending close frame")
    
    if (!blocked() && is_online()) {
        
        block();
        
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
        DBG("stream is blocked");
    }
}

void BeastSession::on_resolve(boost::system::error_code ec, tcp::resolver::results_type results, std::string handshake, std::string host) {
    
    if (ec) {

        offline();
        DBG("Resolver error: ", ec.message())

    } else if(!blocked()) {
        
        DBG("connecting... ");
        
        boost::asio::async_connect(
                                   ws.next_layer(),
                                   results.begin(),
                                   results.end(),
                                   std::bind(
                                             &BeastSession::on_connect,
                                             shared_from_this(),
                                             std::placeholders::_1,
                                             handshake, host
                                             )
                                   );
    }
}

void BeastSession::on_connect(boost::system::error_code ec, std::string handshake, std::string host) {
    
    if (ec) {
        offline();
        DBG("Connection error: ", ec.message())
    }
    else if(!blocked()){
        
        DBG("performing handshake... ");
        
        ws.async_handshake(host, handshake,
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

void BeastSession::send_next() {
    
    DBG("Sending next message from Queue")
    
    boost::asio::post<boost::asio::io_context::strand>(_send_strand, [=] () {
        ws.async_write(
                       boost::asio::buffer(output_queue.front()),
                       boost::asio::bind_executor<boost::asio::io_context::strand>(_send_strand,
                                                                                   std::bind(
                                                                                             &BeastSession::on_write,
                                                                                             shared_from_this(),
                                                                                             std::placeholders::_1,
                                                                                             std::placeholders::_2
                                                                                             )
                                                                                   )
                       );
    });
}

void BeastSession::on_write(boost::system::error_code ec, std::size_t bytes) {
    
    output_queue.pop_front();
    
    if (ec) {
        DBG("Write Error: ", ec.message())
    }
    else {
        if (!output_queue.empty() && !blocked()) {
            send_next();
        }
    }
    
    
    DBG("done writing ", bytes, " bytes")
}

void BeastSession::on_read(boost::system::error_code ec, std::size_t bytes) {
    
    if (ec == websocket::error::closed) {
        DBG("stopped reading: ", ec.message())
        offline();
    }
    else if (ec.value() == 995) {
        DBG("stopped reading: ", ec.message())
        offline();
    }
    else if (ec.failed()) {
        DBG("stopped reading: ", ec.message(), " code: ", ec.value())
        offline();
    }
    else {

        DBG("received ", bytes, " bytes of data: ", boost::beast::buffers_to_string(buffer.data()));
        
        buffer.consume(bytes);
        
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
        DBG("Connection closed")
        offline();
    }
    unblock();
    
}


} //namespace ohlano


