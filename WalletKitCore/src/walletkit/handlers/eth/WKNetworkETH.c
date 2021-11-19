//
//  WKNetworkETH.c
//  WalletKitCore
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKETH.h"
#include "walletkit/WKAmountP.h"
#include "walletkit/WKHashP.h"
#include "ethereum/blockchain/BREthereumNetwork.h"
#include "ethereum/blockchain/BREthereumBlock.h"
#include "ethereum/util/BREthereumLog.h"

static WKNetworkETH
wkNetworkCoerce (WKNetwork network) {
    assert (WK_NETWORK_TYPE_ETH == network->type);
    return (WKNetworkETH) network;
}

private_extern BREthereumNetwork
wkNetworkAsETH (WKNetwork network) {
    WKNetworkETH networkETH = wkNetworkCoerce(network);
    return networkETH->eth;
}

typedef struct {
    BREthereumNetwork eth;
} WKNetworkCreateContextETH;

static void
wkNetworkCreateCallbackETH (WKNetworkCreateContext context,
                                WKNetwork network) {
    WKNetworkCreateContextETH *contextETH = (WKNetworkCreateContextETH*) context;
    WKNetworkETH networkETH = wkNetworkCoerce (network);

    networkETH->eth = contextETH->eth;
}

static WKNetwork
cyptoNetworkCreateETH (WKNetworkListener listener,
                       const char *uids,
                       const char *name,
                       const char *desc,
                       bool isMainnet,
                       uint32_t confirmationPeriodInSeconds,
                       WKAddressScheme defaultAddressScheme,
                       WKSyncMode defaultSyncMode,
                       WKCurrency nativeCurrency) {

    bool useMainnet = (0 == strcmp ("mainnet", desc));
    bool useTestnet = (0 == strcmp ("testnet", desc));
    bool useRinkeby = (0 == strcmp ("rinkeby", desc));

    LOG (LL_INFO, ETH_INIT, "Creating '%s' Ethereum Network", desc);
    
    assert (isMainnet ? useMainnet : (useTestnet || useRinkeby));
    
    WKNetworkCreateContextETH contextETH = {
        (useMainnet ? ethNetworkMainnet : (useTestnet ? ethNetworkTestnet : ethNetworkRinkeby))
    };

    return wkNetworkAllocAndInit (sizeof (struct WKNetworkETHRecord),
                                      WK_NETWORK_TYPE_ETH,
                                      listener,
                                      uids,
                                      name,
                                      desc,
                                      isMainnet,
                                      confirmationPeriodInSeconds,
                                      defaultAddressScheme,
                                      defaultSyncMode,
                                      nativeCurrency,
                                      &contextETH,
                                      wkNetworkCreateCallbackETH);
}

static void
wkNetworkReleaseETH (WKNetwork network) {
    WKNetworkETH networkETH = wkNetworkCoerce (network);
    (void) networkETH;
}

static WKAddress
wkNetworkCreateAddressETH (WKNetwork network,
                               const char *addressAsString) {
    WKNetworkETH networkETH = wkNetworkCoerce (network);
    (void) networkETH;

    return wkAddressCreateFromStringAsETH (addressAsString);
}

static WKBlockNumber
wkNetworkGetBlockNumberAtOrBeforeTimestampETH (WKNetwork network,
                                                   WKTimestamp timestamp) {
    WKNetworkETH networkETH = wkNetworkCoerce (network);
    const BREthereumBlockCheckpoint *checkpoint = ethBlockCheckpointLookupByTimestamp (networkETH->eth, timestamp);
    return (NULL == checkpoint ? 0 : checkpoint->number);
}


static WKBoolean
wkNetworkIsAccountInitializedETH (WKNetwork network,
                                      WKAccount account) {
    WKNetworkETH networkETH = wkNetworkCoerce (network);
    (void) networkETH;

    return WK_TRUE;
}


static uint8_t *
wkNetworkGetAccountInitializationDataETH (WKNetwork network,
                                              WKAccount account,
                                              size_t *bytesCount) {
    WKNetworkETH networkETH = wkNetworkCoerce (network);
    (void) networkETH;

    return NULL;
}

static void
wkNetworkInitializeAccountETH (WKNetwork network,
                                   WKAccount account,
                                   const uint8_t *bytes,
                                   size_t bytesCount) {
    WKNetworkETH networkETH = wkNetworkCoerce (network);
    (void) networkETH;

    return;
}

private_extern BREthereumGasPrice
wkNetworkFeeAsETH (WKNetworkFee fee) {
    WKAmount     amount   = wkNetworkFeeGetPricePerCostFactor (fee);
    BREthereumGasPrice gasPrice = ethGasPriceCreate (ethEtherCreate (wkAmountGetValue(amount)));
    wkAmountGive (amount);

    return gasPrice;
}

static WKHash
wkNetworkCreateHashFromStringETH (WKNetwork network,
                                      const char *string) {
    return wkHashCreateAsETH (ethHashCreate (string));
}

static char *
wkNetworkEncodeHashETH (WKHash hash) {
    return wkHashStringAsHex (hash, true);
}

// MARK: -

WKNetworkHandlers wkNetworkHandlersETH = {
    cyptoNetworkCreateETH,
    wkNetworkReleaseETH,
    wkNetworkCreateAddressETH,
    wkNetworkGetBlockNumberAtOrBeforeTimestampETH,
    wkNetworkIsAccountInitializedETH,
    wkNetworkGetAccountInitializationDataETH,
    wkNetworkInitializeAccountETH,
    wkNetworkCreateHashFromStringETH,
    wkNetworkEncodeHashETH
};
