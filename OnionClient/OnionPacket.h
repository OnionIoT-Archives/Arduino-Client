#ifndef ONION_PACKET_H
#define ONION_PACKET_H

#include <stdint.h>
// Hack for arduino incase they use only NULL versus null
#ifndef null
#define null    NULL
#endif

class OnionPacket {
public:
    OnionPacket(void);
	OnionPacket(unsigned int length);
	OnionPacket(char* buffer,unsigned int length);
	~OnionPacket();
	int getBufferLength(void);
	char* getBuffer(void);
	int getPayloadLength(void);
	int getPayloadMaxLength(void);
	char* getPayload(void);
	void setPayloadLength(int length);
	void setType(uint8_t packet_type);
	bool send(void);
	static OnionPacket* readPacket(void);
    void updateLength(void);

private:
	char* buf;
	unsigned int buf_len;
	unsigned int length;
};

#endif
