#ifndef ONION_CLIENT_H
#define ONION_CLIENT_H

#include <Arduino.h>
#include <Client.h>
//#include <OnionParams.h>
//#include <OnionPacket.h>
//#include "../OnionCore/OnionParams.h"
#include "../OnionCore/OnionPacket.h"
//#include "OnionInterface.h"

// ONION_MAX_PACKET_SIZE : Maximum packet size
#define ONION_MAX_PACKET_SIZE 	128

// ONION CONNECT HEADER LENGTH
#define ONION_HEADER_CONNECT_LENGTH 6

// ONION_KEEPALIVE : keepAlive interval in Seconds
#define ONION_KEEPALIVE 			15
#define ONION_RETRY                 1

#define ONIONPROTOCOLVERSION 	1
#define ONIONCONNECT     		1 << 4  // Client request to connect to Server
#define ONIONCONNACK     		2 << 4  // Connect Acknowledgment
#define ONIONPUBLISH     		3 << 4  // Publish message
#define ONIONPUBACK      		4 << 4  // Publish Acknowledgment
#define ONIONPUBREC      		5 << 4  // Publish Received (assured delivery part 1)
#define ONIONPUBREL      		6 << 4  // Publish Release (assured delivery part 2)
#define ONIONPUBCOMP     		7 << 4  // Publish Complete (assured delivery part 3)
#define ONIONSUBSCRIBE   		8 << 4  // Client Subscribe request
#define ONIONSUBACK      		9 << 4  // Subscribe Acknowledgment
#define ONIONUNSUBSCRIBE 		10 << 4 // Client Unsubscribe request
#define ONIONUNSUBACK    		11 << 4 // Unsubscribe Acknowledgment
#define ONIONPINGREQ     		12 << 4 // PING Request
#define ONIONPINGRESP    		13 << 4 // PING Response
#define ONIONDISCONNECT  		14 << 4 // Client is Disconnecting
#define ONIONReserved    		15 << 4 // Reserved

#define ONIONQOS0        		(0 << 1)
#define ONIONQOS1        		(1 << 1)
#define ONIONQOS2        		(2 << 1)


typedef void(*remoteFunction)(char**);
typedef struct subscription_t {
    uint8_t id;
    char* endpoint;
    char** params;
    uint8_t param_count;
    subscription_t* next;
} subscription_t;
class OnionYunClient {

public:
	OnionYunClient(char*, char*);
	void begin();
	char* registerFunction(char*, remoteFunction, char** params, uint8_t param_count);
    void update(char*, float);
    bool publish(char** dataMap, uint8_t count);
	bool publish(char*, char*);
	bool publish(char*, int);
	bool publish(char*, bool);
	bool publish(char*, double);
	bool loop();

protected:
	//void callback(uint8_t*, uint8_t*, unsigned int);
	void parsePublishData(OnionPacket* pkt);
	void sendPingRequest(void);
	void sendPingResponse(void);
	bool connect();
	bool connected();
	bool subscribe();
	// Interface API
	int8_t open();
	int8_t send(OnionPacket* pkt);
    OnionPacket* getPacket(void);
    void close(void);

	    
	uint16_t readPacket();
	uint8_t readByte();
	
	uint8_t buffer[ONION_MAX_PACKET_SIZE];
	uint16_t nextMsgId;
	unsigned long lastOutActivity;
	unsigned long lastInActivity;
	bool pingOutstanding;

	// Static data for connecting to Onion
	static char domain[];
	static uint16_t port;
	static const uint8_t connectHeader[ONION_HEADER_CONNECT_LENGTH];

	// Array of functions registered as remote functions and length
	remoteFunction* remoteFunctions;
	subscription_t subscriptions;
	subscription_t* lastSubscription;
	uint8_t totalSubscriptions;
	unsigned int totalFunctions;
	//Client* _client;
	//OnionInterface* interface;
	char* deviceId;
	char* deviceKey;
	bool isConnected;
	bool isOnline;

    OnionPacket* recvPkt;
	Client* _client;
};

#endif

