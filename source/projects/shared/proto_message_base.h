#pragma once

#include "../../build/source/projects/websocketclient/generic_max.pb.h"


class max_message : public proto_message_base<generic_max> {
    
}

template <typename ProtoMessage>
class proto_message_base {
    
    
public:
    
    proto_message_base(){
        mess_ = new ProtoMessage();
    }
    
    virtual ~proto_message_base(){
        delete mess_;
    }
    
    template<typename ConstBufferSequence>
    static proto_message_base* from_const_buffers(ConstBufferSequence buffers){
        
    }
    
    void* data(){
        return data_.data();
    }
    
    const void* data() const{
        return data_.data();
    }
    
    size_t size() const{
        return data_.size();
    }
    
    
    
    
private:
    ProtoMessage* mess_;
    std::vector<char> data_;
    
    
};
