//
//  WKNetwork__SYMBOL__.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WK__SYMBOL__.h"
#include "walletkit/WKAccountP.h"
#include "walletkit/WKHashP.h"
#include "support/BRBase58.h"

static WKNetwork__SYMBOL__
wkNetworkCoerce (WKNetwork network) {
    assert (WK_NETWORK_TYPE___SYMBOL__ == network->type);
    return (WKNetwork__SYMBOL__) network;
}

static WKNetwork
cyptoNetworkCreate__SYMBOL__ (WKNetworkListener listener,
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
                                      WK_NETWORK_TYPE___SYMBOL__,
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
wkNetworkRelease__SYMBOL__ (WKNetwork network) {
    WKNetwork__SYMBOL__ network__SYMBOL__ = wkNetworkCoerce (network);
    (void) network__SYMBOL__;
}

static WKAddress
wkNetworkCreateAddress__SYMBOL__ (WKNetwork network,
                               const char *addressAsString) {
    WKNetwork__SYMBOL__ network__SYMBOL__ = wkNetworkCoerce (network);
    (void) network__SYMBOL__;

    return wkAddressCreateFromStringAs__SYMBOL__ (addressAsString);
}

static WKBlockNumber
wkNetworkGetBlockNumberAtOrBeforeTimestamp__SYMBOL__ (WKNetwork network,
                                                   WKTimestamp timestamp) {
    // not supported (used for p2p sync checkpoints)
    return 0;
}

// MARK: Account Initialization

static WKBoolean
wkNetworkIsAccountInitialized__SYMBOL__ (WKNetwork network,
                                      WKAccount account) {
    WKNetwork__SYMBOL__ network__SYMBOL__ = wkNetworkCoerce (network);
    (void) network__SYMBOL__;

    BR__Name__Account __symbol__Account = (BR__Name__Account) wkAccountAs (account,
                                                              WK_NETWORK_TYPE___SYMBOL__);
    assert (NULL != __symbol__Account);
    return AS_WK_BOOLEAN (true);
}


static uint8_t *
wkNetworkGetAccountInitializationData__SYMBOL__ (WKNetwork network,
                                              WKAccount account,
                                              size_t *bytesCount) {
    WKNetwork__SYMBOL__ network__SYMBOL__ = wkNetworkCoerce (network);
    (void) network__SYMBOL__;

    BR__Name__Account __symbol__Account = (BR__Name__Account) wkAccountAs (account,
                                                              WK_NETWORK_TYPE___SYMBOL__);
    assert (NULL != __symbol__Account);
    if (NULL != bytesCount) *bytesCount = 0;
    return NULL;
}

static void
wkNetworkInitializeAccount__SYMBOL__ (WKNetwork network,
                                   WKAccount account,
                                   const uint8_t *bytes,
                                   size_t bytesCount) {
    WKNetwork__SYMBOL__ network__SYMBOL__ = wkNetworkCoerce (network);
    (void) network__SYMBOL__;

    BR__Name__Account __symbol__Account = (BR__Name__Account) wkAccountAs (account,
                                                              WK_NETWORK_TYPE___SYMBOL__);
    assert (NULL != __symbol__Account);
    return;
}

static WKHash
wkNetworkCreateHashFromString__SYMBOL__ (WKNetwork network,
                                      const char *string) {
    return wkHashCreateFromStringAs__SYMBOL__ (string);
}

static char *
wkNetworkEncodeHash__SYMBOL__ (WKHash hash) {
    size_t len = BRBase58CheckEncode (NULL, 0, hash->bytes, __NAME___HASH_BYTES);
    if (0 == len) return NULL;

    char * string = calloc (1, len);
    BRBase58CheckEncode (string, len, hash->bytes, __NAME___HASH_BYTES);
    return string;
}

// MARK: - Handlers

WKNetworkHandlers wkNetworkHandlers__SYMBOL__ = {
    cyptoNetworkCreate__SYMBOL__,
    wkNetworkRelease__SYMBOL__,
    wkNetworkCreateAddress__SYMBOL__,
    wkNetworkGetBlockNumberAtOrBeforeTimestamp__SYMBOL__,
    wkNetworkIsAccountInitialized__SYMBOL__,
    wkNetworkGetAccountInitializationData__SYMBOL__,
    wkNetworkInitializeAccount__SYMBOL__,
    wkNetworkCreateHashFromString__SYMBOL__,
    wkNetworkEncodeHash__SYMBOL__
};

