macro(ohlib_setup input_target)
    
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)	

find_package(Boost COMPONENTS 
					"system" 
					"date_time" 
					"regex" 
                    REQUIRED)
                    
                    
target_link_libraries(${input_target} PUBLIC ${Boost_LIBRARIES})

if(WIN32)
	target_compile_options(${input_target} PUBLIC "/bigobj")
	target_compile_definitions(${input_target} PRIVATE _WIN32_WINNT=0x0A00)				
endif()		

endmacro(ohlib_setup)
