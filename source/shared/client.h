#include "io_object_base.h"
#include "net_url.h"
#include "c74_min.h"

#include <atomic>

#include <boost/system/error_code.hpp>

namespace ohlano {
    
    

    template <typename MessageType>
    class client : public io_object::base<MessageType, io_object::threads::multi> {
        
    public:
        
        using client_base = io_object::base<MessageType, io_object::threads::multi>;
        
        explicit client(){
            this->begin_work(4);
        }
        
        virtual void handle_message(boost::system::error_code, const MessageType*, size_t) = 0;
        
        virtual void on_ready(boost::system::error_code) = 0;
        
        virtual void on_close(boost::system::error_code) = 0;
        
        
        void session_create(net_url<> url){
            
        }
        
    private:
        
        typename client_base::session_type session;

        typename MessageType::factory factory_;
        
        std::atomic<int> connections_refc_;
        
    };
    
}
