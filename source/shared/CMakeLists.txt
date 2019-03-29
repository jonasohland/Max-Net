cmake_minimum_required(VERSION 3.3)

file(GLOB_RECURSE IO_HEADERS "include/*.h")


set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)	

find_package(Boost COMPONENTS 
					"system" 
					"date_time" 
					"regex" 
					REQUIRED)

if ((${CMAKE_GENERATOR} MATCHES "Xcode") OR (${CMAKE_GENERATOR} MATCHES "Visual Studio 15 2017 Win64"))

	source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "o_legacy_headers" FILES ${IO_HEADERS})

	add_library(o_legacy_headers STATIC 
		${IO_HEADERS}
	)

	set_target_properties(o_legacy_headers PROPERTIES LINKER_LANGUAGE CXX)

	set_target_properties(o_legacy_headers PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS YES
	)

	target_include_directories(o_legacy_headers INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

	target_include_directories(o_legacy_headers PUBLIC ${Boost_INCLUDE_DIRS})

	target_include_directories(o_legacy_headers PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../lib/min-api/include")
	target_include_directories(o_legacy_headers PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../lib/min-api/max-api/include")

endif()

add_library(o_legacy_include INTERFACE)

target_link_libraries(o_legacy_include INTERFACE optional_locks)

target_include_directories(o_legacy_include INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_include_directories(o_legacy_include INTERFACE ${Boost_INCLUDE_DIRS})

target_include_directories(o_legacy_include INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../../lib/min-api/include")
target_include_directories(o_legacy_include INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../../lib/min-api/max-api/include")
