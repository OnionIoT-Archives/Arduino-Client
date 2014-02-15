#include <SPI.h>
#include <Ethernet.h>
#include <OnionClient.h>
#include <SoftwareSerial.h>
#include <Thermal.h>


byte mac[]    = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFF, 0xFF };

OnionClient client("device_id", "device_key"); 


int printer_RX_Pin = 2;
int printer_TX_Pin = 3;

Thermal printer(printer_RX_Pin, printer_TX_Pin, 19200);

void printMsg(OnionParams* params) {
  char* msg = params->getChar(0);
  printer.print(msg);
  printer.print("\r\n\r\n\r\n");
}

void setup() {
  Ethernet.begin(mac);
  client.begin();
  client.post("/print", printMsg, "msg");
}

void loop() {
  client.loop();
}


