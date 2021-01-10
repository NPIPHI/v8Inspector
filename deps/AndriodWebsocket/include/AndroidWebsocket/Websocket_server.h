//
// Created by 16182 on 11/30/2020.
//

#ifndef V8DEBUGGER_WEBSOCKET_SERVER_H
#define V8DEBUGGER_WEBSOCKET_SERVER_H

#include<cstdint>
#include<vector>
#include<string_view>
#include<thread>
#include "../../src/InputStream.h"
#include "../../src/SafeQueue.h"
#include "../../src/SocketServer.h"
#include "Message.h"

class Websocket_server {
public:
    Websocket_server(){};
    std::optional<message> get_message();
    message get_message_blocking();
    void start(int port, std::string subdomain);
    void send_message(message msg);
    void wait_frontend_message();
    void shutdown();
    Websocket_server(const Websocket_server&) = delete;
    Websocket_server(const Websocket_server&&) = delete;
    ~Websocket_server();

private:
    static constexpr const char * SERVER_KEY_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::thread sender_thread;
    std::thread receiver_thread;
    InputStream iStream;
    SafeQueue<message> incoming_messages;
    SafeQueue<std::vector<char>> outgoing_messages;
    int port;
    std::atomic<bool> terminated;
    Socket socket;
    SocketServer server;
    std::string subdomain;
    void receiver_start();
    void sender_start();
    void stream_write(const uint8_t *begin, const uint8_t *end);
    void websocket_connect();
    void handle_http_connect();
    void send_message_http10(std::string_view body);
    void send_message_raw(std::string_view msg);
    std::string hash_key(std::string clientKey);
    static std::string extract_key(std::string_view msg);
    void recconect();
};


#endif //V8DEBUGGER_WEBSOCKET_SERVER_H
