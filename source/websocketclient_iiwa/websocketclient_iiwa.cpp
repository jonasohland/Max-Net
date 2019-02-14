#include <mutex>
#include <thread>

#include "../shared/net_url.h"
#include "../shared/connection.h"
#include "../shared/client.h"
#include "../shared/devices/protobuf_decoder_worker.h"

#include "../shared/messages/proto_message_base.h"
#include "../shared/messages/iiwa_message.h"

#include "../shared/devices/devices.h"

#include "../shared/ohlano_min.h"

#include "../shared/min_utils.h"

#include "../shared/types.h"

#include "c74_min.h"


using namespace c74::min;
using namespace std::placeholders;


class websocketclient_iiwa : public object<websocketclient_iiwa>,
public ohlano::client<iiwa_movement_message, ohlano::threads::single> {

public:

    using client_t = ohlano::client<iiwa_movement_message, ohlano::threads::single>;
    
    MIN_DESCRIPTION{ "WebSockets for Max! (Client)" };
    MIN_TAGS{ "net" };
    MIN_AUTHOR{ "Jonas Ohland" };
    MIN_RELATED{ "udpsend, udpreceive" };

    inlet<> main_inlet{ this, "(anything) data in" };
    outlet<thread_check::none, thread_action::assert> data_out{ this, "data out" };
    outlet<> status_out{ this, "status out" };

    explicit websocketclient_iiwa(const atoms& args = {}) {

        net_url<> url = net_url_from_atoms(args);

        cout << "address: " << url.host() << c74::min::endl;
        cout << "port: " << url.port() << c74::min::endl;


        if (url) {
            begin_work();
            session_create(url);
        }
        
    }

    virtual ~websocketclient_iiwa() {

        session_close();

        this->end_work();
        this->await_work_end();
    }

protected:

    std::mutex print_mtx;
    
    const iiwa_movement_message *handle_message(const iiwa_movement_message *msg,
                                              size_t bytes) override {
        return msg;
    }

    void on_ready(boost::system::error_code ec) override {
        cout << "session is ready" << c74::min::endl;
    }

    void on_close(boost::system::error_code ec) override {
        cout << "session closed" << c74::min::endl;
    }

    void on_work_started() override {
        cout << "running network worker" << c74::min::endl;
    }

    void on_work_finished() override {
        cout << "finished running network worker" << c74::min::endl;
    }

    // message<> status{ this, "status", "report status", min_wrap_member(&websocketclient_iiwa::report_status) };
    message<> version{ this, "anything", "print version number",
        [=](const atoms& args, int inlet) -> atoms {

#ifdef VERSION_TAG
            cout << "WebSocket Client for Max "
                << STR(VERSION_TAG) << "-" << STR(CONFIG_TAG)
                << "-" << STR(OS_TAG) << c74::min::endl;
#else
            cout << "test build" << c74::min::endl;
#endif
            return args;
        }
    };

};

void ext_main(void* r) {

#ifdef VERSION_TAG
    c74::max::object_post(
        nullptr,
        "WebSocket Client for Max // (c) Jonas Ohland 2018 -- %s-%s-%s built: %s",
        STR(VERSION_TAG), STR(CONFIG_TAG), STR(OS_TAG), __DATE__);
#else
        c74::max::object_post(nullptr,
                            "WebSocket Client for Max // (c) Jonas Ohland 2018 -- "
                            "built %s - test build",
                            __DATE__);
#endif

  c74::min::wrap_as_max_external<websocketclient_iiwa>("websocketclient.iiwa",
                                                       __FILE__, r);
}
