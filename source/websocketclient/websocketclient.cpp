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

#include <mutex>
#include <thread>

#include "devices/protobuf_decoder_worker.h"
#include "net_url.h"
#include "session.h"

#include "proto_messages/generic_max_message.h"
#include "proto_messages/proto_message_base.h"
#include "proto_messages/proto_message_wrapper.h"

#include "devices/devices.h"

#include "ohlano_min.h"

#include "c74_min.h"

using namespace c74::min;
using namespace std::placeholders;

class websocketclient : public object< websocketclient > {
  public:
    using websocket_stream =
        boost::beast::websocket::stream< boost::asio::ip::tcp::socket >;
    using websocket_connection =
        o::session< boost::beast::websocket::stream< boost::asio::ip::tcp::socket >,
                         o::max_message, o::sessions::roles::client >;

    MIN_DESCRIPTION{ "WebSockets for Max! (Client)" };
    MIN_TAGS{ "net" };
    MIN_AUTHOR{ "Jonas Ohland" };
    MIN_RELATED{ "udpsend, udpreceive" };

    inlet<> main_inlet{ this, "(anything) data in" };
    outlet< thread_check::none, thread_action::assert > data_out{ this, "data out" };
    outlet<> status_out{ this, "status out" };

    explicit websocketclient( const atoms& args = {} ) {

        net_url<>::error_code ec;
        net_url<> url;
        net_url<> t_url;

        if ( args.size() > 0 ) {

            for ( auto& arg : args ) {

                switch ( arg.a_type ) {
                case c74::max::e_max_atomtypes::A_SYM:

                    if ( !url ) {

                        t_url = net_url<>( arg, ec );

                        if ( ec != net_url<>::error_code::SUCCESS )
                            cerr << "symbol argument could not be decoded to an url"
                                 << endl;

                        if ( url.has_port() && t_url.has_port() ) {
                            cerr << "Found multiple port arguments!" << endl;
                        }

                        url = t_url;
                    }

                    break;

                case c74::max::e_max_atomtypes::A_FLOAT:
                    cerr << "float not supported as argument" << endl;
                    break;
                case c74::max::e_max_atomtypes::A_LONG:
                    cout << "long arg: " << std::string( arg ) << endl;
                    if ( !url.has_port() ) {
                        bool success =
                            url.set_port( std::to_string( static_cast< int >( arg ) ) );
                        if ( !success )
                            cerr << "could not set port arg" << c74::min::endl;
                    } else {
                        cerr << "Found multiple port arguments!" << endl;
                    }
                    break;
                default:
                    cerr << "unsupported argument type" << endl;
                    break;
                }
            }

            cout << "address: " << url.host() << c74::min::endl;
            cout << "port: " << url.port() << c74::min::endl;

            if ( url ) {

                // there is work to do
                cout << "running network io worker thread" << endl;

                client_thread_ptr = std::make_unique< std::thread >( [this]() {
                    io_context_.run();
                    cout << "finished running network io worker thread" << endl;
                } );

                make_connection( url );

                dec_worker_.run( 2 );

            } else {
                cout << "no valid websocket address provided" << endl;
            }
        }
    }

    void make_connection( net_url<> url ) {

        if ( !url.is_resolved() ) {
            resolver.resolve( url, [=]( boost::system::error_code ec, net_url<> _url ) {
                cout << "resolver results: " << c74::min::endl;

                for ( auto& endp : _url.endpoints() ) {
                    cout << endp.address().to_string() << c74::min::endl;
                }

                perform_connect( _url );
            } );
        } else {
            perform_connect( url );
        }
    }

    void perform_connect( net_url<> url ) {

        connection_ =
            std::make_shared< websocket_connection >( io_context_, allocator_, &refc );

        connection_->on_ready( [=,
                                con = connection_.get()]( boost::system::error_code ec ) {
            cout << "session is ready status: " << con->status_string() << c74::min::endl;
        } );

        connection_->on_close( [=]( boost::system::error_code ec ) {
            cout << "session closed: " << ec.value() << c74::min::endl;
        } );

        connection_->on_read(
            [=]( boost::system::error_code ec, o::max_message* msg, size_t bytes ) {
                if ( ec ) {
                    cerr << "read operation failed: " << ec.message() << c74::min::endl;
                    return;
                }

                try {
                    if ( !msg->deserialize() ) {
                        cerr << "could not deserialize message" << c74::min::endl;
                    }
                } catch ( ... ) {
                    cerr << "could not deserialize message" << c74::min::endl;
                }

                output_.write( msg );
                allocator_.deallocate( msg );
            } );

        connection_->connect( url );
    }

    ~websocketclient() {

        if ( connection_ ) {
            connection_->close();
        }

        if ( work.owns_work() ) {
            work.reset();
        }

        dec_worker_.stop();

        if ( client_thread_ptr ) {
            if ( client_thread_ptr->joinable() ) {
                client_thread_ptr->join();
            }
        }
    }

    atoms report_status( const atoms& args, int inlet ) {
        if ( connection_ ) {
            status_out.send( connection_->status_string() );
        } else {
            status_out.send( "no_connection" );
        }
        return args;
    }

    message<> status{ this, "status", "report status",
                      min_wrap_member( &websocketclient::report_status ) };
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
                       } };

  private:
    /** The executor that will provide io functionality */
    boost::asio::io_context io_context_;

    /** This object will keep the io_context alive as long as the object exists */
    boost::asio::executor_work_guard< boost::asio::io_context::executor_type > work{
        io_context_.get_executor()
    };

    /** This object is responsible for resolving hostnames to ip addresses */
    o::multi_resolver< boost::asio::ip::tcp > resolver{ io_context_ };

    o::max_message::factory allocator_;

    std::shared_ptr< websocket_connection > connection_;

    std::unique_ptr< std::thread > client_thread_ptr;

    o::protobuf_decoder_worker< o::max_message > dec_worker_{};

    o::outlet_output_adapter< o::max_message > output_{ &data_out };

    std::atomic< int > refc{ 0 };

    std::mutex post_mtx;

    o::console_stream_adapter console_adapter{
        [this]( std::string str ) { cout << str << endl; }, true
    };
    o::console_stream_adapter console_error_adapter{
        [this]( std::string str ) { cerr << str << endl; }, true
    };
};

void ext_main( void* r ) {

#ifdef VERSION_TAG
    c74::max::object_post(
        nullptr,
        "WebSocket Client for Max // (c) Jonas Ohland 2018 -- %s-%s-%s built: %s",
        STR( VERSION_TAG ), STR( CONFIG_TAG ), STR( OS_TAG ), __DATE__ );
#else
    c74::max::object_post(
        nullptr,
        "WebSocket Client for Max // (c) Jonas Ohland 2018 -- built %s - test build",
        __DATE__ );
#endif

    c74::min::wrap_as_max_external< websocketclient >( "websocketclient", __FILE__, r );
}
