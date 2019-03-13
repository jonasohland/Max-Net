cmake_minimum_required(VERSION 3.0)

option(build_protobuf_targets "build externals that depend on the protobuf library" ON)
option(build_iiwa_targets "build externals for communicating with iiwa robots" OFF)
option(use_version_tags "define version tag macros from git tags" ON)
option(build_docs "Build documentation" OFF)

if((build_iiwa_targets) AND (NOT build_protobuf_targets))
	message(WARNING "iiwa targets cant built without protobuf targets enabled")
endif()


string(REGEX REPLACE "(.*)/" "" THIS_PACKAGE_NAME "${CMAKE_CURRENT_SOURCE_DIR}")


if (${CMAKE_GENERATOR} MATCHES "Xcode")
 		if (${XCODE_VERSION} VERSION_LESS 9)
  			message(STATUS "Xcode 9 or higher is required. Please install from the Mac App Store.")
			return ()
		endif ()
endif ()

if(build_docs)

	find_package(Doxygen)
	if (DOXYGEN_FOUND)

		set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
		set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

		configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
		message(STATUS "Generating Doxygen build Task")

		add_custom_target( doc_doxygen ALL
			COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMENT "Generating API documentation with Doxygen"
			VERBATIM )
	else (DOXYGEN_FOUND)
	message("Doxygen need to be installed to generate the doxygen documentation")
	endif (DOXYGEN_FOUND)

endif(build_docs)


if(build_protobuf_targets)

	set(PROTO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/source/shared/proto/")

	message(STATUS "Generating proto classes")

	file(GLOB PROTOS "${CMAKE_CURRENT_SOURCE_DIR}/source/shared/proto/*.proto")


	foreach(PROTO_FILE ${PROTOS})

		get_filename_component(CURRENT_PROTO_FILE_NAME ${PROTO_FILE} NAME)

		message(STATUS "Found: ${CURRENT_PROTO_FILE_NAME}")

		execute_process(
			COMMAND "protoc" "--proto_path=${PROTO_PATH}" "--cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/source/shared/proto/generated"  "${PROTO_FILE}"
			WORKING_DIRECTORY	"${CMAKE_CURRENT_SOURCE_DIR}"
		)

	endforeach()

	file(GLOB PROTO_SRC "${CMAKE_CURRENT_SOURCE_DIR}/source/shared/proto/generated/*.cc")
	file(GLOB PROTO_HDR "${CMAKE_CURRENT_SOURCE_DIR}/source/shared/proto/generated/*.h")

	foreach(_PROTO_SRC_FILE_PATH ${PROTO_SRC})
		get_filename_component(_PROTO_SRC_FILE ${_PROTO_SRC_FILE_PATH} NAME)
		message(STATUS "generated source: ${_PROTO_SRC_FILE}")
	endforeach()

	foreach(_PROTO_HDR_FILE_PATH ${PROTO_HDR})
		get_filename_component(_PROTO_HDR_FILE ${_PROTO_HDR_FILE_PATH} NAME)
		message(STATUS "generated header: ${_PROTO_HDR_FILE}")
	endforeach()

endif()

# Misc setup and subroutines
include(${CMAKE_CURRENT_SOURCE_DIR}/lib/min-api/script/min-package.cmake)

# Add unit tests for the API
# enable_testing()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/optional_locks)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/min-api)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/shared)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/shared/proto)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/checked_ptr)


if(build_protobuf_targets)
	
	if(build_iiwa_targets)
		add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/source/websocketclient_iiwa")
		add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/source/iiwa_move")
		add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/source/iiwa_joints")
		
	endif()
	
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/source/websocketclient")
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/source/websocketserver")

endif()

add_executable(
    TestApp
    "${CMAKE_CURRENT_SOURCE_DIR}/source/test.cpp"
)

target_link_libraries(TestApp PUBLIC  websockets_shared)

set_target_properties(TestApp PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS YES
)

# Generate a project for every folder in the "source/projects" folder
#SUBDIRLIST(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source)
#foreach (project_dir ${PROJECT_DIRS})
#	if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source/${project_dir}/CMakeLists.txt")
#		message("Generating: ${project_dir}")
#		add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/${project_dir})
#	endif ()
#endforeach ()
