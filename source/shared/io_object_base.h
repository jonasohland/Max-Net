#include <memory>
#include <thread>

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/dispatch.hpp>

#include "connection.h"

namespace ohlano {
    
    namespace io_object {
        
        namespace threads {
            struct single{};
            struct multi{};
        }
        
        template <typename ThreadOption>
        class thread_base {};
        
        template<>
        class thread_base<threads::single> {
            
        protected:
            
            void create_threads(){
                worker_thread_ = std::make_unique<std::thread>(std::bind(&thread_base::perform_work, this));
            }
            
            virtual void perform_work() = 0;
            
        private:
            
            std::unique_ptr<std::thread> worker_thread_;
            
        };
        
        template<>
        class thread_base<threads::multi> {
            
        protected:
            
            void create_threads(int num_threads = 1){
                for(int i = 0; i < num_threads; ++i){
                    worker_threads_.push_back(std::make_unique<std::thread>(std::bind(&thread_base::perform_work, this)));
                }
            }
            
            virtual void perform_work() = 0;
            
        private:
            
            std::vector<std::unique_ptr<std::thread>> worker_threads_;
            
        };

        template <typename MessageType, typename ThreadOption>
        class base : thread_base<ThreadOption>{
            
        public:
            
            virtual ~base(){
                
                if(object_work_guard_.owns_work()){
                    object_work_guard_.reset();
                }
                
            }
            
            using message_type = MessageType;
            using session_impl_type = ohlano::connection<boost::beast::websocket::stream<boost::asio::ip::tcp>, MessageType>;
            using session_type = std::shared_ptr<session_impl_type>;
            
            
            
        protected:
            
            virtual void on_work_started(){};
            virtual void on_work_finished(){};
            
            template<typename Opt = ThreadOption>
            typename std::enable_if<std::is_same<Opt, threads::single>::value>::type begin_work(){
                this->create_threads();
            }
            
            template<typename Opt = ThreadOption>
            typename std::enable_if<std::is_same<Opt, threads::multi>::value>::type begin_work(int threads = 1){
                this->create_threads(threads);
            }
            
            template<typename ...Ts>
            void post(Ts&& ...args){
                boost::asio::post(ctx_, std::forward<Ts>(args)...);
            }
            
            template<typename ...Ts>
            void dispatch(Ts&& ...args){
                boost::asio::dispatch(ctx_, std::forward<Ts>(args)...);
            }
            
            virtual void perform_work() override {
                on_work_started();
                ctx_.run();
                on_work_finished();
            }
            
            boost::asio::io_context ctx_;
            boost::asio::executor_work_guard<boost::asio::io_context::executor_type> object_work_guard_{ ctx_.get_executor() };
            
        private:
            
        };
        
    }
    
}
