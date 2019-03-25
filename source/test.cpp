#include "o.h"

using base_app = o::io::signal_listener_app< o::ccy::safe >;
using udpin = o::io::net::udp_receiver< o::ccy::none >;

struct app : public base_app {
    
    void start() {
        this->app_launch(10);
    }
    
    void join(){
        this->app_join();
    }
    
    virtual void on_app_started() override {
        std::cout << "App started from: " << std::this_thread::get_id() << std::endl;
    }
};


int main() {

    app a;
    
    a.start();
    
    a.join();

    return 0;
}
