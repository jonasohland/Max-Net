#include "io_application.h"

using app_base = ohlano::io_app::simple_io_app<ohlano::threads::single>;

struct app : public app_base {
    
    virtual void on_app_exit(int reason) override {
        std::cout << "Goodbye World!" << "sig: " << reason << std::endl;
    }
    
    virtual void on_app_started() override {
        std::cout << "Hello World!" << std::endl;
    }
    
    void run(){
        this->setup();
        this->perform();
    }
};

int main() {
    app a;
    a.run();
    return 0;
}
