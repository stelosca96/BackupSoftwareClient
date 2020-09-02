//
// Created by stefano on 07/08/20.
//

#ifndef FILEWATCHER_CLIENT_H
#define FILEWATCHER_CLIENT_H


#include <cstdio>
#include <string>
#include <memory>
#include <optional>
#include "SyncedFile.h"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

using boost::asio::ip::tcp;

class Client {
private:
    //todo: mettere una dimensione del buffer ragionevole
    const static int N = 10240;
    int socket_fd;
    const static int timeout_value = 20;
    boost::asio::ssl::stream<tcp::socket> socket_;
    tcp::resolver::iterator endpoints_;
    std::string username;
    void sendString(const std::string &str);
    bool verify_certificate(bool preverified, boost::asio::ssl::verify_context &ctx);
    std::string readString();

public:
    Client &operator=(const Client &) = delete; //elimino operatore di assegnazione
    Client(boost::asio::io_context& io_context,
           boost::asio::ssl::context& context,
           tcp::resolver::iterator& iterator);
    ~Client();
    Client(const Client &) = delete; //elimino il costruttore di copia
//    Client(Client &&other) noexcept ;  // costruttore di movimento

    Client &operator=(Client &&other) noexcept ; // costruttore di copia per movimento
    void sendFile(const std::shared_ptr<SyncedFile>& syncedFile);
    void connect();

    void closeConnection();
    std::string getResp();

    void sendJSON(const std::string& JSON);



};


#endif //FILEWATCHER_CLIENT_H