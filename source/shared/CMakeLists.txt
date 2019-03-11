cmake_minimum_required(VERSION 3.1)

file(GLOB SHARED_HEADERS "include/*.h")
file(GLOB DEVICE_HEADERS "include/devices/*.h" )
file(GLOB MESSAGES_HEADERS "include/messages/*.h" )

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)	

find_package(Boost COMPONENTS 
					"system" 
					"date_time" 
					"regex" 
					REQUIRED)

message(STATUS "shared code")

message(STATUS ${CMAKE_GENERATOR})

if ((${CMAKE_GENERATOR} MATCHES "Xcode") OR (${CMAKE_GENERATOR} MATCHES "Visual Studio 15 2017 Win64"))

	add_library(websockets_shared_source STATIC 
		${SHARED_HEADERS}
		${DEVICE_HEADERS}
		${MESSAGES_HEADERS}
	)

	set_target_properties(websockets_shared_source PROPERTIES LINKER_LANGUAGE CXX)

	target_include_directories(websockets_shared_source INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

	target_include_directories(websockets_shared_source PUBLIC ${Boost_INCLUDE_DIRS})

	target_include_directories(websockets_shared_source PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../lib/min-api/include")
	target_include_directories(websockets_shared_source PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../lib/min-api/max-api/include")

endif()

add_library(websockets_shared INTERFACE)

target_link_libraries(websockets_shared INTERFACE optional_locks)

target_include_directories(websockets_shared INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_include_directories(websockets_shared INTERFACE ${Boost_INCLUDE_DIRS})

target_include_directories(websockets_shared INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../../lib/min-api/include")
target_include_directories(websockets_shared INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../../lib/min-api/max-api/include")
