#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "Movement.pb.h"
#include "c74_min.h"
#include "client.h"
#include "connection.h"
#include "devices/devices.h"
#include "messages/bytes_message.h"
#include "min_utils.h"
#include "net_url.h"
#include "ohlano_min.h"
#include "types.h"

using namespace c74::min;
using namespace std::placeholders;

namespace iiwa = de::hsmainz::iiwa::messages::protocolbuffers;

class websocketclient_iiwa
    : public object< websocketclient_iiwa >,
      public ohlano::client< ohlano::bytes_message, ohlano::threads::single > {

  public:
    using client_t = ohlano::client< ohlano::bytes_message, ohlano::threads::single >;

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
            begin_work();
            session_create( url );
        }
    }

    virtual ~websocketclient_iiwa() {

        session_close();

        this->end_work();
        this->await_work_end();
    }

  protected:
    const ohlano::bytes_message* handle_message( const ohlano::bytes_message* msg,
                                                 size_t bytes ) override {
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

    void on_work_started() override {
        cout << "running network worker" << c74::min::endl;
    }

    void on_work_finished() override {
        cout << "finished running network worker" << c74::min::endl;
    }

    /// convert an atom vector to a std::array of any type
    template < typename T, size_t Size >
    std::array< T, Size > get_typed_arg_array( const c74::min::atoms& args ) {

        std::array< T, Size > out;

        if ( args.size() < Size ) {
            throw std::runtime_error( "not enough arguments provided" );
        }

        for ( size_t i = 0; i < Size; ++i ) {
            out[i] = args[i].get< double >();
        }

        return out;
    }

    c74::min::atoms handle_joints_message( const c74::min::atoms& args, int inlet ) {

        try {

            auto joints = get_typed_arg_array< double, 7 >( args );

            for ( double joint : joints ) {
                movement_state.clear_jointpositions();
                movement_state.mutable_jointpositions()->add_joints( joint );
            }

        } catch ( std::exception& ex ) {
            cerr << ex.what() << c74::min::endl;
            return args;
        }

        return args;
    }

    c74::min::atoms handle_filter_params( const c74::min::atoms args, int inlet ) {

        try {

            auto filter_params = get_typed_arg_array< double, 3 >( args );

            movement_state.clear_filterparameter();

            auto filterp = movement_state.mutable_filterparameter();

            filterp->set_stepsize( filter_params[0] );
            filterp->set_friction( filter_params[1] );
            filterp->set_epsilon( filter_params[2] );

        } catch ( c74::min::bad_atom_access& ex ) {
            cerr << ex.what() << c74::min::endl;
            return args;
        }

        return args;
    }

    c74::min::atoms handle_joint_params( const c74::min::atoms args, int inlet ) {

        try {

            auto joint_params = get_typed_arg_array< double, 4 >( args );

            movement_state.clear_filterparameter();

            auto filterp = movement_state.mutable_jointparameter();

            filterp->set_jointvelocity( joint_params[0] );
            filterp->set_jointaccelerationrel( joint_params[1] );
            filterp->set_jointjerkrel( joint_params[2] );
            filterp->set_blendingrel( joint_params[3] );

        } catch ( c74::min::bad_atom_access& ex ) {
            cerr << ex.what() << c74::min::endl;
            return args;
        }

        return args;
    }

    c74::min::atoms handle_set_movetype( c74::min::atoms& args, int inlet ) {

        if ( args.size() < 1 ) {
            cerr << "not enough args" << c74::min::endl;
        }

        try {

            auto ty_sym = c74::min::atom::get< c74::min::symbol >( args[0] );

            if ( ty_sym == joint_sym )
                movement_state.set_movetype(
                    iiwa::Movement_MovementType::Movement_MovementType_JOINT );
            else if ( ty_sym == pip_sym )
                movement_state.set_movetype(
                    iiwa::Movement_MovementType::Movement_MovementType_PIP );
            else if ( ty_sym == lin_sym )
                movement_state.set_movetype(
                    iiwa::Movement_MovementType::Movement_MovementType_LIN );
            else if ( ty_sym == circle_sym )
                movement_state.set_movetype(
                    iiwa::Movement_MovementType::Movement_MovementType_CIRCLE );
            else if ( ty_sym == bezier_sym )
                movement_state.set_movetype(
                    iiwa::Movement_MovementType::Movement_MovementType_BEZIER );
            else if ( ty_sym == batch_sym )
                movement_state.set_movetype(
                    iiwa::Movement_MovementType::Movement_MovementType_BATCH );
            else
                cerr << "unrecognized movement type" << c74::min::endl;

        } catch ( std::exception& ex ) {
            cerr << ex.what() << c74::min::endl;
        }
    }

    c74::min::atoms send_msg( const c74::min::atoms& args, int inlet ) {

        auto out_msg = this->new_msg();

        size_t s = movement_state.ByteSizeLong();

        out_msg->storage().reserve( s );

        movement_state.SerializeToArray( out_msg->data(), s );

        this->send( out_msg );

        return args;
    }

    c74::min::symbol joint_sym{ "JOINT" };
    c74::min::symbol pip_sym{ "PIP" };
    c74::min::symbol lin_sym{ "LIN" };
    c74::min::symbol circle_sym{ "CIRCLE" };
    c74::min::symbol bezier_sym{ "BEZIER" };
    c74::min::symbol batch_sym{ "BATCH" };

    message< threadsafe::yes > set_joints{
        this, "Joints", "set joints", O_CREATE_DEFERRED_CALL( handle_joints_message )
    };

    message< threadsafe::yes > set_joind_params{
        this, "JointParams", "set joints", O_CREATE_DEFERRED_CALL( handle_joint_params )
    };

    message< threadsafe::yes > set_filter_params{
        this, "FilterParams", "set movement filter parameters",
        O_CREATE_DEFERRED_CALL( handle_filter_params )
    };

    message< threadsafe::yes > do_send_msg{ this, "doSend", "send message",
                                            O_CREATE_DEFERRED_CALL( send_msg ) };

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
    iiwa::Movement movement_state;
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
