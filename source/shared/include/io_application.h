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
#include <boost/asio/post.hpp>

#include "session.h"
#include "types.h"

namespace ohlano {

    namespace io_app {

        template < typename ThreadOption >
        class thread_base {};

        /** base for classes that want to perform work on one or many threads */
        template <>
        class thread_base< threads::single > {

          public:
            virtual ~thread_base() {}

            /** determine on how many threads this app is running */
            size_t thread_count() { return static_cast< bool >( worker_thread_ ); }

            /** Call this method if you want your thread to perform work for the app.
             * Handlers will be invoked from this function. It will exit when the app runs
             * out of work and is allowed to exit */
            void perform() { do_run(); }

          protected:
            /** wait for all threads to exit */
            void await_threads_end() const {
                if ( worker_thread_ )
                    if ( worker_thread_->joinable() )
                        worker_thread_->join();
            }

            /** launch the applications thread and begin performing work */
            void create_threads() {
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

            /** Call this method if you want your thread to perform work for the app.
             * Handlers will be invoked from this function. It will exit when the app runs
             * out of work and is allowed to exit */
            void perform() { do_run(); }

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

            /** access the base_mtx that should be locked when performing any operations
             * on the worker threads container */
            std::mutex& base_mtx() { return thread_base_mutex; }

          private:
            std::mutex thread_base_mutex;
            std::vector< std::unique_ptr< std::thread > > worker_threads_;
        };

        /** base class for applications that want to perform io */
        template < typename ThreadOption >
        class base : thread_base< ThreadOption > {

          public:
            virtual ~base() {}

            /** applications thread option (either o::threads::single or
             * o::threads::multi) */
            using thread_option = ThreadOption;

            /** underlying io context type */
            using context_type = boost::asio::io_context;

            /** underlying executor type */
            using executor_type = boost::asio::io_context::executor_type;

            /** Run the application in this thread. This call will return as soon as the
             * application runs out of work. */
            void perform() { do_run(); }
            
            /** Determine if the caller is running in one of the applications threads. */
            bool call_is_in_app() {
                return context().get_executor().running_in_this_thread();
            }

            /** Determine if the caller is running in the applications thread, and thecall
             * to the apps resources would be safe */
            template < typename Opt = ThreadOption >
            bool call_is_safe() {
                return call_is_in_app() && !( multithreaded< Opt > );
            }

            /** Invoke a function from inside the app. If the caller is inside the
             * applications thread, the function will be invoked from this call. Otherwise
             * it will be scheduled for later execution from one of the apps threads. */
            template < typename Callable,
                       typename Allocator = std::allocator< Callable > >
            void call_safe( Callable&& c,
                            const Allocator& alloc = std::allocator< Callable >() ) {
                this->context().get_executor().dispatch( std::forward< Callable >( c ),
                                                         alloc );
            }

            /** allow the app to finish work */
            void exit() { app_end_op(); }

            /** determine if the app is running */
            bool running() const { return is_running_; }

          protected:
            /** is this a multithreaded application? */
            template < typename Opt = ThreadOption >
            static constexpr const bool multithreaded =
                std::is_same< Opt, threads::multi >::value;

            /** begin the apps operation on a new thread */
            template < typename Opt = ThreadOption >
            typename threads::opt_enable_if_single_thread< Opt >::type app_begin_op() {
                this->create_threads();
                is_running_ = true;
            }

            /** Begin this apps operation. This will spawn x new threads and use those to
             * perform work */
            template < typename Opt = ThreadOption >
            typename threads::opt_enable_if_multi_thread< Opt >::type
            app_begin_op( int threads = 1 ) {
                this->create_threads( threads );
                is_running_ = true;
            }

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

            /** implemented by the application to perform actual work */
            virtual void do_run() override {
                call_work_start_notification();
                this->context().run();
                call_work_end_notification();
            }

            /** allow the executor to end its work */
            bool app_end_op() {

                is_running_ = false;

                if ( object_work_guard_.owns_work() ) {
                    object_work_guard_.reset();
                    return true;
                }

                return false;
            }

            /** wait for any work to end on the executor */
            void app_wait_op_end() const { this->await_threads_end(); }

            /** get the underlying io context used to perform io by this app */
            context_type& context() { return ctx_; }

            /** get the underlying executor used by this app */
            executor_type& executor() { return ctx_.get_executor(); }

            /** will be called when the app is started */
            virtual void on_work_started(){};

            /** will be called right after the app finished execution */
            virtual void on_work_finished(){};

          private:
            template < typename Opt = ThreadOption >
            typename threads::opt_enable_if_multi_thread< Opt >::type
            call_work_end_notification() {
                std::lock_guard< std::mutex > call_lock{ this->base_mtx() };
                on_work_finished();
            }

            template < typename Opt = ThreadOption >
            typename threads::opt_enable_if_multi_thread< Opt >::type
            call_work_start_notification() {
                std::lock_guard< std::mutex > call_lock{ this->base_mtx() };
                on_work_started();
            }

            template < typename Opt = ThreadOption >
            typename threads::opt_enable_if_single_thread< Opt >::type
            call_work_end_notification() {
                on_work_finished();
            }

            template < typename Opt = ThreadOption >
            typename threads::opt_enable_if_single_thread< Opt >::type
            call_work_start_notification() {
                on_work_started();
            }

            bool is_running_;
            boost::asio::io_context ctx_;
            boost::asio::executor_work_guard< boost::asio::io_context::executor_type >
                object_work_guard_{ ctx_.get_executor() };
        };
    } // namespace io_app
} // namespace ohlano
