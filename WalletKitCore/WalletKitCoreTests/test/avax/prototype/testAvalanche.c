//
//  testAvalanche.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "support/BRArray.h"
#include "support/BRCrypto.h"
#include "support/BRBIP32Sequence.h"
#include "support/BRBIP39Mnemonic.h"
#include "support/BRBIP39WordsEn.h"
#include "support/BRKey.h"
#include "support/BRBase58.h"

#include "avalanche/prototype/BRAvalancheAccount.h"
#include "avalanche/prototype/BRAvalancheAddress.h"
#include "avalanche/prototype/BRAvalancheTransaction.h"
#include "avalanche/prototype/BRAvaxTransaction.h"

static int debug_log = 0;

static uint8_t char2int(char input)
{
    if(input >= '0' && input <= '9')
        return input - '0';
    if(input >= 'A' && input <= 'F')
        return input - 'A' + 10;
    if(input >= 'a' && input <= 'f')
        return input - 'a' + 10;
    return 0;
}

static void hex2bin(const char* src, uint8_t * target)
{
    while(*src && src[1])
    {
        *(target++) = (char2int(src[0]) << 4) | (char2int(src[1]) & 0x0f);
        src += 2;
    }
}

static void bin2HexString (uint8_t *input, size_t inputSize, char * output) {
    for (size_t i = 0; i < inputSize; i++) {
        sprintf(&output[i*2], "%02x", input[i]);
    }
}

static void printBytes(const char* message, uint8_t * bytes, size_t byteSize)
{
    if (message) printf("%s\n", message);
    for(int i = 0; i < byteSize; i++) {
        if (i >= 0 && i % 8 == 0) printf("\n");
        printf("%02X ", bytes[i]);
    }
    printf("\n");
}

static void printByteString(const char* message, uint8_t * bytes, size_t byteSize)
{
    if (message) printf("%s\n", message);
    for(int i = 0; i < byteSize; i++) {
        printf("%02X", bytes[i]);
    }
    printf("\n");
}

typedef struct {
    const char * paperKey;
    const char * pubKey;
    const char * privKey;
    const char * xaddress;
    const char * caddress;
    const char * ripemd160;
} TestAccount;

//test account was made via :
//https://iancoleman.io/bip39/ - bip32 seed phrase and decoded base58 encoded private seed to:
//https://wallet.avax.network/access/privatekey
TestAccount avaxTestAccount = {
    "patient doctor olympic frog force glimpse endless antenna online dragon bargain someone",
    "029dc79308883267bb49f3924e9eb58d60bcecd17ad3f2f53681ecc5c668b2ba5f",
    "de7176242724956611e9a4f6dfb7a3b3b7eeeec0475b8bccdfec4e52a49c1466",
    "avax1escwyq2hsznvwth6au3gpc77f225uacvwldgal",
    "bbc9bf879c06b13274c200c8b246881ef1ca33a0",
    "cc30e2015780a6c72efaef2280e3de4a954e770c"
    
};

// caller must free
static BRAvalancheAccount
makeAccount(TestAccount accountInfo) {
    UInt512 seed = UINT512_ZERO;
    //generate seed from paperKey
    BRBIP39DeriveKey(seed.u8, accountInfo.paperKey, NULL);
    return avalancheAccountCreateWithSeed(seed);
}

static BRKey
makeKey(TestAccount accountInfo){
    UInt512 seed = UINT512_ZERO;
    //generate seed from paperKey
    BRBIP39DeriveKey(seed.u8, accountInfo.paperKey, NULL);
    BRKey key = deriveAvalanchePrivateKeyFromSeed(seed);
    return key;
}


//Test-Case resource
//https://github.com/ava-labs/ledger-app-avalanche/blob/a340702ec427b15e7e4ed31c47cc5e2fb170ebdf/tests/basic-tests.js#L19


// MARK: -

//Get test cases from ava project itself
//https://github.com/ava-labs/avalanche-wallet/blob/master/tests/js/wallets/SingletonWallet.test.ts
//https://github.com/ava-labs/ledger-app-avalanche/blob/develop/tests/basic-tests.js

void testAddressDecode(){
    //test:
    uint8_t recovered[20];
    size_t rec_len;
   
    avax_addr_bech32_decode(recovered, &rec_len, "avax","avax1escwyq2hsznvwth6au3gpc77f225uacvwldgal" );
    printf("expected:");
    
    for(int i=0; i < rec_len; i++){
        printf("%02x", recovered[i]);
    }
    uint8_t ripemd160[20];
    hex2bin(avaxTestAccount.ripemd160, ripemd160);
    assert(0==memcmp(recovered,ripemd160, sizeof(ripemd160)));
}

extern void runAvalancheTest (void) {
    printf("Running avalanche unit tests...\n");
    BRAvalancheAccount account = makeAccount(avaxTestAccount);
    //printf("%s", (char *)(account->xaddress.bytes));
   // printf("%s", (char *)(account->caddress.bytes));
    assert(0==memcmp(avaxTestAccount.xaddress, (char *)(account->xaddress.bytes), sizeof(account->xaddress.bytes)));
    
    uint8_t ethAddress[20];
    hex2bin(avaxTestAccount.caddress, ethAddress);
    assert(0==memcmp(ethAddress, (char*)account->caddress.bytes,sizeof(account->caddress.bytes)));
    
    struct BaseTxRecord * tx = avaxTransactionCreate("avax1escwyq2hsznvwth6au3gpc77f225uacvwldgal", "avax1escwyq2hsznvwth6au3gpc77f225uacvwldgal", 100, NULL);
    
   
    BRKey key = makeKey(avaxTestAccount);
    key.compressed=0;
    uint8_t msg[5] = { 104,101,108,108,111};
    avaxSignBytes(&key, &msg[0], sizeof(msg));
    
    releaseTransaction(tx);
    
    testAddressDecode();
    printf("DONE");
}


