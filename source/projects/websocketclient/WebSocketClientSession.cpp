#include "WebSocketClientSession.h"

namespace ohlano {

WebSocketClientSession::WebSocketClientSession(console_stream_adapter cout_, console_stream_adapter cerr_) : post(cout_), error(cerr_){

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

bool WebSocketClientSession::setUrl(WebSocketUrl _url){
	url = _url;
    return true;
}



void WebSocketClientSession::report_status(){

	if (session) {
		
		post << ((session->is_online()) ? "online" : "offline") << "host:" << url.get_host() << "port:" << url.get_port() << endl;

		if (session->get_url().has_resolver_results())
			post << "Address Info:" << session->get_url().get_pretty_resolver_results() << endl;

	}
	
}


}//namespace ohlano
