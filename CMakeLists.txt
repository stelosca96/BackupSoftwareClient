cmake_minimum_required(VERSION 3.16)
project(FileWatcher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(FileWatcher main.cpp FileWatcher.cpp FileWatcher.h SyncedFile.cpp SyncedFile.h UploadJobs.cpp UploadJobs.h Socket.cpp Socket.h User.cpp exceptions/socketException.h exceptions/dataException.h exceptions/filesystemException.h)
target_link_libraries(${PROJECT_NAME} PUBLIC ssl crypto)

