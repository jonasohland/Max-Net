#pragma once

#include "../../build/source/websocketclient/generic_max.pb.h"
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <google/protobuf/message.h>



template<typename ProtoMessage>
class proto_message_base {
public:

    proto_message_base(){
        mess_ = new ProtoMessage();
    }
    
    virtual ~proto_message_base(){
        delete mess_;
    }
    
    template<typename ConstBufferSequence>
    static void from_const_buffers(ConstBufferSequence buffers, proto_message_base* msg){

		msg->data_.reserve(boost::asio::buffer_size(buffers));

		for (const auto& buffer : boost::beast::detail::buffers_range(buffers)) {
			std::copy(boost::asio::buffers_begin(buffer), boost::asio::buffers_end(buffer), std::back_inserter(msg->data_));
		}

    }

	std::string& vect() {
		return data_;
	}

	ProtoMessage*& proto() {
		return mess_;
	}

	ProtoMessage* const& proto() const {
		return mess_;
	}
    
    const std::string::value_type* data() const {
        return data_.data();
    }
    
    size_t size() const {
        return data_.size();
    }

	bool serialize() {
		data_.reserve(mess_->ByteSizeLong());
		return mess_->SerializeToString(&data_);
	}

	bool deserialize() {
		return mess_->ParsePartialFromArray(data_.data(), (int) data_.size());
	}
    
    
    
    
private:
    ProtoMessage* mess_;
    std::string data_;
    
    
};

class basic_proto_message {

public:

	basic_proto_message() {
		
	}

	virtual ~basic_proto_message() {

	}

	template<typename ConstBufferSequence>
	static void from_const_buffers(ConstBufferSequence buffers, basic_proto_message* msg) {

		msg->data_.reserve(boost::asio::buffer_size(buffers));

		for (const auto& buffer : boost::beast::detail::buffers_range(buffers)) {
			std::copy(boost::asio::buffers_begin(buffer), boost::asio::buffers_end(buffer), std::back_inserter(msg->data_));
		}

	}

	std::string& vect() {
		return data_;
	}

	google::protobuf::Message* proto() {
		return mess_;
	}

	const google::protobuf::Message* proto() const {
		return mess_;
	}

	const void* data() const {
		return data_.data();
	}

	size_t size() const {
		return data_.size();
	}

	bool serialize() {

		data_.reserve(mess_->ByteSizeLong());
		return mess_->SerializeToString(&data_);
	}

	bool deserialize() {
		return mess_->ParseFromString(data_);
	}


private:
	google::protobuf::Message* mess_;
	std::string data_;

};
