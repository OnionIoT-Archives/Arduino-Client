#ifndef ONION_PACKET_H
#define ONION_PACKET_H

#include <stdint.h>

//class OnionInterface;

class OnionPacket {
public:
    OnionPacket(void);
	OnionPacket(unsigned int length);
	OnionPacket(uint8_t* buffer,unsigned int length);
	~OnionPacket();
	// This will set the data pointer to the next unused byte in the buffer
	// and return the number of bytes free
	uint16_t getFreeBuffer();
	int getBufferLength(void);
	uint8_t* getBuffer(void);
	int getPayloadLength(void);
	int getPayloadMaxLength(void);
	uint8_t* getPayload(void);
	void setPayloadLength(int length);
	void setType(uint8_t packet_type);
	uint8_t getType(void);
	bool send(void);
	static OnionPacket* readPacket(void);
    void updateLength(void);
//    static OnionInterface* interface;
    uint8_t* getPtr(void);
    int8_t incrementPtr(uint16_t count);
    bool isComplete(void);

private:
	uint8_t* buf;
	unsigned int buf_len;
	unsigned int length;
	uint8_t* ptr;
};

#endif
