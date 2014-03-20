#ifndef ONION_PAYLOAD_PACKER_H
#define ONION_PAYLOAD_PACKER_H

#include "OnionPacket.h"

class OnionPayloadPacker {
public:
	OnionPayloadPacker(OnionPacket* pkt);
	~OnionPayloadPacker();
	void packArray(int len);
	void packMap(int len);
	void packInt(int i);
	void packStr(char* c);
	void packStr(char* c, int len);
	void packNil();
	void packBool(bool b);
	int getLength(void);
	uint8_t* getBuffer(void);

private:
	uint8_t* buf;
	unsigned int len;
	unsigned int max_len;
	OnionPacket* pkt;
	void updatePacketLength();
};

#endif
