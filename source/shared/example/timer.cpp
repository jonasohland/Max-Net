#include <chrono>
#include <iostream>
#include <o/io/timer.h>


using namespace std::chrono_literals;
namespace sys = boost::system;

int main() {
    
    boost::asio::io_context ctx;
    
    o::io::every(ctx, 1s).repeat([cnt = 0](sys::error_code ec) mutable {
        if (!ec) std::cout << "Counter is: " << cnt << std::endl;
        return ++cnt >= 10 || ec;
    });
    
    ctx.run();
    
    return 0;
    
}
