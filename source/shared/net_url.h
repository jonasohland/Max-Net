//
//  websocket_url.h
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
#include <boost/system/error_code.hpp>
#include "ohlano.h"
#include <boost/asio/ip/tcp.hpp>

struct websocket_decoder_template{
    static constexpr const char* const url_separator_char = "/";
    static constexpr const char* const url_prefix = "ws:";
    static constexpr const char* const std_port = "80";
};

struct hypertext_decoder_template{
    static constexpr const char* const url_separator_char = "/";
    static constexpr const char* const url_prefix = "http:";
    static constexpr const char* const std_port = "80";
};

template<typename ProtocolType = boost::asio::ip::tcp, typename UrlTemplate = websocket_decoder_template>
class net_url {
    
public:
    
    typedef std::string port_type;
    typedef std::string hostname_type;
    typedef std::string path_type;

	typedef typename std::vector<typename ProtocolType::endpoint> endpoint_sequence_type;
    
    typedef net_url<boost::asio::ip::tcp, websocket_decoder_template> websocket_url;
    
    typedef enum error_code {
        FAIL, SUCCESS
    } error_code;
    
	typedef std::vector<std::pair<std::string, std::string>> trivial_resolver_results_type;
    typedef typename boost::asio::ip::basic_resolver<ProtocolType>::results_type resolver_results_type;

    net_url* operator=(const net_url& other){
        hostname_ = hostname_type(other.hostname_);
        port_ = port_type(other.port_);
        path_ = path_type(other.path_);
		endpoint_seq = endpoint_sequence_type(other.endpoint_seq);
        return this;
    }
    
    net_url(const net_url& other) :
        hostname_(other.hostname_),
        port_(other.port_),
        path_(other.path_),
		endpoint_seq(other.endpoint_seq)
    {
    }
    
    net_url() noexcept {
        
	}
    
    net_url(std::string url, error_code& ec){
        
        boost::char_separator<char> sep{ UrlTemplate::url_separator_char };
        boost::tokenizer<boost::char_separator<char>> tok( url, sep );
        
        auto token_it = tok.begin();
        
        if(*token_it == UrlTemplate::url_prefix){
            token_it++;
        }
        
        //first legit token
        if(token_it != tok.end()){
            
            //may be host:port combination
            auto port_pos = (*token_it).find(":");
            
            //it is
            if(port_pos != std::string::npos){
                
				hostname_ = (*token_it).substr(0, port_pos);

                port_ = (*token_it).substr(port_pos + 1);
                
				if (!is_number(port_)) {
					ec = error_code::FAIL;
					return;
				}
            
            //it is not
            } else {
                hostname_ = *token_it;
            }
        } else {
            ec = error_code::FAIL;
            return;
        }
        
        
        if(++token_it != tok.end()){
            
            std::vector<std::string> address;
            
            for(; token_it != tok.end(); token_it++){
                address.push_back(*token_it);
            }
            for(auto token : address){
                path_.append(UrlTemplate::url_separator_char).append(token);
            }
            
        } else {
            path_ = UrlTemplate::url_separator_char;
        }
        
		if (is_ip()) {
			endpoint_seq = endpoint_seq_from_string(host(), port_int());
		}
        
        ec = error_code::SUCCESS;
    }
    
    net_url(std::string _host, std::string _port, std::string _handshake){

    }
    
    net_url(std::string _host, std::string _port){

    }
    
    void set_host(const hostname_type& hostname) {
        hostname_ = hostname;
		endpoint_seq = endpoint_seq_from_string(host(), port_int());
    }
    
    bool set_port(const port_type& port) {
        if(is_number(port)){
            port_ = port;
			endpoint_seq = endpoint_seq_from_string(host(), port_int());
            return true;
        } else {
            return false;
        }
        
    }
    
    void set_path(path_type&& path) {
        path_ = path;
    }
    
    void set_host(hostname_type&& hostname) {
        hostname_ = hostname;
		endpoint_seq = endpoint_seq_from_string(host(), port_int());
    }
    
    bool set_port(port_type&& port) {
        if(is_number(std::forward<port_type>(port))){
            port_ = port;
			endpoint_seq = endpoint_seq_from_string(host(), port_int());
            return true;
        } else {
            return false;
        }
    }
    
    void set_path(const path_type& path) {
        path_ = path;
    }
    
    hostname_type host(){
        return hostname_;
    }

    port_type port(){
        
		if (has_port()) {
			return port_;
		}
		else {
            return { UrlTemplate::std_port };
		}
		
    }

	short port_int(bool default_ = true) {
		if (has_port()) {
			return std::stoi(port());
		}
		else {
			if (default_) {
				return std::stoi(UrlTemplate::std_port);
			}
			else {
				return 0;
			}
		}
	}

	endpoint_sequence_type endpoints() {
		return endpoint_seq;
	}

    void set_resolver_results(const resolver_results_type& results) {
		endpoint_seq.clear();
		std::copy(results.begin(), results.end(), std::back_inserter(endpoint_seq));
		endpoint_seq.shrink_to_fit();
    }

	bool is_resolved() {
        return !endpoint_seq.empty();
	}

	std::string get_pretty_resolver_results() {
        return {};
	}
    
    std::string path(){
        return path_;
    }
    
    void clear() {
        port_.clear();
        hostname_.clear();
        path_.clear();
		endpoint_seq.clear();
    }
    
    

	bool has_port() {
		if (is_number(port_)) {
			return port_.size() > 0 && std::stoi(port_) <= 65535;
		}
		else return false;
	}
    
    bool operator==(const net_url& other){
        return other.hostname_ == hostname_ &&
        other.port_ == port_ &&
        other.path_ == path_;
    }
    
    bool operator!=(const net_url& other){
        return !(*this == other);
    }
    
    bool valid() const{
        return !hostname_.empty() && (port_.empty() || is_number(port_));
    }
    
    operator bool() const {
        return valid();
    }

	static net_url from_string(std::string url, error_code& ec) {
		return url_t(url, ec);
	}
    
    
private:

	bool is_number(const std::string& s) const {
		return !s.empty() && std::find_if(s.begin(),
			s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
	}

	bool is_ip() {
		try {
			boost::asio::ip::make_address_v4(host());
			return true;
		}
		catch (...) {
			return false;
		}
	}

	endpoint_sequence_type endpoint_seq_from_string(std::string str, short port) {
		return { typename ProtocolType::endpoint(boost::asio::ip::address::from_string(str), port) };
	}
    
    hostname_type hostname_;
    port_type port_;
    path_type path_;

	endpoint_sequence_type endpoint_seq;
    
};

#endif /* WebSocketUrl_h */
