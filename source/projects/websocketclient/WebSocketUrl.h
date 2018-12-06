//
//  WebSocketUrl.h
//  websocketclient
//
//  Created by Jonas Ohland on 04.11.18.
//

#ifndef WebSocketUrl_h
#define WebSocketUrl_h

#include <iostream>
#include <string>
#include <boost/tokenizer.hpp>
#include <boost/asio/ip/address.hpp>
#include "../shared/ohlano.h"

class WebSocketUrl {
    
public:
    
    typedef std::string port_type;
    typedef enum error_code {
        FAIL, SUCCESS
    } error_code;
    
    OHLANO_COPY_ASSIGN(WebSocketUrl){
        handshake = other.handshake;
        port = other.port;
        hostname = other.hostname;
        is_valid = other.is_valid;
        return this;
    }
    
    OHLANO_COPY_CONSTRUCT(WebSocketUrl){
        handshake = other.handshake;
        port = other.port;
        hostname = other.hostname;
        is_valid = other.is_valid;
    }
    
    WebSocketUrl() noexcept {
    }
    
    WebSocketUrl(std::string url, error_code& ec){
        
        boost::char_separator<char> sep{ "/" };
        boost::tokenizer<boost::char_separator<char>> tok( url, sep );
        
        auto token_it = tok.begin();
        
        if(*token_it == "ws:"){
            token_it++;
        }
        
        //first legit token
        if(token_it != tok.end()){
            
            //may be host:port combination
            auto port_pos = (*token_it).find(":");
            
            //it is
            if(port_pos != std::string::npos){
                DBG("found host:port combination");
                
                hostname = (*token_it).substr(0, port_pos);

                port = (*token_it).substr(port_pos + 1);
                
				if (!is_number(port)) {
					ec = error_code::FAIL;
					return;
				}

                DBG("found: Host: ", hostname, " Port: ", port);
            
            //it is not
            } else {
                DBG("assuming ", *token_it, " is hostname/ip");
                hostname = *token_it;
                port = "80";
            }
        } else {
            ec = error_code::FAIL;
            return;
        }
        
        //concat the rest to the address
        
        
        if(++token_it != tok.end()){
            
            std::vector<std::string> address;
            
            for(; token_it != tok.end(); token_it++){
                address.push_back(*token_it);
            }
            for(auto token : address){
                handshake.append("/").append(token);
            }
            
        } else {
            handshake = "/";
        }
        
        DBG("Handshake: ", handshake);
        
        is_valid = true;
        
        ec = error_code::SUCCESS;
    }
    
    WebSocketUrl(std::string _host, std::string _port, std::string _handshake){
        hostname = _host;
        port = _port;
        handshake = _handshake;
        is_valid = true;
    }
    
    WebSocketUrl(std::string _host, std::string _port){
        hostname = _host;
        port = _port;
        handshake = "/";
        is_valid = true;
    }
    
    void set_host(std::string _hostname) {
        
        hostname = _hostname;
        
        if(port.size() == 0){
            port = "80";
        }
        if(handshake.size() == 0){
            port = "/";
        }
        
        is_valid = true;
    }
    
    void set_port(std::string _port) {
        port = _port;
    }
    
    void set_handshake(std::string _handshake) {
        handshake = _handshake;
    }
    
    std::string get_host(){
        return hostname;
    }
    
    std::string get_port(){
        return port;
    }
    
    std::string get_handshake(){
        return handshake;
    }
    
    void clear() {
        port.clear();
        hostname.clear();
        handshake.clear();
		is_valid = false;
    }
    
    bool valid(){
        return is_valid;
    }
    
    bool operator==(const WebSocketUrl& other){
        return other.hostname == hostname &&
                other.port == port &&
                other.handshake == handshake;
    }
    
    bool operator!=(const WebSocketUrl& other){
        return !(*this == other);
    }
    
    operator bool() const {
        return is_valid;
    }

	static WebSocketUrl from_string(std::string url, error_code& ec) {
		return WebSocketUrl(url, ec);
	}
    
    
private:

	bool is_number(const std::string& s)
	{
		return !s.empty() && std::find_if(s.begin(),
			s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
	}
    
    std::string hostname;
    port_type port;
    std::string handshake;
    
    bool is_valid = false;
};

#endif /* WebSocketUrl_h */
