//
//  WKNetworkHBAR.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKHBAR.h"
#include "walletkit/WKAccountP.h"
#include "walletkit/WKHashP.h"

static WKNetworkHBAR
wkNetworkCoerce (WKNetwork network) {
    assert (WK_NETWORK_TYPE_HBAR == network->type);
    return (WKNetworkHBAR) network;
}

static WKNetwork
cyptoNetworkCreateHBAR (WKNetworkListener listener,
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
                                      WK_NETWORK_TYPE_HBAR,
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
wkNetworkReleaseHBAR (WKNetwork network) {
    WKNetworkHBAR networkHBAR = wkNetworkCoerce (network);
    (void) networkHBAR;
}

static WKAddress
wkNetworkCreateAddressHBAR (WKNetwork network,
                                const char *addressAsString) {
    return wkAddressCreateFromStringAsHBAR (addressAsString);
}

static WKBlockNumber
wkNetworkGetBlockNumberAtOrBeforeTimestampHBAR (WKNetwork network,
                                                    WKTimestamp timestamp) {
    // not supported (used for p2p sync checkpoints)
    return 0;
}

// MARK: Account Initialization

static WKBoolean
wkNetworkIsAccountInitializedHBAR (WKNetwork network,
                                       WKAccount account) {
    WKNetworkHBAR networkHBAR = wkNetworkCoerce (network);
    (void) networkHBAR;

    BRHederaAccount hbarAccount = wkAccountAsHBAR (account);
    assert (NULL != hbarAccount);

    return AS_WK_BOOLEAN (hederaAccountHasPrimaryAddress (hbarAccount));
}


static uint8_t *
wkNetworkGetAccountInitializationDataHBAR (WKNetwork network,
                                               WKAccount account,
                                               size_t *bytesCount) {
    WKNetworkHBAR networkHBAR = wkNetworkCoerce (network);
    (void) networkHBAR;

    BRHederaAccount hbarAccount = wkAccountAsHBAR (account);
    assert (NULL != hbarAccount);

    return hederaAccountGetPublicKeyBytes (hbarAccount, bytesCount);
}

static void
wkNetworkInitializeAccountHBAR (WKNetwork network,
                                    WKAccount account,
                                    const uint8_t *bytes,
                                    size_t bytesCount) {
    WKNetworkHBAR networkHBAR = wkNetworkCoerce (network);
    (void) networkHBAR;

    BRHederaAccount hbarAccount = wkAccountAsHBAR (account);
    assert (NULL != hbarAccount);

    char *hederaAddressString = malloc (bytesCount + 1);
    memcpy (hederaAddressString, bytes, bytesCount);
    hederaAddressString[bytesCount] = 0;

    BRHederaAddress hederaAddress = hederaAddressCreateFromString (hederaAddressString, true);
    free (hederaAddressString);

    hederaAccountSetAddress (hbarAccount, hederaAddress);
    hederaAddressFree(hederaAddress);

    return;
}

static WKHash
wkNetworkCreateHashFromStringHBAR (WKNetwork network,
                                      const char *string) {
    BRHederaTransactionHash hash = hederaHashCreateFromString(string);
    return wkHashCreateAsHBAR (hash);
}

static char *
wkNetworkEncodeHashHBAR (WKHash hash) {
    return wkHashStringAsHex (hash, false);
}

// MARK: -

WKNetworkHandlers wkNetworkHandlersHBAR = {
    cyptoNetworkCreateHBAR,
    wkNetworkReleaseHBAR,
    wkNetworkCreateAddressHBAR,
    wkNetworkGetBlockNumberAtOrBeforeTimestampHBAR,
    wkNetworkIsAccountInitializedHBAR,
    wkNetworkGetAccountInitializationDataHBAR,
    wkNetworkInitializeAccountHBAR,
    wkNetworkCreateHashFromStringHBAR,
    wkNetworkEncodeHashHBAR
};

