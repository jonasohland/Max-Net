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



#define OHLANO_NODEFAULT(class) class() = delete;


#include <iostream>
#include <utility>
#include <sstream>



namespace ohlano{

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

#ifdef DEBUG
#define DBG(...) ohlano::log(__VA_ARGS__);
#else
#define DBG(...) ;
#endif

#ifdef DEBUG
#define LOG(...) std::cout << __FILE__ << " " << __LINE__ << ": "; ohlano::log(__VA_ARGS__);
#else
#define LOG(...) ;
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
