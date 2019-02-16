#pragma once

#include "net_url.h"
#include "devices/stats.h"
#include "ohlano.h"
#include <cassert>
#include <atomic>
#include <functional>
#include <deque>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>

#include <boost/tti/has_member_function.hpp>
#include <boost/function_types/property_tags.hpp>
#include <boost/mpl/vector.hpp>

#include "types.h"

namespace ohlano {

    struct status_codes_base {
        enum class status_codes { OFFLINE, ONLINE, BLOCKED, ABORTED, SUSPENDED };
    };

    template < typename T >
    class session_threaded_base {};

    template <>
    class session_threaded_base< threads::single > : public status_codes_base {

      protected:
        void status_set( status_codes cd ) { opt_at_status_ = cd; }
        status_codes status_get() const { return opt_at_status_; }
        status_codes opt_at_status_;
    };

    template <>
    class session_threaded_base< threads::multi > : public status_codes_base {

      protected:
        void status_set( status_codes cd ) { opt_at_status_.store( cd ); }
        status_codes status_get() const { return opt_at_status_.load(); }
        std::atomic< status_codes > opt_at_status_;
    };

    template < typename Stream, typename Message,
               typename Role = sessions::roles::client >
    class connection
        : public std::enable_shared_from_this< connection< Stream, Message > >,
          public session_threaded_base< threads::multi > {
      public:
        typedef std::function< void( boost::system::error_code ) >
            basic_completion_handler_t;
        typedef std::function< void( const Message* ) > write_completion_handler_t;
        typedef std::function< void( boost::system::error_code, Message*, size_t ) >
            read_completion_handler_t;

        using status_t = status_codes;

        /**
             * return a string representation of the current status
             * @return the string
             */
        std::string status_string() const {
            switch ( status_get() ) {
            case status_codes::OFFLINE:
                return "offline";
            case status_codes::ONLINE:
                return "online";
            case status_codes::BLOCKED:
                return "blocked";
            case status_codes::ABORTED:
                return "aborted";
            default:
                return "undefined";
            }
        }

        template < typename M = Message >
        typename ohlano::messages::enable_if_direction_supported< M >::type
        optional_set_direction( bool direction, Message* msg ) {
            msg->set_direction( false );
        };

        template < typename M = Message >
        typename std::enable_if<
            !ohlano::messages::is_direction_supported< M >::value >::type
        optional_set_direction( bool direction, Message* msg ){};

        /// constructor for client role
        template < typename R = Role >
        explicit connection(
            boost::asio::io_context& ctx, typename Message::factory& allocator,
            std::atomic< int >* refc,
            typename sessions::enable_for_client< R, std::nullptr_t >::type dummyarg =
                nullptr )
            : ctx_( ctx )
            , read_strand_( ctx )
            , stream_( ctx_ )
            , allocator_( allocator )
            , stats_( ctx )
            , msg_pool_refc( refc ) {
            status_set( status_t::BLOCKED );
            ( *msg_pool_refc )++;
        }

        /// constructor for server role
        template < typename R = Role >
        explicit connection(
            typename Stream::next_layer_type&& next_layer, boost::asio::io_context& ctx,
            typename Message::factory& allocator, std::atomic< int >* refc,
            typename sessions::enable_for_server< R, std::nullptr_t >::type dummyarg =
                nullptr )
            : ctx_( ctx )
            , read_strand_( ctx )
            , stream_( std::forward< typename Stream::next_layer_type >( next_layer ) )
            , allocator_( allocator )
            , stats_( ctx )
            , msg_pool_refc( refc ) {
            status_set( status_t::BLOCKED );
            ( *msg_pool_refc )++;
        }

        ~connection() {

            DBG( "connection destructor" );

            if ( on_write_done_ != boost::none ) {
                while ( msg_queue.size() > 0 ) {
                    on_write_done_.value()( msg_queue.front() );
                    msg_queue.pop_front();
                }
            } else {
                while ( msg_queue.size() > 0 ) {
                    on_write_done_.value()( msg_queue.front() );
                    msg_queue.pop_front();
                }
            }

            assert( msg_queue.size() == 0 );

            ( *msg_pool_refc )--;
        }

        /**
             * get the current status of the session
             */
        status_t status() const { return status_get(); }

        /**
             * Assign a handler that will be called as soon as the connection is ready to
         * send and receive messages.
             * The method will return immediatly. It is guaranteed that the handler
             * will be called from the service thread.
             * @param a basic_completion_handler that will be called when the session is
         * ready
             */
        void on_ready( basic_completion_handler_t handler ) {
            on_ready_ = boost::make_optional( handler );
        }

        /**
             * Assign a handler that will be called as soon as the connection is receives
         * a message or an error occurs
             * while performing the read operation. If the read operation failed the
         * message given to the handler
             * will be a nullptr.
             * The method will return immediatly. It is guaranteed that the handler
             * will be called from the service thread.
             * @param a read_completion_handler_t to be called
             */
        void on_read( read_completion_handler_t handler ) {
            on_read_ = boost::make_optional( handler );
        }

        /**
             * Assign a handler that will be called if a close operation performed from
         * this side succeeds.
             * @param a basic_completion_handler that will be called on close.
             */
        void on_close( basic_completion_handler_t handler ) {
            on_close_ = boost::make_optional( handler );
        }

        /**
             * Assign a handler that will be called when a write operation completes.
             * The method will return immediatly. It is guaranteed that the handler
             * will be called from the service thread.
             * @param a write_completion_handler that will be called.
             */
        void on_write_done( write_completion_handler_t handler ) {
            on_write_done_ = boost::make_optional( handler );
        }

        /**
             * Begin a connect operation on this session. The on_ready handler will be
         * called when the operation
             * completes. This Method will manage connecting and handshaking. After a
         * successful connect operation, the
             * session will be ready to write and already in a reading state.
             * @param a net_url that is in a resolved state and contains a valid endpoint.
             */
        template < typename R = Role >
        typename sessions::enable_for_client< R >::type connect( net_url<> url ) {

            status_set( status_t::BLOCKED );

            assert( url.valid() );
            assert( url.is_resolved() );

            boost::asio::async_connect( stream_.next_layer(), url.endpoints(),
                                        std::bind( &connection::connect_handler,
                                                   this->shared_from_this(),
                                                   std::placeholders::_1, url ) );
        }

        /**
             * Accept the remote connection. The on_ready handler will be called when the
         * operation completes.
             * After this operation completes, the session will be ready to write and in a
         * reading state.
             */
        template < typename R = Role >
        typename sessions::enable_for_server< R >::type accept() {
            stream_.async_accept( boost::asio::bind_executor(
                read_strand_,
                std::bind( &connection::accepted_handler, this->shared_from_this(),
                           std::placeholders::_1 ) ) );
        }

        void write( const Message* message ) {

            std::lock_guard< std::mutex > write_q_lock{ write_queue_mutex_ };

            msg_queue.push_back( message );

            if ( msg_queue.size() < 2 ) {
                perform_write( message );
            }
        }

        void close() {

            stats().set_enabled( false );

            close_tmt = std::make_unique< boost::asio::steady_timer >(
                stream_.get_executor().context(), std::chrono::milliseconds( 500 ) );

            close_tmt->async_wait( [=]( boost::system::error_code ec ) {

                if ( !ec ) {

                    try {

                        if ( stream_.is_open() ) {
                            stream_.next_layer().cancel();
                        }
                        if ( stream_.next_layer().is_open() ) {
                            stream_.next_layer().close();
                        }
                        if ( stream_.lowest_layer().is_open() ) {
                            stream_.lowest_layer().close();
                        }
                    } catch ( std::exception ex ) {
                        DBG( "exception on close timer callback: ", ex.what() );
                    }
                }
            } );

            if ( stream_.is_open() && status() == status_codes::ONLINE ) {

                auto self = this->shared_from_this();

                boost::asio::dispatch( write_strand_, [self]() {
                    self->stream_.async_close(
                        boost::beast::websocket::close_code::normal,
                        std::bind( &connection::close_handler, self->shared_from_this(),
                                   std::placeholders::_1 ) );
                } );
            }
        }

        Stream& stream() { return stream_; }

        const Stream& stream() const { return stream_; }

        session_stats< unsigned long long, std::chrono::system_clock >& stats() {
            return stats_;
        }

      private:
        void connect_handler( boost::system::error_code ec, net_url<> url ) {

            if ( !ec ) {
                stream_.async_handshake(
                    url.host(), url.path(),
                    boost::asio::bind_executor( read_strand_,
                                                std::bind( &connection::handshake_handler,
                                                           this->shared_from_this(),
                                                           std::placeholders::_1 ) ) );
            } else {
                status_set( status_t::ABORTED );
                stats_.set_enabled( false );
                if ( on_ready_ != boost::none ) {
                    on_ready_.value()( ec );
                }
            }
        }

        void handshake_handler( boost::system::error_code ec ) {
            if ( ec ) {
                status_set( status_t::ABORTED );
                stats_.set_enabled( false );
                if ( on_ready_ != boost::none ) {
                    on_ready_.value()( ec );
                }
            } else {
                status_set( status_t::ONLINE );

                if ( on_ready_ != boost::none ) {
                    on_ready_.value()( ec );
                }

                perform_read();
            }
        }

        void accepted_handler( boost::system::error_code ec ) {

            if ( ec ) {
                status_set( status_t::ABORTED );
                stats_.set_enabled( false );
            } else {
                status_set( status_t::ONLINE );
                perform_read();
            }

            if ( on_ready_ != boost::none ) {
                on_ready_.value()( ec );
            }
        }

        // ----------------   write operations

        void perform_write( const Message* msg ) {
            if ( ctx_.get_executor().running_in_this_thread() ) {
                stream_.async_write(
                    boost::asio::buffer( msg->data(), msg->size() ),
                    boost::asio::bind_executor(
                        this->write_strand_,
                        std::bind( &connection::write_complete_handler,
                                   this->shared_from_this(), std::placeholders::_1,
                                   std::placeholders::_2, msg ) ) );
            } else {
                auto self = this->shared_from_this();

                boost::asio::dispatch( write_strand_, [msg, self]() {
                    self->stream_.async_write(
                        boost::asio::buffer( msg->data(), msg->size() ),
                        boost::asio::bind_executor(
                            self->write_strand_,
                            std::bind( &connection::write_complete_handler,
                                       self->shared_from_this(), std::placeholders::_1,
                                       std::placeholders::_2, msg ) ) );
                } );
            }
        }

        void write_complete_handler( boost::system::error_code ec, std::size_t bytes,
                                     const Message* msg ) {

            std::unique_lock< std::mutex > lock{ write_queue_mutex_ };
            std::unique_lock< std::mutex > stats_lock{ stats().mtx() };

            DBG( ec.message() );

            stats().outbound().data().add( bytes );
            stats().outbound().msgs()++;

            if ( !msg_queue.empty() ) {
                msg_queue.pop_front();

                if ( on_write_done_ != boost::none ) {
                    on_write_done_.value()( msg );
                } else {
                    allocator_.deallocate( msg );
                }

                if ( !msg_queue.empty() ) {
                    if ( stream_.is_open() ) {
                        // perform another write
                        perform_write( msg_queue.front() );
                    } else {
                        // clear the queue
                        while ( !msg_queue.empty() ) {
                            if ( on_write_done_ != boost::none ) {
                                on_write_done_.value()( msg_queue.front() );
                                msg_queue.pop_front();
                            } else {
                                allocator_.deallocate( msg_queue.front() );
                                msg_queue.pop_front();
                            }
                        }
                    }
                }
            }
        }

        // ----------------   read operations

        void perform_read() {

            if ( status() == status_t::ONLINE ) {

                auto self = this->shared_from_this();

                boost::asio::dispatch( read_strand_, [self]() {
                    self->stream_.async_read(
                        self->buffer_,
                        boost::asio::bind_executor(
                            self->read_strand_,
                            std::bind( &connection::read_handler,
                                       self->shared_from_this(), std::placeholders::_1,
                                       std::placeholders::_2 ) ) );
                } );
            }
        }

        void read_handler( boost::system::error_code ec, size_t bytes ) {

            if ( !ec ) {

                std::unique_lock< std::mutex > stats_lock{ stats().mtx() };

                stats().inbound().data().add( bytes );
                stats().inbound().msgs()++;

                if ( on_read_ != boost::none ) {

                    Message* new_msg = static_cast< Message* >( allocator_.allocate() );

                    optional_set_direction( false, new_msg );

                    Message::from_const_buffers( buffer_.data(), new_msg,
                                                 stream_.got_text() );

                    on_read_.value()( ec, new_msg, bytes );
                }

                buffer_.consume( bytes );

                perform_read();
            } else {

                if ( ec != boost::beast::websocket::error::closed && ec.value() != 89 ) {
                    status_set( status_t::ABORTED );
                    stats_.set_enabled( false );

                } else {
                    status_set( status_t::OFFLINE );
                    stats_.set_enabled( false );

                    if ( on_write_done_ != boost::none ) {

                        std::unique_lock< std::mutex > lock{ write_queue_mutex_ };

                        while ( !msg_queue.empty() ) {
                            on_write_done_.value()( msg_queue.front() );
                            msg_queue.pop_front();
                        }
                    }
                }

                if ( on_read_ != boost::none ) {
                    on_read_.value()( ec, nullptr, bytes );
                }
            }
        }

        // ----------------- control operations

        void close_handler( boost::system::error_code ec ) {
            if ( close_tmt ) {
                close_tmt->cancel();
            }

            status_set( status_t::OFFLINE );

            if ( on_close_ != boost::none ) {
                on_close_.value()( ec );
            }
        }

        boost::asio::io_context& ctx_;
        boost::asio::io_context::strand read_strand_;

        Stream stream_;

        boost::beast::multi_buffer buffer_;

        typename Message::factory& allocator_;

        std::atomic< status_t > status_;

        session_stats< unsigned long long, std::chrono::system_clock > stats_;

        std::unique_ptr< boost::asio::steady_timer > close_tmt;

        boost::optional< read_completion_handler_t > on_read_;
        boost::optional< write_completion_handler_t > on_write_done_;
        boost::optional< basic_completion_handler_t > on_close_;
        boost::optional< basic_completion_handler_t > on_ready_;

        std::mutex write_queue_mutex_;
        std::deque< const Message* > msg_queue;
        boost::asio::io_context::strand write_strand_{ ctx_ };

        std::atomic< int >* msg_pool_refc;
    };
}
