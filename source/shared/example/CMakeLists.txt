include("${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/ohlib.cmake")


add_executable(TimerExample timer.cpp)

ohlib_setup(TimerExample)
