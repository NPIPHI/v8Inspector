//
// Created by 16182 on 12/25/2020.
//

#ifndef V8DEBUGGER_MESSAGE_H
#define V8DEBUGGER_MESSAGE_H

#include<memory>
#include<vector>
#include<cstdint>
#include<string_view>

enum class MSG_TYPE {
    STRING_UTF8,
    STRING_UTF16,
    BINARY
};

struct message{
    using buffer = std::vector<uint8_t>;
    message():
            data(std::make_shared<buffer>()),
            _type(MSG_TYPE::STRING_UTF8){}
    message(const void * begin, const void * end, MSG_TYPE type):
            data(std::make_shared<buffer>((uint8_t*)begin, (uint8_t*)end)),
            _type(type){}
    message(std::string_view str):
            data(std::make_shared<buffer>(str.begin(), str.end())),
            _type(MSG_TYPE::STRING_UTF8){}
    message(const char * str):
        data(std::make_shared<buffer>(str, str + strlen(str))),
        _type(MSG_TYPE::STRING_UTF8){}
    inline void set_type(MSG_TYPE type){
        _type = type;
    }
    inline void append(const uint8_t * begin, const uint8_t * end){
       data->insert(data->end(), begin, end);
    }
    inline void append(uint8_t val){
        data->push_back(val);
    }
    inline void reserve(size_t size){
        data->reserve(size);
    }
    inline const uint8_t * begin() const{
        return data->begin().base();
    }
    inline const uint8_t * end() const{
        return data->end().base();
    }
    inline unsigned int size() const {
        return data->size();
    }
    inline std::string_view view() const{
        return {(const char *)data->data(), data->size()};
    }
    inline MSG_TYPE type() const{
        return _type;
    }
    inline std::shared_ptr<buffer> get_backing() const {
        return data;
    }
private:
    std::shared_ptr<buffer> data;
    MSG_TYPE _type;
};

#endif //V8DEBUGGER_MESSAGE_H
