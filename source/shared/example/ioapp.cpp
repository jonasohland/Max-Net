#include "../include/io_application.h"

//! [ioapp_base_example]
using app_base = o::io::io_app_base<o::threads::single>;

struct app : public app_base {
    
    void start() {
        this->app_launch();
    }
    
    void stop(){
        this->app_allow_exit();
        this->app_join();
    }
    
    virtual void on_app_started() override {
        std::cout << "Hello World!" << std::endl;
    }
};

int main() {
    
    app a;
    
    a.start();
    a.stop();
    
    return 0;
}
//! [ioapp_base_example]

//! [ioapp_call_is_safe_example]
using app_base = o::io::io_app_base< ohlano::threads::none >;

std::mutex mtx;

// prints (call from [thing] is [not] safe)
void print( std::string place, bool is_safe ) {
    std::lock_guard< std::mutex > print_lock( mtx );
    std::cout << "call from " << place << ( ( is_safe ) ? " is safe" : " is not safe" )
    << std::endl;
}

struct app : public app_base {
    
    app() : timer( this->context() ) {}
    
    virtual void on_app_started() override {
        
        print( "startup method", this->call_is_safe() );
        
        this->post( [this]() {
            print( "executor", this->call_is_safe() );
        } );
        
        timer.async_wait( [this]( boost::system::error_code ec ) {
            print( "timer callback", this->call_is_safe() );
        } );
        
        timer.expires_from_now( std::chrono::milliseconds( 1 ) );
        
        this->app_allow_exit();
    }
    
    virtual void on_app_exit(int code) override {
        print("exit request handler", this->call_is_safe());
    }
    
    virtual void on_app_stopped() override {
        print("app stopped handler", this->call_is_safe());
    }
    
    boost::asio::steady_timer timer;
};

int main() {
    
    app a;
    std::thread runner{ [&]() { a.run(); } };
    
    print( "other thread", a.call_is_safe() );
    
    runner.join();
    return 0;
}
//! [ioapp_call_is_safe_example]

//! [basic_io_app_ex]
int main(){
    
    o::io::signal_listener_app<o::threads::none> app;
    
    // this call will return as soon as the process receives a SIGINT
    app.run();
    
    return 0;
}
//! [basic_io_app_ex]
