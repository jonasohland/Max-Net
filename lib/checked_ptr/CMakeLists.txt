cmake_minimum_required(VERSION 3.1)

add_library(checked_ptr INTERFACE)

target_include_directories(checked_ptr INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

if ((${CMAKE_GENERATOR} MATCHES "Xcode") OR (${CMAKE_GENERATOR} MATCHES "Visual Studio 15 2017 Win64"))

    add_library(checked_ptr_source STATIC "${CMAKE_CURRENT_SOURCE_DIR}/include/checked_ptr.h")

    set_target_properties(checked_ptr_source PROPERTIES LINKER_LANGUAGE CXX)

endif()