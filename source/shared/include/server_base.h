#pragma once

#include <map>

#include <boost/asio.hpp>

#include "io_application.h"
#include "ohlano.h"
#include "types.h"

namespace ohlano::net::server {

    template < typename Session, typename ThreadOption >
    class base : public io_app::base< ThreadOption > {

      public:
        using sessions_map = std::map< size_t, std::shared_ptr< Session > >;
        using sessions_type =
            ohlano::threads::opt_safe_visitable< sessions_map, ThreadOption >;

        template < typename Opt = ThreadOption >
        typename threads::opt_enable_if_multi_thread< Opt >::type start( int threads ) {
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

                for ( auto & [ key, sess ] : sessions )
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
