#pragma once

#include "c74_min.h"
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include "../../../build/source/projects/websocketclient/generic_max.pb.h"

namespace ohlano {
    
    template<typename Proto>
	class basic_proto_message {
        
	public:
        
        basic_proto_message()
        {}

        basic_proto_message(basic_proto_message&& other): msg_(std::forward<Proto>(other.msg_))
        {}
        
        basic_proto_message(const basic_proto_message& other): msg_(other.msg_)
        {}
        
        Proto& msg() {
            return msg_;
        }

		template<class ConstBufferSequence>
		basic_proto_message(ConstBufferSequence const& seq) {
			msg().ParseFromArray(static_cast<void*>(boost::asio::buffers_begin(seq)), boost::asio::buffer_size(seq));
		}

		template<typename ConstBufferSequence>
		static basic_proto_message from_buffer_sequence(ConstBufferSequence const& buffers) {
			return basic_proto_message(buffers);
		}
        
        void* data(){
            return nullptr;
        }
        
        size_t size(){
            return 0;
        }
        
        virtual ~basic_proto_message(){
        }
        
    protected:
        Proto msg_;
	};
    
    class generic_max_message : public basic_proto_message<generic_max> {
        
    public:
        
        generic_max_message(){}
        
        generic_max_message(const c74::min::atoms& _atoms){
            
        }
        
    };
}
