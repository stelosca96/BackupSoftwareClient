//
// Created by stefano on 07/08/20.
//

#ifndef FILEWATCHER_SOCKET_H
#define FILEWATCHER_SOCKET_H


#include <stdio.h>

class Socket {
private:
    int sockfd;
    Socket(int sockfd);
    Socket(const Socket &) = delete; //elimino il costruttore di copia
    friend class ServerSocket; //è friend perché voglio poter chiamare il suo costruttore privato
public:
    Socket &operator=(const Socket &) = delete; //elimino operatore di assegnazione
    Socket();
    ~Socket();
    Socket(Socket &&other) noexcept ;  // costruttore di movimento
    Socket &operator=(Socket &&other) noexcept ; // costruttore di copia per movimento
    ssize_t read(char *buffer, size_t len);
    ssize_t write(const char *buffer, size_t len);
    void connect(struct sockaddr_in *addr, unsigned int len);
};


#endif //FILEWATCHER_SOCKET_H
