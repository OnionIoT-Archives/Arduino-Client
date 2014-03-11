#ifndef ONION_CLIENT_H
#define ONION_CLIENT_H

#include <Arduino.h>
#include <Client.h>
#include "RPEthernetClient.h"
#include "OnionParams.h"

// ONION_MAX_PACKET_SIZE : Maximum packet size
#define ONION_MAX_PACKET_SIZE 	128

// ONION CONNECT HEADER LENGTH
#define ONION_HEADER_CONNECT_LENGTH 6

// ONION_KEEPALIVE : keepAlive interval in Seconds
#define ONION_KEEPALIVE 			15

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

typedef void(*remoteFunction)(OnionParams*);
typedef struct subscription_t {
    uint8_t id;
    char* endpoint;
    char** params;
    uint8_t param_count;
    subscription_t* next;
} subscription_t;
class OnionClient {

public:
	OnionClient(char*, char*);
	void begin();
	char* registerFunction(char*, remoteFunction, char** params, uint8_t param_count);
    void update(char*, float);
	boolean loop();

protected:
	void callback(char*, uint8_t*, unsigned int);
	void parsePublishData(const char *buf, uint16_t len);
	void sendPingRequest(void);
	void sendPingResponse(void);
	boolean connect(char*, char*);
	boolean connected();
	boolean publish(char*, char*);
	boolean subscribe();
	uint16_t readPacket();
	uint8_t readByte();
	boolean write(uint8_t, uint8_t*, uint16_t);
	uint16_t writeString(char*, uint8_t*, uint16_t);
	
	uint8_t buffer[ONION_MAX_PACKET_SIZE];
	uint16_t nextMsgId;
	unsigned long lastOutActivity;
	unsigned long lastInActivity;
	bool pingOutstanding;

	// Static data for connecting to Onion
	static char domain[];
	static uint16_t port;
	static const char connectHeader[ONION_HEADER_CONNECT_LENGTH];

	// Array of functions registered as remote functions and length
	remoteFunction* remoteFunctions;
	subscription_t subscriptions;
	subscription_t* lastSubscription;
	uint8_t totalSubscriptions;
	unsigned int totalFunctions;
	Client* _client;
	char* deviceId;
	char* deviceKey;

};

#endif

