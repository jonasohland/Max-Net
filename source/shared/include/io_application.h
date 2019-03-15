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

#include <memory>
#include <thread>

#include <boost/asio/dispatch.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/post.hpp>

#include "session.h"
#include "types.h"

#ifdef O_NET_POSIX

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#endif

namespace o {

    namespace io {

        namespace detail {

            template < typename ThreadOption >
            class thread_base {};

            /** base for classes that want to handle thread-creation and stuff on their own */
            template <>
            class thread_base< threads::none > {
                virtual void do_run() = 0;
            };

            /** base for classes that want to perform work on one or many threads */
            template <>
            class thread_base< threads::single > {

              public:
                virtual ~thread_base() {}

                /** determine on how many threads this app is running */
                size_t thread_count() { return static_cast< bool >( worker_thread_ ); }

              protected:
                /** wait for all threads to exit */
                void await_threads_end() const {
                    if ( worker_thread_ )
                        if ( worker_thread_->joinable() )
                            worker_thread_->join();
                }

                /** launch the applications thread and begin performing work */
                void create_threads( int threads ) {
                    worker_thread_ = std::make_unique< std::thread >(
                        std::bind( &thread_base::do_run, this ) );
                }

                /** implemented by the application to perform actual work */
                virtual void do_run() = 0;

              private:
                std::unique_ptr< std::thread > worker_thread_;
            };

            /** base for classes that want to perform work on one or many threads */
            template <>
            class thread_base< threads::multi > {

              public:
                virtual ~thread_base() {}

                /** determine on how many threads this app is running */
                size_t thread_count() { return worker_threads_.size(); }

              protected:
                /** wait for the application thread to exit */
                void await_threads_end() const {
                    for ( auto& thread : worker_threads_ ) {
                        if ( thread )
                            if ( thread->joinable() )
                                thread->join();
                    }
                }

                /** create x new threads and begin performing work on them */
                void create_threads( int num_threads = 1 ) {
                    for ( int i = 0; i < num_threads; ++i ) {
                        worker_threads_.push_back( std::make_unique< std::thread >(
                            std::bind( &thread_base::do_run, this ) ) );
                    }
                }

                /** implemented by the application to perform actual work */
                virtual void do_run() = 0;

                /** access the base_mtx that should be locked when performing any
                 * operations
                 * on the worker threads container */
                std::mutex& base_mtx() { return thread_base_mutex; }

              private:
                std::mutex thread_base_mutex;
                std::vector< std::unique_ptr< std::thread > > worker_threads_;
            };
        } // namespace detail

        /** base class for applications that want to perform io
            \snippet ioapp.cpp ioapp_base_example
         */
        template < typename ThreadOption >
        class io_app_base : public detail::thread_base< ThreadOption > {

          public:
            virtual ~io_app_base() {}

            /** applications thread option (either o::threads::single or
             * o::threads::multi) */
            using thread_option = ThreadOption;

            /** underlying io context type */
            using context_type = boost::asio::io_context;

            /** underlying executor type */
            using executor_type = boost::asio::io_context::executor_type;

            /** Run the application in this thread. This call will return as soon as the
             * application runs out of work. */
            template < typename Opt = ThreadOption >
            typename threads::opt_enable_if_no_threads< Opt >::type run() {
                app_prepare();
                do_run();
            }

            /** Determine if the caller is running in one of the applications threads. */
            bool call_is_in_app() {
                return context().get_executor().running_in_this_thread();
            }

            /** Determine if the caller is running in the applications thread, and the
               call
                to the apps resources would be safe.
                \snippet ioapp.cpp ioapp_call_is_safe_example

                Will output:
                \code
                 call from startup method is not safe
                 call from other thread is not safe
                 call from executor is safe
                 call from timer callback is safe
                 call from exit request handler is safe
                 call from app stopped handler is not safe
                \endcode
             */
            template < typename Opt = ThreadOption >
            bool call_is_safe() {
                return call_is_in_app() && !( multithreaded< Opt > );
            }

            /** determine if the app is running */
            bool running() const { return is_running_; }

          protected:
            /** is this a multithreaded application? */
            template < typename Opt = ThreadOption >
            static constexpr const bool multithreaded =
                std::is_same< Opt, threads::multi >::value;

            /** Begin this apps operation. This will spawn x new threads and use those to
             * perform work */
            template < typename Opt = ThreadOption >
            typename threads::opt_enable_if_threads< Opt >::type
            app_launch( int threads = 1 ) {
                app_prepare();
                this->create_threads( threads );
                is_running_ = true;
            }

            /** Allow the application to exit. This will call the on_app_exit handler with
             * the supplied reason code (or 0 by default) from inside the application and
             * exit after that. */
            bool app_allow_exit( int reason = 0 ) {

                is_running_ = false;

                this->dispatch( [this, reason]() { this->on_app_exit( reason ); } );

                if ( object_work_guard_.owns_work() ) {
                    object_work_guard_.reset();
                    return true;
                }

                return false;
            }

            /** Wait for all threads owned by the app to exit. When this function returns,
             * it is safe to destroy the app. */
            void app_join() const { this->await_threads_end(); }

            /** post a handler to the apps executor*/
            template < typename... Ts >
            void post( Ts&&... args ) {
                boost::asio::post( ctx_, std::forward< Ts >( args )... );
            }

            /** dispatch a handler on the apps executor */
            template < typename... Ts >
            void dispatch( Ts&&... args ) {
                boost::asio::dispatch( ctx_, std::forward< Ts >( args )... );
            }

            /** get the underlying io context used to perform io by this app */
            context_type& context() { return ctx_; }

            /** get the underlying executor used by this app */
            executor_type& executor() { return ctx_.get_executor(); }

            /** will be called when the app is started */
            virtual void on_app_started(){};

            /** will be called right after the app was allowed to exit */
            virtual void on_app_exit( int reason ){};

            /** will be called when the app is stopped */
            virtual void on_app_stopped(){};

            /** will be called when the app is started before any thread was launched */
            virtual void app_prepare(){};

            /** implemented by the io::base<> to perform actual work */
            virtual void do_run() override {
                call_work_start_notification();
                this->context().run();
                call_work_end_notification();
            }

          private:
            template < typename Opt = ThreadOption >
            typename threads::opt_enable_if_multi_thread< Opt >::type
            call_work_end_notification() {
                std::lock_guard< std::mutex > call_lock{ this->base_mtx() };
                on_app_stopped();
            }

            template < typename Opt = ThreadOption >
            typename threads::opt_enable_if_multi_thread< Opt >::type
            call_work_start_notification() {
                std::lock_guard< std::mutex > call_lock{ this->base_mtx() };
                on_app_started();
            }

            template < typename Opt = ThreadOption >
            typename threads::opt_enable_if_single_or_none< Opt >::type
            call_work_end_notification() {
                on_app_stopped();
            }

            template < typename Opt = ThreadOption >
            typename threads::opt_enable_if_single_or_none< Opt >::type
            call_work_start_notification() {
                on_app_started();
            }

            bool is_running_;
            context_type ctx_;
            boost::asio::executor_work_guard< executor_type > object_work_guard_{
                ctx_.get_executor()
            };
        };

        /** Base class for applications that want to perform io and handle os signals. This class provides all features from o::io::io_app_base, but automatically handles signals from the operating system. By default it will handle SIGINT and SIGQUIT. Both Signals will eventually lead to call to o::io::io_app_base::app_allow_exit(int signal_num).
            \snippet ioapp.cpp basic_io_app_ex
         */
        template < typename ThreadOption >
        class basic_io_app : public io_app_base< ThreadOption > {

          public:
            basic_io_app() : signal_set_( this->context(), 2, 15 ) {}

          protected:
            
            /** Implemented to setup the signal set. If you override this, make sure you call setup_signals() yourself */
            virtual void app_prepare() override { setup_signals(); }

            /** implement to be notified about signals you subscribed to with signals().add(num) */
            virtual void on_signal( int signal_number ) {}

            /** Begin listening to signals. This will be called from the app when it is started. */
            void setup_signals() {
                signal_set_.async_wait(
                    [this]( const boost::system::error_code& ec, int signal_number ) {
                        if ( !ec ) {
                            if ( signal_number == 2 || signal_number == 15 )
                                this->app_allow_exit( signal_number );
                            else {
                                on_signal( signal_number );
                                setup_signals();
                            }
                        }
                    } );
            }

            /** Access the underlying boost::asio::signal_set */
            boost::asio::signal_set& signals() { return signal_set_; }

          private:
            boost::asio::signal_set signal_set_;
        };

#if defined( O_NET_POSIX ) || defined( DOXY_GENERATE )

        /** An io application can inherit from this class if it wants to read data from
         stdin
         \code{.cpp}
            using app_base = o::io::simple_io_app<o::threads::none>;

            struct app : public app_base, public o::io::istream_listener {

                app() : o::io::istream_listener(this->context())
                {}

                virtual void on_console_input(std::string str) override {
                    std::cout << "You entered: " << str << std::endl;
                }
            };
         \endcode
         */
        class istream_listener {

          public:
            istream_listener() = delete;

            /** construct the stream listener and give it a context to operate on */
            explicit istream_listener( boost::asio::io_context& ctx )
                : std_istream_desc_( ctx, ::dup( STDIN_FILENO ) ) {
                do_read();
            }

            /** implement this function to get notified if a complete line was read from
             * stdin */
            virtual void on_console_input( std::string ) = 0;

          private:
            void handle_istream_read( const boost::system::error_code& ec,
                                      std::size_t length ) {

                if ( !ec ) {

                    on_console_input(
                        std::string( boost::asio::buffers_begin( input_buffer_.data() ),
                                     boost::asio::buffers_begin( input_buffer_.data() ) +
                                         input_buffer_.size() - 1 ) );
                    input_buffer_.consume( input_buffer_.size() );

                    do_read();
                }
            }

            void do_read() {
                boost::asio::async_read_until(
                    std_istream_desc_, input_buffer_, '\n',
                    std::bind( &istream_listener::handle_istream_read, this,
                               std::placeholders::_1, std::placeholders::_2 ) );
            }

            boost::asio::streambuf input_buffer_;
            boost::asio::posix::stream_descriptor std_istream_desc_;
        };
#endif
    } // namespace io
} // namespace o
