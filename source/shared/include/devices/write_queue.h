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
/*
#include <deque>
#include <mutex>
#include <chrono>
#include <type_traits>
#include "../o.h"
#include <boost/asio/post.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/optional.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core/type_traits.hpp>
#include <boost/system/error_code.hpp>

namespace o {

template<typename Message, typename Stream, typename Enable = void>
class write_queue;

template<typename Message, typename Stream>
    class write_queue<Message, Stream, typename std::enable_if<boost::beast::has_get_executor<Stream>::value>::type> : public std::enable_shared_from_this<write_queue<Message, Stream>> {

public:
        
    typedef std::function<void(const Message*)> sent_handler_type;

	write_queue() = delete;
    
    write_queue(Stream* stream, typename Message::factory* allocator): exec_(stream->get_executor()), strand_(exec_.context()) {
		stream_ = stream;
		alloc_ = allocator;
	}

	void submit(const Message* msg) {

		std::unique_lock<std::mutex> lock{ queue_mtx_ };

		msg_queue.push_back(msg);
        
		if (msg_queue.size() < 2) {
			perform_write(msg);
        } else {
        }
	}

	void purge() {
		std::unique_lock<std::mutex> lock{queue_mtx_};
		msg_queue.clear();
	}
    
    void attach_sent_handler(sent_handler_type handler){
		snt_handler_ = boost::make_optional(handler);
    }

	void binary(bool txt) {
		stream_->binary(txt);
	}
        
    std::mutex& mtx(){
        return write_mtx_;
    }


private:

	void perform_write(const Message* msg) {
        
        
        auto self = this->shared_from_this();
        

        boost::asio::post(strand_, [self, msg](){
            
            std::lock_guard<std::mutex> write_lock{ self->write_mtx_ };
            
            self->stream_->async_write(
                 boost::asio::buffer(msg->data(), msg->size()),
                 boost::asio::bind_executor(
                        self->strand_,
                        std::bind(&write_queue::write_complete_handler,
                              self,
                              std::placeholders::_1,
                              std::placeholders::_2,
                              msg
                              )
                        )
                 );
        });
        
	}


	void write_complete_handler(boost::system::error_code ec, std::size_t bytes, const Message* msg) {
        
        std::unique_lock<std::mutex> lock{ queue_mtx_ };

        msg_queue.pop_front();

		auto self = this->shared_from_this();

		if (snt_handler_ != boost::none) {
			boost::asio::post(self->exec_, [msg, self]() {
				self->snt_handler_.value()(msg);
			});
		}
		else {
			boost::asio::post(self->exec_, [msg, self]() { self->alloc_->deallocate(msg); });
		}
        
        if (msg_queue.size() > 0) {
            
            boost::asio::post(self->exec_, [self](){
                
                if(self->stream_->is_open()){
                    
                    self->perform_write(self->msg_queue.front());
                    
                } else {
                    
                    while(self->msg_queue.size() > 0){
                        
                        if (self->snt_handler_ != boost::none) {
                            
                            self->snt_handler_.value()(self->msg_queue.front());
                            self->msg_queue.pop_front();
                            
                        }
                        else {
                            self->alloc_->deallocate(self->msg_queue.front());
                            self->msg_queue.pop_front();
                        }
                    }
                }
            });
        }

	}

	std::mutex queue_mtx_;
    std::mutex write_mtx_;

	Stream* stream_;
	typename Message::factory* alloc_;
	boost::asio::io_context::executor_type exec_;
    boost::asio::io_context::strand strand_;
    boost::asio::steady_timer wait_timer{exec_.context()};
        
	boost::optional<sent_handler_type> snt_handler_;

    std::deque<const Message*> msg_queue;
    std::atomic<bool> has_handler_;

};
}

*/
