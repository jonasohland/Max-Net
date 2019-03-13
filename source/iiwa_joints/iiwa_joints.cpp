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

#include <checked_ptr.h>

namespace iiwa = de::hsmainz::iiwa::messages::protocolbuffers;

class iiwa_joints : public c74::min::object< iiwa_joints > {
  public:
    explicit iiwa_joints( const c74::min::atoms& args = {} ) {

        if ( args.size() == 0 )
            return;

        try {

        } catch ( c74::min::bad_atom_access& ex ) {
            cout << ex.what() << c74::min::endl;
        }
    }

    c74::min::atoms handle_joints_message( const c74::min::atoms& args, int inlet ) {

        try {

            auto joints = ohlano::array_from_atoms< double, 7 >( args );

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

    c74::min::atoms handle_joint_params( const c74::min::atoms args, int inlet ) {

        try {

            auto joint_params = ohlano::array_from_atoms< double, 4 >( args );

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

    bool ensure_correct_move_type( iiwa::Movement::MovementType ty ) {
        return movement_state->movetype() == ty;
    }

    c74::min::symbol joint_sym{ "JOINT" };
    c74::min::symbol pip_sym{ "PIP" };
    c74::min::symbol lin_sym{ "LIN" };
    c74::min::symbol circle_sym{ "CIRCLE" };
    c74::min::symbol bezier_sym{ "BEZIER" };
    c74::min::symbol batch_sym{ "BATCH" };

    c74::min::inlet<> data_in{ this, "input", "anything" };
    c74::min::outlet<> data_out{ this, "output", "anything" };

  private:
    checkable_obj< iiwa::Movement > movement_state { CONSTEXPR_TYPENAME_HASH(iiwa::message) };
};

void ext_main( void* r ) {

    c74::min::wrap_as_max_external< iiwa_joints >( "iiwa_joints", __FILE__, r );
}
