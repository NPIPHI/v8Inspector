//
// Created by 16182 on 12/26/2020.
//

#ifndef V8DEBUGGER_BASE64_H
#define V8DEBUGGER_BASE64_H

#include <vector>
#include <cstdint>

namespace base64 {
    inline std::vector<uint8_t> encode(const void *begin, const void *end) {
        uint8_t * u8begin = (uint8_t*)begin;
        uint8_t * u8end = (uint8_t*)end;
        std::vector<uint8_t> base64;
        base64.reserve((u8end - u8begin) * 4 / 3 + 4);
        const char *encodings = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        size_t byteLength = (u8end - u8begin);
        size_t byteRemainder = byteLength % 3;
        size_t mainLength = byteLength - byteRemainder;

        // Main loop deals with bytes in chunks of 3
        for (size_t i = 0; i < mainLength; i = i + 3) {
            // Combine the three bytes into a single integer
            uint32_t chunk = (u8begin[i] << 16) | (u8begin[i + 1] << 8) | u8begin[i + 2];

            // Use bitmasks to extract 6-bit segments from the triplet
            uint8_t a = (chunk & 16515072) >> 18; // 16515072 = (2^6 - 1) << 18
            uint8_t b = (chunk & 258048) >> 12; // 258048   = (2^6 - 1) << 12
            uint8_t c = (chunk & 4032) >> 6; // 4032     = (2^6 - 1) << 6
            uint8_t d = chunk & 63;               // 63       = 2^6 - 1

            // Convert the raw binary segments to the appropriate ASCII encoding
            base64.push_back(encodings[a]);
            base64.push_back(encodings[b]);
            base64.push_back(encodings[c]);
            base64.push_back(encodings[d]);

        }

        // Deal with the remaining bytes and padding
        if (byteRemainder == 1) {
            uint32_t chunk = u8begin[mainLength];

            uint8_t a = (chunk & 252) >> 2; // 252 = (2^6 - 1) << 2

            // Set the 4 least significant bits to zero
            uint8_t b = (chunk & 3) << 4; // 3   = 2^2 - 1

            base64.push_back(encodings[a]);
            base64.push_back(encodings[b]);
            base64.push_back('=');
            base64.push_back('=');

        } else if (byteRemainder == 2) {
            uint32_t chunk = (u8begin[mainLength] << 8) | u8begin[mainLength + 1];

            uint8_t a = (chunk & 64512) >> 10; // 64512 = (2^6 - 1) << 10
            uint8_t b = (chunk & 1008) >> 4; // 1008  = (2^6 - 1) << 4

            // Set the 2 least significant bits to zero
            uint8_t c = (chunk & 15) << 2; // 15    = 2^4 - 1
            base64.push_back(encodings[a]);
            base64.push_back(encodings[b]);
            base64.push_back(encodings[c]);
            base64.push_back('=');
        }

        return base64;
    }
}

#endif //V8DEBUGGER_BASE64_H
