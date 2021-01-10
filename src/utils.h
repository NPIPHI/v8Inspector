//
// Created by 16182 on 1/10/2021.
//

#ifndef V8DEBUGGER_UTILS_H
#define V8DEBUGGER_UTILS_H

#include<v8.h>
#include<v8-inspector.h>

[[maybe_unused]] static inline std::string convertToString(v8::Isolate* isolate, const v8_inspector::StringView stringView) {
    int length = static_cast<int>(stringView.length());
    v8::Local<v8::String> message = (
            stringView.is8Bit()
            ? v8::String::NewFromOneByte(isolate, reinterpret_cast<const uint8_t*>(stringView.characters8()), v8::NewStringType::kNormal, length)
            : v8::String::NewFromTwoByte(isolate, reinterpret_cast<const uint16_t*>(stringView.characters16()), v8::NewStringType::kNormal, length)
    ).ToLocalChecked();
    v8::String::Utf8Value utf8(isolate, message);
    return *utf8;
}

[[maybe_unused]] static inline v8_inspector::StringView convertToStringView(std::string_view str) {
    return { (uint8_t *) str.data(), str.length() };
}

[[maybe_unused]] static inline v8::Local<v8::Object> parseJson(const v8::Local<v8::Context> &context, std::string_view json) {
    v8::MaybeLocal<v8::Value> value_ = v8::JSON::Parse(context, v8::String::NewFromUtf8(context->GetIsolate(), json.data(), v8::NewStringType::kNormal, json.size()).ToLocalChecked());
    if (value_.IsEmpty()) {
        return v8::Local<v8::Object>();
    }
    return value_.ToLocalChecked()->ToObject(context).ToLocalChecked();
}

[[maybe_unused]] static inline std::string getPropertyFromJson(v8::Local<v8::Context> & context, const v8::Local<v8::Object> &jsonObject, const std::string &propertyName) {
    v8::Local<v8::Value> property = jsonObject->Get(context, v8::String::NewFromUtf8(context->GetIsolate(), propertyName.c_str(), v8::NewStringType::kNormal).ToLocalChecked()).ToLocalChecked();
    v8::String::Utf8Value utf8Value(context->GetIsolate(), property);
    return *utf8Value;
}

[[maybe_unused]] static std::string getExceptionMessage(v8::Local<v8::Context> & context, const v8::Local<v8::Value> &exception) {
    v8::Local<v8::Object> error = v8::Local<v8::Object>::Cast(exception);
    v8::String::Utf8Value exceptionMessage(context->GetIsolate(), error->ToString(context).ToLocalChecked());
    return *exceptionMessage;
}

#endif //V8DEBUGGER_UTILS_H
