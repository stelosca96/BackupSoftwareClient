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
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // In this example we will simply print the certificate's subject name.
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    std::cout << "Verifying " << subject_name << "\n";
    return preverified;
}

Client::Client(boost::asio::io_context& io_context,
               boost::asio::ssl::context& context,
               boost::asio::ip::tcp::endpoint& endpoints)
        : socket_(io_context),
        endpoint_(std::move(endpoints)),
        deadline_(io_context){
    io_context_ = &io_context;
    deadline_.expires_at(boost::posix_time::pos_infin);
}
Client::~Client() {
    socket_.lowest_layer().close();
    // todo: chiudere socket o si chiude da sola?
}

//Client::Client(Client &&other) noexcept : socket_(std::move(other.socket_)){  // costruttore di movimento
//}

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
//    std::cout << "chiudo la socket? " << !io_context_->stopped() << std::endl;

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
    // todo: gestire eccezioni
    boost::system::error_code error;
    socket_.async_connect(endpoint_,
            [&](const boost::system::error_code& result_error)
            {
                error = result_error;
            });
    run(this->timeout_value);
    // Se scade il timeout lancio un'eccezione
    if (error){
        throw socketException(error.message());
    }
//    boost::asio::connect(socket_.lowest_layer(),this->endpoints_);
    std::cout << "Connected to: " << this->endpoint_ << std::endl;

}

void Client::sendJSON(const std::string &JSON) {
    this->sendString(JSON);
}

void Client::sendString(const std::string& str_b){
    std::string buffer = str_b + "\\\n";
    std::cout << "Buffer inviato: " << buffer << std::endl;

    boost::system::error_code error;
    boost::asio::async_write(socket_, boost::asio::buffer(buffer),
                             [&](const boost::system::error_code& result_error,
                                 std::size_t size)
                             {
                                 error = result_error;
//                                 std::cout << "Size inviata: " << size << std::endl;

                             });

    run(this->timeout_value);
    // Se scade il timeout lancio un'eccezione
    if (error){
        throw socketException("timeout expired " + error.message());
    }
//    std::cout << boost::asio::write(this->socket_, boost::asio::buffer(buffer, buffer.size())) << std::endl;
}

void Client::sendFile(const std::shared_ptr<SyncedFile>& syncedFile) {
    char buffer[N];
    ssize_t size_read = 0;

    const bool isFile = syncedFile->isFile() && syncedFile->getFileStatus()!=FileStatus::erased;
    std::ifstream file_to_send(syncedFile->getPath(), std::ios::binary);

    std::cout << "isFile: " << isFile << std::endl;

    if(isFile) {
        // todo: se apro il file l'os dovrebbe impedire la modifica agli altri programmi?
        // todo: se il file non esiste più cosa faccio? Lo marchio come eliminato o ignoro il problema visto che alla prossima scansione si accorgerà che il file è stato eliminato?
//        file_to_send = std::ifstream(syncedFile->getPath(), std::ios::binary);
        // se il file non esiste ignoro il problema ed esco, alla prossima scansione del file system verrà notata la sua assenza
        std::cout << "Invio il file" << std::endl;
        // leggo e invio il file
        std::cout << "Size to read: " << syncedFile->getFileSize() << std::endl;
        ssize_t size = 1;
        // ciclo finchè non ho letto tutto il file
        while (size_read < syncedFile->getFileSize() && size>0) {
            file_to_send.read(reinterpret_cast<char *>(&buffer), sizeof(buffer));
            size = file_to_send.gcount();
            boost::system::error_code error;
            boost::asio::async_write(socket_, boost::asio::buffer(buffer, size),
                                     [&](const boost::system::error_code& result_error,
                                         std::size_t bytes_transferred)
                                     {
                                         error = result_error;
//                                         std::cout << "Size inviata: " << bytes_transferred << std::endl;

                                     });
            run(this->timeout_value);
            // Se scade il timeout lancio un'eccezione
            if (error){
                throw socketException("timeout expired " + error.message());
            }
//            std::cout << "Size inviata: " << size << std::endl;

//            boost::asio::write(this->socket_, boost::asio::buffer(buffer, size));
            size_read += size;
//            std::cout << "Size read: " << size_read << std::endl;
        }
        std::cout << "File chiuso" << std::endl;
        file_to_send.close();
    }
}

std::string Client::readString(){
    // todo: boost::system::system_error	Thrown on failure.
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

//    // uso il terminatore \ a capo per la ricezione dopo averlo aggiunto nell'invio
//    boost::asio::read_until(this->socket_,buf,"\\\n");

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
    std::cout << value << std::endl;
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
