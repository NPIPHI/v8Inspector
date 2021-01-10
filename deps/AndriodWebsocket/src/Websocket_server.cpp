//
// Created by 16182 on 11/30/2020.
//

#include <string>
#include <optional>
#include <utility>
#include "../include/AndroidWebsocket/Websocket_server.h"
#include "Frame.h"
#include "message_encoder.h"
#include "../include/AndroidWebsocket/Message.h"
#include "Sha1.h"
#include "Base64.h"

Websocket_server::~Websocket_server() {
    terminated = true;
    outgoing_messages.enqueue({});//wake up the sender thread so it can terminate itself
    socket.shutdown();//wake up the receiver thread so it can terminate itself
    receiver_thread.join();
    sender_thread.join();
}

void Websocket_server::stream_write(const uint8_t *begin, const uint8_t *end) {
    socket.write_bytes(begin, end);
}

void Websocket_server::start(int port, std::string subdomain) {
    terminated = false;
    this->port = port;
    this->subdomain = std::move(subdomain);
    server = SocketServer(port);
    receiver_thread = std::thread([&]{receiver_start();});
    sender_thread = std::thread([&]{sender_start();});
}


void Websocket_server::send_message(message msg) {
    auto encoded = message_encoder::encode(msg);
    outgoing_messages.enqueue({encoded.begin().base(), encoded.end().base()});
}

void Websocket_server::websocket_connect() {
    handle_http_connect();
}

void Websocket_server::handle_http_connect() {
    while(true) {
        std::vector<uint8_t> data;
        while(true){
            auto maybe_data = iStream.read();
            if(maybe_data.has_value()){
                data = std::move(*maybe_data);
                break;
            } else {
                recconect();
            }
        }
        std::string_view msg = {(const char *)data.data(), data.size()};
        if (msg.rfind("GET /json HTTP/1.1", 0) == 0) { //starts with
            auto body = "[ {\n"
                        "  \"description\": \"android embedded v8 inspector\",\n"
                        "  \"id\": \"inspector\",\n"
                        "  \"title\": \"android embedded v8\",\n"
                        "  \"type\": \"v8\",\n"
                        "  \"webSocketDebuggerUrl\": \"ws://127.0.0.1:" + std::to_string(port) + "/" + subdomain + "\"\n"
                        "} ]";
            send_message_http10(body);
        } else if (msg.rfind("GET /json/version HTTP/1.1") == 0) {
            auto body = "{\n"
                        "  \"Browser\": \"v8/8.3.110.9\",\n"
                        "  \"Protocol-Version\": \"1.0\"\n"
                        "}";
            send_message_http10(body);
        } else if (msg.rfind("GET /"+subdomain+" HTTP/1.1", 0) == 0) {
            auto serverKey = hash_key(extract_key(msg));
            auto header = "HTTP/1.1 101 Switching Protocols\r\n"
                          "Upgrade: websocket\r\n"
                          "Connection: Upgrade\r\n"
                          "Sec-WebSocket-Accept: " + serverKey + "\r\n\r\n";
            send_message_raw(header);
            break;
        } else {
            throw std::runtime_error("unexpected message: " + std::string(msg));
        }
    }
}

void Websocket_server::send_message_http10(std::string_view body) {
    std::string header = "HTTP/1.0 200 OK\n"
                         "Content-Type: application/json; charset=UTF-8\n"
                         "Cache-Control: no-cache\n"
                         "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
    send_message_raw(header);
    send_message_raw(body);
}

std::string Websocket_server::hash_key(std::string clientKey) {
    auto sha = SHA1();
    sha.update(clientKey + SERVER_KEY_GUID);
    auto sha_str = sha.final();
    auto b64 = base64::encode(sha_str.begin(), sha_str.end());
    return {b64.begin(), b64.end()};
}

std::string Websocket_server::extract_key(std::string_view msg) {
    size_t begin = msg.find("Sec-WebSocket-Key: ") + strlen("Sec-WebSocket-Key: ");
    size_t end = msg.find("\r\n", begin);
    return {msg.data() + begin, msg.data() + end};
}

void Websocket_server::recconect() {
    socket = server.accept_socket();
}

void Websocket_server::send_message_raw(std::string_view msg) {
    outgoing_messages.enqueue({(const uint8_t *) msg.data(), (const uint8_t *) (msg.data() + msg.size())});
}

void Websocket_server::wait_frontend_message() {
    message msg;
    Frame decoder(1<<16);
    while(true) {
        decoder.read(iStream);
        msg.append(decoder.begin(), decoder.end());
        if(decoder.is_binary()){
            msg.set_type(MSG_TYPE::BINARY);
        }
        if (decoder.finished()) {
            incoming_messages.enqueue(msg);
            break;
        }
        if (decoder.connection_closed()){
            terminated = true;
            break;
        }
    }
}

void Websocket_server::receiver_start() {
    socket = server.accept_socket();
    iStream = InputStream(&socket);
    websocket_connect();

    while(!terminated){
        wait_frontend_message();
    }
}

void Websocket_server::sender_start() {
    while(!terminated){
        auto msg = outgoing_messages.blocking_dequeue();
        if(!terminated){
            stream_write((const uint8_t*)msg.begin().base(), (const uint8_t*)msg.end().base());
        }
    }
}

std::optional<message> Websocket_server::get_message() {
    return incoming_messages.dequeue();
}

void Websocket_server::shutdown() {
    terminated = true;
}

message Websocket_server::get_message_blocking() {
    return incoming_messages.blocking_dequeue();
}
