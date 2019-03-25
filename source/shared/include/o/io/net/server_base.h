//
// This file is part of the Max-Net Project
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

#include <map>

#include <boost/asio.hpp>

#include "../io_app_base.h"

namespace o::io::net::server {

    template < typename Session, typename ConcurrencyOption >
    class server_base : public o::io::io_app_base< ConcurrencyOption > {

      public:
        using sessions_map = std::map< size_t, std::shared_ptr< Session > >;
        using sessions_type =
            o::ccy::opt_safe_visitable< sessions_map, ConcurrencyOption >;

        template < typename Opt = ConcurrencyOption >
        typename ccy::opt_enable_if_safe< Opt >::type start( int threads ) {
            this->app_begin_op( threads );
        }

        /// start the server
        void start() { this->app_begin_op(); }

        /// close all connections, delete all sessions, shut down all sessions and
        /// wait for them to exit
        void shutdown() {
            sess_close_all();
            this->app_end_op();
        }

        /// delete all closed/aborted sessions
        void sess_cleanup() {
            sessions_.apply( []( auto& sessions ) {

            } );
        }

        /// close all connections and delete all sessions
        void sess_close_all() {

            sessions_.apply( []( auto& sessions ) {
                for ( auto& [key, sess] : sessions )
                    if ( sess )
                        sess->close();

                sessions.clear();
            } );
        }

        sessions_type& sessions() { return sessions_; }

        const sessions_type& sessions() const { return sessions_; }

      private:
        sessions_type sessions_;
    };
}
