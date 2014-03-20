#ifndef ONION_INTERFACE_H
#define ONION_INTERFACE_H

#include <Arduino.h>
#include <Client.h>
#include <YunClient.h>


// ONION_KEEPALIVE : keepAlive interval in Seconds
#define ONION_KEEPALIVE 			15

class OnionPacket;
class OnionInterface {

public:
	OnionInterface();
	OnionInterface(char*, uint16_t);
	int8_t open(char*, uint16_t);
	int8_t send(OnionPacket* pkt);
	OnionPacket* getPacket(void);
	bool connected(void);
	void close(void);
	

protected:
    OnionPacket* recvPkt;
	Client* _client;
};

#endif

