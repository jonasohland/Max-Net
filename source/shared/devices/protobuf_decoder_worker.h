#pragma once

#include <thread>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include "protobuf_decoder.h"


namespace ohlano {
	template<typename Message>
	class protobuf_decoder_worker {

		boost::asio::io_context ioc_;
		std::vector<std::thread> threads_;

		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_{ ioc_.get_executor() };

		typedef std::function<void(Message*)> decoded_handler;
		typedef std::function<void(Message*)> encoded_handler;

	public:

		protobuf_decoder_worker() {}

		void run(size_t num_threads) {

			for (size_t i = 0; i < num_threads; i++) {
				DBG("running thread: ", i);
				threads_.emplace_back([this]() {ioc_.run(); });
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

		void async_decode(Message* msg, decoded_handler handler) {
			ioc_.post([=]() mutable {
				msg->deserialize();
				handler(msg);
			});
		}

		void async_encode(Message* msg, encoded_handler handler) {
			ioc_.post([=]() mutable {
				msg->serialize();
				handler(msg);
			});
		}

	};
}