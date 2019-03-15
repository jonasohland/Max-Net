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

#pragma once

#include <mutex>
#include "c74_min.h"

namespace o {

    template < typename Message >
    class outlet_output_adapter {

      public:
        outlet_output_adapter() = delete;

        outlet_output_adapter(
            c74::min::outlet< c74::min::thread_check::none,
                              c74::min::thread_action::assert >* outlet )
            : outlet_( outlet ) {}

        void write( Message* message ) {
            std::lock_guard< std::mutex > lock{ outlet_mutex_ };
            outlet_->send( message->get_atoms() );
        }

        template < typename... T >
        void write_raw( T... args ) {
            std::lock_guard< std::mutex > lock{ outlet_mutex_ };
            outlet_->send( args... );
        }

      private:
        c74::min::outlet< c74::min::thread_check::none, c74::min::thread_action::assert >*
            outlet_;
        std::deque< Message* > output_queue_;
        std::mutex outlet_mutex_;
    };
}
