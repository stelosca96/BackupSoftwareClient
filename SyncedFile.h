//
// Created by stefano on 06/08/20.
//

#ifndef FILEWATCHER_SYNCEDFILE_H
#define FILEWATCHER_SYNCEDFILE_H


#include <string>
#include <optional>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <atomic>
#include <shared_mutex>

enum class FileStatus {created, modified, erased, not_valid};

class SyncedFile {
private:
    std::string path;
    std::string filePath;
    std::string hash = "";
    unsigned long file_size = 0;
    bool is_file;
    std::shared_mutex mutex;
    FileStatus fileStatus = FileStatus::not_valid;

    // il file viene aggiunto alla coda di file da modificare,
    // fino a quando non si inizia il caricamento il file non viene riaggiunto
    // alla coda se sono effettuate ulteriori modifiche
    // la variabile is_syncing deve essere letta e modificata solo dalla classe
    // UploadJobs che la protegge con un lock nel caso ci sia un thread pool che effettua l'upload parallelo
    bool is_syncing = false;
    std::atomic<bool> synced = false;
    boost::property_tree::basic_ptree<std::string, std::string> getPtree();
    std::string diffPath(std::string basePath, const std::string& filePath);

public:

    //SyncedFile();

//    explicit SyncedFile(std::string path);
    SyncedFile(const std::string &JSON, const std::string &basePath, bool mode);
    SyncedFile(const std::string& path, const std::string& basePath, FileStatus fileStatus);

    //Deleted solo perchè non li usiamo e perchè evitiamo di usarli inconsciamente
    SyncedFile(SyncedFile const &syncedFile)=delete;
    SyncedFile(SyncedFile&& syncedFile)=delete;
    SyncedFile& operator=(const SyncedFile& syncedFile)=delete;
    SyncedFile&& operator=(SyncedFile&& syncedFile)=delete;

    void update_file_data();
    static std::string CalcSha256(const std::string& filename);
    std::string to_string();

    bool operator==( SyncedFile &rhs) ;
    bool operator!=( SyncedFile &rhs) ;

    [[nodiscard]] const std::string &getPath();
    [[nodiscard]] std::string getFilePath();
    [[nodiscard]] const std::string &getHash();
    [[nodiscard]] FileStatus getFileStatus();
    [[nodiscard]] unsigned long getFileSize();

    [[nodiscard]] bool isSyncing() const;
    [[nodiscard]] bool isFile();

    void setSyncing(bool syncing);
    std::string getJSON();

    void setToSync();
    void setSynced();
    std::optional<std::pair<std::string, boost::property_tree::basic_ptree<std::string, std::string>>> getMapValue();

};


#endif //FILEWATCHER_SYNCEDFILE_H
