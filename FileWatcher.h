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
#include <shared_mutex>
#include "SyncedFile.h"


class FileWatcher {
private:
    bool running = false;
    std::shared_mutex map_mutex;
    std::string path_to_watch;
    std::chrono::duration<int, std::milli> delay;
    std::unordered_map<std::string, std::shared_ptr<SyncedFile>> files_to_watch;
    bool contains(const std::string &key);
    void loadMap();

public:
    FileWatcher(const std::string &pathToWatch, const std::chrono::duration<int, std::milli> &delay);
    void start(const std::function<void (std::shared_ptr<SyncedFile>, FileStatus)> &action);
    void saveMap();
};


#endif //FILEWATCHER_FILEWATCHER_H
