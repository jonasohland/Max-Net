macro(enable_cxx_17 input_target)

    message(STATUS "processing target: ${input_target}")

    set_target_properties(${input_target} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS YES
    )

    target_compile_definitions(${input_target} PRIVATE _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING)
    target_compile_definitions(${input_target} PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
    target_compile_definitions(${input_target} PRIVATE _SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING)
    
# target_compile_definitions(${target} )

endmacro(enable_cxx_17)

macro(ohlib_setup input_target)

if(NOT ${CMAKE_THREAD_LIBS_INIT})
    find_package(Threads)
endif()

if(${CMAKE_THREAD_LIBS_INIT})
	target_link_libraries(${input_target} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif()

if(NOT ${Boost_LIBRARIES})

    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)	

    find_package(Boost COMPONENTS 
                        "system" 
                        "date_time" 
                        "regex" 
                        REQUIRED)

endif()
                    
                    
target_link_libraries(${input_target} PUBLIC ${Boost_LIBRARIES})

if(WIN32)
	target_compile_options(${input_target} PUBLIC "/bigobj")
	target_compile_definitions(${input_target} PRIVATE _WIN32_WINNT=0x0A00)				
endif()		

target_link_libraries(${input_target} PUBLIC ohlib_include)

enable_cxx_17(${input_target})

endmacro(ohlib_setup)

macro(get_version_tag)

    if(use_version_tags)

        execute_process(COMMAND "git" "describe" "--tags" 
                        OUTPUT_VARIABLE VERSION_TAG 
                        ERROR_VARIABLE VERSION_TAG
                        ERROR_STRIP_TRAILING_WHITESPACE
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
                    
    else()

        set(VERSION_TAG "x.x")

    endif()

endmacro(get_version_tag)

macro(version_tag input_target) 

    if(NOT ${VERSION_TAG})
        get_version_tag()
    endif()

    target_compile_definitions(${input_target} PRIVATE VERSION_TAG=${VERSION_TAG})

endmacro(version_tag)