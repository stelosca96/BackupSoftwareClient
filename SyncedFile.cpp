//
// Created by stefano on 06/08/20.
//
#include <openssl/sha.h>
#include <fstream>
#include <iomanip>
#include <utility>
#include "SyncedFile.h"
#include "exceptions/filesystemException.h"
#include <boost/property_tree/json_parser.hpp>
#include <filesystem>
#include <iostream>
#include <mutex>

namespace pt = boost::property_tree;

static const int K_READ_BUF_SIZE{ 1024 * 16 };
//
//SyncedFile::SyncedFile(std::string path) : path(std::move(path)) {
//    update_file_data();
//}

SyncedFile::SyncedFile(const std::string& path, const std::string& basePath, FileStatus fileStatus) : filePath(path) {
    this->path = diffPath(basePath, path);

    update_file_data();
    this->fileStatus = fileStatus;
}

void SyncedFile::update_file_data() {
    std::unique_lock lock(mutex);
    // todo: se il file non esiste marchiarlo come erased => da testare
    if(!(std::filesystem::is_regular_file(this->filePath) || (std::filesystem::is_directory(this->filePath)))){
        this->hash = "";
        this->fileStatus = FileStatus::erased;
        this->file_size = 0;
        this->is_file = false;
        return;
    }
    std::string new_hash = CalcSha256(filePath);
    if(new_hash!= this->hash){
        this->is_file = std::filesystem::is_regular_file(this->filePath);
        this->fileStatus = FileStatus::modified;
        this->hash = new_hash;
        this->file_size = is_file ? std::filesystem::file_size(this->filePath) : 0;
    }
}


std::string SyncedFile::CalcSha256(const std::string& filename){
    // Initialize openssl
    SHA256_CTX context;
    if(!SHA256_Init(&context))
        return "";

    // Read file and update calculated SHA
    char buf[K_READ_BUF_SIZE];
    std::ifstream file(filename, std::ifstream::binary);
    while (file.good()){
        file.read(buf, sizeof(buf));
        if(!SHA256_Update(&context, buf, file.gcount()))
            return "";
    }

    // Get Final SHA
    unsigned char result[SHA256_DIGEST_LENGTH];
    if(!SHA256_Final(result, &context))
        return "";

    // Transform byte-array to string
    std::stringstream sha_str;
    sha_str << std::hex << std::setfill('0');
    for (const auto &byte: result)
        sha_str << std::setw(2) << (int)byte;
    return sha_str.str();
}

bool SyncedFile::operator==(const SyncedFile &rhs) const {
    // todo: devo acquisire i mutex di entrambi?
//    std::scoped_lock lock(mutex, rhs.mutex);
//    std::shared_lock lock(mutex);
    return path == rhs.path &&
           hash == rhs.hash;
}

bool SyncedFile::operator!=(const SyncedFile &rhs) const {
    return !(rhs == *this);
}

SyncedFile::SyncedFile(SyncedFile const &syncedFile) {
    this->fileStatus = syncedFile.fileStatus;
    this->file_size = syncedFile.file_size;
    this->hash = syncedFile.hash;
    this->path = syncedFile.path;
    this->filePath = syncedFile.filePath;
    this->is_file = syncedFile.is_file;
}

// todo: da rimuovere, ma se lo togli non funziona nulla
SyncedFile::SyncedFile() {
    fileStatus = FileStatus::not_valid;
    is_file = false;
}

const std::string &SyncedFile::getPath(){
    std::shared_lock lock(mutex);
    return path;
}

std::string SyncedFile::getFilePath() {
    std::shared_lock lock(mutex);
    return filePath;
}


const std::string &SyncedFile::getHash() {
    std::shared_lock lock(mutex);
    return hash;
}

std::string SyncedFile::to_string() {
    std::shared_lock lock(mutex);
    return this->path + " (" + this->hash + ")";
}

FileStatus SyncedFile::getFileStatus() {
    std::shared_lock lock(mutex);
    return fileStatus;
}

bool SyncedFile::isSyncing() const {
    // funzione chiamata solo dal thread dei jobs, non necessita syncronizzazione
    return is_syncing;
}

void SyncedFile::setSyncing(bool syncing) {
    // funzione chiamata solo dal thread dei jobs, non necessita syncronizzazione
    SyncedFile::is_syncing = syncing;
}

pt::ptree SyncedFile::getPtree(){
    pt::ptree root;
    std::shared_lock lock(mutex);

    root.put("path", this->path);

    //todo: devo ricalcolare l'hash e la dimensione (nel caso il file sia stato modificato) o uso quello salvato?
    // this->update_file_data();
    root.put("hash", this->hash);
    root.put("file_size", this->file_size);
    root.put("file_status", static_cast<int>(this->fileStatus));
    root.put("is_file", this->is_file);
    return root;
}

std::string SyncedFile::getJSON() {
    // non ho bisogno di sicronizzare perchè chiamo la funzione getPtree che è già sincronizzata
    pt::ptree root = this->getPtree();

    std::stringstream ss;
    pt::json_parser::write_json(ss, root);
    return ss.str();
}

bool SyncedFile::isFile() {
    std::shared_lock lock(mutex);
    return is_file;
}

unsigned long SyncedFile::getFileSize() {
    std::shared_lock lock(mutex);
    return file_size;
}

//SyncedFile::SyncedFile(const std::string& path, const std::string& JSON) {
//    // costruttore => solo chi crea conosce il riferimento, non serve sincronizzare
//    std::stringstream ss(JSON);
//
//    boost::property_tree::ptree root;
//    boost::property_tree::read_json(ss, root);
//    this->hash = root.get_child("hash").data();
//
//    this->path = root.get_child("path").data();
//
//    // If no conversion could be performed, an invalid_argument exception is thrown.
//    this->file_size = std::stoul(root.get_child("file_size").data());
//
//    // If no conversion could be performed, an invalid_argument exception is thrown.
//    this->fileStatus = static_cast<FileStatus>(std::stoi(root.get_child("file_status").data()));
//
//    this->is_file = root.get_child("is_file").data()=="true";
//}

void SyncedFile::setToSync() {
    this->synced.store(false);
}

void SyncedFile::setSynced() {
    this->synced.store(true);
}

std::optional<std::pair<std::string, boost::property_tree::basic_ptree<std::string, std::string>>>
SyncedFile::getMapValue() {
    std::shared_lock lock(mutex);
    if(!this->synced.load())
        return std::nullopt;
    return std::make_pair(this->filePath, this->getPtree());
}

SyncedFile::SyncedFile(const std::string &JSON, const std::string &basePath, bool modeSync) {
    // todo: gestire eccezioni
    std::stringstream ss(JSON);

    boost::property_tree::ptree root;
    boost::property_tree::read_json(ss, root);
    this->hash = root.get_child("hash").data();

    if (modeSync){
        this->path = root.get_child("path").data();
        std::filesystem::path bp(basePath[basePath.size()-1]== '/' ? basePath.substr(0, basePath.size()-1) : basePath);
        bp += this->path;
        this->filePath = bp.string();
    } else {
        this->filePath = root.get_child("path").data();
        this->path = diffPath(basePath, this->filePath);
    }

    // If no conversion could be performed, an invalid_argument exception is thrown.
    this->file_size = std::stoul(root.get_child("file_size").data());

    // If no conversion could be performed, an invalid_argument exception is thrown.
    this->fileStatus = static_cast<FileStatus>(std::stoi(root.get_child("file_status").data()));

    this->is_file = root.get_child("is_file").data()=="true";
}

std::string SyncedFile::diffPath(std::string basePath, const std::string& filePath){
    ssize_t size = basePath.size();
    if (basePath[size-1] == '/') {
        basePath = basePath.substr(0,size-1);
        size--;
    }
    std::string bp = filePath.substr(0, size);
    if (bp != basePath){
        throw filesystemException(bp + "!=" + basePath);
    }
    std::string diffPath = filePath.substr(size, filePath.size()-size);
    return diffPath;
}
