
#include <SPI.h>
#include <Ethernet.h>
#include <OnionClient.h>

byte mac[]    = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFF, 0xFF };

OnionClient client("device_id", "device_key"); 


void on(OnionParams* params) {
  digitalWrite(7, HIGH);
}

void off(OnionParams* params) {
  digitalWrite(7, LOW);
} 

void setup() {
  pinMode(7, OUTPUT);
  
  Ethernet.begin(mac);
  client.get("/on", on);
  client.get("/off", off); 
  client.begin();
}

void loop() {
  client.loop();
}

