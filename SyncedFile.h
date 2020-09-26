//
// Created by stefano on 06/08/20.
//

#ifndef FILEWATCHER_SYNCEDFILE_H
#define FILEWATCHER_SYNCEDFILE_H


#include <string>
#include <optional>
#include <boost/property_tree/ptree.hpp>
#include <atomic>
#include <shared_mutex>

enum class FileStatus {created, modified, erased, not_valid};

class SyncedFile {
private:
    std::string path;
    std::string hash = "";
    unsigned long file_size = 0;
    bool is_file;
    //todo: aggiungere ultima modifica
    std::shared_mutex mutex;
    // todo: il fileStatus serve veramente? magari distinguere tra modificato/eliminato/non_valido
    FileStatus fileStatus = FileStatus::not_valid;

    // il file viene aggiunto alla coda di file da modificare,
    // fino a quando non si inizia il caricamento il file non viene riaggiunto
    // alla coda se sono effettuate ulteriori modifiche
    // la variabile is_syncing deve essere letta e modificata solo dalla classe
    // UploadJobs che la protegge con un lock nel caso ci sia un thread pool che effettua l'upload parallelo
    bool is_syncing = false;
    std::atomic<bool> synced = false;
    boost::property_tree::basic_ptree<std::string, std::string> getPtree();

public:
    //todo: togliere costruttore vuoto
    SyncedFile();

    explicit SyncedFile(std::string path);
    SyncedFile(const std::string& path, const std::string& JSON);
    SyncedFile(std::string path, FileStatus fileStatus);
    SyncedFile(SyncedFile const &syncedFile);

    void update_file_data();
    static std::string CalcSha256(const std::string& filename);
    //todo: costruttore di copia e movimento
    std::string to_string();

    bool operator==(const SyncedFile &rhs) const;
    bool operator!=(const SyncedFile &rhs) const;

    [[nodiscard]] const std::string &getPath();
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
