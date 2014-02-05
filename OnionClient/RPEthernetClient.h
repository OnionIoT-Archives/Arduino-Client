#ifndef RP_ETHERNET_CLIENT_H
#define RP_ETHERNET_CLIENT_H
#include <Arduino.h>	
#include <Print.h>
#include <Client.h>
#include <IPAddress.h>

class RPEthernetClient : public Client {

public:
  RPEthernetClient();
  RPEthernetClient(uint8_t);

  uint8_t status();
  virtual int connect(IPAddress, uint16_t);
  virtual int connect(const char*, uint16_t);
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t*, size_t);
  virtual int available();
  virtual int read();
  virtual int read(uint8_t*, size_t);
  virtual int peek();
  virtual void flush();
  virtual void stop();
  virtual uint8_t connected();
  virtual operator bool();

  using Print::write;

private:
  static uint16_t _srcport;
  uint8_t _sock;

};

#endif
