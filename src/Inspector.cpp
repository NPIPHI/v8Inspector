#include "../include/v8Inspector/Inspector.h"
#include "utils.h"

#include<iostream>
#include<websocketpp/config/asio_no_tls_client.hpp>
#include<websocketpp/config/asio_no_tls.hpp>
#include<websocketpp/client.hpp>
#include<websocketpp/server.hpp>

using client = websocketpp::client<websocketpp::config::asio_client>;
using server = websocketpp::server<websocketpp::config::asio>;
using msg_ptr = websocketpp::config::asio_client::message_type::ptr;

Inspector::Inspector(v8::Isolate *isolate, v8::Persistent<v8::Context> *context,
                     std::function<bool()> pump_message_loop) {
    this->isolate = isolate;
    this->context = context;
    websocket_server = std::make_shared<server>();
    inspector_client = std::make_unique<V8InspectorClientImpl>(isolate, context,
                                                               [&](std::string_view msg){send_message(msg);},
                                                               [&]{on_message(pending_messages.blocking_dequeue()); return true;},
                                                               std::move(pump_message_loop));
}

void Inspector::on_message(std::string_view message) {
    auto local_context = context->Get(isolate);
    v8::Context::Scope context_scope(local_context);
    v8_inspector::StringView protocolMessage = convertToStringView(message);
    inspector_client->dispatchProtocolMessage(protocolMessage);
    //TODO find why response not sent until another message is received
}

void Inspector::send_message(std::string_view msg) {
    if(connected()){
        websocket_server->send(websocket_server->get_con_from_hdl(connection_hdl), msg.data(), msg.size(), websocketpp::frame::opcode::text);
    }
}

void Inspector::start_agent(int port) {
    websocket_server->init_asio();
    websocket_server->set_http_handler(http_handler(port, websocket_server));
    websocket_server->set_message_handler([&](auto hdl, server::message_ptr msg){
        connection_hdl = hdl;
        pending_messages.enqueue(msg->get_payload());
    });
    websocket_server->listen(port);
    websocket_server->start_accept();
    std::thread([&](){websocket_server->run();}).detach();
}

void Inspector::poll_messages() {
    while(auto message = pending_messages.dequeue()){
        on_message(*message);
    }
}

bool Inspector::paused() const {
    return inspector_client->paused();
}

void Inspector::set_context(v8::Persistent<v8::Context> * context) {
    this->context = context;
    inspector_client->set_context(context);
}

std::function<void(websocketpp::connection_hdl)>
Inspector::http_handler(int port, std::shared_ptr<server> server) {
    return [=](websocketpp::connection_hdl hdl) {
        auto con = server->get_con_from_hdl(hdl);
        auto res = con->get_resource();
        con->remove_header("Server");
        con->append_header("Cache-Control", "no-cache");
        con->append_header("Content-Type", "application/json; charset=UTF-8");
        if (res == "/json") {
            con->set_body(
                    "[ {\n"
                    "  \"description\": \"android embedded v8 inspector\",\n"
                    "  \"id\": \"inspector\",\n"
                    "  \"title\": \"android embedded v8\",\n"
                    "  \"type\": \"v8\",\n"
                    "  \"webSocketDebuggerUrl\": \"ws://127.0.0.1:" + std::to_string(port) + "/\"\n"
                    "} ]"
            );
        }
        if (res == "/json/version") {
            con->set_body(
                    "{\n"
                    "  \"Browser\": \"v8/8.3.110.9\",\n"
                    "  \"Protocol-Version\": \"1.0\"\n"
                    "}"
            );
        }
        con->set_status(websocketpp::http::status_code::ok);
    };
}

bool Inspector::connected() const {
    return connection_hdl.lock() != nullptr;
}
