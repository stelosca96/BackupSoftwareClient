//
// Created by stefano on 07/08/20.
//

#include "Socket.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>

Socket::Socket(int sockfd) : sockfd(sockfd) {
    std::cout << "Socket " << sockfd << " created" << std::endl;
}

Socket::Socket(){
    sockfd = ::socket(AF_INET, SOCK_STREAM, 0); //solo tcp ipv4
    if (sockfd < 0) throw std::runtime_error("Cannot create socket");
    std::cout << "Socket" << sockfd << " created" << std::endl;
}
Socket::~Socket() {
    if (sockfd != 0) {
        std::cout << "Socket " << sockfd << " closed" << std::endl;
        ::close(sockfd);
    }
}

Socket::Socket(Socket &&other) noexcept : sockfd(other.sockfd){  // costruttore di movimento
    other.sockfd = 0;
}

Socket& Socket::operator=(Socket &&other) noexcept {   // costruttore di copia per movimento
    if(sockfd !=0) close(sockfd);
    sockfd = other.sockfd;
    other.sockfd = 0;
    return *this;
}

ssize_t Socket::read(char *buffer, size_t len){
    ssize_t res = recv(sockfd, buffer, len, MSG_NOSIGNAL);
    if(res < 0) throw std::runtime_error("Cannot receive from socket");
    return res;
}

ssize_t Socket::write(const char *buffer, size_t len){
    ssize_t res = send(sockfd, buffer, len, MSG_NOSIGNAL);
    if(res < 0) throw std::runtime_error("Cannot write to socket");
    return res;
}

void Socket::connect(struct sockaddr_in *addr, unsigned int len){
    if(::connect(sockfd, reinterpret_cast<struct sockaddr*>(addr), len)!=0)
        throw std::runtime_error("Cannot connect to remote socket");
}
