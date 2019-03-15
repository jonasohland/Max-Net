cmake_minimum_required(VERSION 3.3)

file(GLOB_RECURSE IO_HEADERS "include/*.h")


set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)	

find_package(Boost COMPONENTS 
					"system" 
					"date_time" 
					"regex" 
					REQUIRED)

message(STATUS "shared code")
message(STATUS ${IO_HEADERS})
message(STATUS ${CMAKE_GENERATOR})

if ((${CMAKE_GENERATOR} MATCHES "Xcode") OR (${CMAKE_GENERATOR} MATCHES "Visual Studio 15 2017 Win64"))

	source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "o Headers" FILES ${IO_HEADERS})

	add_library(websockets_shared_source STATIC 
		${IO_HEADERS}
	)

	set_target_properties(websockets_shared_source PROPERTIES LINKER_LANGUAGE CXX)

	set_target_properties(websockets_shared_source PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS YES
	)

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
