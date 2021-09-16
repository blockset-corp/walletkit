//
//  testAvalanche.c
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Key WalletKit includes
#include "support/BRArray.h"
#include "support/BRInt.h"
#include "support/BRBIP39Mnemonic.h"
#include "support/util/BRHex.h"

#include "avalanche/BRAvalanche.h"
#include "avalanche/BRAvalancheSupport.h"

// MARK: - Utils Test

static void
runAvalancheUtilsCB58Test (void) {
    static struct { char *data; char *cb58; } vectors[] = {
        { "Hello world", "32UWxgjUJd9s6Kyvxjj1u" },
        { NULL, NULL }
    };
    printf("TST:    Avalanche Utils CB58\n");

    for (size_t index = 0; vectors[index].data != NULL; index++ ) {
        char *data = vectors[index].data;
        char *cb58 = vectors[index].cb58;

        char *cb58Test = BRAvalancheCB58CheckEncodeCreate ((uint8_t*) data, strlen(data));
        assert (0 == strcmp (cb58Test, cb58));

        size_t   dataTestLength;
        uint8_t *dataTest = BRAvalancheCB58CheckDecodeCreate (cb58Test, &dataTestLength);
        assert (dataTestLength == strlen (data));
        assert (0 == memcmp (dataTest, data, dataTestLength));

        free (cb58Test);
        free (dataTest);
    }
}

static void
runAvalancheUtilsTest (void) {
    printf("TST:    Avalanche Utils\n");

    runAvalancheUtilsCB58Test ();

    return;
}

// MARK: - Hash Test

static void
runAvalancheHashTest (void) {
    static struct {
        char *bytesHex;
        char *hashHex;
        char *hashString;

    } vectors[] = {
        {
            "00000000000000000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000700000000631f5dc000000000000000000000000100000001cc30e2015780a6c72efaef2280e3de4a954e770c3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000265ed870000000000000000000000000100000001b47e92d8d0d9125910d56fc1eba52c272b90876c000000021c8acd205ff6161efce0952071fc63ca8e99717bb829e74ce33997c2b369334b000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000500000000773594000000000100000000450a5390bcf287869b9dcef42ca6b4305fde20e5f29d40e719a87fe7dd043600000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000251e6930000000001000000000000000568656c6c6f",
            "31d3ab6136b423dc0b4ed69769dcfd3207067d94534357e6410a61e5a6859b48",
            "Nwm4H32eP8RsQB5iTeEyqqCJSam9qe9BWAvDhwkGssqpbQjfy"
        },

        { NULL, NULL, NULL }
    };
    printf("TST:    Avalanche Hash\n");

    for (size_t index = 0; vectors[index].bytesHex != NULL; index++ ) {
        size_t   bytesCount;
        uint8_t *bytes = hexDecodeCreate(&bytesCount, vectors[index].bytesHex, strlen (vectors[index].bytesHex));

        BRAvalancheHash hash = avalancheHashCreate(bytes, bytesCount);
        char *hashHex = hexEncodeCreate (NULL, hash.bytes, sizeof(hash.bytes));
        assert (0 == strcmp (hashHex, vectors[index].hashHex));
        free (hashHex);
        free (bytes);

        char *hashString = avalancheHashToString (hash);
        assert (0 == strcmp (hashString, vectors[index].hashString));

        BRAvalancheHash hashFromString = avalancheHashFromString(hashString);
        assert (avalancheHashIsEqual (&hash, &hashFromString));
        free (hashString);
    }
}

// MARK: - Address Test

static void
runAvalancheAddressTest (void) {
    static struct {
        const char * paperKey;
        const char * pubKey;
        const char * privKey;
        uint8_t     ripemd160[20];
        const char * ripemd160Str;
        const char * xaddress;
        const char * caddress;
        const char * network;
    } vectors[] = {
        {
            //test account was made via :
            //https://iancoleman.io/bip39/ - bip32 seed phrase and decoded base58 encoded private seed to:
            //https://wallet.avax.network/access/privatekey
            "patient doctor olympic frog force glimpse endless antenna online dragon bargain someone",
            "029dc79308883267bb49f3924e9eb58d60bcecd17ad3f2f53681ecc5c668b2ba5f",
            "de7176242724956611e9a4f6dfb7a3b3b7eeeec0475b8bccdfec4e52a49c1466",
            { 0xcc, 0x30, 0xe2, 0x01, 0x57, 0x80, 0xa6, 0xc7, 0x2e, 0xfa, 0xef, 0x22, 0x80, 0xe3, 0xde, 0x4a, 0x95, 0x4e, 0x77, 0x0c },
            "cc30e2015780a6c72efaef2280e3de4a954e770c",
            "X-avax1escwyq2hsznvwth6au3gpc77f225uacvwldgal",
            "0xbbc9bf879c06b13274c200c8b246881ef1ca33a0",
            "AVAX Mainnet"
        },
        { NULL, NULL, NULL, {0}, NULL, NULL, NULL, NULL }
    };
    printf("TST:    Avalanche Address\n");


    for (size_t index = 0; vectors[index].paperKey != NULL; index++) {
        BRAvalancheNetwork network = (0 == strcmp ("AVAX Mainnet", vectors[index].network)
                                      ? avaxNetworkMainnet
                                      : (0 == strcmp ("AVAX Testnet", vectors[index].network)
                                         ? avaxNetworkTestnet
                                         : NULL));
        assert (NULL != network);

        // 'raw'
        uint8_t addr[20]; size_t addrLen;
        avax_addr_bech32_decode (addr, &addrLen, "avax", &vectors[index].xaddress[2]);
        assert (0 == memcmp (addr, vectors[index].ripemd160, 20));

        UInt512 seed = UINT512_ZERO;
        BRBIP39DeriveKey(seed.u8, vectors[index].paperKey, NULL);
        BRAvalancheAccount account = avalancheAccountCreateWithSeed (seed);


        // X address
        BRAvalancheAddress addressX = avalancheAccountGetAddress (account, AVALANCHE_CHAIN_TYPE_X);
        char *addressXString = avalancheNetworkAddressToString (network, addressX);
        assert (0 == strcmp (addressXString, vectors[index].xaddress));
        assert (avalancheAddressEqual (addressX, avalancheNetworkStringToAddress (network, vectors[index].xaddress, true)));
        free (addressXString);

        // C address
        BRAvalancheAddress addressC = avalancheAccountGetAddress (account, AVALANCHE_CHAIN_TYPE_C);
        // char *addressCString = avalancheNetworkAddressToString (network, addressC);
        //assert (0 == strcmp (addressCString, vectors[index].caddress));
        // assert (avalancheAddressEqual (addressC, avalancheAddressCreateFromString(vectors[index].caddress, true, AVALANCHE_CHAIN_TYPE_C)));

        avalancheAccountFree (account);
    }
}

// MARK: - Account Test

static void
runAvalancheAccountTest (void) {
    static struct {
        const char * paperKey;
    } vectors[] = {
        { "patient doctor olympic frog force glimpse endless antenna online dragon bargain someone" },
        { NULL }
    };

    printf("TST:    Avalanche Account\n");

    for (size_t index = 0; vectors[index].paperKey != NULL; index++) {
        UInt512 seed = UINT512_ZERO;
        BRBIP39DeriveKey(seed.u8, vectors[index].paperKey, NULL);
        BRAvalancheAccount account = avalancheAccountCreateWithSeed (seed);

        // Serialize
        size_t   accountSerializationSize;
        uint8_t *accountSerialization = avalancheAccountGetSerialization (account, &accountSerializationSize);

        // Deserialize
        BRAvalancheAmount accountRecovered = avalancheAccountCreateWithSerialization (accountSerialization,
                                                                                      accountSerializationSize);
        free (accountSerialization);

        assert (avalancheAddressEqual (avalancheAccountGetAddress (account, AVALANCHE_CHAIN_TYPE_X),
                                       avalancheAccountGetAddress (accountRecovered, AVALANCHE_CHAIN_TYPE_X)));

        assert (avalancheAddressEqual (avalancheAccountGetAddress (account, AVALANCHE_CHAIN_TYPE_C),
                                       avalancheAccountGetAddress (accountRecovered, AVALANCHE_CHAIN_TYPE_C)));

        assert (avalancheAccountHasAddress(accountRecovered,
                                           avalancheAccountGetAddress (account, AVALANCHE_CHAIN_TYPE_X)));

        assert (avalancheAccountHasAddress(accountRecovered,
                                           avalancheAccountGetAddress (account, AVALANCHE_CHAIN_TYPE_C)));

        int hasLimit = 0;
        assert (0 == avalancheAccountGetBalanceLimit (accountRecovered, 0, &hasLimit) && 0 == hasLimit);
        assert (0 == avalancheAccountGetBalanceLimit (accountRecovered, 1, &hasLimit) && 0 == hasLimit);

        avalancheAccountFree (account);
        avalancheAccountFree (accountRecovered);
    }
}

// MARK: - Signature Test

static void
runAvalancheSignatureTest (void) {
    static struct {
        const char *paperKey;
        const char *message;
        const char *signatureHexEncoded;
    } vectors[] = {
        {
            "patient doctor olympic frog force glimpse endless antenna online dragon bargain someone",
            "hello",
            "0xf72ca286c8e6f1a0ddf1fb6ee18c93cd649cf058b4ce7e75fab3ab2cabeb29af2fdafd1b57bdefddfaddc3b89d333f5b5dbb02928a416f500792df201ad4424a01"
        },

        { NULL, NULL, NULL }
    };
    printf("TST:    Avalanche Signature\n");

    for (size_t index = 0; vectors[index].paperKey != NULL; index++) {
        UInt512 seed = UINT512_ZERO;
        BRBIP39DeriveKey(seed.u8, vectors[index].paperKey, NULL);
        BRAvalancheAccount account = avalancheAccountCreateWithSeed (seed);

        size_t   messageCount;
        uint8_t *message = avalancheAccountCreateStandardMessage (account,
                                                                  (uint8_t *) vectors[index].message,
                                                                  strlen (vectors[index].message),
                                                                  &messageCount);

        BRAvalancheSignature signature = avalancheAccountSignData (account,
                                                                   message,
                                                                   messageCount,
                                                                   seed);

        BRData signatureData = avalancheSignatureGetBytes(&signature);
        assert (dataEqualsHexString(signatureData, &vectors[index].signatureHexEncoded[2]));
        dataFree (signatureData);
#if 0
        size_t   signatureBytesCount;
        uint8_t *signatureBytes      = avalancheSignatureGetBytes (&signature, &signatureBytesCount);
        char    *signatureHexEncoded = hexEncodeCreate(NULL, signatureBytes, signatureBytesCount);

        assert (0 == strcmp (signatureHexEncoded, &vectors[index].signatureHexEncoded[2]));
        free (signatureBytes);
        free (signatureHexEncoded);
#endif
        free (message);
    }
}
// MARK: - Fee Basis Test

static void
runAvalancheFeeBasisTest (void) {
    printf("TST:    Avalanche FeeBasis\n");
    return;
}

// MARK: - Transaction Test

static void
runAvalancheTransactionUTXOTest () {
    static struct {
        char *transactionIdentifier;
        BRAvalancheIndex transactionIndex;
        char *assetIdentifier;
        BRAvalancheAmount amount;
        char *addresses[3];
    } utxoSpecs[] = {
        { "XQYUrRZUMuHv6GXDer2oU9Gje6YkZWTVHLUdDWipWdMtpNVQh", 1, "U8iRqJoiJm8xZHAacmvYyZVwqQx6uDNtQeP3CQ6fcgQk3JqnK", 9964000000, { "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q", NULL, NULL }},
        { "Da5BCvPhMEXK2bPEC5H7rssQYXiS1jnhGUntFiejtWvAbWmqP", 0, "U8iRqJoiJm8xZHAacmvYyZVwqQx6uDNtQeP3CQ6fcgQk3JqnK", 2000000000, { "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q", NULL, NULL }},
        { NULL, 0, NULL, 0, { NULL, NULL, NULL }}
    };

    static struct {
        ssize_t utxoIndices[5];
        BRAvalancheUTXOSearchType searchType;
        char *source;
        char *asset;
        BRAvalancheAmount amount;
        size_t utxosInSearchCount;
        BRAvalancheAmount utxosInSearchAmount;
    } vectors[] = {

        // MIN First Order
        {
            { 0, 1, -1, -1, -1 },
            AVALANCHE_UTXO_SEARCH_MIN_FIRST,
            "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q",
            "U8iRqJoiJm8xZHAacmvYyZVwqQx6uDNtQeP3CQ6fcgQk3JqnK",
            1000000000,
            1,
            2000000000
        },

        {
            { 0, 1, -1, -1, -1 },
            AVALANCHE_UTXO_SEARCH_MIN_FIRST,
            "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q",
            "U8iRqJoiJm8xZHAacmvYyZVwqQx6uDNtQeP3CQ6fcgQk3JqnK",
            3000000000,
            2,
            (9964000000 + 2000000000)
        },

        {
            { 0, 1, -1, -1, -1 },
            AVALANCHE_UTXO_SEARCH_MIN_FIRST,
            "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q",
            "U8iRqJoiJm8xZHAacmvYyZVwqQx6uDNtQeP3CQ6fcgQk3JqnK",
            (9964000000 + 1),
            2,
            (9964000000 + 2000000000)
        },

        {
            { 0, 1, -1, -1, -1 },
            AVALANCHE_UTXO_SEARCH_MIN_FIRST,
            "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q",
            "U8iRqJoiJm8xZHAacmvYyZVwqQx6uDNtQeP3CQ6fcgQk3JqnK",
            (9964000000 + 2000000000 + 1),
            0,
            0
        },

        // NAX First Order
        {
            { 0, 1, -1, -1, -1 },
            AVALANCHE_UTXO_SEARCH_MAX_FIRST,
            "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q",
            "U8iRqJoiJm8xZHAacmvYyZVwqQx6uDNtQeP3CQ6fcgQk3JqnK",
            1000000000,
            1,
            9964000000
        },

        {
            { 0, 1, -1, -1, -1 },
            AVALANCHE_UTXO_SEARCH_MAX_FIRST,
            "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q",
            "U8iRqJoiJm8xZHAacmvYyZVwqQx6uDNtQeP3CQ6fcgQk3JqnK",
            3000000000,
            1,
            9964000000
        },

        {
            { 0, 1, -1, -1, -1 },
            AVALANCHE_UTXO_SEARCH_MAX_FIRST,
            "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q",
            "U8iRqJoiJm8xZHAacmvYyZVwqQx6uDNtQeP3CQ6fcgQk3JqnK",
            (9964000000 + 1),
            2,
            (9964000000 + 2000000000)
        },

        {
            { 0, 1, -1, -1, -1 },
            AVALANCHE_UTXO_SEARCH_MAX_FIRST,
            "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q",
            "U8iRqJoiJm8xZHAacmvYyZVwqQx6uDNtQeP3CQ6fcgQk3JqnK",
            (9964000000 + 2000000000 + 1),
            0,
            0
        },

        // RANDOM
         {
            { 0, 1, -1, -1, -1 },
            AVALANCHE_UTXO_SEARCH_RANDOM,
            "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q",
            "U8iRqJoiJm8xZHAacmvYyZVwqQx6uDNtQeP3CQ6fcgQk3JqnK",
            3000000000,
            1,
            9964000000
        },

        { { -1, -1, -1, -1, -1 }, NULL, NULL, 0 }
    };
    printf("TST:        Avalanche Transaction UTXO\n");

    BRAvalancheNetwork network = avaxNetworkTestnet;

    BRArrayOf(BRAvalancheUTXO) utxos;
    array_new (utxos, 1);

    // Build the UTXOs
    BRArrayOf(BRAvalancheAddress) addresses;
    array_new (addresses, 1);
    for (size_t index = 0; NULL != utxoSpecs[index].transactionIdentifier; index++) {
        for (size_t indexAddr = 0; NULL != utxoSpecs[index].addresses[indexAddr]; indexAddr++)
            array_add (addresses, avalancheNetworkStringToAddress (network, utxoSpecs[index].addresses[indexAddr], true));

        BRAvalancheUTXO utxo = avalancheUTXOCreate (avalancheHashFromString (utxoSpecs[index].transactionIdentifier),
                                                    utxoSpecs[index].transactionIndex,
                                                    avalancheHashFromString (utxoSpecs[index].assetIdentifier),
                                                    utxoSpecs[index].amount,
                                                    addresses);
        array_clear(addresses);
        array_add (utxos, utxo);
    }
    array_free (addresses);

    for (size_t index = 0; -1 != vectors[index].utxoIndices[0]; index++) {
        BRSetOf (BRAvalanceUTXO) utxoSet = avalancheUTXOSetCreate (2);
        for (size_t indexUTXO = 0; -1 != vectors[index].utxoIndices[indexUTXO]; indexUTXO++)
            BRSetAdd (utxoSet, utxos[vectors[index].utxoIndices[indexUTXO]]);
        size_t utxoCount = BRSetCount(utxoSet);

        BRAvalancheAmount amountUTXOs;
        BRArrayOf(BRAvalancheUTXO) utxosInSearch = avalancheUTXOSearchForAmount (utxoSet,
                                                                                 vectors[index].searchType,
                                                                                 avalancheNetworkStringToAddress (network, vectors[index].source, true),
                                                                                 avalancheHashFromString (vectors[index].asset),
                                                                                 vectors[index].amount,
                                                                                 &amountUTXOs,
                                                                                 false);
        assert (vectors[index].utxosInSearchCount  == array_count(utxosInSearch));
        assert (vectors[index].utxosInSearchAmount == amountUTXOs);

        array_free_all(utxosInSearch, avalancheUTXORelease);
        BRSetFree(utxoSet);
    }

    array_free_all(utxos, avalancheUTXORelease);
}

static void
runAvalancheTransactionCreateTest (void) {
    printf("TST:        Avalanche Transaction Create\n");
    return;
}

static void
runAvalancheTransactionSignTest (void) {
    printf("TST:        Avalanche Transaction Sign\n");
   return;
}

static void
runAvalancheTransactionSerializeTest (void) {
    printf("TST:        Avalanche Transaction Serialize\n");
    BRAvalancheNetwork network = avaxNetworkTestnet;

    BRAvalancheHash assetIdentifier = avalancheHashFromString ("U8iRqJoiJm8xZHAacmvYyZVwqQx6uDNtQeP3CQ6fcgQk3JqnK");

    BRAvalancheAddress utxoAddress = avalancheNetworkStringToAddress (network, "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q", true);
    BRArrayOf(BRAvalancheAddress) utxoAddresses;
    array_new (utxoAddresses, 1);
    array_add (utxoAddresses, utxoAddress);

    BRAvalancheUTXO utxo1 = avalancheUTXOCreate (avalancheHashFromString("XQYUrRZUMuHv6GXDer2oU9Gje6YkZWTVHLUdDWipWdMtpNVQh"),
                                             1,
                                             assetIdentifier,
                                             9964000000,
                                             utxoAddresses);

    BRAvalancheUTXO utxo2 = avalancheUTXOCreate (avalancheHashFromString("Da5BCvPhMEXK2bPEC5H7rssQYXiS1jnhGUntFiejtWvAbWmqP"),
                                                 0,
                                                 assetIdentifier,
                                                 2000000000,
                                                 utxoAddresses);

    BRArrayOf(BRAvalancheUTXO) utxos;
    array_new (utxos, 2);
    array_add (utxos, utxo1);
    array_add (utxos, utxo2);

    BRAvalancheTransaction transaction1 =
    avalancheTransactionCreate (avalancheNetworkStringToAddress (network, "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q", true),
                                avalancheNetworkStringToAddress (network, "fuji1k3lf9kxsmyf9jyx4dlq7hffvyu4eppmv89w2f0", true),
                                avalancheNetworkStringToAddress (network, "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q", true),
                                assetIdentifier,
                                12300000,
                                avalancheFeeBasisCreate((BRAvalancheAmount)(1000000000 * 0.001)),
                                "hello",
                                utxos,
                                network);

    //
    // Check individual encodings
    //

    // output[0]
    BRData output0Data = avalancheTransactionOutputEncode (transaction1->outputs[0]);
    assert (dataEqualsHexString (output0Data, "3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000000bbaee000000000000000000000000100000001b47e92d8d0d9125910d56fc1eba52c272b90876c"));
    dataFree(output0Data);

    // output[1]
    BRData output1Data = avalancheTransactionOutputEncode (transaction1->outputs[1]);
    assert (dataEqualsHexString (output1Data, "3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000700000002511ba1e000000000000000000000000100000001cc30e2015780a6c72efaef2280e3de4a954e770c"));
    dataFree(output1Data);

    // input[0]
    BRData input0Data = avalancheTransactionInputEncode (transaction1->inputs[0]);
    assert (dataEqualsHexString(input0Data, "450a5390bcf287869b9dcef42ca6b4305fde20e5f29d40e719a87fe7dd043600000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000251e693000000000100000000"));
    dataFree(input0Data);

    BRData transaction1Data = avalancheTransactionEncode (transaction1);
    assert (dataEqualsHexString (transaction1Data, "00000000000000000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000000bbaee000000000000000000000000100000001b47e92d8d0d9125910d56fc1eba52c272b90876c3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000700000002511ba1e000000000000000000000000100000001cc30e2015780a6c72efaef2280e3de4a954e770c00000001450a5390bcf287869b9dcef42ca6b4305fde20e5f29d40e719a87fe7dd043600000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000251e6930000000001000000000000000568656c6c6f"));
    assert (307 == transaction1Data.size);
    dataFree(transaction1Data);

//    avalancheTransactionRelease (transaction);

    BRAvalancheTransaction transaction2 =
    avalancheTransactionCreate (avalancheNetworkStringToAddress (network, "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q", true),
                                avalancheNetworkStringToAddress (network, "fuji1k3lf9kxsmyf9jyx4dlq7hffvyu4eppmv89w2f0", true),
                                avalancheNetworkStringToAddress (network, "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q", true),
                                assetIdentifier,
                                10300000000,
                                avalancheFeeBasisCreate((BRAvalancheAmount)(1000000000 * 0.001)),
                                "hello",
                                utxos,
                                network);

    BRData transaction2Data = avalancheTransactionEncode (transaction2);
    assert (dataEqualsHexString (transaction2Data, "00000000000000000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000700000000631f5dc000000000000000000000000100000001cc30e2015780a6c72efaef2280e3de4a954e770c3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000265ed870000000000000000000000000100000001b47e92d8d0d9125910d56fc1eba52c272b90876c000000021c8acd205ff6161efce0952071fc63ca8e99717bb829e74ce33997c2b369334b000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000500000000773594000000000100000000450a5390bcf287869b9dcef42ca6b4305fde20e5f29d40e719a87fe7dd043600000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000251e6930000000001000000000000000568656c6c6f"));

    BRAvalancheHash transaction2HashUnsigned = avalancheHashCreate (transaction2Data.bytes, transaction2Data.size);
    char *transaction2HashUnsignedHex = hexEncodeCreate (NULL, transaction2HashUnsigned.bytes, 32);
    assert (0 == strcmp (transaction2HashUnsignedHex, "31d3ab6136b423dc0b4ed69769dcfd3207067d94534357e6410a61e5a6859b48"));
    free (transaction2HashUnsignedHex);

    //
    // Signature
    //

    UInt512 seed = UINT512_ZERO;
    BRBIP39DeriveKey(seed.u8, "patient doctor olympic frog force glimpse endless antenna online dragon bargain someone", NULL);
    BRAvalancheAccount account = avalancheAccountCreateWithSeed (seed);


    BRAvalancheSignature signature = avalancheAccountSignData (account,
                                                               transaction2Data.bytes,
                                                               transaction2Data.size,
                                                               seed);
    BRData signatureData = avalancheSignatureGetBytes(&signature);
    assert (dataEqualsHexString(signatureData, "c7617e7e105b14481af67595376ab64d24e809f3986ee1133a5b77fafbda0db32cd46b17f884de9dccd1dcc13e9a9bfccb1999e49e6483f7e101a908bf4f6d8900"));
    dataFree (signatureData);

    // Encode multiple signatures
    BRArrayOf(BRAvalancheSignature) signatures;
    array_new (signatures, 1);
    array_add (signatures, signature);
    array_add (signatures, signature);
    BRData signature2Data = avalancheSignatureArrayEncode (signatures);
    assert (dataEqualsHexString (signature2Data, "000000020000000900000001c7617e7e105b14481af67595376ab64d24e809f3986ee1133a5b77fafbda0db32cd46b17f884de9dccd1dcc13e9a9bfccb1999e49e6483f7e101a908bf4f6d89000000000900000001c7617e7e105b14481af67595376ab64d24e809f3986ee1133a5b77fafbda0db32cd46b17f884de9dccd1dcc13e9a9bfccb1999e49e6483f7e101a908bf4f6d8900"));

    // Derive the transaction hash.
    BRData transactionDataSigned = dataConcatTwo (transaction2Data, signature2Data);
    assert (dataEqualsHexString(transactionDataSigned, "00000000000000000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000700000000631f5dc000000000000000000000000100000001cc30e2015780a6c72efaef2280e3de4a954e770c3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000265ed870000000000000000000000000100000001b47e92d8d0d9125910d56fc1eba52c272b90876c000000021c8acd205ff6161efce0952071fc63ca8e99717bb829e74ce33997c2b369334b000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000500000000773594000000000100000000450a5390bcf287869b9dcef42ca6b4305fde20e5f29d40e719a87fe7dd043600000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000251e6930000000001000000000000000568656c6c6f000000020000000900000001c7617e7e105b14481af67595376ab64d24e809f3986ee1133a5b77fafbda0db32cd46b17f884de9dccd1dcc13e9a9bfccb1999e49e6483f7e101a908bf4f6d89000000000900000001c7617e7e105b14481af67595376ab64d24e809f3986ee1133a5b77fafbda0db32cd46b17f884de9dccd1dcc13e9a9bfccb1999e49e6483f7e101a908bf4f6d8900"));

    BRAvalancheHash transactionHashSigned = avalancheHashCreate (transactionDataSigned.bytes, transactionDataSigned.size);
    char *transactionHashSignedHex = hexEncodeCreate (NULL, transactionHashSigned.bytes, 32);
    assert (0 == strcmp (transactionHashSignedHex, "befcbc77362d6534f8deca34077da5b7b1b0107ea8a2274baf81a17aa07b82ff"));
    free (transactionHashSignedHex);

    char *transactionHashSignedString = avalancheHashToString(transactionHashSigned);
    assert (0 == strcmp (transactionHashSignedString, "2T7WkhEYnypDQFGfB8LgNKkfPhVsmBsdyfPfy7fTm6XnaV2DXS"));
    free (transactionHashSignedString);

    dataFree (transactionDataSigned);
    dataFree (signature2Data);
    dataFree (transaction2Data);

    return;
}

static void
runAvalancheTransactionTest (void) {
    printf("TST:    Avalanche Transaction\n");
    runAvalancheTransactionUTXOTest ();
    runAvalancheTransactionCreateTest ();
    runAvalancheTransactionSignTest ();
    runAvalancheTransactionSerializeTest ();
}

// MARK: - Wallet Test

static void
runAvalancheWalletTest (void) {
    printf("TST:    Avalanche Wallet\n");
   return;
}

// MARK: - All Tests

extern void
runAvalancheTest (void /* ... */) {
    printf("TST: Avalanche\n");

    runAvalancheUtilsTest ();
    runAvalancheHashTest ();
    runAvalancheAddressTest();
    runAvalancheAccountTest();
    runAvalancheSignatureTest();
    runAvalancheFeeBasisTest ();
    runAvalancheTransactionTest ();
    runAvalancheWalletTest ();

    printf("TST: Avalanche Done\n");
}

