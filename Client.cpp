//
// Created by stefano on 07/08/20.
//

#include "Client.h"
#include "exceptions/socketException.h"
#include "exceptions/dataException.h"
#include "exceptions/filesystemException.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <fstream>
#include <utility>
#include <boost/asio.hpp>

bool Client::verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx){
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer.

    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    std::cout << "Verifying " << subject_name << "\n";
    return preverified;
}

Client::Client(boost::asio::io_context& io_context,
               boost::asio::ssl::context& context,
               boost::asio::ip::tcp::endpoint& endpoints,
               unsigned timeout_value)
        : socket_(io_context, context),
        endpoint_(std::move(endpoints)),
        deadline_(io_context),
        timeout_value(timeout_value){
    io_context_ = &io_context;
    deadline_.expires_at(boost::posix_time::pos_infin);
    socket_.set_verify_mode(boost::asio::ssl::verify_peer);
    socket_.set_verify_callback(
            std::bind(&Client::verify_certificate, this, std::placeholders::_1, std::placeholders::_2)
    );

}
Client::~Client() {
}

void Client::run(unsigned t){
    std::chrono::seconds timeout(t);
    // Restart the io_context, as it may have been left in the "stopped" state
    // by a previous operation.
    io_context_->restart();

    // Block until the asynchronous operation has completed, or timed out. If
    // the pending asynchronous operation is a composed operation, the deadline
    // applies to the entire operation, rather than individual operations on
    // the socket.
    io_context_->run_for(timeout);

    // If the asynchronous operation completed successfully then the io_context
    // would have been stopped due to running out of work. If it was not
    // stopped, then the io_context::run_for call must have timed out.
    if (!io_context_->stopped())
    {
        std::cout << "chiudo la socket" << std::endl;
        // Close the socket to cancel the outstanding asynchronous operation.
        socket_.lowest_layer().close();

        // Run the io_context again until the operation completes.
        io_context_->run();
    }
}

void Client::connect(){
    boost::system::error_code error;
    socket_.lowest_layer().async_connect(endpoint_,
            [&](const boost::system::error_code& result_error)
            {
                error = result_error;
            });
    run(this->timeout_value);
    // Se scade il timeout lancio un'eccezione
    if (error){
        throw socketException(error.message());
    }
    std::cout << "Connected to: " << this->endpoint_ << std::endl;
    socket_.async_handshake(
            boost::asio::ssl::stream_base::client,
            [&](const boost::system::error_code& result_error)
            {
                error = result_error;
            });

    run(this->timeout_value);
    // Se scade il timeout lancio un'eccezione
    if (error){
        throw socketException("timeout expired " + error.message());
    }
    std::cout << "Handshake OK" << std::endl;
}

void Client::sendJSON(const std::string &JSON) {
    this->sendString(JSON);
}

void Client::sendResp(const std::string& resp) {
    this->sendString(resp);
}

void Client::sendString(const std::string& str_b){
    std::string buffer = str_b + "\\\n";
//    std::cout << "Buffer inviato: " << buffer << std::endl;

    boost::system::error_code error;
    boost::asio::async_write(socket_, boost::asio::buffer(buffer),
                             [&](const boost::system::error_code& result_error,
                                 std::size_t size)
                             {
                                 error = result_error;
                             });

    run(this->timeout_value);
    // Se scade il timeout lancio un'eccezione
    if (error){
        throw socketException("timeout expired " + error.message());
    }
}

void Client::sendFile(const std::shared_ptr<SyncedFile>& syncedFile) {
    char buffer[N];
    ssize_t size_read = 0;

    const bool isFile = syncedFile->isFile() && syncedFile->getFileStatus()!=FileStatus::erased;
    std::ifstream file_to_send(syncedFile->getPath(), std::ios::binary);

    if(isFile) {
        // se il file non esiste ignoro il problema ed esco, alla prossima scansione del file system verrà notata la sua assenza
        std::cout << "Invio il file" << std::endl;
        // leggo e invio il file
//        std::cout << "Size to read: " << syncedFile->getFileSize() << std::endl;
        ssize_t size = 1;
        // ciclo finchè non ho letto tutto il file
        while (size_read < syncedFile->getFileSize() && size>0) {
            // todo: provare senza cast
            file_to_send.read(reinterpret_cast<char *>(&buffer), sizeof(buffer));
            size = file_to_send.gcount();
            boost::system::error_code error;
            boost::asio::async_write(socket_, boost::asio::buffer(buffer, size),
                                     [&](const boost::system::error_code& result_error,
                                         std::size_t bytes_transferred)
                                     {
                                         error = result_error;
                                     });
            run(this->timeout_value);
            // Se scade il timeout lancio un'eccezione
            if (error){
                throw socketException("timeout expired " + error.message());
            }
            size_read += size;
        }
        std::cout << "File chiuso" << std::endl;
        file_to_send.close();
    }
}

std::string Client::readString(){
    // boost::system::system_error	Thrown on failure.
    boost::asio::streambuf buf;
    boost::system::error_code error;
    std::size_t n = 0;

    // uso il terminatore \ a capo per la ricezione dopo averlo aggiunto nell'invio
    boost::asio::async_read_until(this->socket_,
                                 buf,
                                  "\\\n",
                                  [&](const boost::system::error_code& result_error,
                                      std::size_t result_n)
                                  {
                                      error = result_error;
                                      n = result_n;
                                  });
    run(this->timeout_value);
    // Se scade il timeout lancio un'eccezione
    if (error){
        throw socketException("timeout expired " + error.message());
    }

    std::string data = boost::asio::buffer_cast<const char*>(buf.data());
    // rimuovo i terminatori quindi gli ultimi due caratteri
    return data.substr(0, data.length()-2);
}

// attendo la ricezione di uno stato tra {OK, NO, KO}
std::string Client::getResp() {
    std::string value = this->readString();
//    std::cout << value << std::endl;
    // controllo se il valore ricevuto è tra quelli ammissibili, se non lo è ritorno un'eccezione
    if(value != "OK" && value != "NO" && value != "KO"){
        std::cout << "Not valid resp" << std::endl;
        throw dataException("Not valid response");
    }
    return value;
}

void Client::closeConnection() {
    this->socket_.lowest_layer().close();
}

// genero un nome temporaneo per il file dato da tempo corrente + id thread
std::string Client::tempFilePath(){
    std::filesystem::create_directory("temp/");
    auto th_id = std::this_thread::get_id();
    std::time_t t = std::time(nullptr);   // get time now
    std::stringstream ss;
    ss << "temp/" << th_id << "-" << t;
    return ss.str();
}

void Client::moveFile(const std::shared_ptr<SyncedFile>& sfp, const std::string& tempPath){
    // todo: se usiamo i percorsi relativi o no cambia come gestire la posizione di salvataggio
    std::filesystem::path user_path("./");
    user_path += username;
    std::filesystem::path path(sfp->getPath());
    user_path += path;
    std::filesystem::create_directories(user_path.parent_path());
    std::filesystem::copy_file(tempPath, user_path, std::filesystem::copy_options::overwrite_existing);
    if(std::filesystem::is_directory(tempPath) || std::filesystem::is_regular_file(tempPath))
        std::filesystem::remove(tempPath);
    std::cout << user_path.parent_path() << std::endl;
}

void Client::getFile(std::shared_ptr<SyncedFile> sfp) {
    std::string tempPath(tempFilePath());
    auto file = std::ofstream(tempPath, std::ios::binary);
    if(!file.is_open())
        throw filesystemException("Can't open file: " + tempPath);
    ssize_t size_recv = 0;
    while(sfp->getFileSize()>size_recv){
        const ssize_t size_left = sfp->getFileSize()-size_recv;

        // scelgo la dimensione massima del buffer
        const ssize_t buff_size = size_left<N ? size_left: N;
        boost::system::error_code error;
        boost::asio::async_read(
                socket_,
                boost::asio::buffer(data_, buff_size),
                [&](const boost::system::error_code& result_error,
                    std::size_t bytes_transferred){
                        error = result_error;
                        size_recv+=bytes_transferred;
                        // todo: non so se farlo nella callback o dopo il run
                    if(!error){
                        // se non si sono verificati errori scrivo il file
                        data_[bytes_transferred] = '\0';
                        file.write(data_, bytes_transferred);
                    } else {
                        data_[bytes_transferred] = '\0';
                        file.close();
                        std::cout << "Errore ricezione file" << std::endl;
                        std::filesystem::remove(tempPath);
                        throw dataException("File recv error");
                        // altrimenti gestisco l'errore
                    }
                });
        moveFile(sfp, tempPath);
        run(this->timeout_value);
    }
    std::cout << "File chiuso" << std::endl;
    file.close();
}

void Client::sendMode(bool mode) {
    //BACKUP MODE
    if(mode)
        sendString("MODE_BACK");
    else
        sendString("MODE_SYNC");
}
