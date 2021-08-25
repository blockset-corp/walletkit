//
//  File.c
//  
//
//  Created by Amit on 12/08/2021.
//

#include "BRAvaxTransaction.h"
#include "support/BRCrypto.h"
#include "support/BRInt.h"
#include "support/BRKey.h"

void createTx(char * sourceAddress, char * targetAddress, char ** txids, uint64_t amount, char * changeAddress){
    
}

//void avaxTransactionCreate(BRAvalancheXAddress source, BRAvalancheXAddress target, uint64_t amount);
extern struct BaseTxRecord * avaxTransactionCreate(const char* sourceAddress,
                             const char* targetAddress,
                                uint64_t amount, struct BRAvaxUtxoRecord ** utxos){
    
    struct BaseTxRecord * tx = calloc(1, sizeof(struct BaseTxRecord));
    struct TransferableOutputRecord * output = calloc (1, sizeof(struct TransferableOutputRecord));

    struct TranferableInputRecord * input = calloc(1, sizeof(struct TranferableInputRecord));
    
    input->type_id = SECP256K1TransferInput;
    input->input.secp256k1.address_indices_len = 4;
    
    output->type_id = SECP256K1TransferOutput;
    output->output.secp256k1.amount = 166;
    
    tx->network_id = NETWORK_ID_FUJI;
    //tx->blockchain_id
    //tx->memo
    
    tx->outputs = calloc(1, sizeof(struct TransferableOutputRecord *));
    tx->outputs[0] = output;
    tx->outputs_len = 1;
    tx->inputs = calloc(1, sizeof(struct TranferableInputRecord *));
    tx->inputs_len = 1;
    tx->inputs[0]=input;
   
    
    return tx;
}

extern void releaseTransaction(struct BaseTxRecord * tx){
    
    for(int i=0; i < tx->inputs_len; i++){
        free(tx->inputs[i]);
    }
    
    for(int i=0; i < tx->outputs_len; i++){
        free(tx->outputs[i]);
    }
    
    free(tx);
}


const char * MSG_PREFIX = "\x1A""Avalanche Signed Message:\n";
/*
 function digestMessage(msgStr: string) {
     let mBuf = Buffer.from(msgStr, 'utf8')
     let msgSize = Buffer.alloc(4)
     msgSize.writeUInt32BE(mBuf.length, 0)
     let msgBuf = Buffer.from(`\x1AAvalanche Signed Message:\n${msgSize}${msgStr}`, 'utf8')
     return createHash('sha256').update(msgBuf).digest()
 }
 
 */

void digestMessage(uint8_t * buffer32, uint8_t * bytes, size_t len){
    printf("creating array of size: %d",strlen(MSG_PREFIX)+4+len);
    size_t buffer_len =strlen(MSG_PREFIX)+len+4;
    uint8_t * buffer = calloc(buffer_len, sizeof(uint8_t));
    memcpy(buffer, (uint8_t*)MSG_PREFIX, strlen(MSG_PREFIX));
    uint8_t msgsize[4];
    UInt32SetBE(&msgsize[0],len);
    memcpy(&buffer[strlen(MSG_PREFIX)], &msgsize[0]  , sizeof(msgsize));
    memcpy(&buffer[strlen(MSG_PREFIX) + 4], bytes, len);
    
//    printf("result buffer:");
//    for(int i=0; i < strlen(MSG_PREFIX)+len+4 ; i++){
//        printf("%02x", buffer[i]);
//    }
//    printf("expected:1a4176616c616e636865205369676e6564204d6573736167653a0a0000000568656c6c6f\r\n");
    
    BRSHA256(buffer32, buffer,buffer_len);
    free(buffer);
    
}
extern void avaxSignBytes(BRKey * key, uint8_t * bytes, size_t len){
    uint8_t buffer[32];
    //generate digest
    digestMessage(&buffer[0],bytes, len);
        printf("digest created buffer:");
        for(int i=0; i < 32 ; i++){
            printf("%02x", buffer[i]);
        }
        printf("expected:006915e9a6444a3d68ac8fb053eb943e2efa9a54659be2148d2ede63a688c339");
    
    //get safe sig length
    size_t sigLen =BRKeySign(key, NULL, NULL, UInt256Get(buffer));
    uint8_t  sig[sigLen];
    BRKeySign(key, sig, sigLen, UInt256Get(buffer));
    
    
    //this is bizzare, we get the 2 coordinates but a bunch of junk around it
    printf("signature created:");
    for(int i=0; i < sigLen ; i++){
        printf("%02x", sig[i]);
    }
    printf("expected:");
    
}

