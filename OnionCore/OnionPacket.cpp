#include "OnionPacket.h"
//#include "OnionInterface.h"
#include <stdlib.h>
#include <string.h>
#include <Arduino.h>

//OnionInterface* OnionPacket::interface = 0;
OnionPacket::OnionPacket(void) {
    this->buf = 0;
    this->buf_len = 0;
    this->length = 0;
    this->ptr = 0;
}

OnionPacket::OnionPacket(unsigned int length) {
    this->buf = (uint8_t*) calloc(length,sizeof(uint8_t));
    this->ptr = this->buf;
    this->buf_len = length;
    this->length = 0;
}

OnionPacket::OnionPacket(uint8_t* buffer,unsigned int length) {
    this->buf_len = length;
    this->buf = (uint8_t*) calloc(length,sizeof(uint8_t));
    this->ptr = this->buf;
    this->length = 0;
    memcpy(this->buf,buffer,length);
    this->ptr += length;
}

OnionPacket::~OnionPacket() {
    if (this->buf != 0) {
        free(this->buf);
    }
}

uint16_t OnionPacket::getFreeBuffer() {
    // ************************ THIS NEEDS IMPROVEMENT **********************************
    // length is not updated enough and the 3 is static, but on a new packet this should return buf_len
    return buf_len-length-3;
}

int OnionPacket::getBufferLength(void) {
    return this->length+3;
}

int OnionPacket::getPayloadLength(void) {
    return this->length;
}

int OnionPacket::getPayloadMaxLength(void) {
    if (buf_len > 3) {
        return buf_len-3;
    } else {
        return 0;
    }
}

void OnionPacket::setPayloadLength(int length) {
    this->length = length;
}

uint8_t* OnionPacket::getBuffer(void) {
    return this->buf;
}

uint8_t* OnionPacket::getPayload(void) {
    if (this->buf_len > 3) {
        return this->buf+3;
    } else {
        return 0;
    }
}

bool OnionPacket::isComplete(void) {
    if (ptr == buf) {
        Serial.print("-->ptr = buf\n");
        return false;
    }
    uint16_t set_length = (buf[1] << 8) + buf[2];
    if (set_length == length) {
        return true;
    }
    Serial.print("-->set length = ");
    Serial.print(set_length);
    Serial.print(", length = ");
    Serial.print(length);
    Serial.print("\n");
    return false;
}

uint8_t* OnionPacket::getPtr(void) {
    return ptr;
}

int8_t OnionPacket::incrementPtr(uint16_t count) {
    uint16_t len = 0;
    if (ptr == buf) {
        // ********************* THIS HAS A BAD CORNER CASE TO BE HANDLED **********************
        // what if count < 3 in this case?
        len = count - 3;
    } else {
        len = length + count + 3;
    }
    if (len > buf_len) {
        return 0;
    }
    if (ptr == buf) {
        // ********************* THIS HAS A BAD CORNER CASE TO BE HANDLED **********************
        // what if count < 3 in this case?
        length = count - 3;
    } else {
        length += count;
    }
    ptr += count;
}

void OnionPacket::updateLength(void) {
    if (buf != 0) {
        buf[1] = (length >> 8) && 0xff;
        buf[2] = length & 0xff;
    }
}

void OnionPacket::setType(uint8_t packet_type) {
    if (this->buf != 0) {
        buf[0] = packet_type;
    }
}

uint8_t OnionPacket::getType(void) {
    if (this->buf != 0) {
        return buf[0] & 0xF0;
    } else {
        return 0;
    }
}
//
//OnionPacket* OnionPacket::readPacket(void) {
//    OnionPacket *pkt = new OnionPacket(128);
//    // Read packet from socket
//    
//    return pkt;
//}
//
//bool OnionPacket::send(void) {
//    // Ensure we have a buffer
//    if (buf == 0) {
//        return false;
//    }
//    // Update payload length
//    updateLength();
//    // Write this packet to the socket
//    if (OnionPacket::interface != 0) {
//        interface->send(this);
//        return true;
//    }
//    // Report success/fail
//    return false;
//}