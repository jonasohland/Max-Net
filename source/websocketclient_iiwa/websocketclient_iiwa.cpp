//
// This file is part of the Max Network Extensions Project
//
// Copyright (c) 2019, Jonas Ohland
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "c74_min.h"
#include "client.h"
#include "devices/devices.h"
#include "generated/Movement.pb.h"
#include "messages/bytes_message.h"
#include "min_utils.h"
#include "net_url.h"
#include "ohlano_min.h"
#include "session.h"
#include "types.h"

#define CHECKED_PTR_USE_TEMPLATE_HASH

#include <checked_ptr.h>

using namespace c74::min;
using namespace std::placeholders;

namespace iiwa = de::hsmainz::iiwa::messages::protocolbuffers;

class websocketclient_iiwa
    : public object< websocketclient_iiwa >,
      public o::client< o::messages::bytes_message, o::threads::single > {

  public:
    using client_t =
        o::client< o::messages::bytes_message, o::threads::single >;

    MIN_DESCRIPTION{ "WebSockets for Max! (Client)" };
    MIN_TAGS{ "net" };
    MIN_AUTHOR{ "Jonas Ohland" };
    MIN_RELATED{ "udpsend, udpreceive" };

    inlet<> main_inlet{ this, "(anything) data in" };
    outlet< thread_check::none, thread_action::assert > data_out{ this, "data out" };
    outlet<> status_out{ this, "status out" };

    explicit websocketclient_iiwa( const atoms& args = {} ) {

        net_url<> url = net_url_from_atoms( args );

        cout << "address: " << url.host() << c74::min::endl;
        cout << "port: " << url.port() << c74::min::endl;

        if ( url ) {
            this->app_launch();
            session_create( url );
        }
    }

    virtual ~websocketclient_iiwa() {

        session_close();

        this->app_allow_exit();
        this->app_join();
    }

  protected:
    const o::messages::bytes_message*
    handle_message( const o::messages::bytes_message* msg, size_t bytes ) override {
        cout << "received " << bytes << " bytes" << c74::min::endl;
        return msg;
    }

    void on_ready( boost::system::error_code ec ) override {
        cout << "session is ready" << c74::min::endl;
        session()->stream().binary( true );
    }

    void on_close( boost::system::error_code ec ) override {
        cout << "session closed" << c74::min::endl;
    }

    void on_app_started() override {
        cout << "running network worker" << c74::min::endl;
    }

    void on_app_stopped() override {
        cout << "finished running network worker" << c74::min::endl;
    }

    c74::min::atoms send_msg( const c74::min::atoms& args, int inlet ) { return args; }

    c74::min::symbol joint_sym{ "JOINT" };
    c74::min::symbol pip_sym{ "PIP" };
    c74::min::symbol lin_sym{ "LIN" };
    c74::min::symbol circle_sym{ "CIRCLE" };
    c74::min::symbol bezier_sym{ "BEZIER" };
    c74::min::symbol batch_sym{ "BATCH" };

    message< threadsafe::yes > new_mv_msg{
        this, "iiwa_move", "send new iiwa move msg",
        [=]( const c74::min::atoms& args, int inlet ) -> c74::min::atoms {
            auto msg = new_msg();

            try {

                GEN_CHECKED_PTR( iiwa::Movement, mv_ptr, args[0].get< long long >() );

                msg->storage().resize( mv_ptr->ByteSizeLong() );

                mv_ptr->SerializeToArray( msg->data(), msg->size() );

                send( msg );

            } catch ( std::exception& ex ) {
                cerr << ex.what() << c74::min::endl;
                factory().deallocate( msg );
            }

            return args;
        }
    };

    // message<> status{ this, "status", "report status",
    // min_wrap_member(&websocketclient_iiwa::report_status) };
    message<> version{ this, "anything", "print version number",

                       [=]( const atoms& args, int inlet ) -> atoms {
#ifdef VERSION_TAG
                           cout << "WebSocket Client for Max " << STR( VERSION_TAG )
                                << "-" << STR( CONFIG_TAG ) << "-" << STR( OS_TAG )
                                << c74::min::endl;
#else
                           cout << "test build" << c74::min::endl;
#endif
                           return args;
                       }

    };

  private:
    std::mutex movement_lock;
};

void ext_main( void* r ) {

#ifdef VERSION_TAG
    c74::max::object_post(
        nullptr,
        "WebSocket Client for Max // (c) Jonas Ohland 2018 -- %s-%s-%s built: %s",
        STR( VERSION_TAG ), STR( CONFIG_TAG ), STR( OS_TAG ), __DATE__ );
#else
    c74::max::object_post( nullptr,
                           "WebSocket Client for Max // (c) Jonas Ohland 2018 -- "
                           "built %s - test build",
                           __DATE__ );
#endif

    c74::min::wrap_as_max_external< websocketclient_iiwa >( "websocketclient.iiwa",
                                                            __FILE__, r );
}
