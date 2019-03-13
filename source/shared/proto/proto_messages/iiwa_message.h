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

#include "proto_message_base.h"
#include "../proto/generated/Movement.pb.h"
#include "../proto/generated/globals.pb.h"

namespace iiwa = de::hsmainz::iiwa::messages::protocolbuffers;

class iiwa_movement_message : public proto_message_base< iiwa::Movement > {

  public:
    using joints_state = std::array< double, 6 >;

    class allocator {
      public:
        iiwa_movement_message* allocate() { return new iiwa_movement_message(); }

        void deallocate( const iiwa_movement_message* msg ) { delete msg; }
    };

    using factory = allocator;

    void set_move_type( iiwa::Movement::MovementType ty ) {
        this->proto()->set_movetype( ty );
    }

    void set_joint_params( double vel, double acc, double jerk, double blend ) {
        this->proto()->mutable_jointparameter()->set_jointvelocity( vel );
        this->proto()->mutable_jointparameter()->set_jointaccelerationrel( acc );
        this->proto()->mutable_jointparameter()->set_jointjerkrel( jerk );
        this->proto()->mutable_jointparameter()->set_blendingrel( blend );
    }

    void set_filter_params( double step_size, double friction, double epsilon ) {
        this->proto()->mutable_filterparameter()->set_stepsize( step_size );
        this->proto()->mutable_filterparameter()->set_friction( friction );
        this->proto()->mutable_filterparameter()->set_epsilon( epsilon );
    }

    template < typename T >
    void add_joints( T last ) {
        proto()->mutable_jointpositions()->add_joints( last );
    }

    template < typename T, typename... Ts >
    void add_joints( T current, Ts... rest ) {
        proto()->mutable_jointpositions()->add_joints( current );
        set_joints( rest... );
    }

    void set_direction( bool direction ) const {}

    void notify_send() const {}

    void notify_send_done() const {}

  private:
    std::vector< double > acc_joints;
};
