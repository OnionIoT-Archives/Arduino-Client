#include "OnionPayloadData.h"
#include "msgpack_types.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <Arduino.h>

// Public Functions
OnionPayloadData::OnionPayloadData() {
    // Blank object
}

OnionPayloadData::OnionPayloadData(OnionPacket* pkt) {
    OnionPayloadData(pkt,0);
}

OnionPayloadData::OnionPayloadData(OnionPacket* pkt,unsigned int offset) {
    init(pkt,offset);
}

OnionPayloadData::~OnionPayloadData() {
    // Free any malloc'd or new'd data in this object
    if (data != 0) {
        free(data);
    }
    if (dataObjectArray != 0) {
        delete[] dataObjectArray;
    }
}

int OnionPayloadData::getRawLength() {
    return this->rawLength;
}

void OnionPayloadData::init(OnionPacket* pkt,unsigned int offset) {
    // Initialize internal variables
    this->type = 0;
    this->length = 0;
    this->data = 0;
    this->dataObjectArray = 0;
    this->pkt = pkt;
    this->offset = offset;
    this->rawBuffer = pkt->getPayload() + offset;
    this->rawLength = pkt->getPayloadLength() - offset;
    this->dataIsObject = false;
    //printf("->init: pkt->length = %d , offset = %d, rawLength = %d\n",pkt->getLength(), offset,this->rawLength);
}

// Call unpack to try and unpack the data into link list of payload objects
// returns the count of bytes used to unpack
int OnionPayloadData::unpack(void) {
    //printf("->unpack: rawLength = %d\n",this->rawLength);
    if (rawLength == 0) {
        //printf("->unpack: kicked out b/c no rawLength\n");
        return 0;
    }
    int bytesParsed = 0;
    // If we have data then assume length is 1 until we parse an actual value
    length = 1;
    bytesParsed++;  // Add one to parsed bytes since all formats have at least 1 byte
    uint8_t rawType = rawBuffer[0];
    //printf("->unpack: rawType = %02X\n",rawType);
    if (((rawType & 0x80) == 0) || ((rawType & 0xE0) == 0xE0)) {
        type = MSGPACK_FIXINT_HEAD;
        Serial.print("--> unpack type = int\n");
        data = calloc(1,sizeof(int));
        int *ptr = (int*) data;
        *ptr = rawType;
        Serial.print("--> unpack vaue = ");
        Serial.print(*ptr);
        Serial.print("\n");
        //printf("->unpack: found fixint type, value = %d\n",*ptr);
    } else if ((rawType & 0xF0) == MSGPACK_FIXMAP_HEAD) {
        type = MSGPACK_FIXMAP_HEAD;
        Serial.print("--> unpack type = map\n");
        length = rawType & 0x0F;
        
        Serial.print("--> unpack length = ");
        Serial.print(length);
        Serial.print("\n");
        dataObjectArray = new OnionPayloadData*[length];
        dataIsObject = true;
        for (int x=0;x<length;x++) {
            dataObjectArray[x] = new OnionPayloadData(pkt,offset+bytesParsed); // Begining of sub object is current offset + bytes parsed
            bytesParsed += dataObjectArray[x]->unpack();
        }
    } else if ((rawType & 0xF0) == MSGPACK_FIXARRAY_HEAD) {
        type = MSGPACK_FIXARRAY_HEAD;
        Serial.print("--> unpack type = array\n");
        length = rawType & 0x0F;
        Serial.print("--> unpack length = ");
        Serial.print(length);
        Serial.print("\n");
        dataObjectArray = new OnionPayloadData*[length];
        dataIsObject = true;
        //printf("->unpack: found fixArray type, length = %d\n",length);
        for (int x=0;x<length;x++) {
            //printf("->unpack: create a new object with offset=%d & bytesParsed=%d\n",offset,bytesParsed);
            dataObjectArray[x] = new OnionPayloadData(pkt,offset+bytesParsed); // Begining of sub object is current offset + bytes parsed
            bytesParsed += dataObjectArray[x]->unpack();
        }
    } else if ((rawType & 0xE0) == MSGPACK_FIXSTR_HEAD) {
        type = MSGPACK_FIXSTR_HEAD;
        Serial.print("--> unpack type = str\n");
        length = rawType & 0x1F;
        Serial.print("--> unpack length = ");
        Serial.print(length);
        Serial.print("\n");
        data = new uint8_t[length+1];
        uint8_t* ptr = (uint8_t*) data;
        memcpy(ptr,rawBuffer+1,length);
        // Do I really need to add this null? probably, but may not be necessary
        ptr[length] = 0;
        bytesParsed += length;
    } else {
        type = rawType;
        Serial.print("--> unpack type = (other) = ");
        Serial.print(rawType);
        Serial.print("\n");
        switch (type) {
            case MSGPACK_NIL_HEAD: {
                data = 0;
                break;
            }   
            case MSGPACK_FALSE_HEAD: {
                bool* ptr = (bool*) data;
                *ptr = false;
                break;
            }   
            case MSGPACK_TRUE_HEAD: {
                bool* ptr = (bool*) data;
                *ptr = true;
                break;
            }   
            case MSGPACK_BIN8_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_BIN16_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_BIN32_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_EXT8_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_EXT16_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_EXT32_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_FLOAT32_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_FLOAT64_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_UINT8_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_UINT16_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_UINT32_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_UINT64_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_INT8_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_INT16_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_INT32_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_INT64_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_FIXEXT1_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_FIXEXT2_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_FIXEXT4_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_FIXEXT8_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_FIXEXT16_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_STR8_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_STR16_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_STR32_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_ARRAY16_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_ARRAY32_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_MAP16_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_MAP32_HEAD: {
                // Not Implemented yet
                break;
            }   
        }
    }
    return bytesParsed;
}

// getItem(item) will return an item from an array or map so you can dig into the data structure.
// item is 0 based index of the array, i should be less than Length (from getLength() below)
OnionPayloadData* OnionPayloadData::getItem(int i) {
    if (length == 0) {
        return 0;
    } else if (i<length) {
        return dataObjectArray[i];
    }
}

// getType() will return the type of this object
uint8_t OnionPayloadData::getType(void) {
    return this->type;
}

// getLength() will return the number of items in an array or map, will return 1 for other data
// and 0 for any data that could not be parsed correctly.
int OnionPayloadData::getLength(void) {
    return this->length;
}

// getBuffer() will return a uint8_t* pointer to the data parse in the object. Useful for
// getting a pointer to a string/binary/raw data type
uint8_t* OnionPayloadData::getBuffer(void) {
    uint8_t *ptr = (uint8_t*) data;
    return ptr;
}

// ???  Should I add a getString to make sure the data has a null terminated string? Might be nice to be able to
// ???  pass an output directly to str functions
// getInt() will return the raw parse data as an int.  If the type is not int it will return 0
int OnionPayloadData::getInt(void) {
    if (data != 0) {
        int *ptr = (int*) data;
        return *ptr;
    } else {
        return -1;
    }
}

// getBool() will return the parsed data of a bool, or false if it is another type
bool OnionPayloadData::getBool(void) {
    bool *ptr = (bool*) data;
    return *ptr;
}


// Protected Functions
void OnionPayloadData::unpackArray(void) {
    
}

void OnionPayloadData::unpackMap(void) {
    
}

void OnionPayloadData::unpackInt(void) {
    
}

void OnionPayloadData::unpackStr(void) {
    
}

void OnionPayloadData::unpackNil(void) {
    
}

void OnionPayloadData::unpackBool(void) {
    
}


//
//
//void  OnionPayloadData::packArray(int length) {
//    // First ensure the buffer isn't full
//    if (len<max_len) {
//        if (length < 16) {
//            buf[len++] = MSGPACK_FIXARRAY_HEAD + length;
//        } else if (length < 65536) {
//            // Ensure we have at least 3 bytes
//            if (len+2<max_len) {               
//                buf[len++] = MSGPACK_ARRAY16_HEAD;
//                buf[len++] = length >> 8;
//                buf[len++] = length & 0xFF;
//            }
//        } else {
//            // Ensure we have at least 5 bytes
//            if (len+4<max_len) {
//                buf[len++] = MSGPACK_ARRAY32_HEAD;
//                buf[len++] = length >> 24;
//                buf[len++] = (length >> 16) & 0xFF;
//                buf[len++] = (length >> 8) & 0xFF;
//                buf[len++] = length & 0xFF;
//            }
//        }
//    }
//}
//
//void  OnionPayloadData::packMap(int length) {
//    // First ensure the buffer isn't full
//    if (len<max_len) {
//        if (length < 16) {
//            buf[len++] = MSGPACK_FIXMAP_HEAD + length;
//        } else if (length < 65536) {
//            // Ensure we have at least 3 bytes
//            if (len+2<max_len) {               
//                buf[len++] = MSGPACK_MAP16_HEAD;
//                buf[len++] = length >> 8;
//                buf[len++] = length & 0xFF;
//            }
//        } else {
//            // Ensure we have at least 5 bytes
//            if (len+4<max_len) {
//                buf[len++] = MSGPACK_MAP32_HEAD;
//                buf[len++] = length >> 24;
//                buf[len++] = (length >> 16) & 0xFF;
//                buf[len++] = (length >> 8) & 0xFF;
//                buf[len++] = length & 0xFF;
//            }
//        }
//    }
//}
//
//void  OnionPayloadData::packInt(int i) {
//    union {int16_t i;uint8_t byte[2];} i16;
//    // First ensure the buffer isn't full
//    if (len<max_len) {
//        if (i < 128 && i > -33) {
//            // fixInt
//            buf[len++] = (int8_t)i;
//        } else if ((i>=0) && (i<128)) {
//            buf[len++] = (int8_t)i;
//        } else if ((i > -129) && (i<128)) {
//            // Ensure we have at least 2 bytes
//            if (len+1 < max_len) {
//                buf[len++] = MSGPACK_INT8_HEAD;
//                buf[len++] = i;
//            }
//        } else if ((i > -32769) && (i<32768)) {
//            // Ensure we have at least 2 bytes
//            if (len+2 < max_len) {
//                union {int16_t i;uint8_t byte[2];} i16;
//                i16.i = i;
//                buf[len++] = MSGPACK_INT16_HEAD;
//                buf[len++] = i16.byte[0];
//                buf[len++] = i16.byte[1];
//            }
//        } else {
//            // Ensure we have at least 5 bytes
//            if (len+4<max_len) {
//                union {int32_t i;uint8_t byte[2];} i32;
//                i32.i = i;
//                buf[len++] = MSGPACK_MAP32_HEAD;
//                buf[len++] = i32.byte[0];
//                buf[len++] = i32.byte[1];
//                buf[len++] = i32.byte[2];
//                buf[len++] = i32.byte[3];
//            }
//        }
//    }
//}
//
//void  OnionPayloadData::packStr(uint8_t* c) {
//    int len = strlen(c);
//    packStr(c,len);
//}
//
//void  OnionPayloadData::packStr(uint8_t* c, int length) {
//    // First ensure the buffer isn't full
//    if (len+length<max_len) {
//        if (length < 32) {
//            buf[len++] = MSGPACK_FIXSTR_HEAD + length;
//        } else if (length < 256) {
//            // Ensure we have at least 2 bytes
//            buf[len++] = MSGPACK_STR8_HEAD;
//            buf[len++] = length & 0xFF;
//        } else if (length < 65536) {
//            // Ensure we have at least 3 bytes
//            buf[len++] = MSGPACK_STR16_HEAD;
//            buf[len++] = length >> 8;
//            buf[len++] = length & 0xFF;
//        } else {
//            buf[len++] = MSGPACK_STR32_HEAD;
//            buf[len++] = length >> 24;
//            buf[len++] = (length >> 16) & 0xFF;
//            buf[len++] = (length >> 8) & 0xFF;
//            buf[len++] = length & 0xFF;
//        }
//        if (len+length <= max_len) {
//            memcpy(buf+len,c,length);
//            len+=length;
//        }
//    }
//}
//
//void  OnionPayloadData::packNil() {
//    // First ensure the buffer isn't full
//    if (len+length<max_len) {
//        buf[len++] = MSGPACK_NIL_HEAD;
//    }
//}
//
//void  OnionPayloadData::packBool(bool b) {
//    // First ensure the buffer isn't full
//    if (len+length<max_len) {
//        if (b) {
//            buf[len++] = MSGPACK_TRUE_HEAD;
//        } else {
//            buf[len++] = MSGPACK_FALSE_HEAD;
//        }
//    }
//}
//
//int   OnionPayloadData::getLength(void) {
//    return this->len;
//}
//
//uint8_t* OnionPayloadData::getBuffer(void) {
//    return this->buf;
//}