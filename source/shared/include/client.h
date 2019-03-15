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

#include "devices/multi_resolver.h"
#include "net_url.h"
#include "session.h"

#include "o.h"

#include <atomic>

#include <boost/system/error_code.hpp>

namespace o {

    template < typename MessageType, typename ThreadOptions >
    class client : public io::io_app_base< ThreadOptions > {

      public:
        using message_type = MessageType;

        using session_impl_type = o::session<
            boost::beast::websocket::stream< boost::asio::ip::tcp::socket >,
            MessageType >;

        using session_type = std::shared_ptr< session_impl_type >;

        using io_base = io::io_app_base< ThreadOptions >;

        using thread_option = ThreadOptions;

        virtual ~client() {}

        virtual const MessageType* handle_message( const MessageType*, size_t ) = 0;

        virtual void on_ready( boost::system::error_code ) = 0;

        virtual void on_close( boost::system::error_code ) = 0;

        void session_create( net_url<> url ) {

            session_ = std::make_shared< session_impl_type >( this->context(), factory_,
                                                              &connections_refc_ );

            if ( !url.is_resolved() ) {

                resolver_.resolve(
                    url, [=]( boost::system::error_code ec, net_url<> resolved_url ) {
                        do_session_connect( resolved_url );
                    } );

                return;
            }

            do_session_connect( url );
            return;
        }

        void session_close() {
            if ( session_ )
                session_->close();
        }

        void send( const MessageType* msg ) { session_->write( msg ); }

        MessageType* new_msg() { return factory_.allocate(); }

        session_type& session() { return session_; }
        const session_type& session() const { return session_; }

        typename MessageType::factory& factory() { return factory_; }

      private:
        void do_session_connect( net_url<> url ) {

            // why does this work?
            session_->on_ready(
                std::bind( &client::on_ready, this, std::placeholders::_1 ) );

            session_->on_close(
                std::bind( &client::on_close, this, std::placeholders::_1 ) );

            session_->on_read( std::bind( &client::handle_message_wrapper, this,
                                          std::placeholders::_1, std::placeholders::_2,
                                          std::placeholders::_3 ) );

            session_->connect( url );
        }

        void handle_message_wrapper( boost::system::error_code ec, const MessageType* msg,
                                     size_t bytes ) {

            if ( msg != nullptr ) {
                factory_.deallocate( handle_message( msg, bytes ) );
            }

            else
                DBG( ec.message() );
        }

        session_type session_;

        typename MessageType::factory factory_;

        multi_resolver< boost::asio::ip::tcp > resolver_{ this->context() };

        std::atomic< int > connections_refc_;
    };
} // namespace o
