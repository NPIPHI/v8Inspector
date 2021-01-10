//
// Created by 16182 on 11/30/2020.
//

#ifndef V8DEBUGGER_FRAME_H
#define V8DEBUGGER_FRAME_H

#include <cstdint>
#include <vector>
#include <array>
#include "InputStream.h"

class Frame {
public:
    Frame(size_t max_len, std::unique_ptr<uint8_t[]> buffer): max_len(max_len), payloadData(std::move(buffer)){

    }
    Frame(size_t max_len): max_len(max_len), payloadData(std::unique_ptr<uint8_t[]>(new uint8_t[max_len])){

    }
    Frame(bool fin, bool hasMask, uint8_t opcode, const uint8_t *begin, const uint8_t *end) :
            fin(fin), hasMask(hasMask), opcode(opcode), max_len(end-begin), payloadLen(end - begin),
            payloadData(std::unique_ptr<uint8_t[]>(new uint8_t[end-begin])){
        memcpy(payloadData.get(), begin, end-begin);
    }
    //returns the end of the data that it used
    void read(InputStream &input);
    bool finished() const;
    bool connection_closed() const;
    bool is_text() const;
    bool is_binary() const;
    std::vector<uint8_t> encode() const;
    const uint8_t * begin() const;
    const uint8_t * end() const;
    size_t size() const;
    constexpr static uint8_t OPCODE_TEXT_FRAME = 0x1;
    constexpr static uint8_t OPCODE_BINARY_FRAME = 0x2;
    constexpr static uint8_t OPCODE_CONNECTION_CLOSE = 0x4;
    constexpr static uint8_t OPCODE_CONNECTION_PING = 0x9;
    constexpr static uint8_t OPCODE_CONNECTION_PONG = 0xA;
private:
    bool fin = false;
    bool rsv1 = false;
    bool rsv2 = false;
    bool rsv3 = false;
    bool hasMask = false;
    uint8_t opcode{};
    const size_t max_len;
    size_t payloadLen{};
    std::array<uint8_t, 4> maskingKey{};
    const std::unique_ptr<uint8_t[]> payloadData{};
    void decode_first_byte(uint8_t b);
    void read(InputStream &is, int len);
    size_t decode_length(uint8_t firstLenByte, InputStream &input);
    uint8_t encode_first_byte() const;
    std::vector<uint8_t> encode_length(size_t len) const;
    static std::array<uint8_t, 4> decodeMaskingKey(InputStream &input);
    static void
    unmask(std::array<uint8_t, 4> key, uint8_t *begin, size_t len);
    static uint8_t read_byte(InputStream &input);
};


#endif //V8DEBUGGER_FRAME_H
