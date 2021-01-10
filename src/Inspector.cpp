#include "../include/v8Inspector/Inspector.h"
#include "utils.h"

Inspector::Inspector(v8::Isolate *isolate, v8::Persistent<v8::Context> *context,
                     std::function<bool()> pump_message_loop) {
    this->isolate = isolate;
    this->context = context;
    websocket_server = std::make_unique<Websocket_server>();
    inspector_client = std::make_unique<V8InspectorClientImpl>(isolate, context,
                                                               [&](std::string_view msg){send_message(msg);},
                                                               [&](){on_message(websocket_server->get_message_blocking().view()); return true;},
                                                               std::move(pump_message_loop));
}

void Inspector::on_message(std::string_view message) {
    auto local_context = context->Get(isolate);
    v8::Context::Scope context_scope(local_context);
    v8_inspector::StringView protocolMessage = convertToStringView(message);
    inspector_client->dispatchProtocolMessage(protocolMessage);
//    v8::Local<v8::Object> jsonObject = parseJson(local_context, message);
//    if (!jsonObject.IsEmpty()) {
//        std::string method = getPropertyFromJson(local_context, jsonObject, "method");
//        if (method == "Runtime.runIfWaitingForDebugger") {
//        }
//    }
    //TODO find why response not sent until another message is received
}

void Inspector::send_message(message msg) {
    websocket_server->send_message(std::move(msg));
}

void Inspector::start_agent(int port) {
    websocket_server->start(port, "inspector");
}

bool Inspector::compileScript(const v8::Local<v8::String> &source, const std::string &filePath, v8::Local<v8::Script> &script, const v8::TryCatch &tryCatch) {
    v8::ScriptOrigin scriptOrigin = v8::ScriptOrigin(
            v8::String::NewFromUtf8(
                    isolate,
                    ("file://" + filePath).c_str(),
                    v8::NewStringType::kNormal
            ).ToLocalChecked()
    );
    v8::MaybeLocal<v8::Script> script_ = v8::Script::Compile(context->Get(isolate), source, &scriptOrigin);
    if (!script_.IsEmpty()) {
        script = script_.ToLocalChecked();
    }
    return !tryCatch.HasCaught();
}

void Inspector::poll_messages() {
    while(auto message = websocket_server->get_message()){
        on_message(message->view());
    }
}

bool Inspector::paused() const {
    return inspector_client->paused();
}

void Inspector::set_context(v8::Persistent<v8::Context> * context) {
    this->context = context;
    inspector_client->set_context(context);
}
