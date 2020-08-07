//
// Created by stefano on 06/08/20.
//

#include <thread>
#include <iostream>
#include "FileWatcher.h"
#include "SyncedFile.h"

// Check if "paths_" contains a given key
// If your compiler supports C++20 use paths_.contains(key) instead of this function
bool FileWatcher::contains(const std::string &key) {
    auto el = this->files_to_watch.find(key);
    return el != this->files_to_watch.end();
}

FileWatcher::FileWatcher(const std::string &pathToWatch, const std::chrono::duration<int, std::milli> &delay)
        : path_to_watch(pathToWatch), delay(delay) {
    // Keep a record of files from the base directory and their last modification time
    // si potrebbe sostituire il valore di ultima modifica con l'hash code
    // todo: l'inizializzazione andrà fatta leggendo i file della mappa da un db
    for(auto &file : std::filesystem::recursive_directory_iterator(path_to_watch)) {
        files_to_watch[file.path().string()] = std::make_shared<SyncedFile>(file.path().string());
    }
}

void FileWatcher::start(const std::function<void(std::shared_ptr<SyncedFile>, FileStatus)> &action) {
    this->running = true;
    while(running) {
        // Wait for "delay" milliseconds
        std::this_thread::sleep_for(this->delay);
        auto it = files_to_watch.begin();
        while (it != files_to_watch.end()) {
            if (!std::filesystem::exists(it->first)) {
                SyncedFile sf(it->first, FileStatus::erased);
                action(std::make_shared<SyncedFile>(sf), FileStatus::erased);
                it = files_to_watch.erase(it);
            }
            else
                it++;
        }
        // Check if a file was created or modified
        for(auto &file : std::filesystem::recursive_directory_iterator(path_to_watch)) {
            auto current_file_last_write_time = std::filesystem::last_write_time(file);
            // File creation
            if(!contains(file.path().string())) {
                std::shared_ptr<SyncedFile> sfp = std::make_shared<SyncedFile>(file.path().string(), FileStatus::created);
                this->files_to_watch[file.path().string()] = sfp;
                action(sfp, FileStatus::created);
                // File modification
            }
            else {
                // controllo se due percorsi uguali hanno hash diverso il file è stato modificato
                if(files_to_watch[file.path().string()]->getHash() != SyncedFile::CalcSha256(file.path().string())) {
                    files_to_watch[file.path().string()]->update_file_data();
                    action(files_to_watch[file.path().string()], FileStatus::modified);
                }
            }
        }
    }
}
