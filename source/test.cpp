#define _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
#define _WIN32_WINNT 0x0A00

#include "o.h"

using base_app = o::io::basic_io_app< o::threads::none >;
using udpin = o::net::udp_receiver< o::threads::none >;

struct app : public base_app, public udpin {

    app() : udpin( this->context() ) {}

    virtual void on_app_started() {
        this->udp_bind(
            boost::asio::ip::udp::endpoint( boost::asio::ip::udp::v4(), 6000 ) );
    }

    virtual void on_udp_error( udpin::error_case eca,
                               boost::system::error_code ec ) override {

        std::cout << ( ( eca == udpin::error_case::bind ) ? "bind" : "read" ) << " error "
                  << ec.message() << std::endl;
    }

    virtual void on_app_exit( int reason ) override { this->udp_close(); }

    virtual void on_data_received( std::string input ) override {
        std::cout << "received " << input.size()
                  << " bytes: " << std::string( input.data(), input.size() ) << std::endl;
    }
};

int main() {

    app a;

    a.run();

    return 0;
}
