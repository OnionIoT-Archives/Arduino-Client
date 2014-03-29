
#include <Bridge.h>
#include <YunClient.h> 
#include <OnionYunClient.h>
#include <OnionPacket.h>
#include <SoftwareSerial.h>
#include <Thermal.h>

OnionYunClient client("device_id", "device_key"); 

int printer_RX_Pin = 2;
int printer_TX_Pin = 3;

Thermal printer(printer_RX_Pin, printer_TX_Pin, 19200);

void printMsg(char** params) {
  char* msg = params[0];
  printer.print(msg);
  printer.print("\r\n\r\n\r\n");
}
char *printParams[] = {"msg"};
void setup() {

  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  Bridge.begin();
  client.registerFunction("/print", printMsg, printParams,1);
  client.begin();
  
  digitalWrite(13, LOW);
}

void loop() {
  client.loop();
}


