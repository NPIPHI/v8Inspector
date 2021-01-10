//
// Created by 16182 on 11/30/2020.
//

#include "message_encoder.h"
#include "Frame.h"

std::vector<uint8_t> message_encoder::encode(message msg) {
    Frame frame(true, false,
                msg.type() == MSG_TYPE::STRING_UTF8 ? Frame::OPCODE_TEXT_FRAME : Frame::OPCODE_BINARY_FRAME,
                msg.begin(), msg.end());
    return frame.encode();
}