//
//  File.c
//  
//
//  Created by Amit on 17/08/2021.
//

#include "BRAvaxEncode.h"
#include "support/BRInt.h"



uint8_t * avaxPackUInt32(uint32_t value, uint8_t * buffer)
{
    UInt32SetBE(buffer, value);
    return buffer ;
}

uint8_t * avaxPackUInt64(uint64_t value, uint8_t* buffer)
{
    UInt64SetBE(buffer, value);
    return buffer;
}

extern uint8_t * avaxPackByteArray(uint8_t * bytes, size_t len,uint8_t * buffer, int include_len_prefix){
    
    assert(bytes!=NULL);
    assert(buffer != NULL);
    size_t offset = 0;
    if(include_len_prefix){
        avaxPackUInt64((uint64_t)len, buffer);
        offset +=4;
    }
    memcpy(&buffer[offset], bytes, len);
    return buffer;
    
}

void packTransferableInput(struct TransferableInputRecord input){
    
    
}

void packTransferableOutput(struct TransferableOutputRecord output){
    
    
}
extern void avaxPackBaseTx(struct BaseTxRecord baseTx){
    
    size_t len = array_count(baseTx.inputs);
    for(int i=0; i < len; i++){
        packTransferableInput(baseTx.inputs[i]);
    }
    
    len = array_count(baseTx.outputs);
    for(int i=0; i < len; i++){
        packTransferableOutput(baseTx.outputs[i]);
    }
}
