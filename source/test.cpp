#include "o.h"
#include <chrono>

int main() {

    boost::asio::io_context ctx;

    o::io::every(ctx, std::chrono::seconds(1))
        .repeat([counter = size_t()](boost::system::error_code ec) mutable {
            if (!ec) std::cout << "Counter is: " << counter << std::endl;
            return ++counter >= 10 || ec;
        });

    ctx.run();

    return 0;
}

