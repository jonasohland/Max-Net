#pragma once

#include <deque>
#include <mutex>
#include <chrono>
#include <type_traits>
#include "../ohlano.h"
#include <boost/optional.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core/type_traits.hpp>
#include <boost/system/error_code.hpp>

namespace ohlano {

template<typename Message, typename Stream, typename Enable = void>
class write_queue;

template<typename Message, typename Stream>
    class write_queue<Message, Stream, typename std::enable_if<boost::beast::has_get_executor<Stream>::value>::type> : public std::enable_shared_from_this<write_queue<Message, Stream>> {

public:
    typedef std::function<void(const Message*)> sent_handler_type;

	write_queue() = delete;
    
    write_queue(Stream* stream, typename Message::factory* allocator): exec_(stream->get_executor()), strand_(exec_.context()) {
		stream_ = stream;
	}

	void submit(const Message* msg) {

		std::unique_lock<std::mutex> lock{queue_mtx_};

		msg_queue.push_back(msg);
        
		if (msg_queue.size() < 2) {
			perform_write(msg);
        } else {
        }
	}

	void purge() {
		std::unique_lock<std::mutex> lock{queue_mtx_};
		msg_queue.clear();
	}
    
    void attach_sent_handler(sent_handler_type handler){
		snt_handler_ = boost::make_optional(handler);
    }

	void binary(bool txt) {
		stream_->binary(txt);
	}


private:

	void perform_write(const Message* msg) {

        stream_->async_write(
            boost::asio::buffer(msg->data(), msg->size()),
            boost::asio::bind_executor(
                   strand_,
                   std::bind(&write_queue::write_complete_handler,
                             this->shared_from_this(),
                             std::placeholders::_1,
                             std::placeholders::_2,
                             msg
                 )
            )
        );
	}


	void write_complete_handler(boost::system::error_code ec, std::size_t bytes, const Message* msg) {
        
        std::unique_lock<std::mutex> lock{queue_mtx_};

		DBG("sent: ", msg->size(), " bytes");

        msg_queue.pop_front();
        
        if (msg_queue.size() > 0) {
            perform_write(msg_queue.front());
        } 

		boost::asio::post(exec_, [=]() {
			snt_handler_.value_or(sent_handler_type())(msg); 
			boost::asio::post(exec_, [=]() {alloc_->deallocate(msg); 
			}); 
		});		
	}

	std::mutex queue_mtx_;

	Stream* stream_;
	typename Message::factory* alloc_;
	boost::asio::io_context::executor_type exec_;
    boost::asio::io_context::strand strand_;
    boost::asio::steady_timer wait_timer{exec_.context()};
        
	boost::optional<sent_handler_type> snt_handler_;

    std::deque<const Message*> msg_queue;
    std::atomic<bool> has_handler_;

};
}
