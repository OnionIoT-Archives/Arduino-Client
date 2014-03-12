#include "OnionPayloadPacker.h"
#include "msgpack_types.h"


OnionPayloadPacker::OnionPayloadPacker(char* buffer,unsigned int max_length) {
    this->buf = buffer;
    this->max_len = max_length;
    this->len = 0;
}

OnionPayloadPacker::~OnionPayloadPacker() {
    
}

void  OnionPaylaodPacker::packArray(int length) {
    // First ensure the buffer isn't full
    if (len<max_len) {
        if (length < 16) {
            buf[len++] = MSGPACK_FIXARRAY_HEAD + length;
        } else if (length < 65536) {
            // Ensure we have at least 3 bytes
            if (len+2<max_len) {               
                buf[len++] = MSGPACK_ARRAY16_HEAD;
                buf[len++] = length >> 8;
                buf[len++] = length & 0xFF;
            }
        } else {
            // Ensure we have at least 5 bytes
            if (len+4<max_len) {
                buf[len++] = MSGPACK_ARRAY32_HEAD;
                buf[len++] = length >> 24;
                buf[len++] = (length >> 16) & 0xFF;
                buf[len++] = (length >> 8) & 0xFF;
                buf[len++] = length & 0xFF;
            }
        }
    }
}

void  OnionPaylaodPacker::packMap(int length) {
    // First ensure the buffer isn't full
    if (len<max_len) {
        if (length < 16) {
            buf[len++] = MSGPACK_FIXMAP_HEAD + length;
        } else if (length < 65536) {
            // Ensure we have at least 3 bytes
            if (len+2<max_len) {               
                buf[len++] = MSGPACK_MAP16_HEAD;
                buf[len++] = length >> 8;
                buf[len++] = length & 0xFF;
            }
        } else {
            // Ensure we have at least 5 bytes
            if (len+4<max_len) {
                buf[len++] = MSGPACK_MAP32_HEAD;
                buf[len++] = length >> 24;
                buf[len++] = (length >> 16) & 0xFF;
                buf[len++] = (length >> 8) & 0xFF;
                buf[len++] = length & 0xFF;
            }
        }
    }
}

void  OnionPaylaodPacker::packInt(int i) {
    union {int16_t i;uint8_t byte[2];} i16;
    // First ensure the buffer isn't full
    if (len<max_len) {
        if (i < 128 && i > -33) {
            // fixInt
            buf[len++] = (int8_t)i;
        } else if ((i>=0) && (i<128)) {
            buf[len++] = (int8_t)i;
        } else if ((i > -129) && (i<128)) {
            // Ensure we have at least 2 bytes
            if (len+1 < max_len) {
                buf[len++] = MSGPACK_INT8_HEAD;
                buf[len++] = i;
            }
        } else if ((i > -32769) && (i<32768)) {
            // Ensure we have at least 2 bytes
            if (len+2 < max_len) {
                union {int16_t i;uint8_t byte[2];} i16;
                i16.i = i;
                buf[len++] = MSGPACK_INT16_HEAD;
                buf[len++] = i16.byte[0];
                buf[len++] = i16.byte[1];
            }
        } else {
            // Ensure we have at least 5 bytes
            if (len+4<max_len) {
                union {int32_t i;uint8_t byte[2];} i32;
                i32.i = i;
                buf[len++] = MSGPACK_MAP32_HEAD;
                buf[len++] = i32.byte[0];
                buf[len++] = i32.byte[1];
                buf[len++] = i32.byte[2];
                buf[len++] = i32.byte[3];
            }
        }
    }
}

void  OnionPaylaodPacker::packStr(char* c) {
    int len = strlen(c);
    packStr(c,len);
}

void  OnionPaylaodPacker::packStr(char* c, int length) {
    // First ensure the buffer isn't full
    if (len+length<max_len) {
        if (length < 32) {
            buf[len++] = MSGPACK_FIXSTR_HEAD + length;
        } else if (length < 256) {
            // Ensure we have at least 2 bytes
            buf[len++] = MSGPACK_STR8_HEAD;
            buf[len++] = length & 0xFF;
        } else if (length < 65536) {
            // Ensure we have at least 3 bytes
            buf[len++] = MSGPACK_STR16_HEAD;
            buf[len++] = length >> 8;
            buf[len++] = length & 0xFF;
        } else {
            buf[len++] = MSGPACK_STR32_HEAD;
            buf[len++] = length >> 24;
            buf[len++] = (length >> 16) & 0xFF;
            buf[len++] = (length >> 8) & 0xFF;
            buf[len++] = length & 0xFF;
        }
        if (len+length <= max_len) {
            memcpy(buf+len,c,length);
            len+=length;
        }
    }
}

void  OnionPaylaodPacker::packNil() {
    // First ensure the buffer isn't full
    if (len+length<max_len) {
        buf[len++] = MSGPACK_NIL_HEAD;
    }
}

void  OnionPaylaodPacker::packBool(bool b) {
    // First ensure the buffer isn't full
    if (len+length<max_len) {
        if (b) {
            buf[len++] = MSGPACK_TRUE_HEAD;
        } else {
            buf[len++] = MSGPACK_FALSE_HEAD;
        }
    }
}

int   OnionPaylaodPacker::getLength(void) {
    return this->len;
}

char* OnionPaylaodPacker::getBuffer(void) {
    return this->buf;
}