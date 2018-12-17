#pragma once

#include <google/protobuf/message.h>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include "../../../build/source/projects/websocketclient/generic_max.pb.h"

class proto_message_helper {
public:

	using proto_msg_ptr = google::protobuf::Message*;
	typedef std::function<void(proto_msg_ptr, const void*, size_t)> parser_function_type;

	parser_function_type get_parser() {
		return [=](proto_msg_ptr mess, const void* _dat, size_t bytes){

		};
	}
};

class proto_message_wrapper {

	google::protobuf::Message* mess_ = nullptr;
	std::vector<char> data_;

public:

	proto_message_wrapper(proto_message_wrapper&& other) :
		data_(std::forward<std::vector<char>>(other.data_)),
		mess_(std::forward<google::protobuf::Message*>(other.mess_))
	{}

	proto_message_wrapper(const proto_message_wrapper& other):
		data_(other.data_), mess_(other.mess_)
	{}

	proto_message_wrapper* operator=(const proto_message_wrapper& other)
	{ data_ = other.data_; mess_ = other.mess_; }

	proto_message_wrapper(std::string& str): data_(str.begin(), str.end()) {}

	proto_message_wrapper(){}

	~proto_message_wrapper() { if (mess_) { delete mess_; } }

	google::protobuf::Message*& proto() {
		return mess_;
	}

	std::vector<char>& vect() {
		return data_;
	}

	const void* data() const {
		return data_.data();
	}

	const size_t size() const {
		return data_.size();
	}

	std::string str() {
		return std::string(data_.data(), data_.size());
	}

	/*----------------------------------------------------------------------------------------------*/

	static proto_message_helper helper_;

	template<typename ConstBufferSequence>
	static proto_message_wrapper from_const_buffers(ConstBufferSequence const& buffers) {

		proto_message_wrapper wrapper;

		wrapper.vect().reserve(boost::asio::buffer_size(buffers));
		
		for (const auto& buffer : boost::beast::detail::buffers_range(buffers)) {
			DBG("copying buffer to data field");
			std::copy(boost::asio::buffers_begin(buffer), boost::asio::buffers_end(buffer), std::back_inserter(wrapper.vect()));
		}

		wrapper.vect().shrink_to_fit();

		return wrapper;
	}

};