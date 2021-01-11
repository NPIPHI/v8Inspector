#include "v8inspector_client.h"
#include "utils.h"

#include <memory>

V8InspectorClientImpl::V8InspectorClientImpl(v8::Isolate *isolate,
                                             v8::Persistent<v8::Context> * context,
                                             std::function<void(std::string)> on_response,
                                             std::function<bool()> wait_on_message,
                                             std::function<bool()> pump_message_loop) {
    this->pump_message_loop = std::move(pump_message_loop);
    this->wait_on_message = std::move(wait_on_message);
    this->isolate = isolate;
    this->context = context;

    channel = std::make_unique<V8InspectorChannelImp>(isolate, std::move(on_response));
    inspector = v8_inspector::V8Inspector::create(isolate, this);
    session = inspector->connect(kContextGroupId, channel.get(), v8_inspector::StringView());

    auto local_context = context->Get(isolate);
    local_context->SetAlignedPointerInEmbedderData(1, this);
    v8_inspector::StringView contextName = convertToStringView("inspector");
    inspector->contextCreated(v8_inspector::V8ContextInfo(local_context, kContextGroupId, contextName));
}

void V8InspectorClientImpl::dispatchProtocolMessage(const v8_inspector::StringView &message_view) {
    session->dispatchProtocolMessage(message_view);
}

void V8InspectorClientImpl::runMessageLoopOnPause(int contextGroupId) {
    if (run_nested_loop_) {
        return;
    }
    terminated_ = false;
    run_nested_loop_ = true;
    while (!terminated_ && wait_on_message()) {
        while (pump_message_loop()) {}
    }
    terminated_ = true;
    run_nested_loop_ = false;
}

v8::Local<v8::Context> V8InspectorClientImpl::ensureDefaultContextInGroup(int contextGroupId) {
    return context->Get(isolate);
}

void V8InspectorClientImpl::schedulePauseOnNextStatement(const v8_inspector::StringView &reason) {
    session->schedulePauseOnNextStatement(reason, reason);
}

void V8InspectorClientImpl::quitMessageLoopOnPause() {
    terminated_ = true;
    V8InspectorClient::quitMessageLoopOnPause();
}

bool V8InspectorClientImpl::paused() const {
    return run_nested_loop_;
}

void V8InspectorClientImpl::set_context(v8::Persistent<v8::Context> *context) {
    auto old_ctx = this->context->Get(isolate);
    inspector->contextDestroyed(old_ctx);
    this->context = context;
    auto ctx = context->Get(isolate);
    ctx->SetAlignedPointerInEmbedderData(1, this);
    v8_inspector::StringView contextName = convertToStringView("inspector");
    inspector->contextCreated(v8_inspector::V8ContextInfo(ctx, kContextGroupId, contextName));
}
