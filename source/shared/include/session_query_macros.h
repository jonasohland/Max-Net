#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>

#define CON_IS( x ) == session_impl_type::status_codes::x
#define CON_IS_NOT( x ) != session_impl_type::status_codes::x

// will expand to [ status (==)/(!=) ohlano::websocket_connection::STATUS (||)/(&&) ]
#define _OHLANO_EXPAND_STATUS_CHECK_OR( r, data, elem ) status elem ||

#define _OHLANO_EXPAND_STATUS_INCLUDES( r, data, elem )                                  \
    status == session_impl_type::status_codes::elem ||
#define _OHLANO_EXPAND_STATUS_EXCLUDES( r, data, elem )                                  \
    status != session_impl_type::status_codes::elem&&

/* CON_WHERE_STATUS_OR(CON_IS(ONLINE), CON_IS(OFFLINE)) will expand to
 
 [](std::vector<std::shared_ptr<websocket_connection>>::iterator _c) -> bool {
 
    if(!*_c){ return false; }
 
    auto status = (*_c)->status();
 
    if(
        status == ohlano::websocket_connections::status_codes::ONLINE ||    <-- repeats
        status == ohlano::websocket_connections::status_codes::OFFLINE ||
        false
    ) { return true; } else { return false; }
 
 }
 */
#define CON_WHERE_STATUS_OR( ... )                                                       \
    []( session_type& _c ) -> bool {                                                     \
        if ( !_c ) {                                                                     \
            return false;                                                                \
        }                                                                                \
        auto status = _c->status();                                                      \
        if ( BOOST_PP_SEQ_FOR_EACH( _OHLANO_EXPAND_STATUS_CHECK_OR, ,                    \
                                    BOOST_PP_VARIADIC_TO_SEQ( __VA_ARGS__ ) ) false ) {  \
            return true;                                                                 \
        } else {                                                                         \
            return false;                                                                \
        }                                                                                \
    }

#define _OHLANO_EXPAND_STATUS_CHECK_AND( r, data, elem ) status elem&&

#define CON_WHERE_STATUS( ... )                                                          \
    []( session_type& _c ) -> bool {                                                     \
        if ( !_c ) {                                                                     \
            return false;                                                                \
        }                                                                                \
        auto status = _c->status();                                                      \
        if ( BOOST_PP_SEQ_FOR_EACH( _OHLANO_EXPAND_STATUS_CHECK_AND, ,                   \
                                    BOOST_PP_VARIADIC_TO_SEQ( __VA_ARGS__ ) ) true ) {   \
            return true;                                                                 \
        } else {                                                                         \
            return false;                                                                \
        }                                                                                \
    }

#define CON_WHERE_STATUS_INCLUDES( ... )                                                 \
    []( session_type& _c ) -> bool {                                                     \
        if ( !_c ) {                                                                     \
            return false;                                                                \
        }                                                                                \
        auto status = _c->status();                                                      \
        if ( BOOST_PP_SEQ_FOR_EACH( _OHLANO_EXPAND_STATUS_INCLUDES, ,                    \
                                    BOOST_PP_VARIADIC_TO_SEQ( __VA_ARGS__ ) ) false ) {  \
            return true;                                                                 \
        } else {                                                                         \
            return false;                                                                \
        }                                                                                \
    }

#define CON_WHERE_STATUS_EXCLUDES( ... )                                                 \
    []( session_type& _c ) -> bool {                                                     \
        if ( !_c ) {                                                                     \
            return false;                                                                \
        }                                                                                \
        auto status = _c->status();                                                      \
        if ( BOOST_PP_SEQ_FOR_EACH( _OHLANO_EXPAND_STATUS_EXCLUDES, ,                    \
                                    BOOST_PP_VARIADIC_TO_SEQ( __VA_ARGS__ ) ) true ) {   \
            return true;                                                                 \
        } else {                                                                         \
            return false;                                                                \
        }                                                                                \
    }

#define CON_STATUS_IS( _STATUS )                                                         \
    []( session_type& _c ) -> bool {                                                     \
        if ( _c && ( _c )->status() == websocket_connection::status_t::_STATUS )         \
            return true;                                                                 \
        else                                                                             \
            return false;                                                                \
    }

#define OHLANO_WS_ALL_ALLOCATED                                                          \
    []( session_type& _c ) {                                                             \
        if ( _c )                                                                        \
            return true;                                                                 \
        else                                                                             \
            return false;                                                                \
    }

#define OHLANO_WS_ALL []( session_type& _c ) { return true; }
