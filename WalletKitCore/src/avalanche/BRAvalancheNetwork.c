//
//  BRAvalancheNetwork.h
//  WalletKitCore
//
//  Created by Ed Gamble on 9/1/21.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRAvalancheNetwork.h"
#include <pthread.h>

static void
avalancheNetworkInitializeIfAppropriate (void);

struct BRAvalancheNetworkRecord {
    BRAvalancheChainType type;
    BRAvalancheNetworkIdentifier identifier;
    BRAvalancheHash blockchain;
    /* private */ const char *blockchainAsString;
    char *addressPrefix;
};

extern BRAvalancheChainType
avalancheNetworkGetChainType (BRAvalancheNetwork network) {
    avalancheNetworkInitializeIfAppropriate ();
    return network->type;
}

extern BRAvalancheNetworkIdentifier
avalancheNetworkGetIdentifier (BRAvalancheNetwork network) {
    avalancheNetworkInitializeIfAppropriate ();
    return network->identifier;
}

extern BRAvalancheHash
avalancheNetworkGetBlockchain (BRAvalancheNetwork network) {
    avalancheNetworkInitializeIfAppropriate ();
    return network->blockchain;
}

extern const char *
avalancheNetworkGetAddressPrefix (BRAvalancheNetwork network) {
    avalancheNetworkInitializeIfAppropriate ();
    return network->addressPrefix;
}

/// MARK: - Networks

static struct BRAvalancheNetworkRecord avaxNetworkMainnetRecord = {
    AVALANCHE_CHAIN_TYPE_X,
    AVALANCHE_NETWORK_ID_MAINNET,
    ((BRAvalancheHash) { 0xed, 0x5f, 0x38, 0x34, 0x1e, 0x43, 0x6e, 0x5d, 0x46, 0xe2, 0xbb, 0x00, 0xb4, 0x5d, 0x62, 0xae, 0x97, 0xd1, 0xb0, 0x50, 0xc6, 0x4b, 0xc6, 0x34, 0xae, 0x10, 0x62, 0x67, 0x39, 0xe3, 0x5c, 0x4b }),
    "2oYMBNV4eNHyqk2fjjV5nVQLDbtmNJzq5s3qs3Lo6ftnC6FByM",
    "avax"
};
const BRAvalancheNetwork avaxNetworkMainnet = &avaxNetworkMainnetRecord;

static struct BRAvalancheNetworkRecord avaxNetworkTestnetRecord = {
    AVALANCHE_CHAIN_TYPE_X,
    AVALANCHE_NETWORK_ID_FUJI,
    ((BRAvalancheHash) { 0xab, 0x68, 0xeb, 0x1e, 0xe1, 0x42, 0xa0, 0x5c, 0xfe, 0x76, 0x8c, 0x36, 0xe1, 0x1f, 0x0b, 0x59, 0x6d, 0xb5, 0xa3, 0xc6, 0xc7, 0x7a, 0xab, 0xe6, 0x65, 0xda, 0xd9, 0xe6, 0x38, 0xca, 0x94, 0xf7 }),
    "2JVSBoinj9C2J33VntvzYtVJNZdN2NKiwwKjcumHUWEb5DbBrm",
    "fuji"
};
const BRAvalancheNetwork avaxNetworkTestnet = &avaxNetworkTestnetRecord;

/// MARK: - Network Initialization

static void
avalancheNetworkInitialize (void) {
#if 0
    struct BRAvalancheNetworkRecord *networks[] = {
        &avaxNetworkMainnetRecord,
        &avaxNetworkTestnetRecord,
        NULL
    };

    for (size_t index = 0; NULL != networks[index]; index++) {
        networks[index]->blockchain = avalancheHashFromString (networks[index]->blockchainAsString);
    }
#endif
}

// Run once initializer guarantees
static pthread_once_t avalancheNetworkInitOnce = PTHREAD_ONCE_INIT;
static void avalancheNetworkInitializeIfAppropriate (void) {
    pthread_once (&avalancheNetworkInitOnce, avalancheNetworkInitialize);
}

