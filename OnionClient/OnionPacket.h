#ifndef ONION_PACKET_H
#define ONION_PACKET_H

#include <stdint.h>
#include "RPEthernetClient.h"
// Hack for arduino incase they use only NULL versus null
#ifndef null
#define null    NULL
#endif

class OnionPacket {
public:
    OnionPacket(void);
	OnionPacket(unsigned int length);
	OnionPacket(uint8_t* buffer,unsigned int length);
	~OnionPacket();
	int getBufferLength(void);
	uint8_t* getBuffer(void);
	int getPayloadLength(void);
	int getPayloadMaxLength(void);
	uint8_t* getPayload(void);
	void setPayloadLength(int length);
	void setType(uint8_t packet_type);
	bool send(void);
	static OnionPacket* readPacket(void);
    void updateLength(void);
    static Client* client;

private:
	uint8_t* buf;
	unsigned int buf_len;
	unsigned int length;
};

#endif
