//
//  WKNetworkXRP.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXRP.h"
#include "walletkit/WKAccountP.h"
#include "walletkit/WKHashP.h"

static WKNetworkXRP
wkNetworkCoerce (WKNetwork network) {
    assert (WK_NETWORK_TYPE_XRP == network->type);
    return (WKNetworkXRP) network;
}

static WKNetwork
cyptoNetworkCreateXRP (WKNetworkListener listener,
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
                                      WK_NETWORK_TYPE_XRP,
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
wkNetworkReleaseXRP (WKNetwork network) {
    WKNetworkXRP networkXRP = wkNetworkCoerce (network);
    (void) networkXRP;
}

static WKAddress
wkNetworkCreateAddressXRP (WKNetwork network,
                               const char *addressAsString) {
    return wkAddressCreateFromStringAsXRP (addressAsString);
}

static WKBlockNumber
wkNetworkGetBlockNumberAtOrBeforeTimestampXRP (WKNetwork network,
                                                   WKTimestamp timestamp) {
    // not supported (used for p2p sync checkpoints)
    return 0;
}

// MARK: Account Initialization

static WKBoolean
wkNetworkIsAccountInitializedXRP (WKNetwork network,
                                      WKAccount account) {
    WKNetworkXRP networkXRP = wkNetworkCoerce (network);
    (void) networkXRP;

    BRRippleAccount xrpAccount = wkAccountAsXRP (account);
    assert (NULL != xrpAccount);
    return AS_WK_BOOLEAN (true);
}


static uint8_t *
wkNetworkGetAccountInitializationDataXRP (WKNetwork network,
                                              WKAccount account,
                                              size_t *bytesCount) {
    WKNetworkXRP networkXRP = wkNetworkCoerce (network);
    (void) networkXRP;

    BRRippleAccount xrpAccount = wkAccountAsXRP (account);
    assert (NULL != xrpAccount);
    if (NULL != bytesCount) *bytesCount = 0;
    return NULL;
}

static void
wkNetworkInitializeAccountXRP (WKNetwork network,
                                   WKAccount account,
                                   const uint8_t *bytes,
                                   size_t bytesCount) {
    WKNetworkXRP networkXRP = wkNetworkCoerce (network);
    (void) networkXRP;

    BRRippleAccount xrpAccount = wkAccountAsXRP (account);
    assert (NULL != xrpAccount);
    return;
}

static WKHash
wkNetworkCreateHashFromStringXRP (WKNetwork network,
                                      const char *string) {
    BRRippleTransactionHash hash = rippleHashCreateFromString (string);
    return wkHashCreateAsXRP (hash);
}

static char *
wkNetworkEncodeHashXRP (WKHash hash) {
    return wkHashStringAsHex (hash, false);
}

// MARK: - Handlers

WKNetworkHandlers wkNetworkHandlersXRP = {
    cyptoNetworkCreateXRP,
    wkNetworkReleaseXRP,
    wkNetworkCreateAddressXRP,
    wkNetworkGetBlockNumberAtOrBeforeTimestampXRP,
    wkNetworkIsAccountInitializedXRP,
    wkNetworkGetAccountInitializationDataXRP,
    wkNetworkInitializeAccountXRP,
    wkNetworkCreateHashFromStringXRP,
    wkNetworkEncodeHashXRP
};

