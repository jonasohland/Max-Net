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
#include <string>

namespace o::io::messages {
    class string_message {
        std::string mess;

      public:
        string_message( std::string&& mess_in ) { mess = std::move( mess_in ); }

        string_message( const std::string& mess_in ) : mess( mess_in ) {}

        string_message( string_message&& other ) : mess( std::move( other.mess ) ) {}

        string_message( const string_message& other ) : mess( other.mess ) {}

        string_message* operator=( const string_message& other ) {
            mess = other.mess;
            return this;
        }

        bool operator==( const string_message& other ) { return mess == other.mess; }

        bool operator!=( const string_message& other ) { return !( mess == other.mess ); }

        template < typename ConstBufferSequence >
        static string_message from_const_buffers( ConstBufferSequence const& seq ) {

            std::string buf;
            buf.reserve( boost::asio::buffer_size( seq ) );

            for ( auto buffer : boost::beast::detail::buffers_range( seq ) ) {
                buf.append( static_cast< char const* >( buffer.data() ), buffer.size() );
            }

            return string_message( std::move( buf ) );
        }

        std::string str() const { return mess; }

        operator std::string() { return mess; }

        const std::string::value_type* data() const { return mess.data(); }

        const size_t size() const { return mess.size(); }
    };
} // namespace o::io::messages
