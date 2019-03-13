//
// This file is part of the Max Network Extensions Project
//
// Copyright (c) 2019, Jonas Ohland
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <google/protobuf/message.h>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>

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
    
    std::vector<char> data_;
	google::protobuf::Message* mess_ = nullptr;
	

public:

	proto_message_wrapper(proto_message_wrapper&& other) :
		data_(std::forward<std::vector<char>>(other.data_)),
		mess_(std::forward<google::protobuf::Message*>(other.mess_))
    
	{}

	proto_message_wrapper(const proto_message_wrapper& other):
		data_(other.data_), mess_(other.mess_)
	{}

	proto_message_wrapper* operator=(const proto_message_wrapper& other)
    { data_ = other.data_; mess_ = other.mess_; return this; }

	proto_message_wrapper(const std::string& str): data_(str.begin(), str.end()) {}

	proto_message_wrapper(){}

	~proto_message_wrapper() {}

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
	static proto_message_wrapper* from_const_buffers(ConstBufferSequence const& buffers) {

		proto_message_wrapper wrapper;

		wrapper.vect().reserve(boost::asio::buffer_size(buffers));
		
		for (const auto& buffer : boost::beast::detail::buffers_range(buffers)) {
			DBG("copying buffer to data field");
			std::copy(boost::asio::buffers_begin(buffer), boost::asio::buffers_end(buffer), std::back_inserter(wrapper.vect()));
		}

		wrapper.vect().shrink_to_fit();

		return nullptr;
	}

};
