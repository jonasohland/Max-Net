#pragma once

#include <deque>
#include <mutex>
#include <chrono>
#include <type_traits>
#include "../ohlano.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core/type_traits.hpp>
#include <boost/system/error_code.hpp>

namespace ohlano {

template<typename Message, typename StreamType, typename Enable = void>
class write_queue;

template<typename Message, typename StreamType>
class write_queue<Message, StreamType, typename std::enable_if<boost::beast::has_get_executor<StreamType>::value>::type> {

public:


	write_queue() = delete;
    
    write_queue(StreamType* stream): exec_(stream->get_executor()), strand_(exec_.context()) {
		stream_ = stream;
	}

	void submit(Message& msg) {

		std::unique_lock<std::mutex> lock{queue_mtx_};

		msg_queue.emplace_back(msg);
        DBG("Msg pushed to queue, queue size: ", msg_queue.size());
        
		if (msg_queue.size() < 2) {
            DBG("Performing write operation...");
			perform_write(msg);
        } else {
            DBG("Write already in progress");
        }
	}

	void purge() {
		std::unique_lock<std::mutex> lock{queue_mtx_};
		msg_queue.clear();
	}


private:

	void perform_write(Message& msg) {
        stream_->async_write(
            boost::asio::buffer(msg.data(), msg.size()),
            boost::asio::bind_executor(
                   strand_,
                   std::bind(&write_queue::write_complete_handler,
                             this,
                             std::placeholders::_1,
                             std::placeholders::_2
                 )
            )
        );
	}


	void write_complete_handler(boost::system::error_code ec, std::size_t bytes) {
        
        std::unique_lock<std::mutex> lock{queue_mtx_};

        msg_queue.pop_front();
        
        DBG("popped one element from queue front");
        
        if (msg_queue.size() > 0) {
            perform_write(msg_queue.front());
            DBG("Performing write to clear queue, queue size: ", msg_queue.size());
        } else {
            DBG("no more elements in queue");
        }
	}

	std::mutex queue_mtx_;

	StreamType* stream_;
	boost::asio::io_context::executor_type exec_;
    boost::asio::io_context::strand strand_;
    boost::asio::steady_timer wait_timer{exec_.context()};

	std::deque<Message> msg_queue;

};
}
