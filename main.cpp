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
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;
using boost::asio::ip::tcp;

UploadJobs uploadJobs;
std::shared_ptr<FileWatcher> fw_ptr = nullptr;

void saveMap(){
    if(fw_ptr!= nullptr)
        fw_ptr->saveMap();
}

void uploadToServer(std::unique_ptr<Client> client){
    // continuo ad estrarre file dalla coda finchè il programma non termina
    while (!uploadJobs.producer_is_ended()) {
        std::shared_ptr<SyncedFile> syncedFile = uploadJobs.get();

        // la classe fileJobs ritorna un nullptr se la coda è vuota e il programma deve terminare
        if (syncedFile != nullptr) {
            try {
                // 3. invio il json del synced file
                std::string fileJSON(syncedFile->getJSON());
                std::cout << fileJSON << std::endl;
                client->sendJSON(fileJSON);
                std::string resp = client->getResp();
                if (resp == "OK") {
                    syncedFile->setSynced();
                    saveMap();
                    // se il file è già sul server ho concluso il mio lavoro
                    std::cout << "Sincronizzazione file riuscita." << std::endl;
                } else if (resp == "NO") {
                    // se il file non è già sul server devo inviarlo
                    std::cout << "Il server ha richiesto l'invio del file." << std::endl;
                    client->sendFile(syncedFile);
                    resp = client->getResp();
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
}

// todo: vedere come gestire la mappa dei file
// caso 1: ignorare il problema
// caso 2: ricalcolarla durante il sync => soluzione migliore
        // cancello la mappa
        // per ogni file prima della ok lo aggiungo alla mappa
        // alla end salvo la mappa su file
void sync(std::unique_ptr<Client> client){
    while (true) {
        try {
            std::string mex = client->readString();
            if(mex=="END") {
                break;
            }
            std::shared_ptr<SyncedFile> sfp = std::make_shared<SyncedFile>(mex, true);
            // todo: devo controllare se quel file sul filesystem è uguale
            // todo: cosa succede se il file non è presente, mettere un controllo
            if(sfp->getHash() != SyncedFile::CalcSha256(sfp->getPath())) {
                client->sendResp("NO");
                client->getFile(sfp);
                // il trasferimento deve andare a buon fine, se si verificano problemi
                // lancio un'eccezione e quando la catcho manderò una resp KO

                //todo: questa send resp viene chiamata senza sapere se ci sono stati errori nel trasferimento (?!)
                client->sendResp("OK");
            } else client->sendResp("OK");
        }
            // todo: potrei genarilizzare con una std::runtimeError
        catch (dataException &e1) {
            client->sendResp("KO");
            std::cout << e1.what() << std::endl;
        }
        catch (filesystemException &e2) {
            client->sendResp("KO");
            std::cout << e2.what() << std::endl;
        }
        catch (socketException &e3) {
            // riaggiungo il file su cui si è verificato l'errore alla coda
            throw e3;
        }
    }
}

// todo: caricare impostazioni da file .config
// address, port, directory to watch, cert_path, username, password
void connectServer(
        const std::string& serverAddress,
        short serverPort,
        unsigned sleep_time,
        unsigned timeoutValue,
        const std::string& username,
        const std::string& password,
        const std::string& crtPath,
        const bool modeBackup
){
    // todo: gestire eccezioni ed eventualmente mutua esclusione
    try {
        boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv13);
        ctx.load_verify_file(crtPath);
        while (true) {
            try {
                boost::asio::io_context io_context;
//                tcp::resolver resolver(io_context);
//                auto endpoints = resolver.resolve("127.0.0.1", "9999");
                boost::asio::ip::tcp::endpoint endpoint(
                        boost::asio::ip::address::from_string(serverAddress), serverPort);
                std::unique_ptr<Client> client = std::make_unique<Client>(io_context, ctx, endpoint, timeoutValue);
                client->connect();
                User user(username, password);
                // 1. invio le credenziali
                client->sendJSON(user.getJSON());
                // 2. se il server non risponde OK l'auth non è andata a buon fine e chiudo il programma, ma se ricevo dati diversi da OK, KO, NO
                if (client->getResp() != "OK") {
                    std::cout << "Utente già registrato o password errata." << std::endl;
                    exit(-2);
                }
                std::cout << "Login riuscito" << std::endl;

                // invio la modalità al server
                client->sendMode(modeBackup);
                //attendo la risposta
                if(client->getResp() != "OK"){
                    std::cout << "Errore ricezione modalità." << std::endl;
                    exit(-3);
                }
                if (modeBackup)
                    uploadToServer(std::move(client));
                else {
                    std::cout << "RICEVUTO OK PER SYNC MODE" << std::endl;
                    sync(std::move(client));
                    std::filesystem::remove_all("./temp");
                    break;
                }
//                client.closeConnection();
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

// todo: qui si lavora con le split e le string, non so quanto sia sensato
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
    std::string username, password, path, serverAddress, crtPath;
    short serverPort;
    bool modeBackup;
    unsigned retryTime, timeoutTime, fileRescanTime;
    try{
        pt::ptree root;
        pt::read_json("config.json", root);
        username = root.get_child("username").data();
        password = root.get_child("password").data();
        // modeBackup a true se nel file di config scrivo backup nel campo mode
        modeBackup = root.get_child("mode").data() == "backup";
        path = root.get_child("path").data();
        serverAddress = root.get_child("serverAddress").data();
        crtPath = root.get_child("crtPath").data();
        serverPort = std::stoi(root.get_child("serverPort").data());
        retryTime = std::stoi(root.get_child("retryTime").data());
        timeoutTime = std::stoi(root.get_child("timeoutTime").data());
        fileRescanTime = std::stoi(root.get_child("fileRescanTime").data());

    } catch (std::runtime_error &error) {
        std::cerr << "Errore caricamento file configurazione" << std::endl;
        std::cerr << error.what() << std::endl;
        exit(-1);
    }
    if(modeBackup){
        std::thread t1(connectServer, serverAddress, serverPort, retryTime, timeoutTime, username, password, crtPath, modeBackup);
        file_watcher(username, path, fileRescanTime);
        t1.join();
    } else {
        connectServer(serverAddress, serverPort, retryTime, timeoutTime, username, password, crtPath, modeBackup);
    }
}