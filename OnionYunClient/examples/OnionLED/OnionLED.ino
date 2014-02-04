
#include <Bridge.h>
#include <YunClient.h> 
#include <OnionYunClient.h>
Thermal printer(printer_RX_Pin, printer_TX_Pin, 19200);

OnionYunClient client("d001", "p"); 

void lightOn(OnionParams* params) {
  digitalWrite(7, HIGH);
}

void lightOff(OnionParams* params) {
  digitalWrite(7, LOW);
} 

void setup() {

  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  pinMode(7, OUTPUT);
  Bridge.begin();
  client.begin();
  client.get("/on", lightOn);
  client.get("/off", lightOff); 
  
  digitalWrite(13, LOW);
}

void loop() {
  client.loop();
}


