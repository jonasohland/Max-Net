#include <mutex>
#include <thread>

#include "session.h"
#include "server_base.h"

#include "proto_messages/generic_max_message.h"
#include "proto_messages/proto_message_base.h"

#include "devices/devices.h"

#include "ohlano_min.h"

#include "c74_min.h"

class websocketserver : public c74::min::object< websocketserver > {
  public:
    // ------------------------- some types shortcuts

    struct server_session;
    struct iolet;

    // the type used
    using message_type = ohlano::max_message;

    // the websocket session type
    using websocket_session_type =
        ohlano::session< boost::beast::websocket::stream< boost::asio::ip::tcp >,
                         message_type, ohlano::sessions::roles::server >;

    // the websocket session type representation as shared_ptr
    using session_type = std::shared_ptr< websocket_session_type >;

    // vector of server session structs ( session shared_ptrs / iolet pointers )
    using server_sessions = std::vector< server_session >;

    // vector of iolets
    using iolets_type = std::vector< std::unique_ptr< iolet > >;

    // -------------------------- structs for managing connections and in/outlets

    // holds an in and outlet that may be associated with one or more websocket sessions
    struct iolet {

        using output_type = ohlano::outlet_output_adapter< message_type >;

        c74::min::inlet<> inlet_;
        c74::min::outlet< c74::min::thread_check::none, c74::min::thread_action::assert >
            outlet_;

        output_type output_;
        output_type& output() { return output_; }
    };

    // this struct associates sessions and iolets
    // the caller must ensure that the pointer to the iolet remains valid
    struct server_session {

        template < typename... Ts >
        server_session( iolet*, Ts&&... args ) {
            session_ = std::make_shared< websocket_session_type >(
                std::forward< Ts >( args )... );
        }

        iolet* iolet_;
        session_type session_;

        std::mutex session_mtx_;

        iolet::output_type& output() { return iolet_->output(); }
        session_type& session() { return session_; }

        std::mutex& mtx() { return session_mtx_; }

        // shared_ptr holds a ref
        operator bool() const { return static_cast< bool >( session_ ); }
    };

    // construct the websocket server within max/msp
    explicit websocketserver( c74::min::atoms args = {} ) {

        if ( args.size() > 1 ) {

            for ( auto& arg : args ) {

                switch ( arg.a_type ) {
                case c74::max::e_max_atomtypes::A_LONG:

                    break;
                case c74::max::e_max_atomtypes::A_SYM:
                    break;

                default:
                    break;
                }
            }
            start_ctx();
        }
    }

    // destroy (close connections and wait for the context to exit)
    ~websocketserver() { end_ctx(); }

  private:
    // run the context in a own thread
    void start_ctx() {
        network_worker_ = std::make_unique< std::thread >( [this]() {
            ctx_running.store( true );
            ctx_.run();
            ctx_running.store( false );
        } );
    }

    bool is_ctx_running() const { return ctx_running.load(); }

    // stop the context and join its thread
    void end_ctx() {

        boost::ignore_unused( output_bundled );

        if ( work_.owns_work() )
            work_.reset();

        if ( network_worker_ )
            network_worker_->join();
    }

    // ------------------------- state variables

    std::atomic< bool > output_bundled;

    std::atomic< bool > ctx_running{ false };

    // ------------------------- predefined symbols

    // ------------------------- functional member objects

    // handles all network io
    boost::asio::io_context ctx_;

    // prevents the ctx from exiting before the object is destroyed
    boost::asio::executor_work_guard< boost::asio::io_context::executor_type > work_{
        ctx_.get_executor()
    };

    // io_context is running in here
    std::unique_ptr< std::thread > network_worker_;

    // listens for incoming connections;
    ohlano::listener listener_{ ctx_ };

    // list of all connections
    server_sessions connections_;

    // list of all in/outlets
    iolets_type iolets_;

    // ------------------------- max attributes

    c74::min::attribute< int > port{
        this, "port", -1, min_wrap_member( &websocketserver::handle_port_change )
    };

    c74::min::atoms handle_port_change( c74::min::atoms args, int inlet ) {
        return attr_restrict_long( 1, 65535, port, args );
    }

    c74::min::attribute< c74::min::symbol > address{
        this, "address", "0.0.0.0",
        min_wrap_member( &websocketserver::handle_address_change )
    };

    c74::min::atoms handle_address_change( c74::min::atoms args, int inlet ) {
        return attr_check_ip( address, args );
    }

    // check if input atoms[0] is long type and restrict it to attribute range.
    // if type check fails, we fall back to the old atom arg
    template < typename T >
    c74::min::atoms attr_restrict_long( T lo, T hi, const c74::min::attribute< T >& attr,
                                        const c74::min::atoms& args ) {

        c74::min::atoms out;
        T now = static_cast< T >( attr );

        if ( args[0].a_type == c74::max::e_max_atomtypes::A_LONG ) {
            if ( static_cast< T >( args[0] ) > hi ) {
                out.emplace_back( hi );
            } else if ( static_cast< T >( args[0] ) < lo ) {
                out.emplace_back( lo );
            } else {
                out.push_back( args[0] );
            }
        } else {
            out.emplace_back( now );
        }

        return out;
    }

    // check if the supplied atoms[0] can be parsed as ip address, fall back if not
    c74::min::atoms attr_check_ip( const c74::min::attribute< c74::min::symbol >& attr,
                                   const c74::min::atoms& args ) {

        c74::min::atoms out;
        c74::min::symbol now = attr;

        if ( args[0].a_type == c74::max::e_max_atomtypes::A_SYM ) {
            try {
                boost::ignore_unused( boost::asio::ip::address::from_string( args[0] ) );
                out.emplace_back( args[0] );

            } catch ( std::exception ex ) {
                out.emplace_back( now );
            }
        } else {
            out.emplace_back( now );
        }

        return out;
    }
};

using server2 = ohlano::net::server::base<
ohlano::session< boost::beast::websocket::stream<boost::asio::ip::tcp::socket>, ohlano::max_message >,
    ohlano::threads::single >;

struct serv2 : public server2 {
    
};

void ext_main( void* r ) {
    
    serv2 server;
    
    server.is_in_app_call();
    
    server.perform();
    std::cout << "perform done" << std::endl;
    

#ifdef VERSION_TAG
    c74::max::object_post(
        nullptr,
        "WebSocket Server for Max // (c) Jonas Ohland 2018 -- %s-%s-%s built: %s",
        STR( VERSION_TAG ), STR( CONFIG_TAG ), STR( OS_TAG ), __DATE__ );
#else
    c74::max::object_post(
        nullptr,
        "WebSocket Server for Max // (c) Jonas Ohland 2018 -- built %s - test build",
        __DATE__ );
#endif

    c74::min::wrap_as_max_external< websocketserver >( "websocketserver", __FILE__, r );
}
