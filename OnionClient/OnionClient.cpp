#include "OnionClient.h"
#include "msgpack.h"
#include <stdio.h>
#include <Arduino.h>

char OnionClient::domain[] = "zh.onion.io";
uint16_t OnionClient::port = 2721;
const char OnionClient::connectHeader[ONION_HEADER_CONNECT_LENGTH] = { 'O','n','i','o','n', ONIONPROTOCOLVERSION };

OnionClient::OnionClient(char* deviceId, char* deviceKey) {
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
	_client = new RPEthernetClient();
}

void OnionClient::begin() {
    Serial.begin(115200);
	Serial.print("Start Life\n");
	if (connect(deviceId, deviceKey)) {
		//publish("/register", init);
		subscribe();
	}
////	msgpack_sbuffer sbuf;
////	msgpack_sbuffer_init(&sbuf);
////
////	/* serialize values into the buffer using msgpack_sbuffer_write callback function. */
////	msgpack_packer pk;
////	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
////
////	msgpack_pack_array(&pk, 3);
////	msgpack_pack_int(&pk, 1);
////	msgpack_pack_true(&pk);
////	msgpack_pack_raw(&pk, 7);
////	msgpack_pack_raw_body(&pk, "example", 7);
////
////	/* deserialize the buffer into msgpack_object instance. */
////	/* deserialized object is valid during the msgpack_zone instance alive. */
////	msgpack_zone mempool;
////	msgpack_zone_init(&mempool, 2048);
////
////	msgpack_object deserialized;
////	msgpack_unpack(sbuf.data, sbuf.size, NULL, &mempool, &deserialized);
////
////	/* print the deserialized object. */
////	msgpack_object_print(stdout, deserialized);
////	puts("");
////
////	msgpack_zone_destroy(&mempool);
////	msgpack_sbuffer_destroy(&sbuf);
}

boolean OnionClient::connect(char* id, char* key) {
	if (!connected()) {
		int result = _client->connect(OnionClient::domain, OnionClient::port);

		if (result) {
			nextMsgId = 1;
		
			uint8_t *ptr = buffer;
			ptr += 3;
//			uint16_t length = ONION_HEADER_CONNECT_LENGTH;
//			memcpy(ptr,OnionClient::connectHeader,length);
//			ptr += length;
            uint16_t length = 2;
            *ptr++ = 0x93;
            *ptr++ = 0x01;
            *ptr++ = 0xA8;
			memcpy(ptr,id,8);
			ptr += 8;
            *ptr++ = 0xB0;
			memcpy(ptr,key,16);
            length += 26;
			write(ONIONCONNECT, buffer, length);
         
			lastInActivity = lastOutActivity = millis();
         
			while (!_client->available()) {
				unsigned long t = millis();
				if (t - lastInActivity > ONION_KEEPALIVE * 1000UL) {
					_client->stop();
					return false;
				}
			}
			
			uint16_t len = readPacket();
             
			if (len == 4 && buffer[3] == 0) {
				lastInActivity = millis();
				pingOutstanding = false;
				
				return true;
			}
		}
		_client->stop();
	}
	return false;
}

boolean OnionClient::connected() {
	boolean rc;
	if (_client == NULL) {
		rc = false;
	} else {
		rc = (int)_client->connected();
		if (!rc) _client->stop();
	}
	return rc;
}

char* OnionClient::registerFunction(char * endpoint, remoteFunction function, char** params, uint8_t param_count) {
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


void OnionClient::update(char* endpoint, float val) {

	char* value = new char[16];
        value[0]=0;
        dtostrf(val, 0, 2, value);
        

        int payloadLen = strlen(deviceId) + strlen(value) + strlen(endpoint) + 8;
	char* payload = new char[payloadLen];

	payload[0] = 0;

	strcat(payload, deviceId);
	strcat(payload, ";UPDATE;");
	strcat(payload, endpoint);
	strcat(payload, ";");
	strcat(payload, value);
	publish("/register", payload);
	
	delete[] value;
	delete[] payload;
}




void OnionClient::callback(char* topic, byte* payload, unsigned int length) {
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
		
		params = new OnionParams(rawParams);
		delete[] rawParams;
	}
	
	// Call remote function
	if(functionId && functionId < totalFunctions) {
		remoteFunctions[functionId](params);
	}

	delete params;
}

boolean OnionClient::publish(char* topic, char* payload) {
	int plength = strlen(payload);
	if (connected()) {
		// Leave room in the buffer for header and variable length field
		uint16_t length = 5;
		length = writeString(topic, buffer, length);
		uint16_t i;
		for (i = 0; i < plength; i++) {
			buffer[length++] = payload[i];
		}
		uint8_t header = ONIONPUBLISH;

		return write(header, buffer, length - 5);
	}
	return false;
}

boolean OnionClient::subscribe() {
	if (connected()) {
	    // Generate 
	    
	    Serial.print("Start Subscribe: #");
	    Serial.print(totalSubscriptions);
	    Serial.print("\n");
	    if (totalSubscriptions > 0) {
	        uint8_t* ptr = buffer+3;
	        subscription_t *sub_ptr = &subscriptions;
	        *ptr++ = (0x90 + totalSubscriptions);
        	uint8_t string_len = 0;
        	uint8_t param_count = 0;
	        for (uint8_t i=0;i<totalSubscriptions;i++) {
	            param_count = sub_ptr->param_count;
	            *ptr++ = (0x90 + param_count+2);
	            string_len = strlen(sub_ptr->endpoint);
	            *ptr++ = (0xA0 + string_len);
	            memcpy(ptr,sub_ptr->endpoint,string_len);
	            ptr += string_len;
	            *ptr++ = (sub_ptr->id);
	            for (uint8_t j=0;j<param_count;j++) {
	                string_len = strlen(sub_ptr->params[j]);
	                *ptr++ = (0xA0 + string_len);
	                memcpy(ptr,sub_ptr->params[j],string_len);
	                ptr += string_len;
	            }
	            sub_ptr = sub_ptr->next;
	        }
	        return write(ONIONSUBSCRIBE, buffer, ptr-buffer-3);
//	        msgpack_sbuffer sbuf;
//        	msgpack_sbuffer_init(&sbuf);
//        
//        	/* serialize values into the buffer using msgpack_sbuffer_write callback function. */
//        	msgpack_packer pk;
//        	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
//        
//        	msgpack_pack_array(&pk, totalSubscriptions);
//        	subscription_t *sub_ptr = &subscriptions;
//        	uint8_t param_count = 0;
//        	uint8_t string_len = 0;
//        	for (uint8_t i=0;i<totalSubscriptions;i++) {
//        	    param_count = sub_ptr->param_count;
//        	    msgpack_pack_array(&pk, param_count+2);
//        	    string_len = strlen(sub_ptr->endpoint);
//        	    Serial.print("Sub Param Name = ");
//        	    Serial.print(sub_ptr->endpoint);
//        	    Serial.print(", Id = ");
//        	    Serial.print(sub_ptr->id);
//        	    Serial.print("\n");
//        	    msgpack_pack_raw(&pk, string_len);
//        	    msgpack_pack_raw_body(&pk, sub_ptr->endpoint,string_len);
//        	    msgpack_pack_int(&pk, sub_ptr->id);
//        	    for (uint8_t j=0;j<param_count;j++) {
//            	    string_len = strlen(sub_ptr->params[j]);
//            	    msgpack_pack_raw(&pk, string_len);
//            	    msgpack_pack_raw_body(&pk, sub_ptr->params[j],string_len);
//        	    }
//        	}
//        	Serial.print("Buf ");
//        	Serial.print(sbuf.size);
//        	
//        	char* msg = (char *)malloc(sbuf.size+1);
//        	memcpy(msg,sbuf.data,sbuf.size);
//        	msg[sbuf.size] = 0;
//        	
//        	Serial.print("\nMsg=");
//	        Serial.print(msg);
//	        Serial.print("\n");
//	        memcpy(buffer+3,sbuf.data,sbuf.size);
//    		return write(ONIONSUBSCRIBE, buffer, sbuf.size);
	    }
	    
	}
	return false;
}

boolean OnionClient::loop() {
	if (connected()) {
		unsigned long t = millis();
		if ((t - lastInActivity > ONION_KEEPALIVE * 1000UL) || (t - lastOutActivity > ONION_KEEPALIVE * 1000UL)) {
			if (pingOutstanding) {
				_client->stop();
				return false;
			} else {
				buffer[0] = ONIONPINGREQ;
				buffer[1] = 0;
				_client->write(buffer, 2);
				lastOutActivity = t;
				lastInActivity = t;
				pingOutstanding = true;
			}
		}

		if (_client->available()) {
			uint16_t len = readPacket();
			if (len > 0) {
				lastInActivity = t;
				uint8_t type = buffer[0] & 0xF0;
				if (type == ONIONPUBLISH) {
				    uint16_t len = (buffer[1]<<8)+(buffer[2]);
				    // Parse Msg Pack
				    const char* ptr = (const char*)buffer+3;
				    parsePublishData(ptr,len-3);
//					uint16_t tl = (buffer[] << 8) + buffer[llen + 2];
//					char topic[tl + 1];
//					for (uint16_t i=0; i < tl; i++) {
//						topic[i] = buffer[llen + 3 + i];
//					}
//					topic[tl] = 0;
//					// ignore msgID - only support QoS 0 subs
//					uint8_t *payload = buffer + llen + 3 + tl;
//					callback(topic, payload, len - llen - 3 - tl);
				} else if (type == ONIONPINGREQ) {
				    // Functionize this
					buffer[0] = ONIONPINGRESP;
					buffer[1] = 0;
					buffer[2] = 0;
					_client->write(buffer, 2);
				} else if (type == ONIONPINGRESP) {
					pingOutstanding = false;
				}
			}
		}
		return true;
	} else {
	    unsigned long t = millis();
		if (t - lastOutActivity > ONION_KEEPALIVE * 1000UL) {
		    this->connect(deviceId, deviceKey);
		}
	}
}

uint8_t OnionClient::readByte() {
	while(!_client->available()) {}
	return _client->read();
}

uint16_t OnionClient::readPacket() {
	uint16_t len = 0;
	
	Serial.print("In readPacket\n");
	if (_client->available() > 2) {
	    Serial.print("Getting Data\n");
    	buffer[len++] = readByte();
    	buffer[len++] = readByte();
    	buffer[len++] = readByte();
    	uint16_t length = (buffer[1] << 8) + (buffer[2]);
	    Serial.print("Packet Type=");
	    Serial.print(buffer[0]);
	    Serial.print(" Packet Length=");
	    Serial.print(length);
	    Serial.print("\n");
	    if (length > 0) {
        	for (uint16_t i = 0; i < length; i++) {
        		if (len < ONION_MAX_PACKET_SIZE) {
        			buffer[len++] = readByte();
        		} else {
        			readByte();
        			len = 0; // This will cause the packet to be ignored.
        		}
        	}
        }
    }
    Serial.print("Read Packet Competed\n");
	return len;
}


void OnionClient::sendPingRequest(void) {
    
}

void OnionClient::sendPingResponse(void) {
    
}

void OnionClient::parsePublishData(const char *buf, uint16_t len) {
    msgpack_zone mempool;
	msgpack_zone_init(&mempool, 256);

	msgpack_object deserialized;
	msgpack_unpack(buf, len, NULL, &mempool, &deserialized);
	uint8_t count = deserialized.via.array.size;
	uint8_t function_id = deserialized.via.array.ptr[0].via.u64;
	
	OnionParams* params = NULL;
	if (count > 1) {
	    // Get parameters
	}
	if (function_id < totalFunctions) {
	    remoteFunctions[function_id](params);
	}
}

boolean OnionClient::write(uint8_t header, uint8_t* buf, uint16_t length) {
	uint8_t rc;
	
	buf[0] = header;
	buf[1] = length /256;
	buf[2] = length %256;
	rc = _client->write(buf, length + 3);
   
	lastOutActivity = millis();
	return (rc == 3 + length);
}

uint16_t OnionClient::writeString(char* string, uint8_t* buf, uint16_t pos) {
	char* idp = string;
	uint16_t i = 0;
	pos += 2;
	while (*idp) {
		buf[pos++] = *idp++;
		i++;
	}
	buf[pos - i - 2] = (i >> 8);
	buf[pos - i - 1] = (i & 0xFF);
	return pos;
}

