//
//  testStellar.c
//  Core
//
//  Created by Carl Cherry on 2021--5-18.
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
#include "support/BRBase58.h"
#include "stellar/BRStellar.h"
#include "stellar/utils/b64.h"

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

static BRStellarAccount createTestAccount(const char* paper_key,
                              const char* public_key_string, const char* expected_address)
{
    BRStellarAccount account = stellarAccountCreate(paper_key);

    if (public_key_string) {
        uint8_t expected_public_key[32];
        hex2bin(public_key_string, expected_public_key);
        BRKey key = stellarAccountGetPublicKey(account);
        if (debug_log) printBytes("PublicKey:", key.pubKey, 32);
        assert(0 == memcmp(key.pubKey, expected_public_key, sizeof(expected_public_key)));
    }

    BRStellarAddress address = stellarAccountGetAddress(account);
    char * addressAsString = stellarAddressAsString(address);
    if (debug_log) printf("stellar address: %s\n", addressAsString);
    if (expected_address) {
        assert(0 == memcmp(addressAsString, expected_address, strlen(expected_address)));
    }
    free(addressAsString);
    stellarAddressFree(address);
    stellarAccountFree(account);

    // Now work backwards from the string
    if (expected_address && public_key_string) {
        BRStellarAddress stellarAddress = stellarAddressCreateFromString(expected_address, true);
        size_t rawAddressSize = stellarAddressGetRawSize (stellarAddress);
        uint8_t *rawAddress = calloc(1, rawAddressSize);
        stellarAddressGetRawBytes(stellarAddress, rawAddress, rawAddressSize);
        // Convert the incoming public key string to bytes and compare
        uint8_t *expectedRawAddress = calloc(1, strlen(public_key_string) / 2);
        hex2bin(public_key_string, expectedRawAddress);
        assert(0 == memcmp(rawAddress, expectedRawAddress, rawAddressSize));
        free(expectedRawAddress);
        free(rawAddress);
        stellarAddressFree(stellarAddress);
    }
}

static void runAccountTests()
{
    // Account we use for sending on TESTNET
    const char * paper_key = "patient doctor olympic frog force glimpse endless antenna online dragon bargain someone";
    const char* public_key_string = "5562f344b6471448b7b6ebeb5bae9c1cecc930ef28868be2bb78bb742831e710";
    const char* expected_address = "GBKWF42EWZDRISFXW3V6WW5OTQOOZSJQ54UINC7CXN4LW5BIGHTRB3BB";
    createTestAccount(paper_key, public_key_string, expected_address);

    // Test Account - first reference account from
    // https://github.com/stellar/stellar-protocol/blob/master/ecosystem/sep-0005.md
    // illness spike retreat truth genius clock brain pass fit cave bargain toe
    createTestAccount("illness spike retreat truth genius clock brain pass fit cave bargain toe", NULL,
        "GDRXE2BQUC3AZNPVFSCEZ76NJ3WWL25FYFK6RGZGIEKWE4SOOHSUJUJ6");

    // Test Account - second reference account (15 words) from
    // https://github.com/stellar/stellar-protocol/blob/master/ecosystem/sep-0005.md
    // illness spike retreat truth genius clock brain pass fit cave bargain toe
    createTestAccount("resource asthma orphan phone ice canvas fire useful arch jewel impose vague theory cushion top", NULL, "GAVXVW5MCK7Q66RIBWZZKZEDQTRXWCZUP4DIIFXCCENGW2P6W4OA34RH");

    // Test Account - third reference account (24 words) from
    // https://github.com/stellar/stellar-protocol/blob/master/ecosystem/sep-0005.md
    // illness spike retreat truth genius clock brain pass fit cave bargain toe
    createTestAccount("bench hurt jump file august wise shallow faculty impulse spring exact slush thunder author capable act festival slice deposit sauce coconut afford frown better", NULL, "GC3MMSXBWHL6CPOAVERSJITX7BH76YU252WGLUOM5CJX3E7UCYZBTPJQ");

    // Account we use for receiving on TESTNET
    createTestAccount("off enjoy fatal deliver team nothing auto canvas oak brass fashion happy",
                      "240FFEB7CF417181B0B0932035F8BC086B04D16C18B1DB8C629F1105E2687AD1",
                      "GASA77VXZ5AXDANQWCJSANPYXQEGWBGRNQMLDW4MMKPRCBPCNB5NC77I");

    createTestAccount("release pudding vault own maximum correct ramp cactus always cradle split space",
                                NULL, "GCWRMSOP3RKTOORIW4FRQQVS6HKPEA4LC4QAFV5KLBIH3FYCG3DNKUZ7");

    // Account "Ted"
    createTestAccount("brave rival swap wrestle gorilla diet lounge farm tennis capital ecology design",
                                NULL, "GDSTAICFVBHMGZ4HI6YEKZSGDR7QGEM4PPREYW2JV3XW7STVM7L5EDYZ");
}

// MARK: - Transaction Tests

static void serializeAndSign()
{
    char targetAddress[128];
    memset(targetAddress, 0x00, 128);
    strcpy(targetAddress, "GBKWF42EWZDRISFXW3V6WW5OTQOOZSJQ54UINC7CXN4LW5BIGHTRB3BB");
    //const char * targetAddress = "GBKWF42EWZDRISFXW3V6WW5OTQOOZSJQ54UINC7CXN4LW5BIGHTRB3BB";
    BRStellarAddress destination = stellarAddressCreateFromString(targetAddress, true);

    BRStellarAccount account = stellarAccountCreate("off enjoy fatal deliver team nothing auto canvas oak brass fashion happy");
    stellarAccountSetBlockNumberAtCreation(account, 465958);
    // 2001274371309568 = 465958 << 32
    // which means to get orginal sequenc number 2001274371309576 we will need
    // to use a block number of 465958 and a sequence number of 7 (since it get's incremented)
    stellarAccountSetSequence(account, 7);
    stellarAccountSetNetworkType(account, STELLAR_NETWORK_TESTNET);
    BRStellarAddress sourceAddress = stellarAccountGetPrimaryAddress(account);

    BRStellarMemo memo;
    memo.memoType = 1;
    strcpy(memo.text, "Buy yourself a beer!");

    BRStellarFeeBasis fee;
    fee.costFactor = 1;
    fee.pricePerCostFactor = 100;
    BRStellarAmount amount = 105000000;
    BRStellarTransaction transaction = stellarTransactionCreate(sourceAddress, destination,
                                                                amount, fee);
    stellarTransactionSetMemo(transaction, &memo);
    // Generate the 512bit private key using a BIP39 paperKey
    UInt512 seed = UINT512_ZERO;
    BRBIP39DeriveKey(seed.u8, "off enjoy fatal deliver team nothing auto canvas oak brass fashion happy", NULL); // no passphrase

    size_t tx_size = stellarAccountSignTransaction(account, transaction, seed);

    uint8_t *tx_bytes = stellarTransactionSerialize(transaction, &tx_size);
    assert(tx_size > 0);
    assert(tx_bytes);

    // Base64 the bytes
    char * encoded = b64_encode(tx_bytes, tx_size);
    if (debug_log) {
        printf("encoded bytes: %s\n", encoded);
        printBytes("sBytes:", tx_bytes, tx_size);
    }
    // Compare with what we are expecting
    const char* expected_b64 = "AAAAACQP/rfPQXGBsLCTIDX4vAhrBNFsGLHbjGKfEQXiaHrRAAAAZAAHHCYAAAAIAAAAAAAAAAEAAAAUQnV5IHlvdXJzZWxmIGEgYmVlciEAAAABAAAAAAAAAAEAAAAAVWLzRLZHFEi3tuvrW66cHOzJMO8ohoviu3i7dCgx5xAAAAAAAAAAAAZCLEAAAAAAAAAAAeJoetEAAABAzBQpbrqpbfFozHnwpIATkErUPcb5xesMeFClf5dyd4X0kBw3c6gZUVTtHh3iCZ6eUAEge/lCft6NfXzsHy1HBQ==";
    assert(0 == memcmp(encoded, expected_b64, strlen(expected_b64)));
    assert(strlen(encoded) == strlen(expected_b64));
    free(encoded);

    stellarTransactionFree(transaction);
}

static void runTransactionTests()
{
    serializeAndSign();
}

extern void
runStellarTest (void /* ... */) {
    printf("Running stellar unit tests...\n");
    runAccountTests();
    runTransactionTests();
}

