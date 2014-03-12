#ifndef ONION_PAYLOAD_PACKER_H
#define ONION_PAYLOAD_PACKER_H

#include <Arduino.h>

class OnionPayloadPacker {
public:
	OnionPayloadPacker(char* buffer,unsigned int max_length);
	~OnionPayloadPacker();
	void packArray(int len);
	void packMap(int len);
	void packInt(int i);
	void packStr(char* c);
	void packStr(char* c, int len);
	void packNil();
	void packBool(bool b);
	int getLength(void);
	char* getBuffer(void);

private:
	char* buf;
	unsigned int len;
	unsigned int max_len;
};

#endif
