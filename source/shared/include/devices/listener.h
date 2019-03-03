#include "../ohlano.h"
#include <boost/asio.hpp>

namespace ohlano {

    class listener {
      public:
        listener() = delete;

        using new_connection_handler = std::function< void(
            boost::system::error_code, boost::asio::ip::tcp::socket&& ) >;

        enum class status_code { OPENING, OPEN, CLOSING, CLOSED };

        using status_codes = status_code;

        status_code status() { return status_.load(); }

        explicit listener( boost::asio::io_context& ctx )
            : status_( status_code::CLOSED )
            , socket_( ctx )
            , accepted_handler_strand_( ctx )
            , ctx_( ctx )
            , strand_( ctx ) {}

        boost::system::error_code start_listen( boost::asio::ip::tcp::endpoint endpoint,
                                                new_connection_handler handler ) {

            status_.store( status_code::OPENING );

            acceptor_ = std::make_unique< boost::asio::ip::tcp::acceptor >( ctx_ );

            boost::asio::socket_base::reuse_address option( true );

            boost::system::error_code ec;

            acceptor_->open( endpoint.protocol(), ec );
            if ( ec ) {
                DBG( "open error ", ec.message() );
                status_.store( status_code::CLOSED );
                return ec;
            }

            acceptor_->set_option( option, ec );
            if ( ec ) {
                DBG( "addr reuse error ", ec.message() );
                status_.store( status_code::CLOSED );
                return ec;
            }

            acceptor_->bind( endpoint, ec );
            if ( ec ) {
                DBG( "bind error: ", ec.message() );
                status_.store( status_code::CLOSED );
                return ec;
            }

            acceptor_->listen( boost::asio::socket_base::max_listen_connections, ec );
            if ( ec ) {
                DBG( "listen error: ", ec.message() );
                status_.store( status_code::CLOSED );
                return ec;
            }

            status_.store( status_code::OPEN );

            acceptor_->async_accept(
                socket_, boost::asio::bind_executor(
                             strand_, std::bind( &listener::accept_handler, this,
                                                 std::placeholders::_1, handler ) ) );

            return ec;
        }

        void stop_listen() {
            status_.store( status_code::CLOSING );
            acceptor_->close();
            status_.store( status_code::CLOSED );
            return;
        }

      private:
        void accept_handler( boost::system::error_code ec,
                             new_connection_handler handler ) {

            socket_mtx.lock();

            boost::asio::post(
                ctx_, boost::asio::bind_executor(
                          accepted_handler_strand_,
                          [=]() {
                              handler( ec, std::move( socket_ ) );

                              socket_mtx.unlock();

                              if ( !ec ) {
                                  acceptor_->async_accept(
                                      socket_,
                                      boost::asio::bind_executor(
                                          strand_,
                                          std::bind( &listener::accept_handler, this,
                                                     std::placeholders::_1, handler ) ) );
                              }
                          } )

            );
        }

        std::atomic< status_code > status_;
        std::mutex socket_mtx;

        boost::asio::ip::tcp::socket socket_;
        boost::asio::io_context::strand accepted_handler_strand_;
        std::unique_ptr< boost::asio::ip::tcp::acceptor > acceptor_;

        boost::asio::io_context& ctx_;
        boost::asio::io_context::strand strand_;
    };
} // namespace ohlano
