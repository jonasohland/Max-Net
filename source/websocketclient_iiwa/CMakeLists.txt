cmake_minimum_required(VERSION 3.0)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/min-api)

if(use_version_tags)

execute_process(COMMAND "git" "describe" "--tags" 
				OUTPUT_VARIABLE VERSION_TAG 
				ERROR_VARIABLE VERSION_TAG
				ERROR_STRIP_TRAILING_WHITESPACE
				OUTPUT_STRIP_TRAILING_WHITESPACE)

else()

set(VERSION_TAG "x.x")

endif()

include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)	

find_package(Boost COMPONENTS 
					"system" 
					"date_time" 
					"regex" 
					REQUIRED)

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

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS YES
)

target_link_libraries(${PROJECT_NAME} PUBLIC 				
						${Boost_LIBRARIES})

target_link_libraries(${PROJECT_NAME} PUBLIC 					
						${Protobuf_LIBRARIES})

add_dependencies(${PROJECT_NAME} shared_protos)

target_link_libraries(${PROJECT_NAME} PUBLIC shared_protos)
target_link_libraries(${PROJECT_NAME} PUBLIC websockets_shared)
target_link_libraries(${PROJECT_NAME} PUBLIC checked_ptr)
target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})

