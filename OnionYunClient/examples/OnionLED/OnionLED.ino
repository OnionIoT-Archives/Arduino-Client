
#include <Bridge.h>
#include <YunClient.h> 
#include <OnionYunClient.h>

OnionYunClient client("device_id", "device_key"); 

void lightOn(OnionParams* params) {
  digitalWrite(7, HIGH);
}

void lightOff(OnionParams* params) {
  digitalWrite(7, LOW);
} 

void setup() {

  pinMode(7, OUTPUT);
  Bridge.begin();
  client.begin();
  client.get("/on", lightOn);
  client.get("/off", lightOff); 
  
}

void loop() {
  client.loop();
}


