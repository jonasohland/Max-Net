#pragma once

#include <functional>
#include <utility>
#include <mutex>
#include "../net_url.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/system/error_code.hpp>

namespace ohlano {

    template < typename ProtocolType > class multi_resolver {

        typedef std::function< void(boost::system::error_code, net_url<>) >
            resolve_handler_type;
        typedef std::pair< net_url<>, resolve_handler_type > resolve_queue_value_type;

        boost::asio::io_context::strand strand_;
        boost::asio::ip::basic_resolver< ProtocolType > res_;

        std::deque< std::pair< net_url<>, resolve_handler_type > > ws_queue_;

        std::mutex queue_mtx_;

      public:
        explicit multi_resolver(boost::asio::io_context& ctx) : strand_(ctx), res_(ctx) {}

        void resolve(net_url<>& url, resolve_handler_type handler) {
            std::unique_lock< std::mutex > lock{ queue_mtx_ };
            ws_queue_.emplace_back(url, handler);

            if (ws_queue_.size() < 2) {
                begin_resolve(ws_queue_.front());
            }
        }

        void resolve(resolve_queue_value_type qelem) {}

      private:
        void begin_resolve(resolve_queue_value_type qelem) {
            res_.async_resolve(qelem.first.host(), qelem.first.port(),
                               boost::asio::bind_executor(
                                   strand_, std::bind(&multi_resolver::resolve_handler,
                                                      this, std::placeholders::_1,
                                                      std::placeholders::_2, qelem)));
        }

        void resolve_handler(
            boost::system::error_code ec,
            typename boost::asio::ip::basic_resolver< ProtocolType >::results_type
                results,
            resolve_queue_value_type qelem) {
            std::unique_lock< std::mutex > lock{ queue_mtx_ };

            qelem.first.set_resolver_results(results);
            ws_queue_.pop_front();

            if (!ws_queue_.empty()) {
                begin_resolve(ws_queue_.front());
            }
            qelem.second(ec, qelem.first);
        }
    };
}

