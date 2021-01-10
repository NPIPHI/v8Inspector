//
// Created by 16182 on 1/1/2021.
//

#include "../include/AndroidWebsocket/Websocket.h"
#include "message_encoder.h"
#include "Socket.h"
#include "Frame.h"


static std::string upgrade_message(std::string address, int port, std::string subdomain){
    return "GET /" + subdomain + " HTTP/1.1\n"
           "Host: " + address + ":" + std::to_string(port) + "\n"
           "Connection: Upgrade\n"
           "Pragma: no-cache\n"
           "Cache-Control: no-cache\n"
           "Upgrade: websocket\n"
           "Sec-WebSocket-Version: 13\n"
           "Sec-WebSocket-Key: JRczrRXy/iIOBuqNsPm4mg==\n"
           "Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\n\n";
}

Websocket::Websocket(std::string address, int port, std::string subdomain) {
    socket = Socket(address.c_str(), port);
    auto msg = upgrade_message(address, port, subdomain);
    socket.write_bytes(msg.begin().base(), msg.end().base());
    char buff[1024];
    int len = socket.read_bytes(buff, 1024);
    std::string response = {buff, buff + len};
    if(response.rfind("HTTP/1.1 101 Switching Protocols", 0) != 0) throw std::runtime_error("bad upgrade response");
    auto key_index = response.find("Sec-WebSocket-Accept: ") + strlen("Sec-WebSocket-Accept: ");
    auto key = response.substr(key_index, 28);
    if(key != "XEyVbERH15/fuZYA/bPH6wVMqQU=") throw std::runtime_error("bad response key");
    iStream = InputStream(&socket);
}

void Websocket::send(message msg) {
    auto encoded = message_encoder::encode(msg);
    socket.write_bytes(encoded.begin().base(), encoded.end().base());
}

message Websocket::read_blocking() {
    message msg;
    msg.set_type(MSG_TYPE::STRING_UTF8);
    Frame decoder(1<<16);

    while(true) {
        decoder.read(iStream);
        msg.append(decoder.begin(), decoder.end());
        if(decoder.is_binary()){
            msg.set_type(MSG_TYPE::BINARY);
        }
        if (decoder.finished()) {
            return msg;
        }
        if (decoder.connection_closed()){
            throw std::runtime_error("connection closed");
        }
    }
}
