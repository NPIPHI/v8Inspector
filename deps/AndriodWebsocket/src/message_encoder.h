//
// Created by 16182 on 11/30/2020.
//

#ifndef V8DEBUGGER_MESSAGE_ENCODER_H
#define V8DEBUGGER_MESSAGE_ENCODER_H

#include<vector>
#include "../include/AndroidWebsocket/Message.h"

namespace message_encoder {
    std::vector<uint8_t> encode(message msg);
};


#endif //V8DEBUGGER_MESSAGE_ENCODER_H
