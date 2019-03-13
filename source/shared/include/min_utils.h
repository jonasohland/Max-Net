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

#include "c74_min.h"
#include "net_url.h"

#define O_CREATE_DEFERRED_CALL( f )                                                      \
    [this]( const c74::min::atoms& args, int inlet ) -> c74::min::atoms {                \
        this->post( [=]() { f( args, inlet ); } );                                       \
        return args;                                                                     \
    }

net_url<> net_url_from_atoms( const c74::min::atoms args ) {

    net_url<>::error_code ec;
    net_url<> url;
    net_url<> t_url;

    if ( args.size() > 0 ) {

        for ( auto& arg : args ) {

            switch ( arg.a_type ) {
            case c74::max::e_max_atomtypes::A_SYM:

                if ( !url ) {

                    t_url = net_url<>( arg, ec );

                    if ( ec != net_url<>::error_code::SUCCESS )
                        // cerr << "symbol argument could not be decoded to an url" <<
                        // endl;

                        if ( url.has_port() && t_url.has_port() ) {
                            // cerr << "Found multiple port arguments!" << endl;
                        }
                    url = t_url;
                }

                break;

            case c74::max::e_max_atomtypes::A_FLOAT:
                // cerr << "float not supported as argument" << endl;
                break;
            case c74::max::e_max_atomtypes::A_LONG:
                // cout << "long arg: " << std::string(arg) << endl;
                if ( !url.has_port() ) {
                    url.set_port( std::to_string( static_cast< int >( arg ) ) );
                }
                break;
            default:
                // cerr << "unsupported argument type" << endl;
                break;
            }
        }
    }

    return url;
}
