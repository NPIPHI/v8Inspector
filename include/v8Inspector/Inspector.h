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
    /*
     * construct the v8 inspector
     * requires that locks be set before the constructor is called
     */
    Inspector(v8::Isolate *isolate, v8::Persistent<v8::Context> *context,
              std::function<bool()> pump_message_loop);
    /*
     * sets the context that the inspector views
     * removes the old context from the view
     * requires locks to be set
     */
    void set_context(v8::Persistent<v8::Context> * context);

    /*
     * starts the inspector listening on the port specified
     * to use chrome remote debugging, port forwarding needs to be enabled
     * with adb, port forwarding is set up with the command:
     * "adb forward tcp:<pc-port> tcp:<phone-port>"
     */
    void start_agent(int port);

    /*
     * polls the message queue for new inspector messages
     * any new commands are run on the thread that calls this function
     */
    void poll_messages();

    /*
     * returns weather the inspector is paused on a breakpoint
     */
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
