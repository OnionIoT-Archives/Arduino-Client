#ifndef ONION_TYPE_H
#define ONION_TYPE_H

#include <Arduino.h>

class OnionParams {
public:
	OnionParams(char*);
	~OnionParams();
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
