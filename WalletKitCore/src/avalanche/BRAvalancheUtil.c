//
//  BRAvalancheUtil.c
//  
//
//  Created by Amit on 17/06/2021.
//

#include "BRAvalancheUtil.h"
#include <arpa/inet.h>

static WKData pack_byte(uint8_t byte){
    return wkDataCopy(byte, 1);
}
static WKData pack_int16(uint16_t int16){
    uint8_t bytes[2];
    bytes[0] = (int16 >> 8) & 0xFF;
    bytes[1] = int16 & 0xFF;
    return wkDataCopy(bytes,sizeof(bytes));
}

static WKData pack_int32(uint32_t int32){
    uint8_t bytes[4];
    bytes[0] = (int32 >> 24) & 0xFF;
    bytes[1] = (int32 >> 16) & 0xFF;
    bytes[2] = (int32 >> 8) & 0xFF;
    bytes[3] = int32 & 0xFF;
    return wkDataCopy(bytes,sizeof(bytes));
}

static WKData pack_int64(uint64_t int32){
    uint8_t bytes[8];
    bytes[0] = (int32 >> 56) & 0xFF;
    bytes[1] = (int32 >> 48) & 0xFF;
    bytes[2] = (int32 >> 40) & 0xFF;
    bytes[3] = (int32 >> 32) & 0xFF;
    bytes[4] = (int32 >> 24) & 0xFF;
    bytes[5] = (int32 >> 16) & 0xFF;
    bytes[6] = (int32 >> 8) & 0xFF;
    bytes[7] = int32 & 0xFF;
    return wkDataCopy(bytes,sizeof(bytes));
}

static WKData pack_ipv4(char * ipaddr){
    uint8_t bytes[18];
    //set first 12 bytes to zero
    memset(bytes,0, 12);
    struct sockaddr_in sa;
    char str[INET_ADDRSTRLEN];

    // store this IP address in sa:
    inet_pton(AF_INET, ipaddr, &(sa.sin_addr));
    memcpy((void*) &sa.sin_addr, &bytes[12], sizeof(sa.sin_addr));
    memcpy((void*)&sa.sin_port, &bytes[17], sizeof(sa.sin_port));
    return wkDataCopy(bytes, 18);
}






