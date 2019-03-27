#include <chrono>
#include <iostream>
#include <o.h>

using namespace std::chrono_literals;
namespace sys = boost::system;

using app_base = o::io::signal_listener_app<o::ccy::none>;

struct app : public app_base {

    virtual void on_app_started() override {

        timer_ =
            o::io::wait(this->context(), 5s).then([this](sys::error_code ec) {

                if (ec) return;

                std::cout << "Timer Callback" << std::endl;

                this->app_allow_exit();
                this->signals_stop();

            });
    }

    virtual void on_app_exit(int reason) override {

        std::cout << "Exit: " << reason << std::endl;

        if (timer_) timer_->cancel();
    }

    std::shared_ptr<boost::asio::steady_timer> timer_;
};

int main() {

    app a;

    a.run();

    return 0;
}
