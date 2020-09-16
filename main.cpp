#include <iostream>
#include <thread>
#include "FileWatcher.h"
#include "SyncedFile.h"
#include "UploadJobs.h"
#include "Client.h"
#include "User.h"
#include "exceptions/dataException.h"
#include "exceptions/socketException.h"
#include "exceptions/filesystemException.h"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <vector>

using boost::asio::ip::tcp;

UploadJobs uploadJobs;
std::shared_ptr<FileWatcher> fw_ptr = nullptr;

void saveMap(){
    if(fw_ptr!= nullptr)
        fw_ptr->saveMap();
}

// todo: caricare impostazioni da file .config
// address, port, directory to watch, cert_path, username, password
void upload_to_server(
        const std::string& serverAddress,
        short serverPort,
        unsigned sleep_time,
        unsigned timeoutValue,
        const std::string& username,
        const std::string& password,
        const std::string& crtPath
){
    // todo: gestire eccezioni ed eventualmente mutua esclusione
    try {
        boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
        ctx.load_verify_file(crtPath);
        while (true) {
            try {
                boost::asio::io_context io_context;
                tcp::resolver resolver(io_context);
//                auto endpoints = resolver.resolve("127.0.0.1", "9999");
                boost::asio::ip::tcp::endpoint endpoint(
                        boost::asio::ip::address::from_string(serverAddress), serverPort);
                Client client(io_context, ctx, endpoint, timeoutValue);
                client.connect();
                User user(username, password);
                // 1. invio le credenziali
                client.sendJSON(user.getJSON());
                // 2. se il server non risponde OK l'auth non è andata a buon fine e chiudo il programma
                if (client.getResp() != "OK") {
                    std::cout << "User not valid." << std::endl;
                    exit(-2);
                }
                std::cout << "Login riuscito" << std::endl;

                // continuo ad estrarre file dalla coda finchè il programma non termina
                while (!uploadJobs.producer_is_ended()) {
                    std::shared_ptr<SyncedFile> syncedFile = uploadJobs.get();

                    // la classe fileJobs ritorna un nullptr se la coda è vuota e il programma deve terminare
                    if (syncedFile != nullptr) {
                        try {
                            // 3. invio il json del synced file
                            std::string fileJSON(syncedFile->getJSON());
                            std::cout << fileJSON << std::endl;
                            client.sendJSON(fileJSON);
                            std::string resp = client.getResp();
                            if (resp == "OK") {
                                syncedFile->setSynced();
                                saveMap();
                                // se il file è già sul server ho concluso il mio lavoro
                                std::cout << "Sincronizzazione file riuscita." << std::endl;
                            } else if (resp == "NO") {
                                // se il file non è già sul server devo inviarlo
                                std::cout << "Il server ha richiesto l'invio del file." << std::endl;
                                client.sendFile(syncedFile);
                                resp = client.getResp();
                                if (resp == "OK") {
                                    syncedFile->setSynced();
                                    saveMap();
                                    // se il file è stato caricato correttamente ho concluso il mio lavoro
                                    std::cout << "Sincronizzazione file riuscita." << std::endl;
                                } else {
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
                            // todo: potrei genarilizzare con una std::runtimeError
                        catch (dataException &e1) {
                            std::cout << e1.what() << std::endl;
                            syncedFile->update_file_data();
                            uploadJobs.put(syncedFile);
                        }
                        catch (filesystemException &e2) {
                            std::cout << e2.what() << std::endl;
                            syncedFile->update_file_data();
                            uploadJobs.put(syncedFile);
                        }
                        catch (socketException &e3) {
                            // riaggiungo il file su cui si è verificato l'errore alla coda
                            syncedFile->update_file_data();
                            uploadJobs.put(syncedFile);
                            throw e3;
                        }
                    }
                }
                client.closeConnection();
            }
            catch (socketException &e1) {
                // in caso di timeout o errori socket la chiudo e ricreo la connessione
                std::cout << e1.what() << std::endl;
                std::cout << "Ritento la connessione dopo l'errore" << std::endl;
                sleep(sleep_time);

            }
        }
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

std::filesystem::path get_absolute_path(const std::string& path){
    auto p = std::filesystem::absolute(path).string();
    std::vector<std::string> split_path;
    boost::split(split_path, p, boost::is_any_of("\\,/"));

    int count = 0;
    std::string out;
    for(auto start = split_path.rbegin(); start != split_path.rend(); start++){
        if(*start == "..") {
            count++;
            continue;
        }
        else if(count > 0){
            for(; count > 0; count--, start++);
        }

        out = *start + "/" + out;
    }

    return std::filesystem::path(out);
}

void file_watcher(std::string& username, std::string& path, unsigned fileRescanTime){
    // Create a FileWatcher instance that will check the current folder for changes every 5 seconds
//    fw_ptr = std::make_shared<FileWatcher>("/home/stefano/CLionProjects/FileWatcher/test_dir", std::chrono::milliseconds(5000));
    std::string abs_path = get_absolute_path(path).string();
    fw_ptr = std::make_shared<FileWatcher>(username, abs_path, std::chrono::seconds(fileRescanTime));
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


int main() {
    std::string username("stefano");
    std::string password("ciao1234");
    std::string path("../test_dir");
    std::string serverAddress("127.0.0.1");
    std::string crtPath("../cert/user.crt");
    short serverPort = 9999;
    unsigned retryTime = 30;
    unsigned timeoutTime = 2;
    unsigned fileRescanTime = 5;

    std::thread t1(upload_to_server, serverAddress, serverPort, retryTime, timeoutTime, username, password, crtPath);
    file_watcher(username, path, fileRescanTime);
    t1.join();
}