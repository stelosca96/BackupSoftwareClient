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

//todo: forse dovrei lavorare con dei weak ptr, devo considerare il caso in cui il file sia eliminato prima di caricarlo sul server
class UploadJobs {
private:
    std::queue<std::shared_ptr<SyncedFile>> queue;
    std::mutex mutex;
    std::condition_variable cv;
    std::condition_variable cv_put;
    bool producer_ended = false;
public:
    std::shared_ptr<SyncedFile> get();
    void put(const std::shared_ptr<SyncedFile>& sfp);
    void producer_end();
    bool producer_is_ended();
};

#endif //FILEWATCHER_UPLOADJOBS_H
