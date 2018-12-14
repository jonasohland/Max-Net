#pragma once

#include <functional>
#include <utility>
#include <mutex>
#include "net_url.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/system/error_code.hpp>

template<typename ProtocolType>
class multi_resolver : std::enable_shared_from_this<multi_resolver<ProtocolType>> {
    
    typedef std::function<void(net_url<>&)> resolve_handler_type;
    typedef std::pair<net_url<>, resolve_handler_type> resolve_queue_value_type;
    
    boost::asio::io_context::strand strand_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
    boost::asio::ip::basic_resolver<ProtocolType> res_;
    std::deque<std::pair<net_url<>, resolve_handler_type>> ws_queue_;
    
    std::mutex queue_mtx_;
    
    
public:
    explicit multi_resolver(boost::asio::io_context& ctx): strand_(ctx), work_guard_(ctx.get_executor()), res_(ctx)
    {
    }
    
    void resolve(net_url<>& url, resolve_handler_type& handler){
        std::unique_lock<std::mutex> lock{ queue_mtx_ };
        ws_queue_.emplace_back(url, handler);
        
        if(ws_queue_.size() < 2){
            begin_resolve(ws_queue_.front());
        }
        
    }
    
    void resolve(resolve_queue_value_type qelem){
        
    }
    
private:
    
    void begin_resolve(resolve_queue_value_type& qelem){
        res_.async_resolve(qelem.first.host(),
                           boost::asio::bind_executor(
                                                      strand_,
                                                      std::bind(
                                                                &multi_resolver::resolve_handler,
                                                                this->shared_from_this(),
                                                                std::placeholders::_1,
                                                                std::placeholders::_2,
                                                                qelem
                                                                )
                                                      )
                           );
    }
    
    
    void resolve_handler(boost::system::error_code ec,
                         typename boost::asio::ip::basic_resolver<ProtocolType>::results_type results,
                         resolve_queue_value_type& qelem)
    {
        std::unique_lock<std::mutex> lock{ queue_mtx_ };
        
        for(auto& result : results){
            DBG("result: ", result.endpoint().address().to_string());
        }
        
        qelem.second(qelem.first);
        
    }
    
};
