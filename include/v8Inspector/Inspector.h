#include <functional>
#include <vector>
#include <string>
#include "../../src/v8inspector_client.h"
#include "../../src/SafeQueue.h"

#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

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

    ~Inspector() = default;

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
     * returns weather the inspector is currently connected to chrome dev tools
     */
    [[nodiscard]] bool connected() const;

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
    using server = websocketpp::server<websocketpp::config::asio>;
    void on_message(std::string_view message);
    void send_message(std::string_view message);
    std::function<void(websocketpp::connection_hdl)> http_handler(int port, std::shared_ptr<server> server);

    v8::Isolate * isolate;
    std::shared_ptr<server> websocket_server;
    websocketpp::connection_hdl connection_hdl;
    SafeQueue<std::string> pending_messages;
    v8::Persistent<v8::Context> * context;
    std::unique_ptr<V8InspectorClientImpl> inspector_client;
};


#endif //V8_INSPECTOR_EXAMPLE_INSPECTOR_H
