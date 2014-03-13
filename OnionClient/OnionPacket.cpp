#include "OnionPacket.h"
#include <stdlib.h>
#include <string.h>

OnionPacket::OnionPacket(void) {
    this->buf = 0;
    this->len = 0;
}

OnionPacket::OnionPacket(unsigned int length) {
    this->buf = (char*) calloc(length,sizeof(char));
    this->len = length;
}

OnionPacket::OnionPacket(char* buffer,unsigned int length) {
    this->len = length;
    this->buf = (char*) calloc(length,sizeof(char));
    memcpy(this->buf,buffer,length);
}

OnionPacket::~OnionPacket() {
    if (this->buf != 0) {
        free(this->buf);
    }
}

int   OnionPacket::getLength(void) {
    return this->len;
}

char* OnionPacket::getBuffer(void) {
    return this->buf;
}