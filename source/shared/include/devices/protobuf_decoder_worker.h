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

#include <thread>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include "protobuf_decoder.h"

namespace o {
    template < typename Message >
    class protobuf_decoder_worker {

        boost::asio::io_context ioc_;
        std::vector< std::thread > threads_;

        boost::asio::executor_work_guard< boost::asio::io_context::executor_type > work_{
            ioc_.get_executor()
        };

        typedef std::function< void( Message* ) > decoded_handler;
        typedef std::function< void( Message* ) > encoded_handler;

      public:
        protobuf_decoder_worker() {}

        void run( size_t num_threads ) {

            for ( size_t i = 0; i < num_threads; i++ ) {
                threads_.emplace_back( [this]() { ioc_.run(); } );
            }
        }

        void stop() {

            if ( work_.owns_work() ) {
                work_.reset();
            }

            for ( auto& thread : threads_ ) {
                if ( thread.joinable() ) {
                    thread.join();
                }
            }
        }

        void async_decode( Message* msg, decoded_handler handler ) {
            ioc_.post( [=]() mutable {
                msg->deserialize();
                handler( msg );
            } );
        }

        void async_encode( Message* msg, encoded_handler handler ) {
            ioc_.post( [=]() mutable {
                msg->serialize();
                handler( msg );
            } );
        }
    };
}
