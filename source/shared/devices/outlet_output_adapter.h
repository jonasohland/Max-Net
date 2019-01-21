#pragma once

#include <mutex>
#include "c74_min.h"

namespace ohlano {

	template<typename Message>
	class outlet_output_adapter {

	public:
		outlet_output_adapter() = delete;

		outlet_output_adapter(c74::min::outlet<c74::min::thread_check::none, c74::min::thread_action::assert>* outlet) : outlet_(outlet) {

		}

		void write(Message* message) {
			std::lock_guard<std::mutex> lock {outlet_mutex_};
			c74::min::outlet_do_send(outlet_->get_instance(), message->get_atoms());
		}

	private:

		c74::min::outlet<c74::min::thread_check::none, c74::min::thread_action::assert>* outlet_;
		std::deque<Message*> output_queue_;
		std::mutex outlet_mutex_;
	};
}