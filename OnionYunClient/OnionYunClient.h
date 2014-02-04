#ifndef ONION_CLIENT_H
#define ONION_CLIENT_H

#include <Arduino.h>
#include <Client.h>
#include <YunClient.h> 
#include "OnionParams.h"

// MQTT_MAX_PACKET_SIZE : Maximum packet size
#define MQTT_MAX_PACKET_SIZE 	128

// MQTT_KEEPALIVE : keepAlive interval in Seconds
#define MQTT_KEEPALIVE 			15

#define MQTTPROTOCOLVERSION 	3
#define MQTTCONNECT     		1 << 4  // Client request to connect to Server
#define MQTTCONNACK     		2 << 4  // Connect Acknowledgment
#define MQTTPUBLISH     		3 << 4  // Publish message
#define MQTTPUBACK      		4 << 4  // Publish Acknowledgment
#define MQTTPUBREC      		5 << 4  // Publish Received (assured delivery part 1)
#define MQTTPUBREL      		6 << 4  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     		7 << 4  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   		8 << 4  // Client Subscribe request
#define MQTTSUBACK      		9 << 4  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 		10 << 4 // Client Unsubscribe request
#define MQTTUNSUBACK    		11 << 4 // Unsubscribe Acknowledgment
#define MQTTPINGREQ     		12 << 4 // PING Request
#define MQTTPINGRESP    		13 << 4 // PING Response
#define MQTTDISCONNECT  		14 << 4 // Client is Disconnecting
#define MQTTReserved    		15 << 4 // Reserved

#define MQTTQOS0        		(0 << 1)
#define MQTTQOS1        		(1 << 1)
#define MQTTQOS2        		(2 << 1)

typedef void(*remoteFunction)(OnionParams*);

class OnionYunClient {

public:
	OnionYunClient(char*, char*);
	void begin();
	void get(char*, remoteFunction);
	void post(char*, remoteFunction, char*);
        void update(char*, float);
	boolean loop();

protected:
	void callback(char*, uint8_t*, unsigned int);
	char* registerFunction(remoteFunction);
	boolean connect(char*, char*, char*);
	boolean connected();
	boolean publish(char*, char*);
	boolean subscribe(char*);
	uint16_t readPacket(uint8_t *);
	uint8_t readByte();
	boolean write(uint8_t, uint8_t*, uint16_t);
	uint16_t writeString(char*, uint8_t*, uint16_t);
	
	uint8_t buffer[MQTT_MAX_PACKET_SIZE];
	uint16_t nextMsgId;
	unsigned long lastOutActivity;
	unsigned long lastInActivity;
	bool pingOutstanding;

	// Static data for connecting to Onion
	static char domain[];
	static uint16_t port;

	// Array of functions registered as remote functions and length
	remoteFunction* remoteFunctions;
	unsigned int totalFunctions;
	Client* _client;
	char* deviceId;
	char* deviceKey;

};

#endif

