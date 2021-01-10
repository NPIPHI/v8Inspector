//
// Created by 16182 on 12/26/2020.
//

#ifndef V8DEBUGGER_SOCKETSERVER_H
#define V8DEBUGGER_SOCKETSERVER_H

#include "Socket.h"

class SocketServer {
private:
    int _socket;
public:
    Socket accept_socket();
    SocketServer(): _socket(-1){};
    SocketServer(int port);
    SocketServer(SocketServer&) = delete;
    SocketServer(SocketServer&& other);
    SocketServer& operator=(SocketServer&& other);
    ~SocketServer();
};

#endif //V8DEBUGGER_SOCKETSERVER_H
