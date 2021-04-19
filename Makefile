# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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
CMAKE_COMMAND = /usr/bin/cmake3

# The command to remove a file.
RM = /usr/bin/cmake3 -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /src/mesos-command-modules

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /src/mesos-command-modules

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake3 --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/usr/bin/ccmake3 -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /src/mesos-command-modules/CMakeFiles /src/mesos-command-modules/CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /src/mesos-command-modules/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named mesos_command_modules

# Build rule for target.
mesos_command_modules: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 mesos_command_modules
.PHONY : mesos_command_modules

# fast build rule for target.
mesos_command_modules/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/mesos_command_modules.dir/build.make CMakeFiles/mesos_command_modules.dir/build
.PHONY : mesos_command_modules/fast

src/SlaveSorter.o: src/SlaveSorter.cpp.o

.PHONY : src/SlaveSorter.o

# target to build an object file
src/SlaveSorter.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/mesos_command_modules.dir/build.make CMakeFiles/mesos_command_modules.dir/src/SlaveSorter.cpp.o
.PHONY : src/SlaveSorter.cpp.o

src/SlaveSorter.i: src/SlaveSorter.cpp.i

.PHONY : src/SlaveSorter.i

# target to preprocess a source file
src/SlaveSorter.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/mesos_command_modules.dir/build.make CMakeFiles/mesos_command_modules.dir/src/SlaveSorter.cpp.i
.PHONY : src/SlaveSorter.cpp.i

src/SlaveSorter.s: src/SlaveSorter.cpp.s

.PHONY : src/SlaveSorter.s

# target to generate assembly for a file
src/SlaveSorter.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/mesos_command_modules.dir/build.make CMakeFiles/mesos_command_modules.dir/src/SlaveSorter.cpp.s
.PHONY : src/SlaveSorter.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... mesos_command_modules"
	@echo "... src/SlaveSorter.o"
	@echo "... src/SlaveSorter.i"
	@echo "... src/SlaveSorter.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

