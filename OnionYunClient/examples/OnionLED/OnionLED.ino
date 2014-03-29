
#include <Bridge.h>
#include <YunClient.h> 
#include <OnionYunClient.h>
#include <OnionPacket.h>

OnionYunClient client("device_id", "device_key"); 


void on(char** params) {
  digitalWrite(7, HIGH);
}

void off(char** params) {
  digitalWrite(7, LOW);
} 

void setup() {

  pinMode(7, OUTPUT);
  Bridge.begin();
  client.registerFunction("/on", on);
  client.registerFunction("/off", off); 
  client.begin();
}

void loop() {
  client.loop();
}


