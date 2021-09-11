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
#ifndef BRAvalancheNetwork_h
#define BRAvalancheNetwork_h

#include "BRAvalancheBase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AVALANCHE_NETWORK_ID_MAINNET = 1,
    AVALANCHE_NETWORK_ID_FUJI    = 5,
    AVALANCHE_NETWORK_ID_LOCAL   = 12345,
} BRAvalancheNetworkIdentifier;

typedef struct BRAvalancheNetworkRecord *BRAvalancheNetwork;

extern BRAvalancheChainType
avalancheNetworkGetChainType (BRAvalancheNetwork network);

extern BRAvalancheNetworkIdentifier
avalancheNetworkGetIdentifier (BRAvalancheNetwork network);

extern BRAvalancheHash
avalancheNetworkGetBlockchain (BRAvalancheNetwork network);

extern const char *
avalancheNetworkGetAddressPrefix (BRAvalancheNetwork network);

extern const BRAvalancheNetwork avaxNetworkMainnet;
extern const BRAvalancheNetwork avaxNetworkTestnet;

#ifdef __cplusplus
}
#endif

#endif // BRAvalancheNetwork_h

