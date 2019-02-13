//
//  ohlano.h
//  websocketclient
//
//  Created by Jonas Ohland on 04.11.18.
//

#ifndef ohlano_h
#define ohlano_h

#define DEBUG

#define D(s) std::cout << std::string(s)
#define el() std::cout << std::endl
#define S(n) std::to_string(n)

#define OHLANO_NOCOPY(class) \
class (const class&) = delete;\
class& operator=(const class&) = delete;

#define OSTREAM(class, stream, class_ref) \
friend std::ostream& operator<<(std::ostream& stream, const class& class_ref)

#define OHLANO_COPY_CONSTRUCT(class)\
class(const class& other)

#define OHLANO_COPY_ASSIGN(class)\
class* operator=(const class& other)

#define OHLANO_RANGE(x) x.begin(), x.end()
#define OHLANO_CRANGE(x) x.cbegin(), x.cend()

#define OHLANO_IT_RANGE(I, C) auto I = C.begin(); I != C.end(); ++I

#define OHLANO_FOREACH_DO(SEQ, PRED, ACTION) auto _p = PRED; auto _a = ACTION; \
for(auto it = SEQ.begin(); it != SEQ.end(); it++){ \
if(_p(*it)){ \
_a(it);\
}}

#define OHLANO_FOREACH(SEQ, ACTION) auto _a = ACTION; for(auto& elem : SEQ){ _a(elem);  }

#define STR_IMPL_(x) #x
#define STR(x) STR_IMPL_(x)


#define OHLANO_NODEFAULT(class) class() = delete;


#include <iostream>
#include <utility>
#include <sstream>



namespace ohlano {
    
    using void_t = void;

#ifdef _MSC_VER


    template<typename T>
    void log(T thing){

        std::stringstream sstream;

        sstream << std::forward<T>(thing) << std::endl;

        OutputDebugStringA(sstream.str().c_str());
    }
    
    template<typename C, typename ...T>
    void log(C && current, T && ...rest) {

        std::stringstream sstream;
        sstream << std::forward<C>(current);
        OutputDebugStringA(sstream.str().c_str());

        log(std::forward<T>(rest)...);
    }
#else
    template<typename T>
    void log(T thing) {
        std::cout << std::forward<T>(thing) << std::endl;
    }

    template<typename C, typename ...T>
    void log(C && current, T && ...rest) {
        std::cout << std::forward<C>(current);
        log(std::forward<T>(rest)...);
    }
#endif
}

#ifdef _MSC_VER
#ifdef _DEBUG
#define DBG(...) ohlano::log(__VA_ARGS__);
#define CONFIG_TAG d
#else
#define DBG(...) ;
#define CONFIG_TAG r
#endif

#define OS_TAG win

#ifdef _DEBUG
#define LOG(...) std::cout << __FILE__ << " " << __LINE__ << ": "; ohlano::log(__VA_ARGS__);
#else
#define LOG(...) ;
#endif
#else
#ifndef NDEBUG
#define DBG(...) ohlano::log(__VA_ARGS__);
#define CONFIG_TAG d
#else
#define DBG(...) ;
#define CONFIG_TAG r
#endif

#define OS_TAG osx

#ifndef NDEBUG
#define LOG(...) std::cout << __FILE__ << " " << __LINE__ << ": "; ohlano::log(__VA_ARGS__);
#else
#define LOG(...) ;
#endif
#endif



#define DISABLE_DBG \
    _Pragma("push_macro(\"DBG\")")


#define ENABLE_DBG \
    _Pragma("pop_macro(\"DBG\")")


#define DISABLE_LOG \
    _Pragma("push_macro(\"LOG\")")


#define ENABLE_LOG \
    _Pragma("pop_macro(\"LOG\")")


#endif /* ohlano_h */
