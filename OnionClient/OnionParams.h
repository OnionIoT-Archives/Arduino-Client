#ifndef ONION_TYPE_H
#define ONION_TYPE_H

#include <Arduino.h>

class OnionParams {
public:
	OnionParams(char*);
	OnionParams(uint8_t count);
	~OnionParams();
	void setStr(uint8_t index,char* str,uint8_t len);
	int getInt(unsigned int);
	float getFloat(unsigned int);
	bool getBool(unsigned int);
	char* getChar(unsigned int);
	char* getRaw();

private:
	char** data;
	unsigned int length;
	char* rawData;
	unsigned int rawLength;
};

#endif
