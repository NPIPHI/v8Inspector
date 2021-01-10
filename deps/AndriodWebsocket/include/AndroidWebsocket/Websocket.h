//
// Created by 16182 on 1/1/2021.
//

#ifndef V8DEBUGGER_WEBSOCKET_H
#define V8DEBUGGER_WEBSOCKET_H

#include "Message.h"
#include "../../src/Socket.h"
#include "../../src/InputStream.h"
#include <optional>
#include <string>

class Websocket {
public:
    Websocket(std::string address, int port, std::string subdomain);
    void send(message msg);
    message read_blocking();
    ~Websocket() = default;
private:
    Socket socket;
    InputStream iStream;
};


#endif //V8DEBUGGER_WEBSOCKET_H
