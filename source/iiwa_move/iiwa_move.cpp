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

#include "c74_min.h"
#include "generated/Movement.pb.h"
#include "ohlano_min.h"
#define CHECKED_PTR_USE_TEMPLATE_HASH
#include <checked_ptr.h>

namespace iiwa = de::hsmainz::iiwa::messages::protocolbuffers;

class iiwa_movement_message : public c74::min::object< iiwa_movement_message > {
  public:
    explicit iiwa_movement_message( const c74::min::atoms& args = {} ) {

        if ( args.size() == 0 )
            return;

        try {

            auto type_sym = args[0].get< c74::min::symbol >();

            set_move_type( type_sym );

        } catch ( c74::min::bad_atom_access& ex ) {
            cout << ex.what() << c74::min::endl;
        }
    }

    c74::min::atoms handle_joints_message( const c74::min::atoms& args, int inlet ) {

        try {

            auto joints = o::array_from_atoms< double, 7 >( args );

            movement_state->clear_jointpositions();

            for ( double joint : joints ) {
                movement_state->mutable_jointpositions()->add_joints( joint );
            }

        } catch ( std::exception& ex ) {
            cerr << ex.what() << c74::min::endl;
            return args;
        }

        return args;
    }

    c74::min::atoms handle_filter_params( const c74::min::atoms args, int inlet ) {

        try {

            auto filter_params = o::array_from_atoms< double, 3 >( args );

            movement_state->clear_filterparameter();

            auto filterp = movement_state->mutable_filterparameter();

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

            auto joint_params = o::array_from_atoms< double, 4 >( args );

            movement_state->clear_filterparameter();

            auto filterp = movement_state->mutable_jointparameter();

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

        return args;
    }

    c74::min::atoms send_msg( const c74::min::atoms& args, int inlet ) {

        data_out.send( "iiwa_move", reinterpret_cast< long long >( &movement_state ) );

        return args;
    }

    iiwa::Movement::MovementType move_type_from_sym( c74::min::symbol& ty_sym,
                                                     bool& success ) {

        success = true;

        if ( ty_sym == joint_sym )

            return iiwa::Movement_MovementType::Movement_MovementType_JOINT;
        else if ( ty_sym == pip_sym )
            return iiwa::Movement_MovementType::Movement_MovementType_PIP;
        else if ( ty_sym == lin_sym )
            return iiwa::Movement_MovementType::Movement_MovementType_LIN;
        else if ( ty_sym == circle_sym )
            return iiwa::Movement_MovementType::Movement_MovementType_CIRCLE;
        else if ( ty_sym == bezier_sym )
            return iiwa::Movement_MovementType::Movement_MovementType_BEZIER;
        else if ( ty_sym == batch_sym )
            return iiwa::Movement_MovementType::Movement_MovementType_BATCH;
        else {
            success = false;
            cerr << "unrecognized movement type" << c74::min::endl;
            return iiwa::Movement_MovementType::Movement_MovementType_JOINT;
        }
    }

    bool ensure_correct_move_type( iiwa::Movement::MovementType ty ) {
        return movement_state->movetype() == ty;
    }

    bool is_correct_move_type( c74::min::symbol& ty_sym ) {

        bool success;
        auto move_t = move_type_from_sym( ty_sym, success );

        if ( success )
            return ( movement_state->movetype() == move_t );

        return false;
    }

    void set_move_type( c74::min::symbol& ty_sym ) {

        bool succ;
        auto move_t = move_type_from_sym( ty_sym, succ );

        if ( succ ) {
            movement_state->set_movetype( move_t );
        }
    }

    c74::min::symbol joint_sym{ "JOINT" };
    c74::min::symbol pip_sym{ "PIP" };
    c74::min::symbol lin_sym{ "LIN" };
    c74::min::symbol circle_sym{ "CIRCLE" };
    c74::min::symbol bezier_sym{ "BEZIER" };
    c74::min::symbol batch_sym{ "BATCH" };

    c74::min::inlet<> data_in{ this, "input", "anything" };
    c74::min::outlet<> data_out{ this, "output", "anything" };

    c74::min::message< c74::min::threadsafe::yes > set_joints{
        this, "Joints", "set joints",
        min_wrap_member( &iiwa_movement_message::handle_joints_message )
    };

    c74::min::message< c74::min::threadsafe::yes > set_joind_params{
        this, "JointParams", "set joints",
        min_wrap_member( &iiwa_movement_message::handle_joint_params )
    };

    c74::min::message< c74::min::threadsafe::yes > set_filter_params{
        this, "FilterParams", "set movement filter parameters",
        min_wrap_member( &iiwa_movement_message::handle_filter_params )
    };

    c74::min::message< c74::min::threadsafe::yes > do_send_msg{
        this, "bang", "send message", min_wrap_member( &iiwa_movement_message::send_msg )
    };

  private:
    checkable_obj< iiwa::Movement > movement_state { CONSTEXPR_TYPENAME_HASH( iiwa::message ) };
};

void ext_main( void* r ) {

    c74::min::wrap_as_max_external< iiwa_movement_message >( "iiwa_move", __FILE__, r );
}
