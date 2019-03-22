
macro(ohlib_setup input_target)
    

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)	

find_package(Boost COMPONENTS 
					"system" 
					"date_time" 
					"regex" 
                    REQUIRED)
                    
                    
target_link_libraries(${input_target} PUBLIC ${Boost_LIBRARIES})

endmacro(ohlib_setup)
