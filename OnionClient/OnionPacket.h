#ifndef ONION_PACKET_H
#define ONION_PACKET_H


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
	int getLength(void);
	char* getBuffer(void);

private:
	char* buf;
	unsigned int len;
};

#endif
