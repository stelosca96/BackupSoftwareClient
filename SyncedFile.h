//
// Created by stefano on 06/08/20.
//

#ifndef FILEWATCHER_SYNCEDFILE_H
#define FILEWATCHER_SYNCEDFILE_H


#include <string>
#include <optional>

enum class FileStatus {created, modified, erased, not_valid};

class SyncedFile {
private:
    std::string path;
    std::string hash = "";
    FileStatus fileStatus = FileStatus::not_valid;

    // il file viene aggiunto alla coda di file da modificare,
    // fino a quando non si inizia il caricamento il file non viene riaggiunto
    // alla coda se sono effettuate ulteriori modifiche
    // la variabile is_syncing deve essere letta e modificata solo dalla classe
    // UploadJobs che la protegge con un lock nel caso ci sia un thread pool che effettua l'upload parallelo
    bool is_syncing = false;

public:
    //todo: togliere costruttore vuoto
    SyncedFile();

    SyncedFile(std::string path);
    SyncedFile(std::string path, FileStatus fileStatus);
    SyncedFile(SyncedFile const &syncedFile);

    void calculate_hash();
    static std::string CalcSha256(std::string filename);
    //todo: costruttore di copia e movimento
    std::string to_string();

    bool operator==(const SyncedFile &rhs) const;
    bool operator!=(const SyncedFile &rhs) const;

    const std::string &getPath() const;

    const std::string &getHash() const;

    FileStatus getFileStatus() const;

    bool isSyncing() const;

    void setSyncing(bool synching);
};


#endif //FILEWATCHER_SYNCEDFILE_H
