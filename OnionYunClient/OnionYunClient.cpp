#include "OnionYunClient.h"
#include "OnionPacket.h"
#include "OnionPayloadData.h"
#include "OnionPayloadPacker.h"
//#include "OnionInterface.h"
#include <stdio.h>
#include <Arduino.h>

char OnionYunClient::domain[] = "zh.onion.io";
uint16_t OnionYunClient::port = 2721;
   
static char* publishMap[] = {"ipAddr","192.168.137.1","mac","deadbeef"};
OnionYunClient::OnionYunClient(char* deviceId, char* deviceKey) {
	this->deviceId = new char[strlen(deviceId) + 1];
	this->deviceId[0] = 0;
	strcpy(this->deviceId, deviceId);

	this->deviceKey = new char[strlen(deviceKey) + 1];
	this->deviceKey[0] = 0;
	strcpy(this->deviceKey, deviceKey);
	
	this->remoteFunctions = new remoteFunction[1];
	this->remoteFunctions[0] = NULL;
	this->totalFunctions = 1;
	this->lastSubscription = NULL;
	totalSubscriptions = 0;
	this->interface = new OnionInterface();
	this->isOnline = false;
	this->isConnected = false;
    this->_client = new YunClient();
    this->_client->setTimeout(10); // Only wait 10 ms for more data
}

void OnionYunClient::begin() {
    //Serial.begin(115200);
	//Serial.print("Start Connection\n");
	if (connect()) {
	    //Serial.print("Sending Subscription Requests\n");
		//subscribe();
	}
}

bool OnionYunClient::connect() {
    if (_client == 0) {
        return false;
    }
	if (!isOnline) {
	    if (_client->connected()) {
	        close();
	    }
		int result = open();

		if (result) {
            OnionPacket* pkt = new OnionPacket(ONION_MAX_PACKET_SIZE);
            pkt->setType(ONIONCONNECT);
            OnionPayloadPacker* pack = new OnionPayloadPacker(pkt);
            pack->packArray(3);
            pack->packInt(ONIONPROTOCOLVERSION);
            pack->packStr(id);
            pack->packStr(key);
            interface->send(pkt);
			lastInActivity = lastOutActivity = millis();
            delete pack;
//            OnionPacket *recv_pkt = interface->getPacket();
//			while (recv_pkt == 0) {
//				unsigned long t = millis();
//				if (t - lastInActivity > ONION_KEEPALIVE * 1000UL) {
//					_client->stop();
//					return false;
//				}
//			    recv_pkt = getPacket();
//			}
//			uint8_t pkt_type = recv_pkt->getType();
//			uint16_t length = recv_pkt->getPayloadLength();
//			uint8_t* payload = recv_pkt->getPayload();
//			if ((pkt_type == ONIONCONNACK) && (length > 0)) {
//			    if (payload[0] == 0) {
//    				lastInActivity = millis();
//    				pingOutstanding = false;
//    				delete recv_pkt;
//    				return true;
//    			}
//			}
//			delete recv_pkt;
		}
		_client->stop();
	}
	return false;
}


char* OnionYunClient::registerFunction(char * endpoint, remoteFunction function, char** params, uint8_t param_count) {
	remoteFunction* resized = new remoteFunction[totalFunctions + 1];
	if (lastSubscription == NULL) {
	    subscriptions.id = totalFunctions;
	    subscriptions.endpoint = endpoint;
	    subscriptions.params = params;
	    subscriptions.param_count = param_count;
	    lastSubscription = &subscriptions;
	} else {
	    subscription_t* new_sub = (subscription_t*)malloc(sizeof(subscription_t));
	    new_sub->id = totalFunctions;
	    new_sub->endpoint = endpoint;
	    new_sub->params = params;
	    new_sub->param_count = param_count;
	    new_sub->next = NULL;
	    lastSubscription->next = new_sub;
	    lastSubscription = new_sub; 
	}
	totalSubscriptions++;
	
	for (int i = 0; i < totalFunctions; i++) {
		resized[i] = remoteFunctions[i];
	}
	
	// Set the last element of resized as the new function
	resized[totalFunctions] = function;
	
	delete [] remoteFunctions;
	remoteFunctions = resized;
	
	
	
	char* idStr = new char[6];
	idStr[0] = 0;
	sprintf(idStr, "%d", totalFunctions);
	totalFunctions++;

	return idStr;
};



void OnionYunClient::callback(uint8_t* topic, byte* payload, unsigned int length) {
	// Get the function ID
	char idStr[6] = "";
	OnionParams* params = NULL;
	bool hasParams = false;
			
	int i = 0;
	for(i; i < length && i < 5; i++) {
		idStr[i] = (char)payload[i];
		if(i < length - 1 && (int)payload[i + 1] == 59) {
			hasParams = true;
			break;
		}
	}
	// Add NULL to end of string
	idStr[++i] = 0;
	unsigned int functionId = atoi(idStr);
	
	if(hasParams) {
		// skip the first ';'
		int offset = ++i;
		char* rawParams = new char[length - offset + 1];
		rawParams[0] = 0;

		// Load elements into raw params
		for(i; i < length; i++) {
			rawParams[i - offset] = (char)payload[i];
		}
		rawParams[length - offset] = 0;
		
		//params = new OnionParams(rawParams);
		delete[] rawParams;
	}
	
	// Call remote function
	if(functionId && functionId < totalFunctions) {
		remoteFunctions[functionId](params);
	}

	delete params;
}

bool OnionYunClient::publish(char* key, char* value) {
	int key_len = strlen(key);
	int value_len = strlen(value);
	if (interface->connected()) {
        OnionPacket* pkt = new OnionPacket(ONION_MAX_PACKET_SIZE);
        pkt->setType(ONIONPUBLISH);
        OnionPayloadPacker* pack = new OnionPayloadPacker(pkt);
        pack->packMap(1);
        pack->packStr(key);
        pack->packStr(value);
        
	    send(pkt);
        //pkt->send();
        delete pack;
        //delete pkt;
	}
	return false;
}

bool OnionYunClient::publish(char** dataMap, uint8_t count) {
    OnionPacket* pkt = new OnionPacket(ONION_MAX_PACKET_SIZE);
    pkt->setType(ONIONPUBLISH);
    OnionPayloadPacker* pack = new OnionPayloadPacker(pkt);
    pack->packMap(count);
    for (uint8_t x=0; x<count; x++) {
        pack->packStr(*dataMap++);
        pack->packStr(*dataMap++);
    }
    
    interface->send(pkt);
    delete pack;
}

bool OnionYunClient::subscribe() {
	if (isConnected) {
	    // Generate 
	    //Serial.print("->Found ");
	    //Serial.print(totalSubscriptions);
	    //Serial.print(" Subscriptions\n");
	    if (totalSubscriptions > 0) {
            OnionPacket* pkt = new OnionPacket(ONION_MAX_PACKET_SIZE);
            pkt->setType(ONIONSUBSCRIBE);
            OnionPayloadPacker* pack = new OnionPayloadPacker(pkt);
	        subscription_t *sub_ptr = &subscriptions;
	        pack->packArray(totalSubscriptions);
        	uint8_t string_len = 0;
        	uint8_t param_count = 0;
        	//Serial.print("->Packing subs\n");
	        for (uint8_t i=0;i<totalSubscriptions;i++) {
	            param_count = sub_ptr->param_count;
	            pack->packArray(param_count+2);
	            pack->packStr(sub_ptr->endpoint);
	            pack->packInt(sub_ptr->id);
	            for (uint8_t j=0;j<param_count;j++) {
	                pack->packStr(sub_ptr->params[j]);
	            }
	            sub_ptr = sub_ptr->next;
	        }
	        send(pkt);
	        return true;
	        //pkt->send();
	        delete pack;
	        //delete pkt;
	    }
	    
	}
	return false;
}

bool OnionYunClient::loop() {
	if (isConnected) {
		unsigned long t = millis();
		if ((t - lastInActivity > ONION_KEEPALIVE * 1000UL) || (t - lastOutActivity > ONION_KEEPALIVE * 1000UL)) {
			if (pingOutstanding) {
				close();
				return false;
			} else {
			    sendPingRequest();
				lastOutActivity = t;
				lastInActivity = t;
				pingOutstanding = true;
			}
		}
        OnionPacket* pkt = getPacket();
		if (pkt != 0) {
			lastInActivity = t;
			uint8_t type = pkt->getType();
			if (type == ONIONCONNACK) {
			    if (pkt->getPayload()[0] == 0) {
    			    subscribe();
    			} else {
    			    close();
    			    delete pkt;
    			    return false;
    			}
		    } else if (type == ONIONSUBACK) {
        	    //Serial.print("Publishing Data\n");
        		//publish("/onion","isAwesome");
        		publish(publishMap,2);
				lastOutActivity = t;
				isOnline = true;
			} else if (type == ONIONPUBLISH) {
			    parsePublishData(pkt);
			} else if (type == ONIONPINGREQ) {
			    // Functionize this
				sendPingResponse();
				lastOutActivity = t;
			} else if (type == ONIONPINGRESP) {
				pingOutstanding = false;
			} 
			delete pkt;
		}
		return true;
	} else {
	    // not connected
	    unsigned long t = millis();
		if (t - lastOutActivity > ONION_RETRY * 1000UL) {
		    this->begin();
		}
	}
}


void OnionYunClient::sendPingRequest(void) {
    OnionPacket* pkt = new OnionPacket(8);
    pkt->setType(ONIONPINGREQ);
    send(pkt);
}

void OnionYunClient::sendPingResponse(void) {
    OnionPacket* pkt = new OnionPacket(8);
    pkt->setType(ONIONPINGRESP);
    send(pkt);
}

void OnionYunClient::parsePublishData(OnionPacket* pkt) {
    
    uint16_t length = pkt->getBufferLength();
    uint8_t *ptr = pkt->getBuffer();
//    Serial.print("Publish Pkt Length = ");
//    Serial.print(length);
//    Serial.print("\n");
    OnionPayloadData* data = new OnionPayloadData(pkt);
    
//    Serial.print("Payload Raw Length = ");
//    Serial.print(data->getRawLength());
//    Serial.print("\n");
    data->unpack();
    uint8_t count = data->getLength();
    uint8_t function_id = data->getItem(0)->getInt();
//    Serial.print("Param Count=");
//    Serial.print(count-1);
//    Serial.print("\n");
//    Serial.print("Function Id=");
//    Serial.print(function_id);
//    Serial.print("\n");
	OnionParams* params = new OnionParams(count-1);
    
	if (count > 1) {
	    // Get parameters
	    for (uint8_t i=0;i<(count-1);i++) {
	        OnionPayloadData* item = data->getItem(i+1);
	        uint8_t strLen = item->getLength();
	        // Test
	        char* buf_ptr = (char *)(item->getBuffer());
//            Serial.print("param #");
//            Serial.print(i+1);
//            Serial.print(" = ");
//            Serial.print(buf_ptr);
//            Serial.print("\n");
//            delay(100);
	        params->setStr(i,buf_ptr,strLen);
	    }
	}
	if (function_id < totalFunctions) {
	    if (remoteFunctions[function_id] != 0) {
	        remoteFunctions[function_id](params);
	    } else {
	        // if the remote function isn't called
	        // no one will delete params, so we have to
	        delete params;
	    }
	} else {
	    // We need to delete this here since no one else can
	    delete params;
	}
	//delete pkt;
	//delete params;
	delete data;
}


int8_t OnionYunClient::open() {
    if (!connected()) {
		return _client->connect(OnionYunClient::domain, OnionYunClient::port);
	}
	return 1;
}

int8_t OnionInterface::send(OnionPacket* pkt) {
        int length = pkt->getBufferLength();
        int rc = _client->write((uint8_t*)pkt->getBuffer(), length);
        if (length == rc) {
            delete pkt;
            return 1;
        }
        delete pkt;
        return 0;
}

OnionPacket* OnionInterface::getPacket(void) {
    uint16_t count = _client->available();
    if (count > 0) {
        if (recvPkt == 0) {
            recvPkt = new OnionPacket(ONION_MAX_PACKET_SIZE);
        }
        
        if (count > recvPkt->getFreeBuffer()) {
            _client->flush();
            delete recvPkt;
            recvPkt = 0;
            return 0;
        }
        uint8_t *ptr = recvPkt->getPtr();
        for (uint16_t x = 0;x<count;x++) {
            *ptr++ = _client->read();
        }
        recvPkt->incrementPtr(count);
        uint8_t type = recvPkt->getType();
        if (recvPkt->isComplete()) {
            OnionPacket* pkt = recvPkt;
            recvPkt = 0;
            return pkt;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

void OnionInterface::close(void) {
    _client->stop();
    isOnline = false;
    isConnected = false;
}

