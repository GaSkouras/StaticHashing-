# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

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
CMAKE_COMMAND = /home/telesto/Downloads/clion-2018.3.2/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/telesto/Downloads/clion-2018.3.2/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/telesto/CLionProjects/idbs

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/telesto/CLionProjects/idbs/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/idbs.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/idbs.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/idbs.dir/flags.make

CMakeFiles/idbs.dir/main.c.o: CMakeFiles/idbs.dir/flags.make
CMakeFiles/idbs.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/telesto/CLionProjects/idbs/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/idbs.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/idbs.dir/main.c.o   -c /home/telesto/CLionProjects/idbs/main.c

CMakeFiles/idbs.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/idbs.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/telesto/CLionProjects/idbs/main.c > CMakeFiles/idbs.dir/main.c.i

CMakeFiles/idbs.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/idbs.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/telesto/CLionProjects/idbs/main.c -o CMakeFiles/idbs.dir/main.c.s

CMakeFiles/idbs.dir/HT.c.o: CMakeFiles/idbs.dir/flags.make
CMakeFiles/idbs.dir/HT.c.o: ../HT.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/telesto/CLionProjects/idbs/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/idbs.dir/HT.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/idbs.dir/HT.c.o   -c /home/telesto/CLionProjects/idbs/HT.c

CMakeFiles/idbs.dir/HT.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/idbs.dir/HT.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/telesto/CLionProjects/idbs/HT.c > CMakeFiles/idbs.dir/HT.c.i

CMakeFiles/idbs.dir/HT.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/idbs.dir/HT.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/telesto/CLionProjects/idbs/HT.c -o CMakeFiles/idbs.dir/HT.c.s

CMakeFiles/idbs.dir/SHT.c.o: CMakeFiles/idbs.dir/flags.make
CMakeFiles/idbs.dir/SHT.c.o: ../SHT.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/telesto/CLionProjects/idbs/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/idbs.dir/SHT.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/idbs.dir/SHT.c.o   -c /home/telesto/CLionProjects/idbs/SHT.c

CMakeFiles/idbs.dir/SHT.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/idbs.dir/SHT.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/telesto/CLionProjects/idbs/SHT.c > CMakeFiles/idbs.dir/SHT.c.i

CMakeFiles/idbs.dir/SHT.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/idbs.dir/SHT.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/telesto/CLionProjects/idbs/SHT.c -o CMakeFiles/idbs.dir/SHT.c.s

# Object files for target idbs
idbs_OBJECTS = \
"CMakeFiles/idbs.dir/main.c.o" \
"CMakeFiles/idbs.dir/HT.c.o" \
"CMakeFiles/idbs.dir/SHT.c.o"

# External object files for target idbs
idbs_EXTERNAL_OBJECTS =

idbs: CMakeFiles/idbs.dir/main.c.o
idbs: CMakeFiles/idbs.dir/HT.c.o
idbs: CMakeFiles/idbs.dir/SHT.c.o
idbs: CMakeFiles/idbs.dir/build.make
idbs: ../lib/BF_64.a
idbs: CMakeFiles/idbs.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/telesto/CLionProjects/idbs/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C executable idbs"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/idbs.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/idbs.dir/build: idbs

.PHONY : CMakeFiles/idbs.dir/build

CMakeFiles/idbs.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/idbs.dir/cmake_clean.cmake
.PHONY : CMakeFiles/idbs.dir/clean

CMakeFiles/idbs.dir/depend:
	cd /home/telesto/CLionProjects/idbs/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/telesto/CLionProjects/idbs /home/telesto/CLionProjects/idbs /home/telesto/CLionProjects/idbs/cmake-build-debug /home/telesto/CLionProjects/idbs/cmake-build-debug /home/telesto/CLionProjects/idbs/cmake-build-debug/CMakeFiles/idbs.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/idbs.dir/depend
