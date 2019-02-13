#pragma once

#include <boost/tti/has_type.hpp>
#include <boost/mpl/lambda.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/type_traits/is_same.hpp>

namespace ohlano {
    
    namespace sessions {
        
        namespace roles {
            struct server {};
            struct client {};
        }
        
        namespace features {
            struct timeout {};
        }
    
        template<typename Role, typename T = void>
        struct enable_for_client {};
        
        template<typename Ty>
        struct enable_for_client<roles::client, Ty> {
            using type = Ty;
        };
        
        template<typename Role, typename T = void>
        struct enable_for_server {};
        
        template<typename Ty>
        struct enable_for_server<roles::server, Ty> {
            using type = Ty;
        };
        
    }
    
    namespace threads {
        
        // used to indicate that the object will be used in a single threaded context
        struct single{};
        
        // obvious
        struct multi{};
        
        template<typename T>
        struct opt_is_multi : public std::false_type {};
        
        template<>
        struct opt_is_multi<threads::multi> : public std::true_type {};
        
        template<class Op, class Ty = void>
        struct opt_enable_if_multi_thread {};
        
        template<class Ty>
        struct opt_enable_if_multi_thread<threads::multi, Ty> {
            using type = Ty;
        };
        
        template<class Op, class Ty = void>
        struct opt_enable_if_single_thread {};
        
        template<class Ty>
        struct opt_enable_if_single_thread<threads::single, Ty> {
            using type = Ty;
        };
        
        BOOST_TTI_HAS_TYPE(thread_option);
        
        template<typename T>
        struct is_multi_thread_enabled {
          static constexpr const bool value = has_type_thread_option<
              T, boost::is_same<boost::mpl::placeholders::_1, multi>>::value;
        };
        
        template<typename T, typename Ty = void>
        using enable_if_multi_thread_enabled = typename std::enable_if<is_multi_thread_enabled<T>::value, Ty>;
        
        template<typename T, typename Ty = void>
        using enable_if_multi_thread_enabled_t = typename enable_if_multi_thread_enabled<T, Ty>::type;
    
    }
    
    namespace utility {
        
    }
};
