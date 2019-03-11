#include "devices/multi_resolver.h"
#include "io_application.h"
#include "net_url.h"
#include "types.h"

#include <atomic>

#include <boost/system/error_code.hpp>

namespace ohlano {

    template < typename MessageType, typename ThreadOptions >
    class client : public io_app::base< ThreadOptions > {

      public:
        using message_type = MessageType;

        using session_impl_type = ohlano::session<
            boost::beast::websocket::stream< boost::asio::ip::tcp::socket >,
            MessageType >;

        using session_type = std::shared_ptr< session_impl_type >;

        using io_base = io_app::base< ThreadOptions >;

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
} // namespace ohlano
