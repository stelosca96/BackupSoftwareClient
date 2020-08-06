//
// Created by stefano on 06/08/20.
//

#ifndef FILEWATCHER_FILEWATCHER_H
#define FILEWATCHER_FILEWATCHER_H

#include <string>
#include <chrono>
#include <unordered_map>
#include <filesystem>
#include <functional>

enum class FileStatus {created, modified, erased};

class FileWatcher {
private:
    bool running = false;
    std::string path_to_watch;
    std::chrono::duration<int, std::milli> delay;
    std::unordered_map<std::string, std::filesystem::file_time_type> children_paths_to_watch;
    bool contains(const std::string &key);

public:
    FileWatcher(const std::string &pathToWatch, const std::chrono::duration<int, std::milli> &delay);
    void start(const std::function<void (std::string, FileStatus)> &action);
};


#endif //FILEWATCHER_FILEWATCHER_H
