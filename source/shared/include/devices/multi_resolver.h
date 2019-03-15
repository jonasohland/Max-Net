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

#include <functional>
#include <utility>
#include <mutex>
#include "../net_url.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/system/error_code.hpp>

namespace o {

    template < typename ProtocolType >
    class multi_resolver {

        typedef std::function< void( boost::system::error_code, net_url<> ) >
            resolve_handler_type;
        typedef std::pair< net_url<>, resolve_handler_type > resolve_queue_value_type;

        boost::asio::io_context::strand strand_;
        boost::asio::ip::basic_resolver< ProtocolType > res_;

        std::deque< std::pair< net_url<>, resolve_handler_type > > ws_queue_;

        std::mutex queue_mtx_;

      public:
        explicit multi_resolver( boost::asio::io_context& ctx )
            : strand_( ctx ), res_( ctx ) {}

        void resolve( net_url<>& url, resolve_handler_type handler ) {
            std::unique_lock< std::mutex > lock{ queue_mtx_ };
            ws_queue_.emplace_back( url, handler );

            if ( ws_queue_.size() < 2 ) {
                begin_resolve( ws_queue_.front() );
            }
        }

        void resolve( resolve_queue_value_type qelem ) {}

      private:
        void begin_resolve( resolve_queue_value_type qelem ) {
            res_.async_resolve(
                qelem.first.host(), qelem.first.port(),
                boost::asio::bind_executor( strand_,
                                            std::bind( &multi_resolver::resolve_handler,
                                                       this, std::placeholders::_1,
                                                       std::placeholders::_2, qelem ) ) );
        }

        void resolve_handler(
            boost::system::error_code ec,
            typename boost::asio::ip::basic_resolver< ProtocolType >::results_type
                results,
            resolve_queue_value_type qelem ) {
            std::unique_lock< std::mutex > lock{ queue_mtx_ };

            qelem.first.set_resolver_results( results );
            ws_queue_.pop_front();

            if ( !ws_queue_.empty() ) {
                begin_resolve( ws_queue_.front() );
            }
            qelem.second( ec, qelem.first );
        }
    };
}

