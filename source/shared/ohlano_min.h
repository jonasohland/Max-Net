#pragma once

#include <functional>
#include <string>
#include <sstream>

#define min_wrap_member( x )                                                             \
    ohlano::make_func< c74::min::atoms( const c74::min::atoms&, int ) >(                 \
        std::bind( x, this, std::placeholders::_1, std::placeholders::_2 ) )

namespace ohlano {

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
}
