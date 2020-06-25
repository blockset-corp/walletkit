//
//  testTezos.c
//  Core
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
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include "support/BRArray.h"
#include "support/BRCrypto.h"
#include "support/BRBIP32Sequence.h"
#include "support/BRBIP39Mnemonic.h"
#include "support/BRBIP39WordsEn.h"
#include "support/BRKey.h"

#include "tezos/BRTezosTransaction.h"
#include "tezos/BRTezosAccount.h"
#include "tezos/BRTezosWallet.h"

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

// MARK: - Account Tests

static void
testCreateTezosAccountWithSeed (void) {
    const char * paper_key = "patient doctor olympic frog force glimpse endless antenna online dragon bargain someone";
    
    UInt512 seed = UINT512_ZERO;
    BRBIP39DeriveKey(seed.u8, paper_key, NULL); // no passphrase
    
    uint8_t expectedPubKey[32];
    hex2bin("efc82a1445744a87fec55fce35e1b7ec80f9bbed9df2a03bcdde1a346f3d4294", expectedPubKey);
    
    BRTezosAccount account = tezosAccountCreateWithSeed(seed);
    assert(account);
    
    BRKey pubKey = tezosAccountGetPublicKey(account);
    assert (0 == (memcmp(expectedPubKey, pubKey.pubKey, 32)));
    
    BRTezosAddress address = tezosAccountGetAddress (account);
    
    const char * expectedAddress = "tz1SeV3tueHQMTfquZSU7y98otvQTw6GDKaY";
    assert (0 == strcmp (expectedAddress, tezosAddressAsString(address)));

    BRTezosAddress addressFromString = tezosAddressCreateFromString(expectedAddress, true);
    assert (1 == tezosAddressEqual(address, addressFromString));
}

static void tezosAccountTests() {
    testCreateTezosAccountWithSeed();
}

// MARK: -

extern void
runTezosTest (void /* ... */) {
    printf("Running tezos unit tests...\n");
    tezosAccountTests();
}

