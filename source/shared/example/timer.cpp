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
        timer_ = o::io::wait(this->context(), 5s)
                     .then(o::io::timer_cb_bind(&app::handle_quit_cb, this));

        // bind a second callback to the same timer
        o::io::new_wait(timer_.lock())
            .then(o::io::timer_cb_bind(&app::other_op, this));

        // bind a third callback to the same timer
        o::io::new_wait(timer_.lock()).then(example_callback());

        // bind via the regular boost async_wait
        // in this case, you must ensure that the timer will not
        // be deleted as long as the callback is waiting
        timer_.lock()->async_wait([](sys::error_code) {
            std::cout << "And i was called last :(" << std::endl;
        });

        // Annoy the user every 500 milliseconds
        o::io::every(this->context(), 500ms)
            .repeat(o::io::timer_cb_bind(&app::annoy, this));
    }

    bool annoy(boost::system::error_code ec) {

        // Annoy
        std::cout << ((ec || stop) ? "Goodbye!" : "Hello!") << std::endl;

        // Return true (stop repeating) if the stop flag was set or ec
        // holds a value (an error ocurred, or the operation was
        // cancelled)
        return ec || stop;
    }

    // a useless function
    void other_op(boost::system::error_code) {
        std::cout << "I was called!" << std::endl;
    }

    // timer callbacks can also be functor-like objects
    struct example_callback : o::io::timer_callback {

        // will be called from the timer
        virtual void operator()(sys::error_code) override {
            std::cout << "I was called too!" << std::endl;
        }
    };

    void handle_quit_cb(boost::system::error_code ec) {
        // do nothing if ec holds a value (an error ocurred, or
        // the operation was cancelled)
        if (ec) return;

        std::cout << "Timer Callback" << std::endl;

        // allow the application to exit
        this->app_allow_exit();
    }

    // The app was allowed to exit either through a signal from the os or via
    // the app_allow_exit() call
    virtual void on_app_exit(int reason) override {

        std::cout << "Exit reason: " << reason << std::endl;

        // stop waiting for signals
        this->signals_stop();

        // set the stop flag
        stop = true;

        // cancel the exit timeout
        o::io::weak_timer_cancel(timer_);
    }

    bool stop = false;

    o::io::weak_steady_timer timer_;
};

int main() {

    app a;

    a.run();

    return 0;
}
