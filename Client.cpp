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


//Client::Client(int sock_fd) : socket_fd(sock_fd) {
//    std::cout << "Client " << sock_fd << " created" << std::endl;
//}

Client::Client(boost::asio::io_context& io_context,
               boost::asio::ssl::context& context,
               tcp::resolver::iterator& endpoints)
        : socket_(io_context, context), endpoints_(std::move(endpoints)){
    socket_.set_verify_mode(boost::asio::ssl::verify_peer);
    socket_.set_verify_callback(
            std::bind(&Client::verify_certificate, this, std::placeholders::_1, std::placeholders::_2)
    );

}
Client::~Client() {
    // todo: chiudere socket
}

//Client::Client(Client &&other) noexcept : socket_(std::move(other.socket_)){  // costruttore di movimento
//}

Client& Client::operator=(Client &&other) noexcept {   // costruttore di copia per movimento
    if(socket_fd != 0) close(socket_fd);
    socket_fd = other.socket_fd;
    other.socket_fd = 0;
    return *this;
}

void Client::connect(){
    // todo: gestire eccezioni
    boost::asio::connect(socket_.lowest_layer(),this->endpoints_);
    std::cout << "Connected to: " << this->endpoints_->endpoint() << std::endl;
    socket_.handshake(boost::asio::ssl::stream_base::client);
    std::cout << "Handshake OK" << std::endl;

}

void Client::sendJSON(const std::string &JSON) {
    this->sendString(JSON);
}

void Client::sendString(const std::string& str_b){
    std::string buffer = str_b + "\\\n";
    std::cout << "Buffer inviato: " << buffer << std::endl;
    std::cout << boost::asio::write(this->socket_, boost::asio::buffer(buffer, buffer.size())) << std::endl;
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
        unsigned long size = 1;
        // ciclo finchè non ho letto tutto il file
        while (size_read < syncedFile->getFileSize() && size>0) {
            file_to_send.read(reinterpret_cast<char *>(&buffer), sizeof(buffer));
            size = file_to_send.gcount();
            // todo: boost::system::system_error	Thrown on failure.
            // todo: timeout
            boost::asio::write(this->socket_, boost::asio::buffer(buffer, size));
            size_read += size;
            std::cout << "Size read: " << size_read << std::endl;
        }
        std::cout << "File chiuso" << std::endl;
        file_to_send.close();
    }
}

std::string Client::readString(){
    // todo: boost::system::system_error	Thrown on failure.
    // todo: timeout
    boost::asio::streambuf buf;

    // uso il terminatore \ a capo per la ricezione dopo averlo aggiunto nell'invio
    boost::asio::read_until(this->socket_,buf,"\\\n");

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
    // todo: scoprire come chiudere
    this->socket_.shutdown();
}
