#include "OnionPacket.h"
#include <stdlib.h>
#include <string.h>

Client* OnionPacket::client = 0;
OnionPacket::OnionPacket(void) {
    this->buf = 0;
    this->buf_len = 0;
    this->length = 0;
}

OnionPacket::OnionPacket(unsigned int length) {
    this->buf = (uint8_t*) calloc(length,sizeof(uint8_t));
    this->buf_len = length;
    this->length = 0;
}

OnionPacket::OnionPacket(uint8_t* buffer,unsigned int length) {
    this->buf_len = length;
    this->buf = (uint8_t*) calloc(length,sizeof(uint8_t));
    this->length = 0;
    memcpy(this->buf,buffer,length);
}

OnionPacket::~OnionPacket() {
    if (this->buf != 0) {
        free(this->buf);
    }
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

OnionPacket* OnionPacket::readPacket(void) {
    OnionPacket *pkt = new OnionPacket(256);
    // Read packet from socket
    
    return pkt;
}

bool OnionPacket::send(void) {
    // Ensure we have a buffer
    if (buf == 0) {
        return false;
    }
    // Update payload length
    updateLength();
    // Write this packet to the socket
    if (OnionPacket::client != 0) {
        int length = getBufferLength();
        int rc = client->write((uint8_t*)buf, length);
        if (length == rc) {
            return true;
        }
    }
    // Report success/fail
    return false;
}