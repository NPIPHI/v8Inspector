//
// Created by 16182 on 11/30/2020.
//

#include "Frame.h"
#include <string>

#include<thread>

struct connection_closed_error : public std::exception {};

void Frame::read(InputStream &input) {
    try {
        decode_first_byte(read_byte(input));
        uint8_t maskAndFirstLengthBits = read_byte(input);
        hasMask = (maskAndFirstLengthBits & 0x80U) != 0;
        payloadLen = decode_length(maskAndFirstLengthBits & ~0x80U, input);
        maskingKey = hasMask ? decodeMaskingKey(input) : std::array<uint8_t, 4>{};
        read(input, payloadLen);
        unmask(maskingKey, payloadData.get(), payloadLen);
    } catch (connection_closed_error&){
        opcode = OPCODE_CONNECTION_CLOSE;
        fin = false;
    }
}


void Frame::decode_first_byte(uint8_t b) {
    fin = bool(b & 0x80U);
    rsv1 = bool(b & 0x40U);
    rsv2 = bool(b & 0x20U);
    rsv3 = bool(b & 0x10U);
    opcode = b & 0x0fU;
}

size_t Frame::decode_length(uint8_t firstLenByte, InputStream &input) {
    if (firstLenByte <= 125) {
        return firstLenByte;
    } else if (firstLenByte == 126) {
        return ((read_byte(input) & 0xffU) << 8) | (read_byte(input) & 0xffU);
    } else if (firstLenByte == 127) {
        size_t len = 0;
        for (int i = 0; i < 8; i++) {
            len <<= 8U;
            len |= (read_byte(input) & 0xffU);
        }
        return len;
    } else {
        throw std::runtime_error("Unexpected length byte: " + std::to_string(firstLenByte));
    }
}

bool Frame::finished() const {
    return fin;
}

std::array<uint8_t, 4> Frame::decodeMaskingKey(InputStream &input){
    return {read_byte(input),read_byte(input),read_byte(input),read_byte(input)};
}

void
Frame::unmask(std::array<uint8_t, 4> key, uint8_t *begin, size_t len) {

    // this optimization is necessary, clang does not automatically vectorize at all
    uint64_t k = uint32_t(key[0]) + (uint32_t(key[1]) << 8) + (uint32_t(key[2]) << 16) + (uint32_t(key[3]) << 24);
    k += k << 32;
    auto *d = (uint64_t *)begin;

    for(int i = 0; i < len/8; i++) {
        d[i] ^= k;
    }

    for(int i = len/8*8; i < len; i++){
        begin[i] ^= key[i % key.size()];
    }
    //    for(int i = 0; i < data.size(); i++){
    //        data[i] ^= key[i % key.size()];
    //    }
}

const uint8_t * Frame::end() const {
    return payloadData.get() + payloadLen;
}

const uint8_t * Frame::begin() const {
    return payloadData.get();
}

bool Frame::connection_closed() const {
    return opcode == OPCODE_CONNECTION_CLOSE;
}

uint8_t Frame::encode_first_byte() const {
    uint8_t b = 0;
    if (fin) {
        b |= 0x80U;
    }
    if (rsv1) {
        b |= 0x40U;
    }
    if (rsv2) {
        b |= 0x20U;
    }
    if (rsv3) {
        b |= 0x10U;
    }
    b |= (opcode & 0xfU);
    return b;
}

std::vector<uint8_t> Frame::encode_length(size_t len) const{
    if (len <= 125) {
        return { (uint8_t) len };
    } else if (len <= 0xffff) {
        return {
                126,
                (uint8_t)((len >> 8) & 0xff),
                (uint8_t)((len) & 0xff)
        };
    } else {
        return {
                127,
                (uint8_t)((len >> 56) & 0xff),
                (uint8_t)((len >> 48) & 0xff),
                (uint8_t)((len >> 40) & 0xff),
                (uint8_t)((len >> 32) & 0xff),
                (uint8_t)((len >> 24) & 0xff),
                (uint8_t)((len >> 16) & 0xff),
                (uint8_t)((len >> 8) & 0xff),
                (uint8_t)((len) & 0xff)
        };
    }
}

std::vector<uint8_t> Frame::encode() const {
    std::vector<uint8_t> output;
    output.push_back(encode_first_byte());
    auto lengthAndMaskBit = encode_length(payloadLen);
    if (hasMask) {
        lengthAndMaskBit[0] |= 0x80U;
    }
    output.insert(output.end(), lengthAndMaskBit.begin(), lengthAndMaskBit.end());

    if (hasMask) {
        throw std::runtime_error("Writing masked data not implemented");
    }

    output.insert(output.end(), begin(), end());
    return output;
}

size_t Frame::size() const {
    return payloadLen;
}


uint8_t Frame::read_byte(InputStream &input) {
    auto b = input.read_byte();
    if(b.has_value()){
        return *b;
    } else {
        throw connection_closed_error();
    }
}

bool Frame::is_text() const {
    return opcode == OPCODE_TEXT_FRAME;
}

bool Frame::is_binary() const {
    return opcode == OPCODE_BINARY_FRAME;
}
#include "SocketServer.h"

void Frame::read(InputStream &is, int len) {
    if(max_len < len){
        throw std::runtime_error("Too large chunck: len=" + std::to_string(len) + ", max_len=" + std::to_string(max_len));
    }
    auto head = payloadData.get();
    auto tail = head;
    while(head != tail + len){
        auto read = is.read(head, head - tail + len);
        if(read == -1){
            throw connection_closed_error();
        } else {
            head += read;
        }
    }
}