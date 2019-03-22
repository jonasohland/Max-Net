cmake_minimum_required(VERSION 3.1)

find_package(Protobuf REQUIRED)	

file(GLOB PROTO_SOURCES "generated/*.cc")
file(GLOB PROTO_HEADERS "generated/*.h")

file(GLOB PROTO_MESSAGES "proto_messages/*.h")

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/generated")


add_library(shared_protos STATIC ${PROTO_SOURCES} ${PROTO_HEADERS} ${PROTO_MESSAGES})

set_property(TARGET shared_protos PROPERTY CXX_STANDARD 14)

target_include_directories(shared_protos PUBLIC ${Protobuf_INCLUDE_DIRS})

target_include_directories(shared_protos INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
