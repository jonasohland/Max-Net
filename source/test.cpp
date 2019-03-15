#include "o.h"
/*
using app_base = o::io::simple_io_app<o::threads::none>;

struct app : public app_base, public o::io::istream_listener {

    app() : o::io::istream_listener(this->context())
    {}
 
    virtual void on_console_input(std::string str) override {
        std::cout << "You entered: " << str << std::endl;
    }
};

int main() {
 
    app a;
 
    a.run();
 
    return 0;
}
*/

using app_base = o::io::io_app_base< o::threads::none >;

std::mutex mtx;
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
