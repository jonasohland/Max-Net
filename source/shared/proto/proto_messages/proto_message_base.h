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

#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <google/protobuf/message.h>

template < typename ProtoMessage > class proto_message_base {
  public:
    using proto_msg_type = ProtoMessage;

    proto_message_base() { mess_ = new ProtoMessage(); }

    virtual ~proto_message_base() { delete mess_; }

    template < typename ConstBufferSequence >
    static void from_const_buffers(ConstBufferSequence buffers, proto_message_base* msg,
                                   bool text) {

        if (!text) {
            msg->data_.reserve(boost::asio::buffer_size(buffers));

            for (const auto& buffer : boost::beast::detail::buffers_range(buffers)) {
                std::copy(boost::asio::buffers_begin(buffer),
                          boost::asio::buffers_end(buffer),
                          std::back_inserter(msg->data_));
            }
        }
    }

    std::string& vect() { return data_; }

    ProtoMessage*& proto() { return mess_; }

    ProtoMessage* const& const_proto() const { return mess_; }

    const std::string::value_type* data() const { return data_.data(); }

    size_t size() const { return data_.size(); }

    bool serialize() {
        data_.reserve(mess_->ByteSizeLong());
        return mess_->SerializeToString(&data_);
    }

    bool deserialize() {
        return mess_->ParsePartialFromArray(data_.data(), (int)data_.size());
    }

  private:
    ProtoMessage* mess_;
    std::string data_;
};

class basic_proto_message {

  public:
    basic_proto_message() {}

    virtual ~basic_proto_message() {}

    template < typename ConstBufferSequence >
    static void from_const_buffers(ConstBufferSequence buffers, basic_proto_message* msg,
                                   bool text) {

        if (!text) {
            msg->data_.reserve(boost::asio::buffer_size(buffers));

            for (const auto& buffer : boost::beast::detail::buffers_range(buffers)) {
                std::move(boost::asio::buffers_begin(buffer),
                          boost::asio::buffers_end(buffer),
                          std::back_inserter(msg->data_));
            }
        }
    }

    std::string& vect() { return data_; }

    google::protobuf::Message* proto() { return mess_; }

    const google::protobuf::Message* proto() const { return mess_; }

    const void* data() const { return data_.data(); }

    size_t size() const { return data_.size(); }

    bool serialize() {

        data_.reserve(mess_->ByteSizeLong());
        return mess_->SerializeToString(&data_);
    }

    bool deserialize() { return mess_->ParseFromString(data_); }

  private:
    google::protobuf::Message* mess_;
    std::string data_;
};
