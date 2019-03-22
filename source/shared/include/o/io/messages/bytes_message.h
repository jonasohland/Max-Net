//
// This file is part of the Max-Net Project
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
#include <boost/beast.hpp>
#include <string>

namespace o::io::messages {

    class bytes_message {

        class alloc {

          public:
            bytes_message* allocate() { return new bytes_message(); }

            void deallocate( const bytes_message* msg ) { delete msg; }
        };

      public:
        using factory = alloc;

        template < typename ConstBufferSequence >
        static void from_const_buffers( ConstBufferSequence buffers, bytes_message* msg,
                                        bool text ) {
            if ( !text ) {

                msg->storage().reserve( boost::asio::buffer_size( buffers ) );

                for ( const auto& buffer :
                      boost::beast::detail::buffers_range( buffers ) ) {
                    std::copy( boost::asio::buffers_begin( buffer ),
                               boost::asio::buffers_end( buffer ),
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

} // namespace o::io::messages
