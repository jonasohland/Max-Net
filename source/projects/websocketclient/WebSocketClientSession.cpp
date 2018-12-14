#include "WebSocketClientSession.h"

namespace ohlano {

WebSocketClientSession::WebSocketClientSession(console_stream_adapter cout_, console_stream_adapter cerr_) : post(cout_), error(cerr_), res(ioc){

    ioc.stop();

}


WebSocketClientSession::~WebSocketClientSession()
{

	DBG("SESSION DESTRUCTOR");

	if (session) {

		DBG("DISCONNECTING FROM DESTRUCTOR");

		disconnect();

		session.reset();
	}
}

void WebSocketClientSession::connect() {

    if(ioc.stopped()){

		session = std::make_shared<BeastSession>(ioc, post, error);
        
        if(client_thread.joinable()){
            client_thread.join();
        }
        
		session->set_url(url);
        session->connect();
        
        client_thread = std::thread(std::bind(&WebSocketClientSession::run_io_context, this));
        
    } else {
		error("session already running");
    }
}

void WebSocketClientSession::disconnect() {

    if(!ioc.stopped()){
		post("disconnecting...");
		session->disconnect();
    } else {

    }

	if (client_thread.joinable()) {
		client_thread.join();
	}

	session->clear_output_queue();
}

void WebSocketClientSession::send(std::string msg) {
	if (session) {
		session->send_queue(msg);
	}
}

bool WebSocketClientSession::setUrl(net_url<> _url){
	url = _url;
    return true;
}



void WebSocketClientSession::report_status(){

	if (session) {
		
		post << ((session->is_online()) ? "online" : "offline") << "host:" << url.host() << "port:" << url.port() << endl;


	}
	
}


}//namespace ohlano
