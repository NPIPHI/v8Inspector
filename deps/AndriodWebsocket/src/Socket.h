//
// Created by 16182 on 12/26/2020.
//

#ifndef V8DEBUGGER_SOCKET_H
#define V8DEBUGGER_SOCKET_H


class Socket {
private:
    static int connect(const char * address, int port);
    struct cSocket {int socket = -1;};
    int _socket;
    Socket(cSocket);
    friend class SocketServer;
public:
    Socket(): _socket(-1){};
    Socket(const char * address, int port);
    Socket(int port);
    Socket(Socket&) = delete;
    Socket& operator=(Socket&) = delete;
    Socket(Socket&& other);
    Socket& operator=(Socket&& other) noexcept;
    ~Socket();

    int read_bytes(void * out, int max_len);
    void write_bytes(const void * begin, const void * end);
    void shutdown();
};


#endif //V8DEBUGGER_SOCKET_H
