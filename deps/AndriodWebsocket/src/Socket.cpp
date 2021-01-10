//
// Created by 16182 on 12/26/2020.
//

#include "Socket.h"

#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <stdexcept>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <cstring>


int Socket::connect(const char *address, int port) {
    int _socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_address{};
    if(_socket < 0){
        throw std::runtime_error("failed to open socket");
    }
    hostent * host = gethostbyname(address);
    if(!host){
        throw std::runtime_error("failed to connect to host");
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    memcpy(&server_address.sin_addr.s_addr, host->h_addr, host->h_length);
    if(::connect(_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0){
        throw std::runtime_error("failed to connect to socket");
    }
    return _socket;
}

Socket::Socket(cSocket socket) {
    _socket = socket.socket;
}

Socket::Socket(const char * address, int port) {
    _socket = connect(address, port);
}

Socket::Socket(int port){
    _socket = connect("127.0.0.1", port);
}

Socket::Socket(Socket&& other) {
    _socket = other._socket;
    other._socket = -1;
}

Socket &Socket::operator=(Socket &&other) noexcept {
    if(_socket != -1) {
        close(_socket);
    }
    _socket = other._socket;
    other._socket = -1;
    return *this;
}


Socket::~Socket() {
    if(_socket != -1){
        close(_socket);
    }
}

int Socket::read_bytes(void *out, int max_len) {
    if(max_len == 0) return 0;
    int read_bytes = read(_socket, out, max_len);
    return read_bytes == 0 ? -1 : read_bytes;
}

void Socket::write_bytes(const void * begin, const void * end) {
    int w = write(_socket, begin, ((char*)end - (char*)begin));
    if(w < 0){
        throw std::runtime_error("write failed");
    }
}

void Socket::shutdown() {
    if(_socket != -1){
        ::shutdown(_socket, SHUT_RDWR);
    }
    _socket = -1;
}