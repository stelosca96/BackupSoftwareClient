//
// Created by stefano on 06/08/20.
//
#include <openssl/sha.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <utility>
#include "SyncedFile.h"

static const int K_READ_BUF_SIZE{ 1024 * 16 };

SyncedFile::SyncedFile(std::string path) : path(std::move(path)) {
    calculate_hash();
}

SyncedFile::SyncedFile(std::string path, FileStatus fileStatus) : path(std::move(path)) {
    calculate_hash();
    this->fileStatus = fileStatus;
}

void SyncedFile::calculate_hash() {
    std::string new_hash = CalcSha256(path);
    if(new_hash!= this->hash){
        this->fileStatus = FileStatus::modified;
        this->hash = new_hash;
    }
}


std::string SyncedFile::CalcSha256(std::string filename)
{
    // Initialize openssl
    SHA256_CTX context;
    if(!SHA256_Init(&context))
    {
        return "";
    }

    // Read file and update calculated SHA
    char buf[K_READ_BUF_SIZE];
    std::ifstream file(filename, std::ifstream::binary);
    while (file.good())
    {
        file.read(buf, sizeof(buf));
        if(!SHA256_Update(&context, buf, file.gcount()))
        {
            return "";
        }
    }

    // Get Final SHA
    unsigned char result[SHA256_DIGEST_LENGTH];
    if(!SHA256_Final(result, &context))
    {
        return "";
    }

    // Transform byte-array to string
    std::stringstream shastr;
    shastr << std::hex << std::setfill('0');
    for (const auto &byte: result)
    {
        shastr << std::setw(2) << (int)byte;
    }
    return shastr.str();
}

bool SyncedFile::operator==(const SyncedFile &rhs) const {
    return path == rhs.path &&
           hash == rhs.hash;
}

bool SyncedFile::operator!=(const SyncedFile &rhs) const {
    return !(rhs == *this);
}

SyncedFile::SyncedFile(SyncedFile const &syncedFile) {
    this->fileStatus = syncedFile.fileStatus;
    this->hash = syncedFile.hash;
    this->path = syncedFile.path;
}

SyncedFile::SyncedFile() {
    fileStatus = FileStatus::not_valid;
}

const std::string &SyncedFile::getPath() const {
    return path;
}

const std::string &SyncedFile::getHash() const {
    return hash;
}

std::string SyncedFile::to_string() {
    return this->path + " (" + this->hash + ")";
}

FileStatus SyncedFile::getFileStatus() const {
    return fileStatus;
}

bool SyncedFile::isSyncing() const {
    return is_syncing;
}

void SyncedFile::setSyncing(bool syncing) {
    SyncedFile::is_syncing = syncing;
}



