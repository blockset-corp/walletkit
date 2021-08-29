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

extern uint32_t avaxOutputTypeToInt32(output_type type){
    if(type == SECP256K1TransferOutput){
        return 0x07;
    }
    return 0;
};

extern uint32_t avaxInputTypeToInt32(output_type type){};

void avaxPackTransferableOutput(struct TransferableOutputRecord output, uint8_t * outbuffer){
    
    uint32_t numAddresses =(uint32_t) array_count(output.output.secp256k1.addresses);
    uint32_t offset = 0;
    
    uint8_t buffer[4+8+8+4+4+numAddresses*AVAX_RMD_BYTES]; //bytes type_id+ amount+ locktime + threshold + addresses_len + addresses[20]...
    //TODO: can I use the enum directly for its value or will it break portability ? If mapping then use avaxOutputTypeToInt32
    UInt32SetBE(&buffer[offset],output.type_id);
    offset+=4;
    UInt64SetBE(&buffer[offset], output.output.secp256k1.amount);
    offset+=8;
    UInt64SetBE(&buffer[offset], output.output.secp256k1.locktime);
    offset+=8;
    UInt32SetBE(&buffer[offset], output.output.secp256k1.threshold);
    offset+=4;
    UInt32SetBE(&buffer[offset], numAddresses);
    offset+=4;
    for(uint32_t i =0; i < numAddresses; i++){
        memcpy(&buffer[offset],output.output.secp256k1.addresses[i].rmd160, AVAX_RMD_BYTES);
        offset+= (i*AVAX_RMD_BYTES);
    }
    printf("\r\nproduced following output buffer:\r\n");
    for(int i=0; i < 4+8+8+4+4+numAddresses*AVAX_RMD_BYTES; i++){
        printf("%02x", buffer[i]);
    }
    printf("\r\n");
}
extern void avaxPackBaseTx(struct BaseTxRecord baseTx){
//
//    size_t len = array_count(baseTx.inputs);
//    for(int i=0; i < len; i++){
//        packTransferableInput(baseTx.inputs[i]);
//    }
//
//    len = array_count(baseTx.outputs);
//    for(int i=0; i < len; i++){
//        packTransferableOutput(baseTx.outputs[i]);
//    }
}
