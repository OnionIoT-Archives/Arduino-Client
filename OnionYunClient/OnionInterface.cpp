#include "OnionInterface.h"
#include "OnionPacket.h"
#include <stdio.h>
    
OnionInterface::OnionInterface() {
    recvPkt = 0;
    _client = new YunClient();
    _client->setTimeout(10); // Only wait 10 ms for more data
}

OnionInterface::OnionInterface(char* host, uint16_t port) {
    OnionInterface();
    open(host,port);
}

int8_t OnionInterface::open(char* host, uint16_t port) {
    if (!connected()) {
		return _client->connect(host, port);
	}
	return 1;
}

bool OnionInterface::connected(void) {
	boolean rc;
	if (_client == NULL) {
		rc = false;
	} else {
		rc = (int)_client->connected();
		if (!rc) {
		    _client->stop();
		}
	}
	return rc;
}


int8_t OnionInterface::send(OnionPacket* pkt) {
        int length = pkt->getBufferLength();
        int rc = _client->write((uint8_t*)pkt->getBuffer(), length);
        if (length == rc) {
            delete pkt;
            return 1;
        }
        delete pkt;
        return 0;
}

OnionPacket* OnionInterface::getPacket(void) {
    uint16_t count = _client->available();
    if (count > 0) {
        if (recvPkt == 0) {
            recvPkt = new OnionPacket(128);
        }
        
        if (count > recvPkt->getFreeBuffer()) {
            _client->flush();
            delete recvPkt;
            recvPkt = 0;
            return 0;
        }
        uint8_t *ptr = recvPkt->getPtr();
        for (uint16_t x = 0;x<count;x++) {
            *ptr++ = _client->read();
        }
        recvPkt->incrementPtr(count);
        uint8_t type = recvPkt->getType();
        if (recvPkt->isComplete()) {
            OnionPacket* pkt = recvPkt;
            recvPkt = 0;
            return pkt;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

void OnionInterface::close(void) {
    _client->stop();
}

