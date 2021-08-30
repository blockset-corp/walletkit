//
//  File.c
//  
//
//  Created by Amit on 12/08/2021.
//

#include "BRAvaxTransaction.h"
#include "BRAvaxEncode.h"
#include "support/BRCrypto.h"
#include "support/BRInt.h"
#include "support/BRKey.h"
#include <assert.h>


static size_t // This is BRBase58CheckDecode but w/ only BRSHA256 --
BRAvalancheCB58CheckDecode(uint8_t *data, size_t dataLen, const char *str) {
    size_t len, bufLen = (str) ? strlen(str) : 0;
    uint8_t _buf[0x1000], *buf = (bufLen <= 0x1000) ? _buf : malloc(bufLen);
    UInt256 md;

    assert(str != NULL);
    assert(buf != NULL);
    len = BRBase58Decode(buf, bufLen, str);

    if (len >= 4) {
        len -= 4;
        BRSHA256(md.u8, buf, len);
        if (memcmp(&buf[len], &md.u8[28], sizeof(uint32_t)) != 0) len = 0; // verify checksum
        if (data && len <= dataLen) memcpy(data, buf, len);
    }
    else len = 0;

    mem_clean(buf, bufLen);
    if (buf != _buf) free(buf);
    return (! data || len <= dataLen) ? len : 0;
}

extern UInt256 avaxAssetDecodeAssetId(struct BRAssetRecord asset){
    size_t asset_len = 32;
    uint8_t data[asset_len];
    BRAvalancheCB58CheckDecode(&data[0], asset_len, asset.base58);
    return  UInt256Get(data);
}

extern UInt256 avaxTxidDecodeBase58(struct TxIdRecord tx){
    size_t tx_len=32;
    uint8_t data[tx_len];
    BRAvalancheCB58CheckDecode(&data[0], tx_len, tx.base58);
    return UInt256Get(data);
}

extern UInt256 avaxBlockchainIdDecodeBase58(char * cb58chainId){
    size_t tx_len=32;
    uint8_t data[tx_len];
    BRAvalancheCB58CheckDecode(&data[0], tx_len,cb58chainId);
    return UInt256Get(data);
}

static uint64_t total(BRArrayOf(struct BRAvaxUtxoRecord) utxos){
    size_t len = array_count(utxos);
    uint64_t accum = utxos[0].amount;
    for(int i=1; i< len; i++){
        accum+= utxos[i].amount;
    }
    return accum;
}

//static fee for send: https://docs.avax.network/learn/platform-overview/transaction-fees
static uint64_t fee(){
    return (uint64_t)(1000000000 * 0.001);
}

static int find_utxo_amount(struct BRAvaxUtxoRecord utxo, void * params){
    int * amount = (int *)(params);
    if(utxo.amount >*amount){
        return 1;
    }
    return 0;
}



//https://github.com/ava-labs/avalanchejs/blob/cdcf134bec4a3d85a6b62ec1f02f4afa58351abc/src/apis/avm/utxos.ts#L223
//void findUtxos(BRArrayOf(struct BRAvaxUtxoRecord) utxos){
//    do{
//
//    }while(
//}

int findAddressIndex(uint32_t * index, BRArrayOf(struct AddressRecord) addresses, uint8_t * rmd160){
    assert(index!=NULL);
    for(int i = 0; i < array_count(addresses); i++){
        if(memcmp(addresses[i].rmd160,rmd160,20)==0){
            *index = (uint32_t)i;
            return 1;
        }
    }
    return 0;
}

//expects a sorted list of utxos
BRArrayOf(struct TransferableInputRecord) getMinSpend(BRArrayOf(struct BRAvaxUtxoRecord) utxos, uint8_t * rmd160Source, struct BRAssetRecord asset, uint64_t amount, uint64_t * change){
    uint64_t accum =0;
    size_t num_inputs;
    BRArrayOf(struct TransferableInputRecord) inputs;
    array_new(inputs, 1);
    for(int i=0; i < array_count(utxos) && accum < amount ; i++){
        if(strcmp(utxos[i].asset.base58, asset.base58)==0){
            BRArrayOf(uint32_t) address_indices;
            array_new(address_indices, 1);
            struct TransferableInputRecord input;
            input.asset = asset;
            input.tx = utxos[i].tx;
            //NOTE: we do not support multiple output addresses
            uint32_t index;
            if(!findAddressIndex(&index, utxos[i].addresses, rmd160Source)){
                return NULL;
            }
            array_add(address_indices,index);
            input.input.secp256k1.address_indices = address_indices;
            input.input.secp256k1.address_indices_len = array_count(address_indices);
            input.type_id = SECP256K1TransferInput;
            input.utxo_index = utxos[i].output_index;
            //NOTE: we do not support musig
            //input.input.secp256k1.address_indices_len = 1;
            input.input.secp256k1.amount = utxos[i].amount;
            array_add(inputs,input);
            accum+=utxos[i].amount;
        }
    }
    //we could not process the send
    if(amount>accum){
        return NULL;
    }
    
    if (accum > amount){
        //ensures no overflow on unsign value
        *change = accum - amount;
    }
    return inputs;
}

extern struct BaseTxRecord avaxTransactionCreate(const char* sourceAddress,
                             const char* targetAddress,const char * changeAddress,
                                                 const char *  cb58AssetId,
                                uint64_t amount, BRArrayOf(struct BRAvaxUtxoRecord) utxos, const char * memo,
                                                network_id_t networkId, const char * cb58BlockchainId){
    
    assert(utxos!=NULL);
    
    //Convert char AssetId -> BRAssetRecord
    struct BRAssetRecord asset;
    memcpy(&asset.base58[0], cb58AssetId,strlen(cb58AssetId));
    asset.id = avaxAssetDecodeAssetId(asset);
    
    //Convert char sourceAddress -> uint8_t rmd160
    uint8_t rmd160Source[20];
    size_t rec_len;
    avax_addr_bech32_decode(rmd160Source, &rec_len, "fuji", sourceAddress);
    uint64_t changeAmount;
    BRArrayOf(struct TransferableInputRecord) inputs = getMinSpend(utxos,&rmd160Source[0], asset, amount, &changeAmount);
    
    //SET OUTPUTS
    BRArrayOf(struct AddressRecord) taddresses;
    array_new(taddresses, 1);
    
    BRArrayOf(struct TransferableOutputRecord) outputs;
    array_new(outputs, 2);
    
    size_t address_len;
    struct AddressRecord tAddress;
    avax_addr_bech32_decode(&tAddress.rmd160[0] , &address_len, "fuji", targetAddress);
    array_add(taddresses, tAddress);
    
    struct TransferableOutputRecord targetOutput;
    //TODO:These addresses should be sorted in multi spend environments
    targetOutput.output.secp256k1.addresses = taddresses;
    targetOutput.output.secp256k1.amount = amount;
    targetOutput.output.secp256k1.locktime = 0;
    targetOutput.output.secp256k1.threshold = 1;
    targetOutput.type_id = SECP256K1TransferOutput;
    targetOutput.asset = asset;
    array_add(outputs, targetOutput);
    
    
    //SET change address
    BRArrayOf(struct AddressRecord) caddresses;
    array_new(caddresses, 1);
    struct AddressRecord cAddress;
    avax_addr_bech32_decode(&cAddress.rmd160[0], &address_len, "fuji", changeAddress);
    
    array_add(caddresses,cAddress);
    struct TransferableOutputRecord changeOutput;
    //TODO:These addresses should be sorted in multi spend environments
    changeOutput.output.secp256k1.addresses = caddresses;
    changeOutput.output.secp256k1.amount = changeAmount - fee();
    changeOutput.output.secp256k1.locktime = 0;
    changeOutput.output.secp256k1.threshold = 1;
    changeOutput.type_id = SECP256K1TransferOutput;
    changeOutput.asset = asset;
    array_add(outputs, changeOutput);
    struct BaseTxRecord tx;
    
    tx.inputs = inputs;
    tx.outputs = outputs;
    
    memcpy(&tx.memo[0], memo, strlen(memo));
    tx.network_id = networkId;
    memcpy(&tx.blockchain_id[0], avaxBlockchainIdDecodeBase58(cb58BlockchainId).u8, sizeof(tx.blockchain_id));
    tx.type_id =BaseTx;
    tx.codec = 0x00;
    return tx;
}


extern void releaseTransaction(struct BaseTxRecord * tx){
    for(uint i = 0; i < array_count(tx->inputs); i++){
        array_clear(tx->inputs[i].input.secp256k1.address_indices);
        array_free(tx->inputs[i].input.secp256k1.address_indices);
    }
    array_clear(tx->inputs);
    array_free(tx->inputs);
    array_clear(tx->outputs);
    array_free(tx->outputs);
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
extern void avaxSignBytes(BRKey * key, uint8_t * bytes, size_t len, uint8_t * sig65){
    assert(sig65!= NULL);
    
    uint8_t buffer[32];
    //generate digest
    digestMessage(&buffer[0],bytes, len);
//        printf("digest created buffer:");
//        for(int i=0; i < 32 ; i++){
//            printf("%02x", buffer[i]);
//        }
//        printf("expected:006915e9a6444a3d68ac8fb053eb943e2efa9a54659be2148d2ede63a688c339");
//
    //get safe sig length
    //size_t sigLen =BRKeySign(key, NULL, NULL, UInt256Get(buffer));
    uint8_t  sig[65];
    BRKeyCompactSignEthereum(key, sig, sizeof(sig), UInt256Get(buffer));
//    size_t b58buffer_len =  BRBase58CheckEncode(NULL, NULL, &sig[0], 65);
//    uint8_t b58buffer[b58buffer_len];
//    //TODO: Replaec this Base58 with the single sha hash and varying bytes selection for checksum
//    BRBase58CheckEncode(b58buffer, b58buffer_len, &sig[0], 65);
    
    //this is bizzare, we get the 2 coordinates but a bunch of junk around it
//
//    printf("cb58 created:");
//    printf("%s", b58buffer);
//
//    printf("expected:3TjPMbnKynALAFiRqYoQBTB62HZC7sd62R1yZwNK5ha45z2bN2uGPUoTpmKpGufjRpGxLtptcEAtsYmTZYWdLvUJqyR9CWh");
    
    printf("signature created:");
    for(int i=0; i < 65 ; i++){
        printf("%02x", sig[i]);
    }
    
    memcpy(sig65, &sig[0], 65);
    
    
}

