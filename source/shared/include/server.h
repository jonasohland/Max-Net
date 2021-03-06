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

#include "devices/listener.h"
#include "io_application.h"
#include "messages/bytes_message.h"
#include "session.h"
#include "session_query_macros.h"
#include "types.h"

namespace o::io::net {
    

    template < typename MessageType, typename ThreadOption >
    class websocket_server : public io_app::base< ThreadOption > {

      public:
        using message_type = MessageType;

        using session_impl_type = ohlano::session<
            boost::beast::websocket::stream< boost::asio::ip::tcp::socket >, MessageType,
            sessions::roles::server >;

        using session_type = std::shared_ptr< session_impl_type >;
        using session_sequence = std::vector< session_type >;

        using io_base = io_app::base< ThreadOption >;

        using thread_option = ThreadOption;

      public:
        virtual const MessageType* on_message( session_type& session, const MessageType*,
                                               size_t ) = 0;

        virtual void on_connect( session_type&, boost::system::error_code ) = 0;

        virtual void on_leave( session_type&, boost::system::error_code ) = 0;

        void start( boost::asio::ip::tcp::endpoint endpoint ) {
            listener_.start_listen(
                endpoint, std::bind( &websocket_server::do_handle_session, this,
                                     std::placeholders::_1, std::placeholders::_2 ) );
            this->begin_work();
        }

        void do_handle_session( boost::system::error_code ec,
                                boost::asio::ip::tcp::socket&& sock ) {

            sessions_.emplace_back( new session_impl_type(
                std::forward< boost::asio::ip::tcp::socket >( sock ), this->context(),
                factory_, nullptr ) );
        }

        void shutdown() {

            if ( listener_.status() == listener::status_codes::OPEN )
                listener_.stop_listen();

            OHLANO_FOREACH_DO(
                sessions_, CON_WHERE_STATUS_INCLUDES( ONLINE, SUSPENDED ),
                [=]( session_sequence::iterator sess ) { ( *sess )->close(); } );

            this->end_work();
        }

      private:
        typename MessageType::factory factory_;
        listener listener_;
        session_sequence sessions_;
        
    };

} // namespace o::io::net
