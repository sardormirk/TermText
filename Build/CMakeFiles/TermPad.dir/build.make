# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.27

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.27.4/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.27.4/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/voidblob/Desktop/TermText

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/voidblob/Desktop/TermText/Build

# Include any dependencies generated for this target.
include CMakeFiles/TermPad.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/TermPad.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/TermPad.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/TermPad.dir/flags.make

CMakeFiles/TermPad.dir/TermText.c.o: CMakeFiles/TermPad.dir/flags.make
CMakeFiles/TermPad.dir/TermText.c.o: /Users/voidblob/Desktop/TermText/TermText.c
CMakeFiles/TermPad.dir/TermText.c.o: CMakeFiles/TermPad.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/voidblob/Desktop/TermText/Build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/TermPad.dir/TermText.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/TermPad.dir/TermText.c.o -MF CMakeFiles/TermPad.dir/TermText.c.o.d -o CMakeFiles/TermPad.dir/TermText.c.o -c /Users/voidblob/Desktop/TermText/TermText.c

CMakeFiles/TermPad.dir/TermText.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/TermPad.dir/TermText.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/voidblob/Desktop/TermText/TermText.c > CMakeFiles/TermPad.dir/TermText.c.i

CMakeFiles/TermPad.dir/TermText.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/TermPad.dir/TermText.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/voidblob/Desktop/TermText/TermText.c -o CMakeFiles/TermPad.dir/TermText.c.s

# Object files for target TermPad
TermPad_OBJECTS = \
"CMakeFiles/TermPad.dir/TermText.c.o"

# External object files for target TermPad
TermPad_EXTERNAL_OBJECTS =

TermPad: CMakeFiles/TermPad.dir/TermText.c.o
TermPad: CMakeFiles/TermPad.dir/build.make
TermPad: CMakeFiles/TermPad.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/voidblob/Desktop/TermText/Build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable TermPad"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/TermPad.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/TermPad.dir/build: TermPad
.PHONY : CMakeFiles/TermPad.dir/build

CMakeFiles/TermPad.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/TermPad.dir/cmake_clean.cmake
.PHONY : CMakeFiles/TermPad.dir/clean

CMakeFiles/TermPad.dir/depend:
	cd /Users/voidblob/Desktop/TermText/Build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/voidblob/Desktop/TermText /Users/voidblob/Desktop/TermText /Users/voidblob/Desktop/TermText/Build /Users/voidblob/Desktop/TermText/Build /Users/voidblob/Desktop/TermText/Build/CMakeFiles/TermPad.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/TermPad.dir/depend
