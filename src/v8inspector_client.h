#include <iostream>
#include <v8.h>
#include <v8-inspector.h>
#include <libplatform/libplatform.h>
#include "v8inspector_channel.h"

#ifndef V8INSPECTORCLIENTIMPL_H
#define V8INSPECTORCLIENTIMPL_H

class V8InspectorClientImpl final: public v8_inspector::V8InspectorClient {
public:
  V8InspectorClientImpl(v8::Isolate *isolate,
                        v8::Persistent<v8::Context> * context,
                        std::function<void(std::string)> on_response,
                        std::function<bool()> wait_on_message,
                        std::function<bool()> pump_message_loop);

  void dispatchProtocolMessage(const v8_inspector::StringView &message_view);

  void runMessageLoopOnPause(int contextGroupId) override;

  void set_context(v8::Persistent<v8::Context> * context);

  void quitMessageLoopOnPause() override;

  void schedulePauseOnNextStatement(const v8_inspector::StringView &reason);

  bool paused() const;

private:
  v8::Local<v8::Context> ensureDefaultContextInGroup(int contextGroupId) override;

  bool run_nested_loop_ = false;
  bool terminated_ = false;
  static const int kContextGroupId = 1;
  v8::Isolate * isolate;
  std::function<bool()> wait_on_message;
  std::unique_ptr<v8_inspector::V8Inspector> inspector;
  std::unique_ptr<v8_inspector::V8InspectorSession> session;
  std::unique_ptr<V8InspectorChannelImp> channel;
  v8::Persistent<v8::Context> * context;
  std::function<bool()> pump_message_loop;
};

#endif // V8INSPECTORCLIENTIMPL_H
