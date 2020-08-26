#include <iostream>
#include <thread>
#include "FileWatcher.h"
#include "SyncedFile.h"
#include "UploadJobs.h"
#include "Socket.h"
#include "User.h"
#include "exceptions/dataException.h"
#include "exceptions/socketException.h"
#include "exceptions/filesystemException.h"

UploadJobs uploadJobs;
std::shared_ptr<FileWatcher> fw_ptr = nullptr;

void saveMap(){
    if(fw_ptr!= nullptr)
        fw_ptr->saveMap();
}

void upload_to_server(){
    // quando apro la connessione tcp?
    // una per ogni file => grande spreco di risorse
    // una e la tengo aperta => spreco di risorse a tenere una connessione aperta
    // la apro e la tengo aperta finchè la coda è piena? Può avere senso ma è più complicata da implementare
    // todo: gestire eccezioni ed eventualmente mutua esclusione
    try {
        Socket socket;
        socket.connectToServer("127.0.0.1", 6034);
        User user("ste", "ciao1234");
        // 1. invio le credenziali
        socket.sendJSON(user.getJSON());

        // 2. se il server non risponde OK l'auth non è andata a buon fine e chiudo il programma
        if(socket.getResp() != "OK") {
            std::cout << "User not valid." << std::endl;
            exit(-2);
        }


        // continuo ad estrarre file dalla coda finchè il programma non termina
        while (!uploadJobs.producer_is_ended()) {
            std::shared_ptr<SyncedFile> syncedFile = uploadJobs.get();

            // la classe fileJobs ritorna un nullptr se la coda è vuota e il programma deve terminare
            if (syncedFile != nullptr) {
                try {
                    // 3. invio il json del synced file
                    socket.sendJSON(syncedFile->getJSON());
                    std::string resp = socket.getResp();
                    if  (resp == "OK") {
                        syncedFile->setSynced();
                        saveMap();
                        // se il file è già sul server ho concluso il mio lavoro
                        std::cout << "Sincronizzazione file riuscita." << std::endl;
                    } else if (resp=="NO") {
                        // se il file non è già sul server devo inviarlo
                        std::cout << "Il server ha richiesto l'invio del file." << std::endl;
                        socket.sendFile(syncedFile);
                        resp = socket.getResp();
                        if(resp == "OK") {
                            syncedFile->setSynced();
                            saveMap();
                            // se il file è stato caricato correttamente ho concluso il mio lavoro
                            std::cout << "Sincronizzazione file riuscita." << std::endl;
                        }
                        else{
                            // "KO" si sono verificati errori, il client deve rieffettuare l'invio a partire dal punto 3.1
                            // se si verificano errori riaggiungo il file alla lista dopo avere ricalcolato hash e size
                            std::cout << "Si è verificato un errore nella coerenza dei dati." << std::endl;
                            syncedFile->update_file_data();
                            uploadJobs.put(syncedFile);
                        }
                    } else {
                        // "KO" si sono verificati errori, il client deve rieffettuare l'invio a partire dal punto 3.1
                        // se si verificano errori riaggiungo il file alla lista dopo avere ricalcolato hash e size
                        std::cout << "Si è verificato un errore nella coerenza dei dati." << std::endl;
                        syncedFile->update_file_data();
                        uploadJobs.put(syncedFile);
                    }
                }
                catch (socketException &e1) {
                    std::cout << e1.what() << std::endl;
                    exit(-3);
                }
                    // todo: potrei genarilizzare con una std::runtimeError
                catch (dataException &e2) {
                    std::cout << e2.what() << std::endl;
                    syncedFile->update_file_data();
                    uploadJobs.put(syncedFile);
                }
                catch (filesystemException &e3) {
                    std::cout << e3.what() << std::endl;
                    syncedFile->update_file_data();
                    uploadJobs.put(syncedFile);
                }

            }
        }
        socket.closeConnection();
    }
    catch (std::runtime_error &error) {
        std::cout << error.what() << std::endl;
        exit(-1);
    }
}

void add_to_queue(const std::shared_ptr<SyncedFile>& sfp){
    //todo: gestire sincronizzazione
    sfp->setToSync();
    uploadJobs.put(sfp);
}

void file_watcher(){
    // Create a FileWatcher instance that will check the current folder for changes every 5 seconds
    fw_ptr = std::make_shared<FileWatcher>("/home/stefano/CLionProjects/FileWatcher/test_dir", std::chrono::milliseconds(5000));
    // Start monitoring a folder for changes and (in case of changes)
    // run a user provided lambda function
    fw_ptr->start([] (const std::shared_ptr<SyncedFile>& sfp, FileStatus status) -> void {
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
        add_to_queue(sfp);
    });
}


//void test_f(){
//    Socket socket;
//    socket.connectToServer("127.0.0.1", 6019);
//    SyncedFile sf("/home/stefano/CLionProjects/FileWatcher/test_dir/testdd.txt");
//    std::shared_ptr<SyncedFile> sfp(std::make_shared<SyncedFile>(sf));
//    std::cout << sfp->getJSON() << std::endl;
//    socket.sendFile(sfp);
//    std::optional<std::string> resp = socket.getResp();
//    if(resp.has_value())
//        std::cout << "RESP: " << resp.value() << std::endl;
//    else
//        std::cout << "RESP error" << std::endl;
//}


int main() {
    std::thread t1(upload_to_server);
    file_watcher();
    t1.join();
}