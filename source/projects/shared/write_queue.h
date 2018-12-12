#pragma once

#include <deque>
#include <mutex>
#include <type_traits>
#include <boost/asio/io_context.hpp>
#include <boost/beast/core/type_traits.hpp>
#include <boost/system/error_code.hpp>

namespace ohlano {

template<typename Message, typename StreamType, typename Enable = void>
class write_queue;

template<typename Message, typename StreamType>
class write_queue<Message, StreamType, typename std::enable_if<boost::beast::has_get_executor<StreamType>::value>::type> {

public:


	write_queue() = delete;

	write_queue(StreamType* stream): ctx_(stream->get_executor()) {
		stream_ = stream;
	}

	void submit(Message& msg) {

		std::unique_lock<std::mutex>{queue_mtx_};

		Message* new_msg = new Message();

		std::swap(msg, *new_msg);

		msg_queue.push_back(new_msg);

		if (msg_queue.size() == 1) {
			perform_write(new_msg);
		}
	}

	void purge() {
		std::unique_lock<std::mutex>{queue_mtx_};
		msg_queue.clear();
	}


private:

	void perform_write(Message* msg) {
		stream_->async_write(
			boost::asio::buffer(msg->data(), msg->size()),
			std::bind(&write_queue::write_complete_handler,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
	}


	void write_complete_handler(boost::system::error_code ec, std::size_t bytes) {
		std::unique_lock<std::mutex>{queue_mtx_};
		
		delete msg_queue.front();
		msg_queue.pop_front();
		
		perform_next_write();
	
	}


	void perform_next_write() {
		std::unique_lock<std::mutex>{queue_mtx_};

		if (msg_queue.size() > 0) {
			perform_write(msg_queue.front());
		}
	}

	std::mutex queue_mtx_;

	StreamType* stream_;
	boost::asio::io_context::executor_type& ctx_;

	std::deque<Message*> msg_queue;

};
}