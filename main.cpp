#include <iostream>
#include <thread>
#include "FileWatcher.h"
#include "SyncedFile.h"
#include "UploadJobs.h"
#include "Socket.h"
#include "User.h"

UploadJobs uploadJobs;

void upload_to_server(){
    // quando apro la connessione tcp?
    // una per ogni file => grande spreco di risorse
    // una e la tengo aperta => spreco di risorse a tenere una connessione aperta
    // la apro e la tengo aperta finchè la coda è piena? Può avere senso ma è più complicata da implementare
    // todo: gestire eccezioni ed eventualmente mutua esclusione
    try {
        Socket socket;
        socket.connectToServer("127.0.0.1", 6016);
        User user("ste", "ciao1234");

        // 1. invio le credenziali
        socket.sendJSON(user.getJSON());

        // 2. se il server non risponde OK l'auth non è andata a buon fine e chiudo il programma
        if(socket.getResp() != "OK")
            return;

        // continuo ad estrarre file dalla coda finchè il programma non termina
        while (!uploadJobs.producer_is_ended()){
            std::shared_ptr<SyncedFile> syncedFile = uploadJobs.get();

            // la classe fileJobs ritorna un nullptr se la coda è vuota e il programma deve terminare
            if(syncedFile!= nullptr){
                // 3. invio il json del synced file
                socket.sendJSON(syncedFile->getJSON());
                std::optional<std::string> resp = socket.getResp();
                if(resp.has_value()){
                    // se la risposta è NO invio il file, altrimenti procedo
                    if(resp.value() == "NO"){
                        socket.sendFile(syncedFile);
                        resp = socket.getResp();
                        if(resp.has_value()){
                            // "KO" si sono verificati errori, il client deve rieffettuare l'invio a partire dal punto 3.1
                            // se si verificano errori riaggiungo il file alla lista dopo avere ricalcolato hash e size
                            if(resp.value() == "KO"){
                                syncedFile->update_file_data();
                                uploadJobs.put(syncedFile);
                            }
                        }
                        else {
                            // todo: gestire errore
                            std::cout << "Errore trasmissione" << std::endl;
                        }
                    }
                } else {
                    // todo: gestire errore
                    std::cout << "Errore trasmissione" << std::endl;
                }


            }
        }
        socket.closeConnection();
    } catch (std::exception &exception) {
        std::cout << exception.what() << std::endl;
    }
}

void add_to_queue(const std::shared_ptr<SyncedFile>& sfp){
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
        add_to_queue(sfp);
    });
}


int main() {
    std::thread t1(upload_to_server);
    file_watcher();
    t1.join();
}