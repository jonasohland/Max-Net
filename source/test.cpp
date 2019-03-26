#include "o.h"

int main() {

    boost::asio::io_context ctx;

    o::io::wait(ctx, std::chrono::seconds(5))
        .then([](boost::system::error_code ec) {
            std::cout << "Hello World!" << std::endl;
        });

    ctx.run();

    return 0;
}
