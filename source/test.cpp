#include "o.h"
#include <chrono>

int main() {

    boost::asio::io_context ctx;

    auto repeater =
        o::io::every(
            ctx, std::chrono::seconds(3));

    auto timer =
        repeater.repeat([counter = 0](boost::system::error_code ec) mutable {
            if (!ec) std::cout << "Counter is: " << counter << std::endl;
            return ++counter >= 10;
        });

    ctx.run();

    return 0;
}

