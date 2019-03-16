#pragma once

#include "../types.h"
#include <boost/asio.hpp>
#include <functional>

namespace o::net {

    /**
     * An UDP receiver.
     *
     * @author  Jonas Ohland
     * @date    16.03.2019
     *
     * @tparam  ThreadOption    Type of the thread option.
     */
    template < typename ThreadOption >
    class udp_receiver {

      public:
        enum class error_case { connect, bind, read };

        udp_receiver() = delete;

        /**
         * Constructor
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         *
         * @param [in,out]  ctx The context.
         */
        udp_receiver( boost::asio::io_context& ctx ) : sock_( ctx ) {}

        /**
         * Handles data received signals
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         *
         * @param   parameter1  The first parameter.
         */
        virtual void on_data_received( std::vector< char > ) = 0;

        /**
         * Executes the UDP error action
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         *
         * @param   eca The eca.
         * @param   eco The eco.
         */
        virtual void on_udp_error( error_case eca, boost::system::error_code eco ) {}

        /**
         * UDP bind
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         *
         * @param   remote_endp The remote endp.
         */
        void udp_bind( boost::asio::ip::udp::endpoint remote_endp ) {

            boost::system::error_code ec;

            sock_.open( remote_endp.protocol() );

            sock_.bind( remote_endp, ec );

            if ( ec )
                return on_udp_error( error_case::bind, ec );

            sock_.async_receive( buf_.prepare( 1024 ),
                                 std::bind( &udp_receiver::udp_on_data_received, this,
                                            std::placeholders::_1,
                                            std::placeholders::_2 ) );
        }

        /**
         * UDP close
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         */
        void udp_close() {

            if ( sock_.is_open() ) {
                sock_.shutdown(
                    boost::asio::basic_socket< boost::asio::ip::udp >::shutdown_both );
                sock_.close();
            }
        }

        boost::asio::basic_datagram_socket< boost::asio::ip::udp >& udp_sock() {
            return sock_;
        }

      private:
        void udp_on_data_received( boost::system::error_code ec, size_t bytes_s ) {

            if ( ec )
                return on_udp_error( error_case::read, ec );

            std::vector< char > bytes;

            buf_.consume( bytes_s );

            std::copy( boost::asio::buffers_begin( buf_.data() ),
                       boost::asio::buffers_end( buf_.data() ),
                       std::back_inserter( bytes ) );

            on_data_received( bytes );

            sock_.async_receive( buf_.prepare( 1024 ),
                                 std::bind( &udp_receiver::udp_on_data_received, this,
                                            std::placeholders::_1,
                                            std::placeholders::_2 ) );
        }

        boost::asio::streambuf buf_;
        boost::asio::ip::udp::socket sock_;
    };

} // namespace o::net
