#include "WebSocketClientSession.h"

namespace ohlano {

WebSocketClientSession::WebSocketClientSession(console_stream_adapter cout_, console_stream_adapter cerr_) : post(cout_), error(cerr_){

    session = std::make_shared<BeastSession>(ioc);

	post << "created session" << endl;

    ioc.stop();

}


WebSocketClientSession::~WebSocketClientSession()
{
}

void WebSocketClientSession::connect() {

    if(ioc.stopped()){
        
        if(client_thread.joinable()){
            client_thread.join();
        }
        
        session->connect();
        
        client_thread = std::thread(std::bind(&WebSocketClientSession::run_io_context, this));
        
        DBG("Client Thread running with pid: ", client_thread.get_id());
        
    } else {
       
    }
}
    
void WebSocketClientSession::connect(std::string host, std::string port, std::string handshake){
    
    if(ioc.stopped()){
        
        if(client_thread.joinable()){
            client_thread.join();
        }
        client_thread = std::thread(std::bind(&WebSocketClientSession::run_io_context, this));
        
    } else {
		
    }
}

void WebSocketClientSession::disconnect() {
    if(!ioc.stopped()){
        end_session();
    } else {
    }
}

bool WebSocketClientSession::setUrl(std::string input_str){
    
    WebSocketUrl::error_code ec;
    
    WebSocketUrl url { input_str, ec };
    
    if(ec != WebSocketUrl::error_code::FAIL){
        
        DBG("success parsing url");
        
        if(url != session->get_url()){
            
            DBG("stating dc -> cn process");
            
            end_session();
            session->set_url(url);
            connect();
        }
    }
    
    return true;
}

void WebSocketClientSession::end_session() {
    
    boost::asio::steady_timer timer(observer_ioc);
    
    timer.expires_from_now(std::chrono::seconds(5));
    
    timer.async_wait([=] (boost::system::error_code ec) {
        if(!ec) {
            try {
                session->cancel_socket();
            } catch (std::exception ex) {
            }
        }
    });
    
    std::thread cancel_thread(std::bind(&WebSocketClientSession::run_obs_io_context, this));
    
	session->disconnect();
    
    if(client_thread.joinable()){
        client_thread.join();
    }
    
    timer.cancel();
    cancel_thread.join();
    
    session->clear_output_queue();
}


void WebSocketClientSession::report_status(){
}


}//namespace ohlano
