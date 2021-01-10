//
// Created by 16182 on 12/2/2020.
//

#include <optional>
#include "InputStream.h"

int InputStream::read(void *out, int len) {
    if(_buffer_tail - _buffer_head < len){
        fill_buffer();
    }
    if(_buffer_tail == _buffer_head){
        return -1;
    } else {
        int max_len = _buffer_tail - _buffer_head;
        auto read_len = std::min(len, max_len);
        memcpy(out, _buffer_head, read_len);
        _buffer_head += read_len;
        return read_len;
    }
}

std::optional<uint8_t> InputStream::read_byte() {
    if(_buffer_tail == _buffer_head){
        fill_buffer();
    }
    if(_buffer_tail == _buffer_head){
        return std::nullopt;
    } else {
        return *(_buffer_head++);
    }
}

void InputStream::fill_buffer() {
    if(_buffer_tail == BUFFER_SIZE + _buffer.get()) rebase_buffer();
    if(_buffer_tail == BUFFER_SIZE + _buffer.get()) throw std::runtime_error("input stream buffer filled");
    int len = _socket->read_bytes(_buffer_tail, BUFFER_SIZE  + _buffer.get() - _buffer_tail);
    if(len == -1){
        return;
    }
    _buffer_tail += len;
}

InputStream::InputStream(InputStream && source) {
    _socket = source._socket;
    _buffer = std::move(source._buffer);
    _buffer_head = source._buffer_head;
    _buffer_tail = source._buffer_tail;
}

InputStream &InputStream::operator=(InputStream &&source)  noexcept {
    _socket = source._socket;
    _buffer = std::move(source._buffer);
    _buffer_head = source._buffer_head;
    _buffer_tail = source._buffer_tail;
    return *this;
}

std::optional<std::vector<uint8_t>> InputStream::read() {
    if(_buffer_tail == _buffer_head){
        fill_buffer();
    }
    if(_buffer_tail == _buffer_head){
        return std::nullopt;
    }
    std::vector<uint8_t> ret = {_buffer_head, _buffer_tail};
    _buffer_head = _buffer_tail;
    return ret;
}

void InputStream::rebase_buffer(){
    std::copy(_buffer_head, _buffer_tail, _buffer.get());
    _buffer_tail = _buffer_tail - _buffer_head + _buffer.get();
    _buffer_head = _buffer.get();
}

InputStream::InputStream(Socket * socket) {
    _socket = socket;
    _buffer = std::unique_ptr<uint8_t[]>(new uint8_t[BUFFER_SIZE]);
    _buffer_tail = _buffer_head = _buffer.get();
}
