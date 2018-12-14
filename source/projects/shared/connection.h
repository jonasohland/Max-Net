#pragma once
#include "net_url.h"
#include "ohlano.h"
#include <cassert>


template<typename ConnectionType>
class connection{

public:
    connection() = default;

    explicit connection(net_url<> url){
        assert(url.valid());

        if(url.is_ip()){
            DBG("is ip");
        } else {
            DBG("is not ip");
        }


    }
};
