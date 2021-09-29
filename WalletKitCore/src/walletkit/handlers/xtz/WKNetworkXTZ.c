//
//  WKNetworkXTZ.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXTZ.h"
#include "walletkit/WKAccountP.h"
#include "walletkit/WKHashP.h"
#include "support/BRBase58.h"

static WKNetworkXTZ
wkNetworkCoerce (WKNetwork network) {
    assert (WK_NETWORK_TYPE_XTZ == network->type);
    return (WKNetworkXTZ) network;
}

static WKNetwork
cyptoNetworkCreateXTZ (WKNetworkListener listener,
                       const char *uids,
                       const char *name,
                       const char *desc,
                       bool isMainnet,
                       uint32_t confirmationPeriodInSeconds,
                       WKAddressScheme defaultAddressScheme,
                       WKSyncMode defaultSyncMode,
                       WKCurrency nativeCurrency) {
    assert (0 == strcmp (desc, (isMainnet ? "mainnet" : "testnet")));
    
    return wkNetworkAllocAndInit (sizeof (struct WKNetworkRecord),
                                      WK_NETWORK_TYPE_XTZ,
                                      listener,
                                      uids,
                                      name,
                                      desc,
                                      isMainnet,
                                      confirmationPeriodInSeconds,
                                      defaultAddressScheme,
                                      defaultSyncMode,
                                      nativeCurrency,
                                      NULL,
                                      NULL);
}

static void
wkNetworkReleaseXTZ (WKNetwork network) {
    WKNetworkXTZ networkXTZ = wkNetworkCoerce (network);
    (void) networkXTZ;
}

static WKAddress
wkNetworkCreateAddressXTZ (WKNetwork network,
                               const char *addressAsString) {
    WKNetworkXTZ networkXTZ = wkNetworkCoerce (network);
    (void) networkXTZ;

    return wkAddressCreateFromStringAsXTZ (addressAsString);
}

static WKBlockNumber
wkNetworkGetBlockNumberAtOrBeforeTimestampXTZ (WKNetwork network,
                                                   WKTimestamp timestamp) {
    // not supported (used for p2p sync checkpoints)
    return 0;
}

// MARK: Account Initialization

static WKBoolean
wkNetworkIsAccountInitializedXTZ (WKNetwork network,
                                      WKAccount account) {
    WKNetworkXTZ networkXTZ = wkNetworkCoerce (network);
    (void) networkXTZ;

    BRTezosAccount xtzAccount = (BRTezosAccount) wkAccountAs (account,
                                                              WK_NETWORK_TYPE_XTZ);
    assert (NULL != xtzAccount);
    return AS_WK_BOOLEAN (true);
}


static uint8_t *
wkNetworkGetAccountInitializationDataXTZ (WKNetwork network,
                                              WKAccount account,
                                              size_t *bytesCount) {
    WKNetworkXTZ networkXTZ = wkNetworkCoerce (network);
    (void) networkXTZ;

    BRTezosAccount xtzAccount = (BRTezosAccount) wkAccountAs (account,
                                                              WK_NETWORK_TYPE_XTZ);
    assert (NULL != xtzAccount);
    if (NULL != bytesCount) *bytesCount = 0;
    return NULL;
}

static void
wkNetworkInitializeAccountXTZ (WKNetwork network,
                                   WKAccount account,
                                   const uint8_t *bytes,
                                   size_t bytesCount) {
    WKNetworkXTZ networkXTZ = wkNetworkCoerce (network);
    (void) networkXTZ;

    BRTezosAccount xtzAccount = (BRTezosAccount) wkAccountAs (account,
                                                              WK_NETWORK_TYPE_XTZ);
    assert (NULL != xtzAccount);
    return;
}

static WKHash
wkNetworkCreateHashFromStringXTZ (WKNetwork network,
                                      const char *string) {
    return wkHashCreateFromStringAsXTZ (string);
}

static char *
wkNetworkEncodeHashXTZ (WKHash hash) {
    size_t len = BRBase58CheckEncode (NULL, 0, hash->bytes, TEZOS_HASH_BYTES);
    if (0 == len) return NULL;

    char * string = calloc (1, len);
    BRBase58CheckEncode (string, len, hash->bytes, TEZOS_HASH_BYTES);
    return string;
}

// MARK: - Handlers

WKNetworkHandlers wkNetworkHandlersXTZ = {
    cyptoNetworkCreateXTZ,
    wkNetworkReleaseXTZ,
    wkNetworkCreateAddressXTZ,
    wkNetworkGetBlockNumberAtOrBeforeTimestampXTZ,
    wkNetworkIsAccountInitializedXTZ,
    wkNetworkGetAccountInitializationDataXTZ,
    wkNetworkInitializeAccountXTZ,
    wkNetworkCreateHashFromStringXTZ,
    wkNetworkEncodeHashXTZ
};

