#include "../ohlano.h"
#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <chrono>
#include <mutex>

template < typename T >
class stats_category {

    class stat {

        T total = 0;
        T last = 0;

      public:
        void diff_callback() {
            last = total;
            total = 0;
        }

        T get() { return last; }

        T count() { return total; }

        T operator++() { return total++; }

        T operator++( int i ) { return ++total; }

        void add( T amount ) { total += amount; }

        void reset() {
            total = 0;
            last = 0;
        }
    };

    stat data_;
    stat msgs_;

  public:
    stat& data() { return data_; }
    stat& msgs() { return msgs_; }

    void reset() {
        data().reset();
        msgs().reset();
    }
};

template < typename T, typename Clock >
class session_stats {

    using clock = Clock;

    stats_category< T > inbound_;
    stats_category< T > outbound_;

    boost::asio::basic_waitable_timer< Clock > timer_;
    std::mutex mutex_;

    void do_stat_check( boost::system::error_code ec ) {

        if ( ec ) {
            is_enabled = false;
            return;
        }

        inbound_.data().diff_callback();
        inbound_.msgs().diff_callback();
        outbound_.data().diff_callback();
        outbound_.msgs().diff_callback();

        timer_.expires_after( std::chrono::seconds( 1 ) );
        timer_.async_wait(
            std::bind( &session_stats::do_stat_check, this, std::placeholders::_1 ) );
    }

  public:
    explicit session_stats( boost::asio::io_context& ctx ) : timer_( ctx ) {}

    ~session_stats() {
        DBG( "timer cancel..." );
        timer_.cancel();
        DBG( "stats destructor ran" );
    }

    void set_enabled( bool enabled ) {
        if ( enabled && !is_enabled ) {

            inbound().reset();
            outbound().reset();

            is_enabled = true;

            timer_.expires_from_now( std::chrono::seconds( 1 ) );
            timer_.async_wait(
                std::bind( &session_stats::do_stat_check, this, std::placeholders::_1 ) );

        } else {
            if ( is_enabled ) {
                timer_.cancel();
            }
        }
    }

    bool is_enabled = false;

    stats_category< T >& inbound() { return inbound_; }
    stats_category< T >& outbound() { return outbound_; }

    std::mutex& mtx() { return mutex_; };
};
