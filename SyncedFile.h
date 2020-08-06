//
// Created by stefano on 06/08/20.
//

#ifndef FILEWATCHER_SYNCEDFILE_H
#define FILEWATCHER_SYNCEDFILE_H


#include <string>
#include <optional>

class SyncedFile {
private:
    std::string path;
    std::string hash;
    bool synced = false;

public:
    SyncedFile(const std::string &path);
    void calculate_hash();
    static std::optional<std::string> CalcSha256(std::string filename);
};


#endif //FILEWATCHER_SYNCEDFILE_H
