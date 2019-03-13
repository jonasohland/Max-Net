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

        template <>
        class thread_base< threads::single > {
          public:
            virtual ~thread_base() {}

            size_t thread_count() { return static_cast< bool >( worker_thread_ ); }

            void await_threads_end() const {
                if ( worker_thread_ )
                    if ( worker_thread_->joinable() )
                        worker_thread_->join();
            }
            
            void perform() {
                do_run();
            }

          protected:
            void create_threads() {
                worker_thread_ = std::make_unique< std::thread >(
                    std::bind( &thread_base::do_run, this ) );
            }

            virtual void do_run() = 0;

          private:
            std::unique_ptr< std::thread > worker_thread_;
        };

        template <>
        class thread_base< threads::multi > {
          public:
            virtual ~thread_base() {}

            size_t thread_count() { return worker_threads_.size(); }

          protected:
            void await_threads_end() const {
                for ( auto& thread : worker_threads_ ) {
                    if ( thread )
                        if ( thread->joinable() )
                            thread->join();
                }
            }

            void create_threads( int num_threads = 1 ) {
                for ( int i = 0; i < num_threads; ++i ) {
                    worker_threads_.push_back( std::make_unique< std::thread >(
                        std::bind( &thread_base::do_run, this ) ) );
                }
            }

            virtual void do_run() = 0;

            std::mutex& base_mtx() { return thread_base_mutex; }

          private:
            std::mutex thread_base_mutex;
            std::vector< std::unique_ptr< std::thread > > worker_threads_;
        };

        template < typename ThreadOption >
        class base : thread_base< ThreadOption > {

          public:
            virtual ~base() {}

            using thread_option = ThreadOption;
            using context_type = boost::asio::io_context;
            using executor_type = boost::asio::io_context::executor_type;
            using strand_type = boost::asio::io_context::strand;

            /// Run the application in this thread. This call will return as soon as the
            /// application runs out of work.
            void perform() { do_run(); }

            /** Determine if the caller is running in one of the applications threads. */
            bool is_in_app_call() {
                return context().get_executor().running_in_this_thread();
            }

            /// Determine if the caller is running in the applications thread, and the
            /// call to the apps resources would be safe
            template < typename Opt = ThreadOption >
            bool call_is_safe() {
                return is_in_app_call() && !( multithreaded< Opt > );
            }

            /// Invoke a function from inside the app. If the caller is inside the
            /// applications thread, the function will be invoked from this call.
            /// Otherwise it will be scheduled for later execution from one of the apps
            /// threads.
            template < typename Callable,
                       typename Allocator = std::allocator< Callable > >
            void call_safe( Callable&& c,
                            const Allocator& alloc = std::allocator< Callable >() ) {
                this->context().get_executor().dispatch( std::forward< Callable >( c ),
                                                         alloc );
            }

            void exit() { app_end_op(); }

            bool running() const { return is_running_; }

          protected:
            template < typename Opt = ThreadOption >
            static constexpr const bool multithreaded =
                std::is_same< Opt, threads::multi >::value;

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

            virtual void on_work_started(){};
            virtual void on_work_finished(){};

            template < typename Opt = ThreadOption >
            typename threads::opt_enable_if_single_thread< Opt >::type app_begin_op() {
                this->create_threads();
                is_running_ = true;
            }

            template < typename Opt = ThreadOption >
            typename threads::opt_enable_if_multi_thread< Opt >::type
            app_begin_op( int threads = 1 ) {
                this->create_threads( threads );
                is_running_ = true;
            }

            template < typename... Ts >
            void post( Ts&&... args ) {
                boost::asio::post( ctx_, std::forward< Ts >( args )... );
            }

            template < typename... Ts >
            void dispatch( Ts&&... args ) {
                boost::asio::dispatch( ctx_, std::forward< Ts >( args )... );
            }

            virtual void do_run() override {
                call_work_start_notification();
                this->context().run();
                call_work_end_notification();
            }

            /// allow the executor to end its work
            bool app_end_op() {
                
                is_running_ = false;

                if ( object_work_guard_.owns_work() ) {
                    object_work_guard_.reset();
                    return true;
                }

                return false;
            }

            /// wait for any work to end on the executor
            void app_wait_op_end() const { this->await_threads_end(); }

            boost::asio::io_context& context() { return ctx_; }

            boost::asio::io_context::executor_type& executor() {
                return ctx_.get_executor();
            }

          private:
            bool is_running_;
            boost::asio::io_context ctx_;
            boost::asio::executor_work_guard< boost::asio::io_context::executor_type >
                object_work_guard_{ ctx_.get_executor() };
        };
    } // namespace io_app
} // namespace ohlano
