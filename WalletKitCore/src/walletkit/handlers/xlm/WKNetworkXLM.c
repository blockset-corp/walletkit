//
//  WKNetworkXLM.c
//  WalletKitCore
//
//  Created by Carl Cherry on 2020-05-19.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXLM.h"
#include "walletkit/WKAccountP.h"
#include "walletkit/WKHashP.h"

static WKNetworkXLM
wkNetworkCoerce (WKNetwork network) {
    assert (WK_NETWORK_TYPE_XLM == network->type);
    return (WKNetworkXLM) network;
}

static WKNetwork
cyptoNetworkCreateXLM (WKNetworkListener listener,
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
                                      WK_NETWORK_TYPE_XLM,
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
wkNetworkReleaseXLM (WKNetwork network) {
    WKNetworkXLM networkXLM = wkNetworkCoerce (network);
    (void) networkXLM;
}

static WKAddress
wkNetworkCreateAddressXLM (WKNetwork network,
                               const char *addressAsString) {
    return wkAddressCreateFromStringAsXLM (addressAsString);
}

static WKBlockNumber
wkNetworkGetBlockNumberAtOrBeforeTimestampXLM (WKNetwork network,
                                                   WKTimestamp timestamp) {
    // not supported (used for p2p sync checkpoints)
    return 0;
}

// MARK: Account Initialization

static WKBoolean
wkNetworkIsAccountInitializedXLM (WKNetwork network,
                                      WKAccount account) {
    WKNetworkXLM networkXLM = wkNetworkCoerce (network);
    (void) networkXLM;

    BRStellarAccount xlmAccount = (BRStellarAccount) wkAccountAs (account,
                                                                  WK_NETWORK_TYPE_XLM);
    assert (NULL != xlmAccount);
    return AS_WK_BOOLEAN (true);
}


static uint8_t *
wkNetworkGetAccountInitializationDataXLM (WKNetwork network,
                                              WKAccount account,
                                              size_t *bytesCount) {
    WKNetworkXLM networkXLM = wkNetworkCoerce (network);
    (void) networkXLM;

    BRStellarAccount xlmAccount = (BRStellarAccount) wkAccountAs (account,
                                                                  WK_NETWORK_TYPE_XLM);
    assert (NULL != xlmAccount);
    if (NULL != bytesCount) *bytesCount = 0;
    return NULL;
}

static void
wkNetworkInitializeAccountXLM (WKNetwork network,
                                   WKAccount account,
                                   const uint8_t *bytes,
                                   size_t bytesCount) {
    WKNetworkXLM networkXLM = wkNetworkCoerce (network);
    (void) networkXLM;

    BRStellarAccount xlmAccount = (BRStellarAccount) wkAccountAs (account,
                                                                  WK_NETWORK_TYPE_XLM);
    assert (NULL != xlmAccount);
    return;
}

static WKHash
wkNetworkCreateHashFromStringXLM (WKNetwork network,
                                      const char *string) {
    BRStellarTransactionHash hash = stellarHashCreateFromString (string);
    return wkHashCreateAsXLM (hash);
}

static char *
wkNetworkEncodeHashXLM (WKHash hash) {
    return wkHashStringAsHex (hash, false);
}

// MARK: - Handlers

WKNetworkHandlers wkNetworkHandlersXLM = {
    cyptoNetworkCreateXLM,
    wkNetworkReleaseXLM,
    wkNetworkCreateAddressXLM,
    wkNetworkGetBlockNumberAtOrBeforeTimestampXLM,
    wkNetworkIsAccountInitializedXLM,
    wkNetworkGetAccountInitializationDataXLM,
    wkNetworkInitializeAccountXLM,
    wkNetworkCreateHashFromStringXLM,
    wkNetworkEncodeHashXLM
};

