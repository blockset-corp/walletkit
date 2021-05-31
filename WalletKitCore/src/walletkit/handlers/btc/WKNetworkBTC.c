//
//  WKNetworkBTC.c
//  WalletKitCore
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKBTC.h"
#include "bitcoin/BRBitcoinChainParams.h"
#include "bcash/BRBCashParams.h"
#include "bsv/BRBSVParams.h"
#include "walletkit/WKHashP.h"

static WKNetworkBTC
wkNetworkCoerce (WKNetwork network, WKNetworkType type) {
    assert (type == network->type);
    return (WKNetworkBTC) network;
}

static WKNetworkBTC
wkNetworkCoerceANY (WKNetwork network) {
    assert (WK_NETWORK_TYPE_BTC == network->type ||
            WK_NETWORK_TYPE_BCH == network->type ||
            WK_NETWORK_TYPE_BSV == network->type);
    return (WKNetworkBTC) network;
}

extern const BRBitcoinChainParams *
wkNetworkAsBTC (WKNetwork network) {
    return wkNetworkCoerce(network, network->type)->params;
}

typedef struct {
    const BRBitcoinChainParams *params;
} WKNetworkCreateContextBTC;

static void
wkNetworkCreateCallbackBTC (WKNetworkCreateContext context,
                                WKNetwork network) {
    WKNetworkCreateContextBTC *contextBTC = (WKNetworkCreateContextBTC*) context;
    WKNetworkBTC networkBTC = wkNetworkCoerceANY (network);

    networkBTC->params = contextBTC->params;
}

static WKNetwork
wkNetworkCreateBTC (WKNetworkListener listener,
                        const char *uids,
                        const char *name,
                        const char *desc,
                        bool isMainnet,
                        uint32_t confirmationPeriodInSeconds,
                        WKAddressScheme defaultAddressScheme,
                        WKSyncMode defaultSyncMode,
                        WKCurrency nativeCurrency) {
    assert (0 == strcmp (desc, (isMainnet ? "mainnet" : "testnet")));

    WKNetworkCreateContextBTC contextBTC = {
        btcChainParams(isMainnet)
    };

    return wkNetworkAllocAndInit (sizeof (struct WKNetworkBTCRecord),
                                      WK_NETWORK_TYPE_BTC,
                                      listener,
                                      uids,
                                      name,
                                      desc,
                                      isMainnet,
                                      confirmationPeriodInSeconds,
                                      defaultAddressScheme,
                                      defaultSyncMode,
                                      nativeCurrency,
                                      &contextBTC,
                                      wkNetworkCreateCallbackBTC);
}

static WKNetwork
wkNetworkCreateBCH (WKNetworkListener listener,
                        const char *uids,
                        const char *name,
                        const char *desc,
                        bool isMainnet,
                        uint32_t confirmationPeriodInSeconds,
                        WKAddressScheme defaultAddressScheme,
                        WKSyncMode defaultSyncMode,
                        WKCurrency nativeCurrency) {
    assert (0 == strcmp (desc, (isMainnet ? "mainnet" : "testnet")));

    WKNetworkCreateContextBTC contextBTC = {
        bchChainParams(isMainnet)
    };

    return wkNetworkAllocAndInit (sizeof (struct WKNetworkBTCRecord),
                                      WK_NETWORK_TYPE_BCH,
                                      listener,
                                      uids,
                                      name,
                                      desc,
                                      isMainnet,
                                      confirmationPeriodInSeconds,
                                      defaultAddressScheme,
                                      defaultSyncMode,
                                      nativeCurrency,
                                      &contextBTC,
                                      wkNetworkCreateCallbackBTC);
}

static WKNetwork
wkNetworkCreateBSV (WKNetworkListener listener,
                        const char *uids,
                        const char *name,
                        const char *desc,
                        bool isMainnet,
                        uint32_t confirmationPeriodInSeconds,
                        WKAddressScheme defaultAddressScheme,
                        WKSyncMode defaultSyncMode,
                        WKCurrency nativeCurrency) {
    assert (0 == strcmp (desc, (isMainnet ? "mainnet" : "testnet")));

    WKNetworkCreateContextBTC contextBTC = {
        bsvChainParams(isMainnet)
    };

    return wkNetworkAllocAndInit (sizeof (struct WKNetworkBTCRecord),
                                      WK_NETWORK_TYPE_BSV,
                                      listener,
                                      uids,
                                      name,
                                      desc,
                                      isMainnet,
                                      confirmationPeriodInSeconds,
                                      defaultAddressScheme,
                                      defaultSyncMode,
                                      nativeCurrency,
                                      &contextBTC,
                                      wkNetworkCreateCallbackBTC);
}

static void
wkNetworkReleaseBTC (WKNetwork network) {
    WKNetworkBTC networkBTC = wkNetworkCoerceANY (network);
    (void) networkBTC;
}

static WKAddress
wkNetworkCreateAddressBTC (WKNetwork network,
                                const char *addressAsString) {
    WKNetworkBTC networkBTC = wkNetworkCoerce (network, WK_NETWORK_TYPE_BTC);
    assert (btcChainParamsIsBitcoin (networkBTC->params));
    return wkAddressCreateFromStringAsBTC (networkBTC->params->addrParams, addressAsString);
}

static WKAddress
wkNetworkCreateAddressBCH (WKNetwork network,
                                const char *addressAsString) {
    WKNetworkBTC networkBTC = wkNetworkCoerce (network, WK_NETWORK_TYPE_BCH);
    assert (!btcChainParamsIsBitcoin (networkBTC->params));
    return wkAddressCreateFromStringAsBCH (networkBTC->params->addrParams, addressAsString);
}

static WKAddress
wkNetworkCreateAddressBSV (WKNetwork network,
                                const char *addressAsString) {
    WKNetworkBTC networkBTC = wkNetworkCoerce (network, WK_NETWORK_TYPE_BSV);
    assert (bsvChainParamsHasParams (networkBTC->params));
    return wkAddressCreateFromStringAsBSV (networkBTC->params->addrParams, addressAsString);
}

static WKBlockNumber
wkNetworkGetBlockNumberAtOrBeforeTimestampBTC (WKNetwork network,
                                                   WKTimestamp timestamp) {
    WKNetworkBTC networkBTC = wkNetworkCoerce (network, network->type);
    const BRBitcoinCheckPoint *checkpoint = btcChainParamsGetCheckpointBefore (networkBTC->params, (uint32_t) timestamp);
    return (NULL == checkpoint ? 0 : checkpoint->height);
}

static WKBoolean
wkNetworkIsAccountInitializedBTC (WKNetwork network,
                                      WKAccount account) {
    WKNetworkBTC networkBTC = wkNetworkCoerceANY (network);
    (void) networkBTC;

    return WK_TRUE;
}


static uint8_t *
wkNetworkGetAccountInitializationDataBTC (WKNetwork network,
                                              WKAccount account,
                                              size_t *bytesCount) {
    WKNetworkBTC networkBTC = wkNetworkCoerceANY (network);
    (void) networkBTC;

    return NULL;
}

static void
wkNetworkInitializeAccountBTC (WKNetwork network,
                                   WKAccount account,
                                   const uint8_t *bytes,
                                   size_t bytesCount) {
    WKNetworkBTC networkBTC = wkNetworkCoerceANY (network);
    (void) networkBTC;
    
    return;
}

static WKHash
wkNetworkCreateHashFromStringBTC (WKNetwork network,
                                      const char *string) {
    assert(64 == strlen (string));
    UInt256 hash = uint256(string);
    return wkHashCreateAsBTC (hash);
}

static char *
wkNetworkEncodeHashBTC (WKHash hash) {
    return wkHashStringAsHex (hash, false);
}

// MARK: - Network Fee

extern uint64_t
wkNetworkFeeAsBTC (WKNetworkFee networkFee) {
    WKBoolean overflow;
    uint64_t value = wkAmountGetIntegerRaw (networkFee->pricePerCostFactor, &overflow);
    assert (WK_FALSE == overflow);
    return value;
}

WKNetworkHandlers wkNetworkHandlersBTC = {
    wkNetworkCreateBTC,
    wkNetworkReleaseBTC,
    wkNetworkCreateAddressBTC,
    wkNetworkGetBlockNumberAtOrBeforeTimestampBTC,
    wkNetworkIsAccountInitializedBTC,
    wkNetworkGetAccountInitializationDataBTC,
    wkNetworkInitializeAccountBTC,
    wkNetworkCreateHashFromStringBTC,
    wkNetworkEncodeHashBTC
};

WKNetworkHandlers wkNetworkHandlersBCH = {
    wkNetworkCreateBCH,
    wkNetworkReleaseBTC,
    wkNetworkCreateAddressBCH,
    wkNetworkGetBlockNumberAtOrBeforeTimestampBTC,
    wkNetworkIsAccountInitializedBTC,
    wkNetworkGetAccountInitializationDataBTC,
    wkNetworkInitializeAccountBTC,
    wkNetworkCreateHashFromStringBTC,
    wkNetworkEncodeHashBTC
};

WKNetworkHandlers wkNetworkHandlersBSV = {
    wkNetworkCreateBSV,
    wkNetworkReleaseBTC,
    wkNetworkCreateAddressBSV,
    wkNetworkGetBlockNumberAtOrBeforeTimestampBTC,
    wkNetworkIsAccountInitializedBTC,
    wkNetworkGetAccountInitializationDataBTC,
    wkNetworkInitializeAccountBTC,
    wkNetworkCreateHashFromStringBTC,
    wkNetworkEncodeHashBTC
};


//static WKNetwork
//wkNetworkCreateAsBTC (const char *uids,
//                          const char *name,
//                          const BRBitcoinChainParams *params) {
//    WKNetwork network = wkNetworkCreate (uids, name, WK_NETWORK_TYPE_BTC);
//    network->type = BLOCK_CHAIN_TYPE_BTC;
//    network->u.btc = params;
//
//    return network;
//}
//
//static WKNetwork
//wkNetworkCreateAsBCH (const char *uids,
//                          const char *name,
//                          const BRBitcoinChainParams *params) {
//    WKNetwork network = wkNetworkCreate (uids, name, WK_NETWORK_TYPE_BCH);
//    network->type = BLOCK_CHAIN_TYPE_BTC;
//    network->u.btc = params;
//
//    return network;
//}
//
//static WKNetwork
//wkNetworkCreateAsETH (const char *uids,
//                          const char *name,
//                          BREthereumNetwork net) {
//    WKNetwork network = wkNetworkCreate (uids, name, WK_NETWORK_TYPE_ETH);
//    network->type = BLOCK_CHAIN_TYPE_ETH;
//    network->u.eth = net;
//
//    return network;
//}
//
//static WKNetwork
//wkNetworkCreateAsGEN (const char *uids,
//                          const char *name,
//                          uint8_t isMainnet,
//                          WKNetworkCanonicalType canonicalType) {
//    WKNetwork network = wkNetworkCreate (uids, name, canonicalType);
//    network->type = BLOCK_CHAIN_TYPE_GEN;
//    network->u.gen = genNetworkCreate(canonicalType, isMainnet);
//    return network;
//}

