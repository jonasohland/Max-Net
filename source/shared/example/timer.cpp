#include <chrono>
#include <iostream>
#include <o.h>

using namespace std::chrono_literals;
namespace sys = boost::system;

using app_base = o::io::signal_listener_app<o::ccy::none>;

struct app : public app_base {

    // the app was started via the run() call
    virtual void on_app_started() override {

        // create a new timer that waits 5 seconds
        timer_ =
            o::io::wait(this->context(), 5s).then([this](sys::error_code ec) {
                // do nothing if ec holds a value (an error ocurred, or the
                // operation was cancelled)
                if (ec) return;

                std::cout << "Timer Callback" << std::endl;

                // allow the application to exit
                this->app_allow_exit();
            });

        // Annoy the user every 500 milliseconds
        o::io::every(this->context(), 500ms)
            .repeat([this](boost::system::error_code ec) {
                // Annoy
                std::cout << "Hello!" << std::endl;

                // Return true (stop repeating) if the stop flag was set or ec
                // holds a value (an error ocurred, or the operation was
                // cancelled)
                return ec || stop;
            });
    }

    // The app was allowed to exit either through a signal from the os or via
    // the app_allow_exit() call
    virtual void on_app_exit(int reason) override {

        std::cout << "Exit: " << reason << std::endl;

        // stop waiting for signals
        this->signals_stop();

        // set the stop flag
        stop = true;

        // cancel the exit timeout
        if (timer_) timer_->cancel();
    }

    bool stop = false;

    std::shared_ptr<boost::asio::steady_timer> timer_;
};

int main() {

    app a;

    a.run();

    return 0;
}
