# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

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
CMAKE_COMMAND = /home/pierluigi/clion-2020.2.2/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/pierluigi/clion-2020.2.2/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/pierluigi/CLionProjects/BackupSoftwareClient

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pierluigi/CLionProjects/BackupSoftwareClient/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/FileWatcher.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/FileWatcher.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/FileWatcher.dir/flags.make

CMakeFiles/FileWatcher.dir/main.cpp.o: CMakeFiles/FileWatcher.dir/flags.make
CMakeFiles/FileWatcher.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pierluigi/CLionProjects/BackupSoftwareClient/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/FileWatcher.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/FileWatcher.dir/main.cpp.o -c /home/pierluigi/CLionProjects/BackupSoftwareClient/main.cpp

CMakeFiles/FileWatcher.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/FileWatcher.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pierluigi/CLionProjects/BackupSoftwareClient/main.cpp > CMakeFiles/FileWatcher.dir/main.cpp.i

CMakeFiles/FileWatcher.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/FileWatcher.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pierluigi/CLionProjects/BackupSoftwareClient/main.cpp -o CMakeFiles/FileWatcher.dir/main.cpp.s

CMakeFiles/FileWatcher.dir/FileWatcher.cpp.o: CMakeFiles/FileWatcher.dir/flags.make
CMakeFiles/FileWatcher.dir/FileWatcher.cpp.o: ../FileWatcher.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pierluigi/CLionProjects/BackupSoftwareClient/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/FileWatcher.dir/FileWatcher.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/FileWatcher.dir/FileWatcher.cpp.o -c /home/pierluigi/CLionProjects/BackupSoftwareClient/FileWatcher.cpp

CMakeFiles/FileWatcher.dir/FileWatcher.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/FileWatcher.dir/FileWatcher.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pierluigi/CLionProjects/BackupSoftwareClient/FileWatcher.cpp > CMakeFiles/FileWatcher.dir/FileWatcher.cpp.i

CMakeFiles/FileWatcher.dir/FileWatcher.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/FileWatcher.dir/FileWatcher.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pierluigi/CLionProjects/BackupSoftwareClient/FileWatcher.cpp -o CMakeFiles/FileWatcher.dir/FileWatcher.cpp.s

CMakeFiles/FileWatcher.dir/SyncedFile.cpp.o: CMakeFiles/FileWatcher.dir/flags.make
CMakeFiles/FileWatcher.dir/SyncedFile.cpp.o: ../SyncedFile.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pierluigi/CLionProjects/BackupSoftwareClient/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/FileWatcher.dir/SyncedFile.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/FileWatcher.dir/SyncedFile.cpp.o -c /home/pierluigi/CLionProjects/BackupSoftwareClient/SyncedFile.cpp

CMakeFiles/FileWatcher.dir/SyncedFile.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/FileWatcher.dir/SyncedFile.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pierluigi/CLionProjects/BackupSoftwareClient/SyncedFile.cpp > CMakeFiles/FileWatcher.dir/SyncedFile.cpp.i

CMakeFiles/FileWatcher.dir/SyncedFile.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/FileWatcher.dir/SyncedFile.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pierluigi/CLionProjects/BackupSoftwareClient/SyncedFile.cpp -o CMakeFiles/FileWatcher.dir/SyncedFile.cpp.s

CMakeFiles/FileWatcher.dir/UploadJobs.cpp.o: CMakeFiles/FileWatcher.dir/flags.make
CMakeFiles/FileWatcher.dir/UploadJobs.cpp.o: ../UploadJobs.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pierluigi/CLionProjects/BackupSoftwareClient/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/FileWatcher.dir/UploadJobs.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/FileWatcher.dir/UploadJobs.cpp.o -c /home/pierluigi/CLionProjects/BackupSoftwareClient/UploadJobs.cpp

CMakeFiles/FileWatcher.dir/UploadJobs.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/FileWatcher.dir/UploadJobs.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pierluigi/CLionProjects/BackupSoftwareClient/UploadJobs.cpp > CMakeFiles/FileWatcher.dir/UploadJobs.cpp.i

CMakeFiles/FileWatcher.dir/UploadJobs.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/FileWatcher.dir/UploadJobs.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pierluigi/CLionProjects/BackupSoftwareClient/UploadJobs.cpp -o CMakeFiles/FileWatcher.dir/UploadJobs.cpp.s

CMakeFiles/FileWatcher.dir/Client.cpp.o: CMakeFiles/FileWatcher.dir/flags.make
CMakeFiles/FileWatcher.dir/Client.cpp.o: ../Client.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pierluigi/CLionProjects/BackupSoftwareClient/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/FileWatcher.dir/Client.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/FileWatcher.dir/Client.cpp.o -c /home/pierluigi/CLionProjects/BackupSoftwareClient/Client.cpp

CMakeFiles/FileWatcher.dir/Client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/FileWatcher.dir/Client.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pierluigi/CLionProjects/BackupSoftwareClient/Client.cpp > CMakeFiles/FileWatcher.dir/Client.cpp.i

CMakeFiles/FileWatcher.dir/Client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/FileWatcher.dir/Client.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pierluigi/CLionProjects/BackupSoftwareClient/Client.cpp -o CMakeFiles/FileWatcher.dir/Client.cpp.s

CMakeFiles/FileWatcher.dir/User.cpp.o: CMakeFiles/FileWatcher.dir/flags.make
CMakeFiles/FileWatcher.dir/User.cpp.o: ../User.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pierluigi/CLionProjects/BackupSoftwareClient/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/FileWatcher.dir/User.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/FileWatcher.dir/User.cpp.o -c /home/pierluigi/CLionProjects/BackupSoftwareClient/User.cpp

CMakeFiles/FileWatcher.dir/User.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/FileWatcher.dir/User.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pierluigi/CLionProjects/BackupSoftwareClient/User.cpp > CMakeFiles/FileWatcher.dir/User.cpp.i

CMakeFiles/FileWatcher.dir/User.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/FileWatcher.dir/User.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pierluigi/CLionProjects/BackupSoftwareClient/User.cpp -o CMakeFiles/FileWatcher.dir/User.cpp.s

# Object files for target FileWatcher
FileWatcher_OBJECTS = \
"CMakeFiles/FileWatcher.dir/main.cpp.o" \
"CMakeFiles/FileWatcher.dir/FileWatcher.cpp.o" \
"CMakeFiles/FileWatcher.dir/SyncedFile.cpp.o" \
"CMakeFiles/FileWatcher.dir/UploadJobs.cpp.o" \
"CMakeFiles/FileWatcher.dir/Client.cpp.o" \
"CMakeFiles/FileWatcher.dir/User.cpp.o"

# External object files for target FileWatcher
FileWatcher_EXTERNAL_OBJECTS =

FileWatcher: CMakeFiles/FileWatcher.dir/main.cpp.o
FileWatcher: CMakeFiles/FileWatcher.dir/FileWatcher.cpp.o
FileWatcher: CMakeFiles/FileWatcher.dir/SyncedFile.cpp.o
FileWatcher: CMakeFiles/FileWatcher.dir/UploadJobs.cpp.o
FileWatcher: CMakeFiles/FileWatcher.dir/Client.cpp.o
FileWatcher: CMakeFiles/FileWatcher.dir/User.cpp.o
FileWatcher: CMakeFiles/FileWatcher.dir/build.make
FileWatcher: CMakeFiles/FileWatcher.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/pierluigi/CLionProjects/BackupSoftwareClient/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX executable FileWatcher"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/FileWatcher.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/FileWatcher.dir/build: FileWatcher

.PHONY : CMakeFiles/FileWatcher.dir/build

CMakeFiles/FileWatcher.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/FileWatcher.dir/cmake_clean.cmake
.PHONY : CMakeFiles/FileWatcher.dir/clean

CMakeFiles/FileWatcher.dir/depend:
	cd /home/pierluigi/CLionProjects/BackupSoftwareClient/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pierluigi/CLionProjects/BackupSoftwareClient /home/pierluigi/CLionProjects/BackupSoftwareClient /home/pierluigi/CLionProjects/BackupSoftwareClient/cmake-build-debug /home/pierluigi/CLionProjects/BackupSoftwareClient/cmake-build-debug /home/pierluigi/CLionProjects/BackupSoftwareClient/cmake-build-debug/CMakeFiles/FileWatcher.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/FileWatcher.dir/depend
