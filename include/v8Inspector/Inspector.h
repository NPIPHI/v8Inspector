#include <functional>
#include <vector>
#include <string>
#include "../../src/v8inspector_client.h"
#include <AndroidWebsocket/Message.h>
#include <AndroidWebsocket/Websocket_server.h>

#ifndef V8_INSPECTOR_EXAMPLE_INSPECTOR_H
#define V8_INSPECTOR_EXAMPLE_INSPECTOR_H


class Inspector {
public:
    Inspector(v8::Isolate *isolate, v8::Persistent<v8::Context> *context,
              std::function<bool()> pump_message_loop);
    void set_context(v8::Persistent<v8::Context> * context);
    void start_agent(int port);
    void poll_messages();
    bool paused() const;
private:
    void on_message(std::string_view message);
    void send_message(message msg);
    bool compileScript(const v8::Local<v8::String> &source, const std::string &filename, v8::Local<v8::Script> &script, const v8::TryCatch &tryCatch);

    v8::Isolate * isolate;
    std::unique_ptr<Websocket_server> websocket_server;
    v8::Persistent<v8::Context> * context;
    std::unique_ptr<V8InspectorClientImpl> inspector_client;
    std::vector<std::string> scripts = {};
};


#endif //V8_INSPECTOR_EXAMPLE_INSPECTOR_H
