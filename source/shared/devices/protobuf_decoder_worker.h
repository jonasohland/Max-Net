#pragma once

#include <thread>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include "protobuf_decoder.h"

class protobuf_decoder_worker {
	
	boost::asio::io_context ioc_;
	std::vector<std::thread> threads_;

	boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_{ioc_.get_executor()};

	typedef std::function<void(proto_message_wrapper&)> decoded_handler;
    typedef std::function<void(proto_message_wrapper&)> encoded_handler;


	protobuf_decoder& dec_;

public:

	protobuf_decoder_worker(protobuf_decoder& decoder): dec_(decoder){}

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
			dec_.decode(msg);
			handler(msg);
		});
	}
    
    void async_encode(proto_message_wrapper& msg, encoded_handler handler){
        ioc_.post([this, msg, handler]() mutable {
            dec_.encode(msg);
            handler(msg);
        });
    }

};
