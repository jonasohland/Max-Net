#------------------------------------------PRE----------------------------------------------#

cmake_minimum_required(VERSION 3.0)

execute_process(COMMAND "git" "describe" "--tags" 
				OUTPUT_VARIABLE VERSION_TAG 
				ERROR_VARIABLE VERSION_TAG
				ERROR_STRIP_TRAILING_WHITESPACE
				OUTPUT_STRIP_TRAILING_WHITESPACE)

message(STATUS "GENERATING VERSION: ${VERSION_TAG}")

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/min-api) 	# C74 support dirs

file(GLOB PROTOS "${CMAKE_CURRENT_SOURCE_DIR}/../shared/proto/*.proto")

include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)			# C74 api build scripts

# set(BOOST_ROOT "C:/boost")
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)									# BOOST Module settings

find_package(Boost COMPONENTS 
					"system" 
					"date_time" 
					"regex" 
					REQUIRED)				# find Boost

message(STATUS "Please make sure the header-only libs Asio and Beast are also installed")

find_package(Protobuf REQUIRED)									# find Protobuf



#-------------------------------------SOURCE / TARGET----------------------------------------#


protobuf_generate_cpp(PROTO_SRC PROTO_HDR ${PROTOS})

set( SOURCE_FILES												# Project source files 
	${PROJECT_NAME}.cpp
	${PROTO_SRC}
	../shared/devices/devices.h
	../shared/devices/write_queue.h
	../shared/devices/protobuf_decoder.h
	../shared/devices/protobuf_decoder_worker.h
	../shared/devices/outlet_output_adapter.h
	../shared/devices/listener.h
	../shared/devices/multi_acceptor.h
	../shared/messages/string_message.h
	../shared/messages/proto_message_wrapper.h
	../shared/messages/generic_max_message.h
	../shared/messages/proto_message_base.h
	../shared/net_url.h
	../shared/connection.h
	../shared/ohlano_min.h
	../shared/ohlano.h
)

add_library( 													# target config
	${PROJECT_NAME} 
	MODULE
	${SOURCE_FILES}
)

if(WIN32)
	add_compile_definitions(_WIN32_WINNT=0x0A00)				# definition for Windows.h 
endif()															# (Makes Boost.Asio happy)

target_compile_definitions(${PROJECT_NAME} PUBLIC VERSION_TAG=${VERSION_TAG})

#----------------------------------------INCLUDES--------------------------------------------#




target_include_directories(${PROJECT_NAME} BEFORE PUBLIC ${PROTO_HDR})	

include_directories("${C74_INCLUDES}")							# C74 Headers

include_directories(${Protobuf_INCLUDE_DIRS})					# Protobuf Headers
include_directories(${Boost_INCLUDE_DIRS})						# Boost Headers

#------------------------------------------LINKER--------------------------------------------#


include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)			# C74 post build tasks

target_link_libraries(${PROJECT_NAME} PUBLIC 					# link boost libs
						${Boost_LIBRARIES})

target_link_libraries(${PROJECT_NAME} PUBLIC 					# link google Protobuf libs
						${Protobuf_LIBRARIES})


#-------------------------------------------POST---------------------------------------------#

