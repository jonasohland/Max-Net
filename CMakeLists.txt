cmake_minimum_required(VERSION 3.0)


string(REGEX REPLACE "(.*)/" "" THIS_PACKAGE_NAME "${CMAKE_CURRENT_SOURCE_DIR}")


if (${CMAKE_GENERATOR} MATCHES "Xcode")
 		if (${XCODE_VERSION} VERSION_LESS 9)
  			message(STATUS "Xcode 9 or higher is required. Please install from the Mac App Store.")
			return ()
		endif ()
endif ()


message(STATUS "Updating Git Submodules")


execute_process(
	COMMAND				git submodule update --init --recursive
	WORKING_DIRECTORY	"${CMAKE_CURRENT_SOURCE_DIR}"
)

set(PROTO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/source/shared/proto/")

message(STATUS "Generating proto classes")

file(GLOB PROTOS "${CMAKE_CURRENT_SOURCE_DIR}/source/shared/proto/*.proto")


foreach(PROTO_FILE ${PROTOS})

	message(STATUS "Found: ${PROTO_FILE}")

	execute_process(
		COMMAND "protoc" "--proto_path=${PROTO_PATH}" "--cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/source/shared/proto/generated"  "${PROTO_FILE}"
		WORKING_DIRECTORY	"${CMAKE_CURRENT_SOURCE_DIR}"
	)

endforeach()



# Misc setup and subroutines
include(${CMAKE_CURRENT_SOURCE_DIR}/lib/min-api/script/min-package.cmake)


# Add unit tests for the API
# enable_testing()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/min-api)


# Add the Lib, if it exists
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/min-lib/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/min-lib)
endif ()


# Generate a project for every folder in the "source/projects" folder
SUBDIRLIST(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source)
foreach (project_dir ${PROJECT_DIRS})
	if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source/${project_dir}/CMakeLists.txt")
		message("Generating: ${project_dir}")
		add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/${project_dir})
	endif ()
endforeach ()

