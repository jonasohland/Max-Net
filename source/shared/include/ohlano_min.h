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

#include <functional>
#include <sstream>
#include <string>

#define min_wrap_member( x )                                                        \
    o::make_func< c74::min::atoms( const c74::min::atoms&, int ) >(                 \
        std::bind( x, this, std::placeholders::_1, std::placeholders::_2 ) )

namespace o {

    class console_stream_adapter_endl_marker {};

    static console_stream_adapter_endl_marker endl;

    class console_stream_adapter {
      public:
        console_stream_adapter() = delete;

        typedef std::function< void( std::string ) > handler_function_type;

        console_stream_adapter( handler_function_type handler,
                                bool _space_separated = false,
                                std::string _separator_char = std::string( "" ) ) {
            separator_char = _separator_char;
            space_separator = _space_separated;
            string_handler = handler;
        }

        console_stream_adapter( const console_stream_adapter& other ) {
            string_handler = other.string_handler;
            separator_char = other.separator_char;
            space_separator = other.space_separator;
        }

        console_stream_adapter* operator=( const console_stream_adapter& other ) {
            string_handler = other.string_handler;
            separator_char = other.separator_char;
            space_separator = other.space_separator;
            return this;
        }

        bool has_sep_char() { return separator_char.size() > 0; }

        bool space_separated() { return space_separator; }

        template < typename T >
        console_stream_adapter& operator<<( T&& input ) {

            sstr << std::forward< T >( input );

            if ( space_separated() )
                sstr << " ";

            if ( has_sep_char() ) {
                sstr << separator_char;
                if ( space_separated() )
                    sstr << " ";
            }

            return *this;
        }

        console_stream_adapter& operator<<( console_stream_adapter_endl_marker x ) {

            std::string out = sstr.str();

            if ( space_separated() ) {
                out.pop_back();
            }
            if ( has_sep_char() ) {
                out.pop_back();
            }

            string_handler( out );

            sstr.str( std::string() );
            return *this;
        }

        template < typename T >
        void operator()( T&& thing_to_post ) {
            *this << thing_to_post << endl;
        }

        template < typename C, typename... T >
        void operator()( C&& current, T&&... rest ) {
            *this << std::forward< C >( current );
            ( *this )( rest... );
        }

      private:
        handler_function_type string_handler;

        std::stringstream sstr;
        std::string separator_char;
        bool space_separator;
    };

    // static console_stream_adapter::endl_type endl =
    // console_stream_adapter::endl_type();

    template < typename F, typename T >
    std::enable_if_t< std::is_bind_expression< T >::value, std::function< F > >
    make_func( T&& bind_expr ) {
        return std::function< F >( std::forward< T >( bind_expr ) );
    }

    /// convert an atom vector to a std::array of any type
    template < typename T, size_t Size >
    std::array< T, Size > array_from_atoms( const c74::min::atoms& args ) {

        std::array< T, Size > out;

        if ( args.size() < Size ) {
            throw std::runtime_error( "not enough arguments provided" );
        }

        for ( size_t i = 0; i < Size; ++i ) {
            out[i] = args[i].get< double >();
        }

        return out;
    }

} // namespace o
