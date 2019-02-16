#pragma once

#include <string>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include "../types.h"

class bytes_message {

  public:
    template < typename ConstBufferSequence >
    static void from_const_buffers( ConstBufferSequence buffers, bytes_message* msg,
                                    bool text ) {
        if ( !text ) {

            msg->storage().reserve( boost::asio::buffer_size( buffers ) );

            for ( const auto& buffer : boost::beast::detail::buffers_range( buffers ) ) {
                std::copy( boost::asio::buffers_iterator< ConstBufferSequence >
                               ( boost::asio::buffers_begin( buffer ) ),
                           boost::asio::buffers_iterator< ConstBufferSequence >
                               ( boost::asio::buffers_end( buffer ) ),
                           std::back_inserter( msg->data_ ) );
            }
        }
    }

    std::vector< char >& storage() { return data_; };
    const std::vector< char >& storage() const { return data_; };

    void* data() { return static_cast< void* >( data_.data() ); }
    const void* data() const { return static_cast< const void* >( data_.data() ); }

    size_t size() const { return data_.size(); }

  private:
    std::vector< char > data_;
};
