#include "OnionYunClient.h"

char OnionYunClient::domain[] = "dev.onion.io";
uint16_t OnionYunClient::port = 1883;


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
	_client = new YunClient();
}

void OnionYunClient::begin() {
	char* topic = new char[strlen(deviceId) + 2];
	topic[0] = 0;
	strcat(topic, "/");
	strcat(topic, deviceId);

	char* init = new char[strlen(deviceId) + 11];
	init[0] = 0;
	strcat(init, deviceId);
	strcat(init, ";CONNECTED");

	if (connect(deviceId, deviceId, deviceKey)) {
		publish(topic, "Onion is Online!");
		publish("/register", init);
		subscribe(topic);
	}

	delete[] init;
	delete[] topic;
}

boolean OnionYunClient::connect(char* id, char* user, char* pass) {
	if (!connected()) {
		int result = _client->connect(OnionYunClient::domain, OnionYunClient::port);

		if (result) {
			nextMsgId = 1;
			uint8_t d[9] = { 0x00, 0x06, 'M', 'Q', 'I', 's', 'd', 'p', MQTTPROTOCOLVERSION };
			
			// Leave room in the buffer for header and variable length field
			uint16_t length = 5;
			unsigned int j;
			for (j = 0; j < 9; j++) {
				buffer[length++] = d[j];
			}
			
			// No WillMsg
         	uint8_t v = 0x02;
			
			// User
			v = v | 0x80;
			
			// Password
            v = v | (0x80 >> 1);

			buffer[length++] = v;

			buffer[length++] = ((MQTT_KEEPALIVE) >> 8);
			buffer[length++] = ((MQTT_KEEPALIVE) & 0xFF);
		
			// Writing id, user and pass
			length = writeString(id, buffer, length);
			length = writeString(user, buffer, length);
			length = writeString(pass, buffer, length);
         
			write(MQTTCONNECT, buffer, length - 5);
         
			lastInActivity = lastOutActivity = millis();
         
			while (!_client->available()) {
				unsigned long t = millis();
				if (t - lastInActivity > MQTT_KEEPALIVE * 1000UL) {
					_client->stop();
					return false;
				}
			}
			uint8_t llen;
			uint16_t len = readPacket(&llen);

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

boolean OnionYunClient::connected() {
	boolean rc;
	if (_client == NULL) {
		rc = false;
	} else {
		rc = (int)_client->connected();
		if (!rc) _client->stop();
	}
	return rc;
}

char* OnionYunClient::registerFunction(remoteFunction function) {
	remoteFunction* resized = new remoteFunction[totalFunctions + 1];
	
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

void OnionYunClient::get(char* endpoint, remoteFunction function) {
	char* functionId = registerFunction(function);
	char* payload = new char[strlen(deviceId) + strlen(endpoint) + strlen(functionId) + 7];
	payload[0] = 0;

	strcat(payload, deviceId);
	strcat(payload, ";GET;");
	strcat(payload, endpoint);
	strcat(payload, ";");
	strcat(payload, functionId);
	publish("/register", payload);
	
	delete[] functionId;
	delete[] payload;
}

void OnionYunClient::post(char* endpoint, remoteFunction function, char* dataStructure) {
	char* functionId = registerFunction(function);
	char* payload = new char[strlen(deviceId) + strlen(dataStructure) + strlen(endpoint) + strlen(functionId) + 10];
	payload[0] = 0;

	strcat(payload, deviceId);
	strcat(payload, ";POST;");
	strcat(payload, endpoint);
	strcat(payload, ";");
	strcat(payload, functionId);
	strcat(payload, ";");
	strcat(payload, dataStructure);
	publish("/register", payload);
	
	delete[] functionId;
	delete[] payload;
}

void OnionYunClient::update(char* endpoint, float val) {

	char* value = new char[16];
        value[0]=0;
        dtostrf(val, 0, 2, value);
        

        int payloadLen = strlen(deviceId) + strlen(value) + strlen(endpoint) + 8;
	char* payload = new char[payloadLen];

	payload[0] = 0;

	strcat(payload, deviceId);
	strcat(payload, ";");
	strcat(payload, endpoint);
	strcat(payload, ";");
	strcat(payload, value);
	publish("/update", payload);
	
	delete[] value;
	delete[] payload;
}





void OnionYunClient::callback(char* topic, byte* payload, unsigned int length) {
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

boolean OnionYunClient::publish(char* topic, char* payload) {
	int plength = strlen(payload);
	if (connected()) {
		// Leave room in the buffer for header and variable length field
		uint16_t length = 5;
		length = writeString(topic, buffer, length);
		uint16_t i;
		for (i = 0; i < plength; i++) {
			buffer[length++] = payload[i];
		}
		uint8_t header = MQTTPUBLISH;

		return write(header, buffer, length - 5);
	}
	return false;
}

boolean OnionYunClient::subscribe(char* topic) {
	if (connected()) {
		// Leave room in the buffer for header and variable length field
		uint16_t length = 5;
		nextMsgId++;
		if (nextMsgId == 0) {
			nextMsgId = 1;
		}
		buffer[length++] = (nextMsgId >> 8);
		buffer[length++] = (nextMsgId & 0xFF);
		length = writeString(topic, buffer, length);
		buffer[length++] = 0; // Only do QoS 0 subs
		return write(MQTTSUBSCRIBE | MQTTQOS1, buffer, length - 5);
	}
	return false;
}

boolean OnionYunClient::loop() {
	if (connected()) {
		unsigned long t = millis();
		if ((t - lastInActivity > MQTT_KEEPALIVE * 1000UL) || (t - lastOutActivity > MQTT_KEEPALIVE * 1000UL)) {
			if (pingOutstanding) {
				_client->stop();
				return false;
			} else {
				buffer[0] = MQTTPINGREQ;
				buffer[1] = 0;
				_client->write(buffer, 2);
				lastOutActivity = t;
				lastInActivity = t;
				pingOutstanding = true;
			}
		}

		if (_client->available()) {
			uint8_t llen;
			uint16_t len = readPacket(&llen);
			if (len > 0) {
				lastInActivity = t;
				uint8_t type = buffer[0] & 0xF0;
				if (type == MQTTPUBLISH) {
					uint16_t tl = (buffer[llen + 1] << 8) + buffer[llen + 2];
					char topic[tl + 1];
					for (uint16_t i=0; i < tl; i++) {
						topic[i] = buffer[llen + 3 + i];
					}
					topic[tl] = 0;
					// ignore msgID - only support QoS 0 subs
					uint8_t *payload = buffer + llen + 3 + tl;
					callback(topic, payload, len - llen - 3 - tl);
				} else if (type == MQTTPINGREQ) {
					buffer[0] = MQTTPINGRESP;
					buffer[1] = 0;
					_client->write(buffer, 2);
				} else if (type == MQTTPINGRESP) {
					pingOutstanding = false;
				}
			}
		}
		return true;
	} else {
		this->connect(deviceId, deviceId, deviceKey);
	}
}

uint8_t OnionYunClient::readByte() {
	while(!_client->available()) {}
	return _client->read();
}

uint16_t OnionYunClient::readPacket(uint8_t* lengthLength) {
	uint16_t len = 0;
	buffer[len++] = readByte();
	uint8_t multiplier = 1;
	uint16_t length = 0;
	uint8_t digit = 0;
	do {
		digit = readByte();
		buffer[len++] = digit;
		length += (digit & 127) * multiplier;
		multiplier *= 128;
	} while ((digit & 128) != 0);
	*lengthLength = len - 1;
	for (uint16_t i = 0; i < length; i++) {
		if (len < MQTT_MAX_PACKET_SIZE) {
			buffer[len++] = readByte();
		} else {
			readByte();
			len = 0; // This will cause the packet to be ignored.
		}
	}

	return len;
}

boolean OnionYunClient::write(uint8_t header, uint8_t* buf, uint16_t length) {
	uint8_t lenBuf[4];
	uint8_t llen = 0;
	uint8_t digit;
	uint8_t pos = 0;
	uint8_t rc;
	uint8_t len = length;
	do {
		digit = len % 128;
		len = len / 128;
		if (len > 0) {
			digit |= 0x80;
		}
		lenBuf[pos++] = digit;
		llen++;
	} while(len > 0);

	buf[4-llen] = header;
	for (int i = 0; i < llen; i++) {
		buf[5-llen+i] = lenBuf[i];
	}
	rc = _client->write(buf + (4 - llen), length + 1 + llen);
   
	lastOutActivity = millis();
	return (rc == 1 + llen + length);
}

uint16_t OnionYunClient::writeString(char* string, uint8_t* buf, uint16_t pos) {
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

