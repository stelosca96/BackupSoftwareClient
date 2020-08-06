cmake_minimum_required(VERSION 3.16)
project(FileWatcher)

set(CMAKE_CXX_STANDARD 17)

add_executable(FileWatcher main.cpp FileWatcher.cpp FileWatcher.h SyncedFile.cpp SyncedFile.h)