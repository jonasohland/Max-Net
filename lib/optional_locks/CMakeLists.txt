cmake_minimum_required(VERSION 3.1)

add_library(optional_locks INTERFACE)

target_include_directories(optional_locks INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

if ((${CMAKE_GENERATOR} MATCHES "Xcode") OR (${CMAKE_GENERATOR} MATCHES "Visual Studio 15 2017 Win64"))

    add_library(optional_locks_source STATIC 
        "${CMAKE_CURRENT_SOURCE_DIR}/include/optional_locks.h"
    )

    set_target_properties(optional_locks_source PROPERTIES LINKER_LANGUAGE CXX)

endif()