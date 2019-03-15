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

#include <boost/asio.hpp>

#include "../types.h"
#include "io_app_base.h"



namespace o::io {
    
        /** Base class for applications that want to perform io and handle os signals.
         This class provides all features from o::io::io_app_base, but automatically
         handles signals from the operating system. By default it will handle SIGINT and
         SIGQUIT. Both Signals will eventually lead to call to
         o::io::io_app_base::app_allow_exit(int signal_num).
         \snippet ioapp.cpp basic_io_app_ex
         */
        template < typename ThreadOption >
        class basic_io_app : public io_app_base< ThreadOption > {

      public:
        basic_io_app() : signal_set_( this->context(), 2, 15 ) {}

      protected:
        /** Implemented to setup the signal set. If you override this, make sure you call
         * setup_signals() yourself */
        virtual void app_prepare() override { setup_signals(); }

        /** implement to be notified about signals you subscribed to with
         * signals().add(num) */
        virtual void on_signal( int signal_number ) {}

        /** Begin listening to signals. This will be called from the app when it is
         * started. */
        void setup_signals() {
            signal_set_.async_wait(
                [this]( const boost::system::error_code& ec, int signal_number ) {
                    if ( !ec ) {
                        if ( signal_number == 2 || signal_number == 15 )
                            this->app_allow_exit( signal_number );
                        else {
                            on_signal( signal_number );
                            setup_signals();
                        }
                    }
                } );
        }

        /** Access the underlying boost::asio::signal_set */
        boost::asio::signal_set& signals() { return signal_set_; }

      private:
        boost::asio::signal_set signal_set_;
    };
}
