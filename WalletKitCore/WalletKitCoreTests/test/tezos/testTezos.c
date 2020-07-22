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

typedef struct {
    const char * paperKey;
    const char * pubKey;
    const char * address;
} TestAccount;

TestAccount testAccount1 = {
    "patient doctor olympic frog force glimpse endless antenna online dragon bargain someone",
    "efc82a1445744a87fec55fce35e1b7ec80f9bbed9df2a03bcdde1a346f3d4294",
    "tz1SeV3tueHQMTfquZSU7y98otvQTw6GDKaY"
};

// caller must free
static BRTezosAccount
makeAccount(TestAccount accountInfo) {
    UInt512 seed = UINT512_ZERO;
    BRBIP39DeriveKey(seed.u8, testAccount1.paperKey, NULL); // no passphrase
    BRTezosAccount account = tezosAccountCreateWithSeed(seed);
    return account;
}

static void
testCreateTezosAccountWithSeed() {
    BRTezosAccount account = makeAccount(testAccount1);
    assert(account);
    
    uint8_t expectedPubKey[32];
    hex2bin(testAccount1.pubKey, expectedPubKey);
    BRKey pubKey = tezosAccountGetPublicKey(account);
    assert (0 == (memcmp(expectedPubKey, pubKey.pubKey, 32)));
    
    BRTezosAddress address = tezosAccountGetAddress (account);
    
    const char * expectedAddress = testAccount1.address;
    assert (0 == strcmp (expectedAddress, tezosAddressAsString(address)));

    BRTezosAddress addressFromString = tezosAddressCreateFromString(expectedAddress, true);
    assert (1 == tezosAddressEqual(address, addressFromString));
}

static void
testCreateTezosAccountWithSerializedAccount() {
    BRTezosAccount account = makeAccount(testAccount1);
    assert(account);
    
    // Serialize the account
    size_t bytesCount = 0;
    uint8_t * serializedAccount = tezosAccountGetSerialization(account, &bytesCount);
    assert(32 == bytesCount);
    assert(serializedAccount);
    
    // Create a new account with the serialized bytes
    BRTezosAccount account2 = tezosAccountCreateWithSerialization(serializedAccount, bytesCount);
    assert(account2);
    
    BRTezosAddress expectedAddress = tezosAddressCreateFromString(testAccount1.address, true);
    BRTezosAddress account2Address = tezosAccountGetAddress(account2);
    
    assert(1 == tezosAddressEqual(expectedAddress, account2Address));
    
    tezosAddressFree(expectedAddress);
    tezosAddressFree(account2Address);
    tezosAccountFree(account);
    tezosAccountFree(account2);
    free(serializedAccount);
}

// MARK: - Address Tests

static void testAddressCreate() {
    // Prefixes
    assert(NULL != tezosAddressCreateFromString("tz1i5JJDhq7x8gVkpWq2Fwef3k7NEcBj2nJS", true));
    assert(NULL != tezosAddressCreateFromString("tz2TSvNTh2epDMhZHrw73nV9piBX7kLZ9K9m", true));
    assert(NULL != tezosAddressCreateFromString("tz3bEQoFCZEEfZMskefZ8q8e4eiHH1pssRax", true));
    assert(NULL == tezosAddressCreateFromString("tz4i5JJDhq7x8gVkpWq2Fwef3k7NEcBj2nJS", true));
    
    // Originating address
    //assert(NULL != tezosAddressCreateFromString("KT1VG2WtYdSWz5E7chTeAdDPZNy2MpP8pTfL", true));
}

static void testAddressEqual() {
    BRTezosAddress a1 = tezosAddressCreateFromString("tz1i5JJDhq7x8gVkpWq2Fwef3k7NEcBj2nJS", true);
    BRTezosAddress a2 = tezosAddressClone (a1);
    BRTezosAddress a3 = tezosAddressCreateFromString("tz1i5JJDhq7x8gVkpWq2Fwef3k7NEcBj2nJS", true);
    assert(1 == tezosAddressEqual(a1, a2));
    assert(1 == tezosAddressEqual(a1, a3));

    // now check no equal
    BRTezosAddress a4 = tezosAddressCreateFromString("tz1Vs2z88hHRnFLss81M7dXHnbwhZNMDrSgD", true);
    assert(0 == tezosAddressEqual(a1, a4));

    tezosAddressFree (a1);
    tezosAddressFree (a2);
    tezosAddressFree (a3);
    tezosAddressFree (a4);
}

static void testAddressClone() {
    BRTezosAddress a1 = tezosAddressCreateFromString("tz1eEnQhbwf6trb8Q8mPb2RaPkNk2rN7BKi8", true);
    BRTezosAddress a2 = tezosAddressClone (a1);
    BRTezosAddress a3 = tezosAddressClone (a1);
    BRTezosAddress a4 = tezosAddressClone (a1);
    BRTezosAddress a5 = tezosAddressClone (a2);
    BRTezosAddress a6 = tezosAddressClone (a3);

    assert(1 == tezosAddressEqual(a1, a2));
    assert(1 == tezosAddressEqual(a1, a3));
    assert(1 == tezosAddressEqual(a1, a4));
    assert(1 == tezosAddressEqual(a1, a5));
    assert(1 == tezosAddressEqual(a1, a6));
    assert(1 == tezosAddressEqual(a2, a3));

    tezosAddressFree (a1);
    tezosAddressFree (a2);
    tezosAddressFree (a3);
    tezosAddressFree (a4);
    tezosAddressFree (a5);
    tezosAddressFree (a6);
}

static void testFeeAddress() {
    BRTezosAddress feeAddress = tezosAddressCreateFromString("__fee__", false);
    assert (1 == tezosAddressIsFeeAddress(feeAddress));
    char * feeAddressString = tezosAddressAsString (feeAddress);
    assert(0 == strcmp(feeAddressString, "__fee__"));
    free (feeAddressString);
    tezosAddressFree(feeAddress);

    BRTezosAddress address = tezosAddressCreateFromString("tz1eEnQhbwf6trb8Q8mPb2RaPkNk2rN7BKi8", true);
    assert (0 == tezosAddressIsFeeAddress(address));
    tezosAddressFree(address);
    
    assert(NULL == tezosAddressCreateFromString("__fee__", true));
}

void testUnknownAddress() {
    BRTezosAddress address = tezosAddressCreateFromString("unknown", false);
    assert(address);

    char * addressString = tezosAddressAsString(address);
    assert (strcmp(addressString, "unknown") == 0);
    free (addressString);
    tezosAddressFree(address);
    
    assert(NULL == tezosAddressCreateFromString("unknown", true));
}

// MARK: - Transaction Tests



// MARK: - Wallet Tests

static void testCreateWallet() {
    BRTezosAccount account = makeAccount(testAccount1);

    BRTezosWallet wallet = tezosWalletCreate(account);
    assert(wallet);

    BRTezosAddress expectedAddress = tezosAddressCreateFromString(testAccount1.address, true);

    BRTezosAddress sourceAddress = tezosWalletGetSourceAddress(wallet);
    assert(tezosAddressEqual(sourceAddress, expectedAddress) == 1);

    BRTezosAddress targetAddress = tezosWalletGetTargetAddress(wallet);
    assert(tezosAddressEqual(targetAddress, expectedAddress) == 1);

    tezosAccountFree(account);
    tezosWalletFree(wallet);
    tezosAddressFree (expectedAddress);
    tezosAddressFree (sourceAddress);
    tezosAddressFree (targetAddress);
}

static void testWalletBalance() {
    BRTezosAccount account = makeAccount(testAccount1);
    BRTezosWallet wallet = tezosWalletCreate(account);
    BRTezosUnitMutez expectedBalance = 0;
    assert(expectedBalance == tezosWalletGetBalance(wallet));
    
    tezosWalletSetBalance(wallet, 250000);
    assert(250000 == tezosWalletGetBalance(wallet));
    
    tezosWalletSetBalance(wallet, 0);
    
    //TODO:TEZOS add transfers and check balance
    
    tezosWalletFree(wallet);
}

// MARK: -

static void
tezosAccountTests() {
    testCreateTezosAccountWithSeed();
    testCreateTezosAccountWithSerializedAccount();
}

static void
tezosAddressTests() {
    testAddressCreate();
    testAddressEqual();
    testAddressClone();
    testFeeAddress();
    testUnknownAddress();
}

static void
tezosTransactionTests() {
}

static void
tezosWalletTests() {
    testCreateWallet();
    testWalletBalance();
}

// MARK: -

extern void
runTezosTest (void /* ... */) {
    printf("Running tezos unit tests...\n");
    tezosAccountTests();
    tezosAddressTests();
    tezosTransactionTests();
    tezosWalletTests();
}

