//
// Created by stefano on 07/08/20.
//

#ifndef FILEWATCHER_UPLOADJOBS_H
#define FILEWATCHER_UPLOADJOBS_H


#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "SyncedFile.h"

class UploadJobs {
private:
    std::queue<std::shared_ptr<SyncedFile>> queue;
    std::mutex mutex;
    std::condition_variable cv;

    bool producer_ended = false;
public:
    std::shared_ptr<SyncedFile> get();
    void put(const std::shared_ptr<SyncedFile>& sfp);
    void producer_end();
    bool producer_is_ended();
    int queue_size();
};

#endif //FILEWATCHER_UPLOADJOBS_H
