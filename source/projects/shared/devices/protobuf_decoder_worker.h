#pragma once

#include <thread>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>

class protobuf_decoder_worker {
	
	boost::asio::io_context ioc_;
	std::vector<std::thread> threads_;

	boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_{ioc_.get_executor()};

	typedef std::function<void(proto_message_wrapper&)> decoded_handler;

public:

	protobuf_decoder_worker(){}

	void run(size_t num_threads) {

		for (size_t i = 0; i < num_threads; i++) {
			DBG("running thread: ", i);
			threads_.emplace_back([this](){ioc_.run();});
		}
	}

	void stop() {

		if (work_.owns_work()) {
			work_.reset();
		}

		for (auto& thread : threads_) {
			if (thread.joinable()) {
				thread.join();
			}
		}
	}

	void async_decode(proto_message_wrapper& msg, decoded_handler handler) {

		ioc_.post([=]() mutable {

			msg.vect().clear();

			ioc_.post([msg, handler]() mutable { handler(msg); });

		});
	}

};