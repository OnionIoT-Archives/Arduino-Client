
#include <SPI.h>
#include <Ethernet.h>
#include <OnionClient.h>
#include <OnionPacket.h>

byte mac[]    = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFF, 0xFF };

OnionClient client("device_id", "device_key"); 


void on(char** params) {
  digitalWrite(7, HIGH);
}

void off(char** params) {
  digitalWrite(7, LOW);
} 

void setup() {
  pinMode(7, OUTPUT);
  
  Ethernet.begin(mac);
  client.registerFunction("/on", on);
  client.registerFunction("/off", off); 
  client.begin();
}

void loop() {
  client.loop();
}

