//
//  BRAvalancheAccount.c
//  
//
//  Created by Amit on 10/06/2021.
//

#include "BRAvalancheAccount.h"

#define AVAX_BIP32_DEPTH 5
//44'/9000'/0'/0/0
#define AVAX_BIP32_CHILD ((const uint32_t []){ 44 | BIP32_HARD, 9000 | BIP32_HARD, 0 | BIP32_HARD, 0, 0 })

#define AVAX_PUBKEY_LENGTH 33

struct BRAvalancheAccountRecord {
    BRAvalancheXAddress xaddress;
    BREthereumAddress caddress;
};

//derives the root key for
static BRKey
deriveAvalanchePrivateKeyFromSeed (UInt512 seed) {
    BRKey privateKey;
    BRBIP32PrivKeyPath(&privateKey, &seed, sizeof(UInt512), AVAX_BIP32_DEPTH, AVAX_BIP32_CHILD);
    printf("000102030405060708090a0b0c0d0e0f/0H/1/2H prv = %s\n", u256hex(privateKey.secret));
    return privateKey;
}

    
extern BRAvalancheAccount  /* caller must free - using "free" function */
    avalancheAccountCreateWithSeed (UInt512 seed){
        BRAvalancheAccount account = calloc(1, sizeof(struct BRAvalancheAccountRecord));
        BRKey privateKey;
        BRBIP32PrivKeyPath(&privateKey, &seed, sizeof(UInt512), AVAX_BIP32_DEPTH, AVAX_BIP32_CHILD);
        privateKey.compressed=0;
        BRKeyPubKey(&privateKey, &(privateKey.pubKey) , 65);
        //we need uncompressed pubkey to generate address
        account->caddress = ethAddressCreateKey(&privateKey);
        //zero out the pubkey so we can reuse
        memset(&privateKey.pubKey, 0,65);
        
        //Avalanche requires compressed pub key
        privateKey.compressed=1;
        BRKeyPubKey(&privateKey, &privateKey.pubKey, 33);
        account->xaddress = avalancheAddressCreateFromKey(&privateKey.pubKey, 33);
        
        //memcpy(&(account->caddress), ethAddress.bytes, sizeof(ethAddress.bytes));
        //cleanup the privateKey
        BRKeyClean(&privateKey);
        return account;
}


//extern BRAvalancheAccount  /* caller must free - using "free" function */
//    avalancheAccountCreateWithSeedIndex (UInt512 seed, uint32_t index){
//        BRAvalancheAccount account = calloc(1, sizeof(struct BRAvalanceAccountRecord));
//        uint32_t * bip32 = malloc(AVAX_BIP32_DEPTH * sizeof(uint32_t));
//        memcpy(bip32,AVAX_BIP32_CHILD, (AVAX_BIP32_DEPTH-1)*sizeof(uint32_t));
//        bip32[AVAX_BIP32_DEPTH-1]= index;
//        BRMasterPubKey pubKey = BRBIP32MasterPubKeyPath(seed.u8, sizeof(seed.u8), AVAX_BIP32_DEPTH,bip32);
//        // Private key
//        BRKey privateKey = deriveAvalanchePrivateKeyFromSeed(seed);
//        avalancheAddressCreateFromKey(pubKey.pubKey, 32);
//        //we dont need reference to the hd wallet path anymore
//        free(bip32);
//        return account;
//}





//generate c-chain address from seed
