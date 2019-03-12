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

        using sessions_visitor = std::function< void( sessions_map& ) >;
        using const_sessions_visitor = std::function< void( const sessions_map& ) >;

        using sessions_visitor_fptr = void ( * )( sessions_map& );
        using const_sessions_visitor_fptr = void ( * )( const sessions_map& );

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
        void sess_cleanup() {}

        template < typename Visitor, typename Opt = ThreadOption >
        typename threads::opt_enable_if_multi_thread< Opt >::type
        visit_sessions( Visitor visitor ) const {
            std::lock_guard< std::mutex > sess_lock{ sessions_mtx_ };
            visitor( sessions() );
        }

        template < typename Visitor, typename Opt = ThreadOption >
        typename threads::opt_enable_if_single_thread< Opt >::type
        visit_sessions( Visitor visitor ) const {
            visitor( sessions() );
        }

        template < typename Visitor, typename Opt = ThreadOption >
        typename threads::opt_enable_if_multi_thread< Opt >::type
        visit_sessions( Visitor visitor ) {
            std::lock_guard< std::mutex > sess_lock{ sessions_mtx_ };
            visitor( sessions() );
        }

        template < typename Visitor, typename Opt = ThreadOption >
        typename threads::opt_enable_if_single_thread< Opt >::type
        visit_sessions( Visitor visitor ) {
            visitor( sessions() );
        }
        
        template<typename Visitor>
        void visit_sess(Visitor v){
            sessions_.apply(v);
        }

        /// close all connections and delete all sessions
        void sess_close_all() {
            
            sessions_.apply([](sessions_map& sessions){
                
                for ( auto it = sessions.begin(); it != sessions.end(); ++it ) {
                    
                    auto[key, sess] = *it;
                    
                    if ( sess )
                        sess->close();
                    
                    sessions.erase( it );
                }
                
            });
            
        }

        ohlano::safe_visitable<sessions_map, ThreadOption>& sessions() { return sessions_; }

        const ohlano::safe_visitable<sessions_map, ThreadOption>& sessions() const { return sessions_; }

      private:
        
        ohlano::safe_visitable<sessions_map, ThreadOption> sessions_;
        std::mutex sessions_mtx_;
    };
}
