# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/jonasohland/hsmainz/max-websockets

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug

# Include any dependencies generated for this target.
include source/projects/websocketclient/CMakeFiles/websocketclient.dir/depend.make

# Include the progress variables for this target.
include source/projects/websocketclient/CMakeFiles/websocketclient.dir/progress.make

# Include the compile flags for this target's objects.
include source/projects/websocketclient/CMakeFiles/websocketclient.dir/flags.make

source/projects/websocketclient/CMakeFiles/websocketclient.dir/websocketclient.cpp.o: source/projects/websocketclient/CMakeFiles/websocketclient.dir/flags.make
source/projects/websocketclient/CMakeFiles/websocketclient.dir/websocketclient.cpp.o: ../source/projects/websocketclient/websocketclient.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object source/projects/websocketclient/CMakeFiles/websocketclient.dir/websocketclient.cpp.o"
	cd /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/websocketclient.dir/websocketclient.cpp.o -c /Users/jonasohland/hsmainz/max-websockets/source/projects/websocketclient/websocketclient.cpp

source/projects/websocketclient/CMakeFiles/websocketclient.dir/websocketclient.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/websocketclient.dir/websocketclient.cpp.i"
	cd /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/jonasohland/hsmainz/max-websockets/source/projects/websocketclient/websocketclient.cpp > CMakeFiles/websocketclient.dir/websocketclient.cpp.i

source/projects/websocketclient/CMakeFiles/websocketclient.dir/websocketclient.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/websocketclient.dir/websocketclient.cpp.s"
	cd /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/jonasohland/hsmainz/max-websockets/source/projects/websocketclient/websocketclient.cpp -o CMakeFiles/websocketclient.dir/websocketclient.cpp.s

source/projects/websocketclient/CMakeFiles/websocketclient.dir/BeastSession.cpp.o: source/projects/websocketclient/CMakeFiles/websocketclient.dir/flags.make
source/projects/websocketclient/CMakeFiles/websocketclient.dir/BeastSession.cpp.o: ../source/projects/websocketclient/BeastSession.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object source/projects/websocketclient/CMakeFiles/websocketclient.dir/BeastSession.cpp.o"
	cd /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/websocketclient.dir/BeastSession.cpp.o -c /Users/jonasohland/hsmainz/max-websockets/source/projects/websocketclient/BeastSession.cpp

source/projects/websocketclient/CMakeFiles/websocketclient.dir/BeastSession.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/websocketclient.dir/BeastSession.cpp.i"
	cd /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/jonasohland/hsmainz/max-websockets/source/projects/websocketclient/BeastSession.cpp > CMakeFiles/websocketclient.dir/BeastSession.cpp.i

source/projects/websocketclient/CMakeFiles/websocketclient.dir/BeastSession.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/websocketclient.dir/BeastSession.cpp.s"
	cd /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/jonasohland/hsmainz/max-websockets/source/projects/websocketclient/BeastSession.cpp -o CMakeFiles/websocketclient.dir/BeastSession.cpp.s

source/projects/websocketclient/CMakeFiles/websocketclient.dir/WebSocketClientSession.cpp.o: source/projects/websocketclient/CMakeFiles/websocketclient.dir/flags.make
source/projects/websocketclient/CMakeFiles/websocketclient.dir/WebSocketClientSession.cpp.o: ../source/projects/websocketclient/WebSocketClientSession.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object source/projects/websocketclient/CMakeFiles/websocketclient.dir/WebSocketClientSession.cpp.o"
	cd /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/websocketclient.dir/WebSocketClientSession.cpp.o -c /Users/jonasohland/hsmainz/max-websockets/source/projects/websocketclient/WebSocketClientSession.cpp

source/projects/websocketclient/CMakeFiles/websocketclient.dir/WebSocketClientSession.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/websocketclient.dir/WebSocketClientSession.cpp.i"
	cd /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/jonasohland/hsmainz/max-websockets/source/projects/websocketclient/WebSocketClientSession.cpp > CMakeFiles/websocketclient.dir/WebSocketClientSession.cpp.i

source/projects/websocketclient/CMakeFiles/websocketclient.dir/WebSocketClientSession.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/websocketclient.dir/WebSocketClientSession.cpp.s"
	cd /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/jonasohland/hsmainz/max-websockets/source/projects/websocketclient/WebSocketClientSession.cpp -o CMakeFiles/websocketclient.dir/WebSocketClientSession.cpp.s

source/projects/websocketclient/CMakeFiles/websocketclient.dir/iiwaPosition.pb.cc.o: source/projects/websocketclient/CMakeFiles/websocketclient.dir/flags.make
source/projects/websocketclient/CMakeFiles/websocketclient.dir/iiwaPosition.pb.cc.o: ../source/projects/websocketclient/iiwaPosition.pb.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object source/projects/websocketclient/CMakeFiles/websocketclient.dir/iiwaPosition.pb.cc.o"
	cd /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/websocketclient.dir/iiwaPosition.pb.cc.o -c /Users/jonasohland/hsmainz/max-websockets/source/projects/websocketclient/iiwaPosition.pb.cc

source/projects/websocketclient/CMakeFiles/websocketclient.dir/iiwaPosition.pb.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/websocketclient.dir/iiwaPosition.pb.cc.i"
	cd /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/jonasohland/hsmainz/max-websockets/source/projects/websocketclient/iiwaPosition.pb.cc > CMakeFiles/websocketclient.dir/iiwaPosition.pb.cc.i

source/projects/websocketclient/CMakeFiles/websocketclient.dir/iiwaPosition.pb.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/websocketclient.dir/iiwaPosition.pb.cc.s"
	cd /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/jonasohland/hsmainz/max-websockets/source/projects/websocketclient/iiwaPosition.pb.cc -o CMakeFiles/websocketclient.dir/iiwaPosition.pb.cc.s

# Object files for target websocketclient
websocketclient_OBJECTS = \
"CMakeFiles/websocketclient.dir/websocketclient.cpp.o" \
"CMakeFiles/websocketclient.dir/BeastSession.cpp.o" \
"CMakeFiles/websocketclient.dir/WebSocketClientSession.cpp.o" \
"CMakeFiles/websocketclient.dir/iiwaPosition.pb.cc.o"

# External object files for target websocketclient
websocketclient_EXTERNAL_OBJECTS =

../externals/websocketclient.mxo/Contents/MacOS/websocketclient: source/projects/websocketclient/CMakeFiles/websocketclient.dir/websocketclient.cpp.o
../externals/websocketclient.mxo/Contents/MacOS/websocketclient: source/projects/websocketclient/CMakeFiles/websocketclient.dir/BeastSession.cpp.o
../externals/websocketclient.mxo/Contents/MacOS/websocketclient: source/projects/websocketclient/CMakeFiles/websocketclient.dir/WebSocketClientSession.cpp.o
../externals/websocketclient.mxo/Contents/MacOS/websocketclient: source/projects/websocketclient/CMakeFiles/websocketclient.dir/iiwaPosition.pb.cc.o
../externals/websocketclient.mxo/Contents/MacOS/websocketclient: source/projects/websocketclient/CMakeFiles/websocketclient.dir/build.make
../externals/websocketclient.mxo/Contents/MacOS/websocketclient: /usr/local/lib/libboost_date_time.a
../externals/websocketclient.mxo/Contents/MacOS/websocketclient: /usr/local/lib/libprotobuf.dylib
../externals/websocketclient.mxo/Contents/MacOS/websocketclient: source/projects/websocketclient/CMakeFiles/websocketclient.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX CFBundle shared module ../../../../externals/websocketclient.mxo/Contents/MacOS/websocketclient"
	cd /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/websocketclient.dir/link.txt --verbose=$(VERBOSE)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "Copy PkgInfo"
	cd /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient && cp /Users/jonasohland/hsmainz/max-websockets/source/min-api/max-api/script/PkgInfo /Users/jonasohland/hsmainz/max-websockets/source/projects/websocketclient/../../../externals/websocketclient.mxo/Contents/PkgInfo

# Rule to build all files generated by this target.
source/projects/websocketclient/CMakeFiles/websocketclient.dir/build: ../externals/websocketclient.mxo/Contents/MacOS/websocketclient

.PHONY : source/projects/websocketclient/CMakeFiles/websocketclient.dir/build

source/projects/websocketclient/CMakeFiles/websocketclient.dir/clean:
	cd /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient && $(CMAKE_COMMAND) -P CMakeFiles/websocketclient.dir/cmake_clean.cmake
.PHONY : source/projects/websocketclient/CMakeFiles/websocketclient.dir/clean

source/projects/websocketclient/CMakeFiles/websocketclient.dir/depend:
	cd /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/jonasohland/hsmainz/max-websockets /Users/jonasohland/hsmainz/max-websockets/source/projects/websocketclient /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient /Users/jonasohland/hsmainz/max-websockets/cmake-build-debug/source/projects/websocketclient/CMakeFiles/websocketclient.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : source/projects/websocketclient/CMakeFiles/websocketclient.dir/depend

