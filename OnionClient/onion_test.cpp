// Test for OnionPayloadData

#include <iostream>
#include "OnionPayloadData.h"
#include "OnionPacket.h"
#include "OnionPayloadPacker.h"
#include <stdlib.h>

main() {
    printf("Hello World!\n");
//    char buffer[200];
    OnionPacket* packet = new OnionPacket(200);
    OnionPayloadPacker *packer = new OnionPayloadPacker(packet);
    packer->packArray(3);
    packer->packInt(1);
    //packer->packInt(2);
    //packer->packInt(3);
    packer->packStr("One"); 
    packer->packArray(1);
    packer->packStr("Two");
    int size = packet->getBufferLength();
    packet->setType(0x30);
    packet->updateLength();
//    buffer[0] = 0x10;
//    buffer[1] = 0x00;
//    buffer[2] = size;
//    size+= 3;
    char* buffer = packet->getBuffer();
    char* buf_str = (char*) malloc (2*size + 1);
    char* buf_ptr = buf_str;
    for (int i = 0; i < size; i++)
    {
        buf_ptr += sprintf(buf_ptr, "%02X", (uint8_t)buffer[i]);
    }
    sprintf(buf_ptr,"\n");
    *(buf_ptr + 1) = '\0';
    //printf("Packed data = ");
    printf("%s", buf_str);
    
    // Now unpack the data
    //OnionPacket* pkt = new OnionPacket(buffer,size);
    OnionPacket* pkt = packet;
    //printf("Packet length = %d\n",pkt->getLength());
    //OnionPayloadData* data = new OnionPayloadData(pkt);
    OnionPayloadData* data = new OnionPayloadData();
    //printf("PayloadData's Pkt Pre-init RawLength = %d\n",data->getRawLength());
    data->init(pkt,0);
    //printf("PayloadData's Pkt Post-init RawLength = %d\n",data->getRawLength());
    data->unpack();
    int length = data->getLength();
    //printf ("Unpacked data length=%d",length);
    if (length > 0) {
        OnionPayloadData *item = data->getItem(0);
        //printf ("Unpacked item 1 = %d\n",item->getInt());
        printf ("Unpacked item 1 = %d\n",data->getItem(0)->getInt());
        printf ("Unpacked item 2 = %s\n",data->getItem(1)->getBuffer());
        printf ("Unpacked item 3 = %s\n",data->getItem(2)->getItem(0)->getBuffer());
    }  
    return 0;
}