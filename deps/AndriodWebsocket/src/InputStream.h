//
// Created by 16182 on 12/2/2020.
//

#ifndef V8DEBUGGER_INPUTSTREAM_H
#define V8DEBUGGER_INPUTSTREAM_H

#include<vector>
#include<cstdint>
#include<optional>
#include "Socket.h"


class InputStream {
    static constexpr size_t BUFFER_SIZE = 1 << 20;
    std::unique_ptr<uint8_t[]> _buffer;
    uint8_t * _buffer_head;
    uint8_t * _buffer_tail;
    Socket * _socket;
    void fill_buffer();
public:
    InputStream(){};
    InputStream(Socket * socket);
    InputStream(const InputStream&) = delete;
    InputStream& operator=(InputStream && source) noexcept;
    InputStream(InputStream &&source);
    std::optional<std::vector<uint8_t>> read();
    int read(void *out, int len);
    std::optional<uint8_t> read_byte();

    void rebase_buffer();
};


#endif //V8DEBUGGER_INPUTSTREAM_H
