#pragma once

#include "c74_min.h"
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include "../../../build/source/projects/websocketclient/generic_max.pb.h"

namespace ohlano {
	class generic_max_message {
	public:
		generic_max* max_msg;

		static google::protobuf::Arena arena;

		generic_max_message(){}

		template<class ConstBufferSequence>
		generic_max_message(ConstBufferSequence const& seq) {
			max_msg->ParseFromArray(static_cast<void*>(boost::asio::buffers_begin(seq)), boost::asio::buffer_size(seq));
		}

		generic_max_message(c74::min::atoms const& _atoms) {

		}

		template<typename ConstBufferSequence>
		static generic_max_message from_buffer_sequence(ConstBufferSequence const& buffers) {
			return generic_max_message(buffers);
		}

	};
}