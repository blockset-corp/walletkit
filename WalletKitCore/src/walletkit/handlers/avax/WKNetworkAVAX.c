//
//  WKNetworkAVAX.c
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKAVAX.h"
#include "walletkit/WKAccountP.h"
#include "walletkit/WKHashP.h"
#include "support/BRBase58.h"

static WKNetwork
cyptoNetworkCreateAVAX (WKNetworkListener listener,
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
                                      WK_NETWORK_TYPE_AVAX,
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
wkNetworkReleaseAVAX (WKNetwork network) {
    WKNetworkAVAX networkAVAX = wkNetworkCoerceAVAX (network);
    (void) networkAVAX;
}

static WKAddress
wkNetworkCreateAddressAVAX (WKNetwork network,
                               const char *addressAsString) {
    WKNetworkAVAX networkAVAX = wkNetworkCoerceAVAX (network);
    (void) networkAVAX;

    return wkAddressCreateFromStringAsAVAX (addressAsString);
}

static WKBlockNumber
wkNetworkGetBlockNumberAtOrBeforeTimestampAVAX (WKNetwork network,
                                                      WKTimestamp timestamp) {
    // not supported (used for p2p sync checkpoints)
    return 0;
}

// MARK: Account Initialization

static WKBoolean
wkNetworkIsAccountInitializedAVAX (WKNetwork network,
                                      WKAccount account) {
    WKNetworkAVAX networkAVAX = wkNetworkCoerceAVAX (network);
    (void) networkAVAX;

    BRAvalancheAccount avaxAccount = wkAccountGetAsAVAX (account);

    // No initialization required
    (void) avaxAccount;
    return AS_WK_BOOLEAN (true);
}


static uint8_t *
wkNetworkGetAccountInitializationDataAVAX (WKNetwork network,
                                              WKAccount account,
                                              size_t *bytesCount) {
    WKNetworkAVAX networkAVAX = wkNetworkCoerceAVAX (network);
    (void) networkAVAX;

    BRAvalancheAccount avaxAccount = wkAccountGetAsAVAX (account);

    // No initialization required
    (void) avaxAccount;
    if (NULL != bytesCount) *bytesCount = 0;
    return NULL;
}

static void
wkNetworkInitializeAccountAVAX (WKNetwork network,
                                   WKAccount account,
                                   const uint8_t *bytes,
                                   size_t bytesCount) {
    WKNetworkAVAX networkAVAX = wkNetworkCoerceAVAX (network);
    (void) networkAVAX;

    BRAvalancheAccount avaxAccount = wkAccountGetAsAVAX (account);

    // No initialization required
    (void) avaxAccount;
    return;
}

static WKHash
wkNetworkCreateHashFromStringAVAX (WKNetwork network,
                                      const char *string) {
    return wkHashCreateFromStringAsAVAX (string);
}

static char *
wkNetworkEncodeHashAVAX (WKHash hash) {
#if 0
    size_t len = BRBase58CheckEncode (NULL, 0, hash->bytes, AVALANCHE_HASH_BYTES);
    if (0 == len) return NULL;

    char * string = calloc (1, len);
    BRBase58CheckEncode (string, len, hash->bytes, AVALANCHE_HASH_BYTES);
    return string;
#endif
    return NULL;
}

// MARK: - Handlers

WKNetworkHandlers wkNetworkHandlersAVAX = {
    cyptoNetworkCreateAVAX,
    wkNetworkReleaseAVAX,
    wkNetworkCreateAddressAVAX,
    wkNetworkGetBlockNumberAtOrBeforeTimestampAVAX,
    wkNetworkIsAccountInitializedAVAX,
    wkNetworkGetAccountInitializationDataAVAX,
    wkNetworkInitializeAccountAVAX,
    wkNetworkCreateHashFromStringAVAX,
    wkNetworkEncodeHashAVAX
};

