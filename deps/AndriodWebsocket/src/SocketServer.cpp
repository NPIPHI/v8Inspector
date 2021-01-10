//
// Created by 16182 on 12/26/2020.
//

#include "SocketServer.h"
#include <unistd.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

SocketServer::SocketServer(int port) {
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket < 0){
        throw std::runtime_error("error opening socket");
    }
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if(bind(_socket, (sockaddr*)(&server_address), sizeof(server_address)) < 0){
        throw std::runtime_error("error binding socket with port=" + std::to_string(port));
    }

    listen(_socket, 5);
}

SocketServer::SocketServer(SocketServer &&other) {
    _socket = other._socket;
    other._socket = -1;
}

SocketServer& SocketServer::operator=(SocketServer &&other) {
    if(_socket != -1) close(_socket);
    _socket = other._socket;
    other._socket = -1;
    return *this;
}

SocketServer::~SocketServer() {
    if(_socket != -1){
        close(_socket);
    }
}


Socket SocketServer::accept_socket() {
    sockaddr_in client_address{};
    socklen_t client_len = sizeof(client_address);
    int accepted = accept(_socket, (sockaddr *) &client_address, &client_len);
    if(accepted < 0){
        throw std::runtime_error("error accepting socket");
    }
    return Socket(Socket::cSocket{accepted});
}