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
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#if defined(O_NET_POSIX) || defined(DOXY_GENERATE)

#include "../types.h"
#include <boost/asio.hpp>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

namespace o::io {

    /** An io application can inherit from this class if it wants to read data
     from stdin \code{.cpp} using app_base =
     o::io::simple_io_app<o::threads::none>;

     struct app : public app_base, public o::io::istream_listener {

     app() : o::io::istream_listener(this->context())
     {}

     virtual void on_console_input(std::string str) override {
     std::cout << "You entered: " << str << std::endl;
     }
     };
     \endcode
     */
    class istream_listener {

      public:
        istream_listener() = delete;

        /** construct the stream listener and give it a context to operate on */
        explicit istream_listener(boost::asio::io_context& ctx)
            : std_istream_desc_(ctx, ::dup(STDIN_FILENO)) {
            do_read();
        }

        /** implement this function to get notified if a complete line was read
         * from stdin */
        virtual void on_console_input(std::string) = 0;

      private:
        void handle_istream_read(const boost::system::error_code& ec,
                                 std::size_t length) {

            if (!ec) {

                on_console_input(std::string(
                    boost::asio::buffers_begin(input_buffer_.data()),
                    boost::asio::buffers_begin(input_buffer_.data()) +
                        input_buffer_.size() - 1));
                input_buffer_.consume(input_buffer_.size());

                do_read();
            }
        }

        void do_read() {
            boost::asio::async_read_until(
                std_istream_desc_, input_buffer_, '\n',
                std::bind(&istream_listener::handle_istream_read, this,
                          std::placeholders::_1, std::placeholders::_2));
        }

        boost::asio::streambuf input_buffer_;
        boost::asio::posix::stream_descriptor std_istream_desc_;
    };
} // namespace o::io
#endif
