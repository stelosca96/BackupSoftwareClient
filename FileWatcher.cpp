//
// Created by stefano on 06/08/20.
//

#include <thread>
#include <iostream>
#include "FileWatcher.h"
#include "SyncedFile.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <utility>

namespace pt = boost::property_tree;

// Check if "paths_" contains a given key
// If your compiler supports C++20 use paths_.contains(key) instead of this function
bool FileWatcher::contains(const std::string &key) {
    auto el = this->files_to_watch.find(key);
    return el != this->files_to_watch.end();
}

FileWatcher::FileWatcher(std::string username, std::string pathToWatch, const std::chrono::duration<int, std::milli> &delay)
        : path_to_watch(std::move(pathToWatch)), delay(delay), username(std::move(username)) {
    // Keep a record of files from the base directory and their last modification time
    // si potrebbe sostituire il valore di ultima modifica con l'hash code
    // todo: l'inizializzazione andrà fatta leggendo i file della mappa da un db
//    for(auto &file : std::filesystem::recursive_directory_iterator(path_to_watch)) {
//        files_to_watch[file.path().string()] = std::make_shared<SyncedFile>(file.path().string());
//    }
    try {
        loadMap();
    } catch (std::runtime_error &error) {
        std::cout << error.what() << std::endl;
        // se si verifica un problema nella lettura del file ignoro il problema, alla prima scansione invierà
        // tutti i file della cartella e se già presenti riceverò un OK per i file già sincronizzati
        saveMap();
        // salvo una mappa vuota
    }
}

void FileWatcher::start(const std::function<void(std::shared_ptr<SyncedFile>, FileStatus)> &action) {
    this->running = true;
    // uso un unique lock perchè effettuo modifiche sulla mappa
    std::unique_lock map_lock(map_mutex, std::defer_lock);
    while(running) {
        // Wait for "delay" milliseconds
        std::this_thread::sleep_for(this->delay);
        map_lock.lock();
        auto it = files_to_watch.begin();
        while (it != files_to_watch.end()) {
            if (!std::filesystem::exists(it->first)) {
                SyncedFile sf(it->first, FileStatus::erased);
                action(std::make_shared<SyncedFile>(sf), FileStatus::erased);
                it = files_to_watch.erase(it);
            }
            else
                it++;
        }
        // Check if a file was created or modified
        for(auto &file : std::filesystem::recursive_directory_iterator(path_to_watch)) {
            auto current_file_last_write_time = std::filesystem::last_write_time(file);
            // File creation
            if(!contains(file.path().string())) {
                // non ho bisogno di gestire la sincronizzazione perchè la mappa è usata solo da un thread e ogni volta che trovo un nuovo file creo un nuovo puntatore
                std::shared_ptr<SyncedFile> sfp = std::make_shared<SyncedFile>(file.path().string(), FileStatus::created);
                this->files_to_watch[file.path().string()] = sfp;
                action(sfp, FileStatus::created);
                // File modification
            }
            else {
                // controllo se due percorsi uguali hanno hash diverso il file è stato modificato
                if(files_to_watch[file.path().string()]->getHash() != SyncedFile::CalcSha256(file.path().string())) {
                    // todo: qua invece devo gestire la sincronizzazione, perchè il puntatore potrebbe essere conosciuto da più thread
                    files_to_watch[file.path().string()]->update_file_data();
                    action(files_to_watch[file.path().string()], FileStatus::modified);
                }
            }
        }
        map_lock.unlock();
    }
}

void FileWatcher::saveMap() {
    pt::ptree pt;
    // accedo alla mappa solo in lettura, quindi uso uno shared
//    std::cout << "Provo a prendere il lock per salvare la mappa" << std::endl;
    std::shared_lock map_lock(map_mutex);
    for(auto const& [key, val] : files_to_watch){
//        std::cout << val->getPath() << std::endl;
        auto map_value = val->getMapValue();
        // todo: gestire sincronizzazione, sto usando l'oggetto syncedFile in due thread contemporaneamente
        if(map_value.has_value())
            pt.push_back(map_value.value());
    }
    map_lock.unlock();
//    std::cout << "Rilascio il lock per salvare la mappa" << std::endl;
    pt::json_parser::write_json("synced_maps.json", pt);
}

void FileWatcher::loadMap() {
    pt::ptree root;
    pt::read_json("synced_maps.json", root);
    std::cout << "File loaded :" << std::endl;
    // questa funzione è usata solo durante il caricamento iniziale della mappa,
    // quindi non dovrebbero esserci problemi di sincronizzazione, perchè il thread di upload è bloccato sul .get()
    // quindi non può chiamare la saveMap() che è l'unica funzione che espone esternamente la mappa, quindi il lock
    // potrebbe essere inutile, ma nel caso si chiamasse la funzione in altri momenti il lock diventa fondamentale
    // todo: valutare se togliere lock
    std::unique_lock map_lock(map_mutex);
    for(const auto& p: root){
        std::stringstream ss;
        pt::json_parser::write_json(ss, p.second);
        this->files_to_watch[p.first] = std::make_shared<SyncedFile>(p.first, ss.str());
        this->files_to_watch[p.first]->setSynced();
        std::cout << p.first << std::endl;
    }
}
