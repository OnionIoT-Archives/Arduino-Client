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
    this->init(pkt,0);
}

OnionPayloadData::OnionPayloadData(OnionPacket* pkt,uint16_t offset) {
    this->init(pkt,offset);
}

OnionPayloadData::~OnionPayloadData() {
    // Free any malloc'd or new'd data in this object
    if (dataObjectArray != 0) {
        uint16_t len = length;
        if (dataIsMap) {
            len *= 2;
        }
        for (uint16_t x = 0;x<len;x++) {
            delete dataObjectArray[x];
        }
    }
    
    if (data != 0) {
        delete[] data;
    }
    
}

int16_t OnionPayloadData::getRawLength() {
    return this->rawLength;
}

void OnionPayloadData::init(OnionPacket* pkt,uint16_t offset) {
    // Initialize internal variables
    this->type = 0;
    this->length = 0;
    this->data = 0;
    this->dataObjectArray = 0;
    this->pkt = pkt;
    this->offset = offset;
    this->rawBuffer = (pkt->getPayload()) + offset;
    this->rawLength = pkt->getPayloadLength() - offset;
    this->dataIsObject = false;
    this->dataIsMap = false;
}

// Call unpack to try and unpack the data into link list of payload objects
// returns the count of bytes used to unpack
uint16_t OnionPayloadData::unpack(void) {
    if (rawLength == 0) {
        return 0;
    }
    uint16_t bytesParsed = 0;
    // If we have data then assume length is 1 until we parse an actual value
    length = 1;
    bytesParsed++;  // Add one to parsed bytes since all formats have at least 1 byte
    uint8_t rawType = rawBuffer[0];
    if (((rawType & 0x80) == 0) || ((rawType & 0xE0) == 0xE0)) {
        type = MSGPACK_FIXINT_HEAD;
        data = calloc(1,sizeof(int));
        int *ptr = (int*) data;
        *ptr = rawType;
    } else if ((rawType & 0xF0) == MSGPACK_FIXMAP_HEAD) {
        type = MSGPACK_FIXMAP_HEAD;
        length = rawType & 0x0F;
        bytesParsed += unpackMap(bytesParsed,length);
    } else if ((rawType & 0xF0) == MSGPACK_FIXARRAY_HEAD) {
        type = MSGPACK_FIXARRAY_HEAD;
        length = rawType & 0x0F;
        bytesParsed += unpackArray(bytesParsed,length);
    } else if ((rawType & 0xE0) == MSGPACK_FIXSTR_HEAD) {
        type = MSGPACK_FIXSTR_HEAD;
        length = rawType & 0x1F;
        bytesParsed += unpackStr(rawBuffer+1,length);
    } else {
        type = rawType;
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
                data = new uint8_t;
                uint8_t *ptr = (uint8_t*) data;
                *ptr = rawBuffer[1];
                bytesParsed++;
                break;
            }   
            case MSGPACK_UINT16_HEAD: {
                data = new uint16_t;
                uint16_t *ptr = (uint16_t*) data;
                *ptr = rawBuffer[1]<<8 + rawBuffer[2];
                bytesParsed+=2;
                break;
            }   
            case MSGPACK_UINT32_HEAD: {
                data = new uint32_t;
                uint32_t *ptr = (uint32_t*) data;
                *ptr = rawBuffer[1]<<24 + rawBuffer[2]<<16 + rawBuffer[3]<<8 + rawBuffer[4];
                bytesParsed+=4;
                break;
            }   
            case MSGPACK_UINT64_HEAD: {
                data = new uint64_t;
                uint64_t *ptr = (uint64_t*) data;
                *ptr = rawBuffer[1]<<56 + rawBuffer[2]<<48 + rawBuffer[3]<<40 + rawBuffer[4]<<32 + rawBuffer[5]<<24 + rawBuffer[6]<<16 + rawBuffer[7]<<8 + rawBuffer[8];
                bytesParsed+=8;
                break;
            }   
            case MSGPACK_INT8_HEAD: {
                data = new int8_t;
                int8_t *ptr = (int8_t*) data;
                *ptr = rawBuffer[1];
                bytesParsed++;
                break;
            }   
            case MSGPACK_INT16_HEAD: {
                data = new int16_t;
                int16_t *ptr = (int16_t*) data;
                *ptr = rawBuffer[1]<<8 + rawBuffer[2];
                bytesParsed+=2;
                break;
            }   
            case MSGPACK_INT32_HEAD: {
                data = new int32_t;
                int32_t *ptr = (int32_t*) data;
                *ptr = rawBuffer[1]<<24 + rawBuffer[2]<<16 + rawBuffer[3]<<8 + rawBuffer[4];
                bytesParsed+=4;
                break;
            }   
            case MSGPACK_INT64_HEAD: {
                data = new int64_t;
                int64_t *ptr = (int64_t*) data;
                *ptr = rawBuffer[1]<<56 + rawBuffer[2]<<48 + rawBuffer[3]<<40 + rawBuffer[4]<<32 + rawBuffer[5]<<24 + rawBuffer[6]<<16 + rawBuffer[7]<<8 + rawBuffer[8];
                bytesParsed+=8;
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
                length = rawBuffer[1];
                bytesParsed++;
                bytesParsed += unpackStr(rawBuffer+2,length);
                break;
            }   
            case MSGPACK_STR16_HEAD: {
                length = (rawBuffer[1]<<8) + rawBuffer[2];
                bytesParsed += 2;
                bytesParsed += unpackStr(rawBuffer+3,length);
                break;
            }   
            case MSGPACK_STR32_HEAD: {
                // Not Implemented (should never be this big since a single packet can only be 64k)
                break;
            }   
            case MSGPACK_ARRAY16_HEAD: {
                length = rawBuffer[1] << 8 + rawBuffer[2];
                bytesParsed += 2;
                bytesParsed += unpackArray(bytesParsed,length);
                break;
            }   
            case MSGPACK_ARRAY32_HEAD: {
                // Not Implemented (should never be this big since a single packet can only be 64k)
                break;
            }   
            case MSGPACK_MAP16_HEAD: {
                length = rawBuffer[1] << 8 + rawBuffer[2];
                bytesParsed += 2;
                bytesParsed += unpackMap(bytesParsed,length);
                break;
            }   
            case MSGPACK_MAP32_HEAD: {
                // Not Implemented (should never be this big since a single packet can only be 64k)
                break;
            }   
        }
    }
    return bytesParsed;
}

// getItem(item) will return an item from an array or map so you can dig into the data structure.
// item is 0 based index of the array, i should be less than Length (from getLength() below)
OnionPayloadData* OnionPayloadData::getItem(uint16_t i) {
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
uint16_t OnionPayloadData::getLength(void) {
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
int16_t OnionPayloadData::getInt(void) {
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
uint16_t OnionPayloadData::unpackArray(uint16_t bytesParsed,uint16_t length) {
    dataObjectArray = new OnionPayloadData*[length];
    dataIsObject = true;
    for (int x=0;x<length;x++) {
        dataObjectArray[x] = new OnionPayloadData(pkt,offset+bytesParsed); // Begining of sub object is current offset + bytes parsed
        bytesParsed += dataObjectArray[x]->unpack();
    }
    return bytesParsed;
}

uint16_t OnionPayloadData::unpackMap(uint16_t bytesParsed,uint16_t length) {
    dataObjectArray = new OnionPayloadData*[2*length];
    dataIsObject = true;
    dataIsMap = true;
    for (int x=0;x<(2*length);x++) {
        dataObjectArray[x] = new OnionPayloadData(pkt,offset+bytesParsed); // Begining of sub object is current offset + bytes parsed
        bytesParsed += dataObjectArray[x]->unpack();
    }
}

//uint16_t OnionPayloadData::unpackInt(uint8_t* raw,uint8_t bytes) {
//    // The bytes should be 1,2,4,8 for 8/16/32/64 bit ints
//    data = calloc(bytes, sizeof(uint8_t));
//    
//}

uint16_t OnionPayloadData::unpackStr(uint8_t* raw,uint16_t length) {
    data = new char[length+1];
    char* ptr = (char*) data;
    memcpy(ptr,raw,length);
    // Do I really need to add this null? probably, but may not be necessary
    ptr[length] = 0;
    return length;
}

//void OnionPayloadData::unpackNil(void) {
//    
//}
//
//void OnionPayloadData::unpackBool(void) {
//    
//}