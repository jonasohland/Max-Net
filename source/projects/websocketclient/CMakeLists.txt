#------------------------------------------PRE----------------------------------------------#

cmake_minimum_required(VERSION 3.0)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../min-api) 	# C74 support dirs

include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)			# C74 api build scripts

# set(BOOST_ROOT "C:/boost")
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)									# BOOST Module settings

find_package(Boost COMPONENTS "date_time" REQUIRED)				# find Boost

find_package(Protobuf REQUIRED)									# find Protobuf


#----------------------------------------INCLUDES--------------------------------------------#


include_directories("${C74_INCLUDES}")							# C74 Headers

include_directories(${Protobuf_INCLUDE_DIRS})					# Protobuf Headers
include_directories(${Boost_INCLUDE_DIRS})						# Boost Headers

#-------------------------------------SOURCE / TARGET----------------------------------------#


set( SOURCE_FILES												# Project source files 
	${PROJECT_NAME}.cpp
	BeastSession.cpp
	BeastSession.h
	WebSocketClientSession.cpp
	WebSocketClientSession.h
	WebSocketUrl.h
	iiwaPosition.pb.cc
	iiwaPosition.pb.h
	../shared/ohlano_min.h
	../shared/ohlano.h
)

add_library( 													# target config
	${PROJECT_NAME} 
	MODULE
	${SOURCE_FILES}
)

if(WIN32)
	add_compile_definitions(_WIN32_WINNT=0x0A00)					# definition for Windows.h 
endif()															# (Makes Boost.Asio happy)

																

#------------------------------------------LINKER--------------------------------------------#


include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)			#C74 post build tasks

target_link_libraries(${PROJECT_NAME} PUBLIC 					# link boost libs
						${Boost_LIBRARIES})

target_link_libraries(${PROJECT_NAME} PRIVATE 					# link google Protobuf libs
						${Protobuf_LIBRARIES})


#-------------------------------------------POST---------------------------------------------#

