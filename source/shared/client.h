#include "io_object_base.h"
#include "net_url.h"
#include "c74_min.h"
#include "devices/multi_resolver.h"

#include <atomic>

#include <boost/system/error_code.hpp>

namespace ohlano {
    
    

    template <typename MessageType, typename ThreadOptions>
    class client : public io_object::base<MessageType,
                            ThreadOptions> {
        
    public:
        
        using client_base = io_object::base<MessageType, ThreadOptions>;
      
        virtual ~client() {

        }

        virtual const MessageType* handle_message(const MessageType *, size_t) = 0;

        virtual void on_ready(boost::system::error_code) = 0;

        virtual void on_close(boost::system::error_code) = 0;

        void session_create(net_url<> url) {

          session = std::make_shared<typename client_base::session_impl_type>(
              this->context(), factory_, &connections_refc_);
            
            DBG((session->send_detected())? "send found" : "send not found");

              if (!url.is_resolved()) {
                resolver_.resolve(
                    url, [=](boost::system::error_code ec, net_url<> resolved_url) {
                      do_session_connect(resolved_url);
                    });

                  return;
              }

          do_session_connect(url);
          return;
        }

        void session_close() {
            if (session)
                session->close();
        }

    private:

        void do_session_connect(net_url<> url) {

            // why does this work?
            session->on_ready(std::bind(&client::on_ready, this,
                                        std::placeholders::_1));
            session->on_close(std::bind(&client::on_close, this,
                                        std::placeholders::_1));
            session->on_read(std::bind(
                &client::handle_message_wrapper, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));

            session->connect(url);
        }

        void handle_message_wrapper(boost::system::error_code ec, const MessageType* msg, size_t bytes) {

            if (msg != nullptr) {
                factory_.deallocate(handle_message(msg, bytes));
            }
            else DBG(ec.message());
        }

        typename client_base::session_type session;

        typename MessageType::factory factory_;

        multi_resolver<boost::asio::ip::tcp> resolver_{this->context()};

        std::atomic<int> connections_refc_;
};
    
}
