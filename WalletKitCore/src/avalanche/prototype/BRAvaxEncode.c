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


int avaxPackTransferableOutput(struct TransferableOutputRecord output, uint8_t * out_buffer, size_t * output_size){
   
   
    uint32_t numAddresses =(uint32_t) array_count(output.output.secp256k1.addresses);
    uint32_t offset = 0;
    if(out_buffer==NULL){
        *output_size =32+4+8+8+4+4+numAddresses*AVAX_RMD_BYTES;
        return 1;
    }
    uint8_t buffer[32+4+8+8+4+4+numAddresses*AVAX_RMD_BYTES]; //bytes type_id+ amount+ locktime + threshold + addresses_len + addresses[20]...
    //TODO: can I use the enum directly for its value or will it break portability ? If mapping then use avaxOutputTypeToInt32
    UInt256Set(&buffer[offset], output.asset.id);
    offset+=32;
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
        offset+= (AVAX_RMD_BYTES);
    }
    printf("\r\nproduced following output buffer:\r\n");
    for(int i=0; i < 32+4+8+8+4+4+numAddresses*AVAX_RMD_BYTES; i++){
        printf("%02x", buffer[i]);
    }
    printf("\r\n");
    *output_size = offset;
    memcpy(out_buffer, &buffer[0], offset);
    return 1;
}

int avaxPackTransferableInput(struct TransferableInputRecord input, uint8_t * out_buffer, size_t * output_size){

    uint32_t numAddresses =(uint32_t) array_count(input.input.secp256k1.address_indices);
    uint32_t offset = 0;
    if(out_buffer==NULL){
        *output_size =32+4+32+4+8+4+numAddresses*4;
        return 1;
    }
    uint8_t buffer[32+4+32+4+8+4+numAddresses*4]; //bytes txid , output_idx + assetId +  type_id+ amount+ addresses_len + address_indices[4]...
    
    UInt256Set(&buffer[offset], input.tx.id);
    offset+=32;
    UInt32SetBE(&buffer[offset], input.utxo_index);
    offset+=4;
    UInt256Set(&buffer[offset], input.asset.id);
    offset+=32;
    UInt32SetBE(&buffer[offset],input.type_id);
    offset+=4;
    UInt64SetBE(&buffer[offset], input.input.secp256k1.amount);
    offset+=8;
//    UInt64SetBE(&buffer[offset], output.output.secp256k1.locktime);
//    offset+=8;
//    UInt32SetBE(&buffer[offset], output.output.secp256k1.threshold);
//    offset+=4;
    UInt32SetBE(&buffer[offset], numAddresses);
    offset+=4;
    for(uint32_t i =0; i < numAddresses; i++){
        UInt32SetBE(&buffer[offset],input.input.secp256k1.address_indices[i]);
        offset+= 4;
    }
    printf("\r\nproduced following input buffer:\r\n");
    for(int i=0; i < 32+4+32+4+8+4+numAddresses*4; i++){
        printf("%02x", buffer[i]);
    }
    printf("\r\n");
    *output_size = offset;
    memcpy(out_buffer, &buffer[0], offset);
    return 1;
}
