#include "OnionParams.h"
OnionParams::OnionParams(uint8_t count) {
    if (count > 0) {
        data = new char*[count];
    } else {
        data = NULL;
    }
}

void OnionParams::setStr(uint8_t index,char* str,uint8_t len) {
    data[index] = new char[len+1];
    char* ptr = data[index];
    memcpy(ptr,str,len);
    ptr[len]= 0;
    length = len;
}

OnionParams::OnionParams(char* payload) {
	//Serial.begin(9600);
	length = 0;
	rawData = payload;
	rawLength = strlen(payload);
	
	if(rawLength) {
		char* chunk;
		
		// Add the first chunk
		data = new char*[length + 1];
		data[0] = 0;
		chunk = strtok(payload, ",");
		data[length] = new char[strlen(chunk) + 1];
		strcpy(data[length], chunk);
		length++;

		while(chunk != NULL) {
			chunk = strtok(NULL, ",");
			if(chunk != NULL) {
				char** resized = new char*[length + 1];
				for(int i = 0; i < length; i++) {
					resized[i] = new char[strlen(data[i]) + 1];
					strcpy(resized[i], data[i]);
				}
				delete[] data;
				data = resized;
				data[length] = chunk;
				length++;
			}
		}
	}
}

OnionParams::~OnionParams() {
	for(int i = 0; i < length; i++) {
		delete[] data[i];
	}
	delete[] data;
}

int OnionParams::getInt(unsigned int index) {
	int rc = 0;
	if(index < length) {
		rc = atoi(data[index]);
	}
	return rc;
}

float OnionParams::getFloat(unsigned int index) {
	float rc = 0.0;
	if(index < length) {
		rc = atof(data[index]);
	}
	return rc;
}

bool OnionParams::getBool(unsigned int index) {
	bool rc = false;
	if(index < length) {
		rc = strcmp(data[index], "true");
	}
	return rc;
}

char* OnionParams::getChar(unsigned int index) {
	char* rc = NULL;
	if(index < length) {
		rc = data[index];
	}
	return rc;
}

char* OnionParams::getRaw() {
	return rawData;
}


