
#include <Bridge.h>
#include <YunClient.h> 
#include <OnionYunClient.h>
#include <SoftwareSerial.h>
#include <Thermal.h>

int printer_RX_Pin = 2;
int printer_TX_Pin = 3;

Thermal printer(printer_RX_Pin, printer_TX_Pin, 19200);

OnionYunClient client("d001", "p"); 

void printMsg(OnionParams* params) {
  char* msg = params->getChar(0);
  printer.print(msg);
  printer.print("\r\n\r\n\r\n");
}

void setup() {

  //printer.doubleHeightOn();
  printer.boldOn();
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  Bridge.begin();
  client.begin();
  client.post("/print", printMsg, "msg");
  
  digitalWrite(13, LOW);
}

void loop() {
  client.loop();
}


