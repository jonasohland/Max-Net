cmake_minimum_required(VERSION 3.0)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/min-api)

include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)
include("../../cmake/cxx17.cmake")
include("../../cmake/ohlib.cmake")
include("../../cmake/version_tag.cmake")

get_version_tag()

find_package(Protobuf REQUIRED)

add_library( 												
	${PROJECT_NAME} 
	MODULE
	${PROJECT_NAME}.cpp
)


if(WIN32)
	target_compile_options(${PROJECT_NAME} PUBLIC "/bigobj")
	add_compile_definitions(_WIN32_WINNT=0x0A00)				
endif()															

target_compile_definitions(${PROJECT_NAME} PUBLIC VERSION_TAG=${VERSION_TAG})

include_directories(${C74_INCLUDES})

include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)

target_link_libraries(${PROJECT_NAME} PUBLIC 					
						${Protobuf_LIBRARIES})

add_dependencies(${PROJECT_NAME} shared_protos)

target_link_libraries(${PROJECT_NAME} PUBLIC shared_protos)
target_link_libraries(${PROJECT_NAME} PUBLIC ohlib_include)
target_link_libraries(${PROJECT_NAME} PUBLIC checked_ptr)
target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})

ohlib_setup(${PROJECT_NAME})
enable_cxx_17(${PROJECT_NAME})