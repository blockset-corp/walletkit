//
//  BRAvalancheAccount.c
//  
//
//  Created by Amit on 10/06/2021.
//

#include "BRAvalancheAccount.h"
#include "BRAvalancheAddress.h"

#define AVAX_BIP32_DEPTH 5
//44'/9000'/0'/0/0
#define AVAX_BIP32_CHILD ((const uint32_t []){ 44 | BIP32_HARD, 9000 | BIP32_HARD, 0 | BIP32_HARD, 0, 0 })

#define AVAX_PUBKEY_LENGTH 33

struct BRAvalanceAccountRecord {
    BRAvalancheAddress address;
    uint8_t publicKey[33];
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
        BRAvalancheAccount account = calloc(1, sizeof(struct BRAvalanceAccountRecord));
        BRMasterPubKey pubKey = BRBIP32MasterPubKeyPath(seed.u8, sizeof(seed.u8), AVAX_BIP32_DEPTH, AVAX_BIP32_CHILD);
        // Private key
        BRKey privateKey = deriveAvalanchePrivateKeyFromSeed(seed);
        avalancheAddressCreateFromKey(pubKey.pubKey, AVAX_PUBKEY_LENGTH);
        
        //tezosKeyGetPublicKey(privateKey, account->publicKey);

        //account->address = tezosAddressCreateFromKey(account->publicKey, TEZOS_PUBLIC_KEY_SIZE);

        return account;
}


extern BRAvalancheAccount  /* caller must free - using "free" function */
    avalancheAccountCreateWithSeedIndex (UInt512 seed, uint32_t index){
        BRAvalancheAccount account = calloc(1, sizeof(struct BRAvalanceAccountRecord));
        BRMasterPubKey pubKey = BRBIP32MasterPubKeyPath(seed.u8, sizeof(seed.u8), AVAX_BIP32_DEPTH, ((const uint32_t []){ 44 | BIP32_HARD, 9000 | BIP32_HARD, 0 | BIP32_HARD, 0, index }));
        // Private key
        BRKey privateKey = deriveAvalanchePrivateKeyFromSeed(seed);
        avalancheAddressCreateFromKey(pubKey.pubKey, 32);
        //tezosKeyGetPublicKey(privateKey, account->publicKey);

        //account->address = tezosAddressCreateFromKey(account->publicKey, TEZOS_PUBLIC_KEY_SIZE);

        return account;
}





//generate c-chain address from seed
