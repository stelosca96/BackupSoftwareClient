#include <iostream>
#include "FileWatcher.h"
#include "SyncedFile.h"
#include "UploadJobs.h"

UploadJobs uploadJobs;

// serve veramente un thread pool per il client?
void start_upload_pool(){

}

void upload_to_server(std::shared_ptr<SyncedFile> sfp){
    //todo: gestire sincronizzazione
    uploadJobs.put(sfp);
}

void file_watcher(){
    // Create a FileWatcher instance that will check the current folder for changes every 5 seconds
    FileWatcher fw{"/home/stefano/CLionProjects/FileWatcher/test_dir", std::chrono::milliseconds(5000)};

    // Start monitoring a folder for changes and (in case of changes)
    // run a user provided lambda function
    fw.start([] (const std::shared_ptr<SyncedFile>& sfp, FileStatus status) -> void {
        // Process only regular files, all other file types are ignored
        if(!std::filesystem::is_regular_file(std::filesystem::path(sfp->getPath())) && status != FileStatus::erased) {
            return;
        }

        switch(status) {
            case FileStatus::created:
                std::cout << "File created: " << sfp->to_string() << '\n';
                break;
            case FileStatus::modified:
                std::cout << "File modified: " << sfp->to_string() << '\n';
                break;
            case FileStatus::erased:
                std::cout << "File erased: " << sfp->to_string() << '\n';
                break;
            default:
                std::cout << "Error! Unknown file status.\n";
        }
        upload_to_server(sfp);
    });
}



int main() {

    file_watcher();
}