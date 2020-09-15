//
//  BRCryptoNetworkETH.c
//  Core
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoETH.h"
#include "crypto/BRCryptoAmountP.h"
#include "crypto/BRCryptoHashP.h"
#include "ethereum/blockchain/BREthereumNetwork.h"
#include "ethereum/blockchain/BREthereumBlock.h"

static BRCryptoNetworkETH
cryptoNetworkCoerce (BRCryptoNetwork network) {
    assert (CRYPTO_NETWORK_TYPE_ETH == network->type);
    return (BRCryptoNetworkETH) network;
}

private_extern BREthereumNetwork
cryptoNetworkAsETH (BRCryptoNetwork network) {
    BRCryptoNetworkETH networkETH = cryptoNetworkCoerce(network);
    return networkETH->eth;
}

typedef struct {
    BREthereumNetwork eth;
} BRCryptoNetworkCreateContextETH;

static void
cryptoNetworkCreateCallbackETH (BRCryptoNetworkCreateContext context,
                                BRCryptoNetwork network) {
    BRCryptoNetworkCreateContextETH *contextETH = (BRCryptoNetworkCreateContextETH*) context;
    BRCryptoNetworkETH networkETH = cryptoNetworkCoerce (network);

    networkETH->eth = contextETH->eth;
}

static BRCryptoNetwork
cryptoNetworkCreateAsETH (BRCryptoNetworkListener listener,
                          const char *uids,
                          const char *name,
                          const char *desc,
                          bool isMainnet,
                          uint32_t confirmationPeriodInSeconds,
                          BREthereumNetwork eth) {
    BRCryptoNetworkCreateContextETH contextETH = {
        eth
    };

    return cryptoNetworkAllocAndInit (sizeof (struct BRCryptoNetworkETHRecord),
                                      CRYPTO_NETWORK_TYPE_ETH,
                                      listener,
                                      uids,
                                      name,
                                      desc,
                                      isMainnet,
                                      confirmationPeriodInSeconds,
                                      &contextETH,
                                      cryptoNetworkCreateCallbackETH);
}

static BRCryptoNetwork
cyptoNetworkCreateETH (BRCryptoNetworkListener listener,
                       const char *uids,
                       const char *name,
                       const char *desc,
                       bool isMainnet,
                       uint32_t confirmationPeriodInSeconds) {
    if      (0 == strcmp ("mainnet", desc))
        return cryptoNetworkCreateAsETH (listener, uids, name, desc, true, confirmationPeriodInSeconds, ethNetworkMainnet);
    else if (0 == strcmp ("testnet", desc))
        return cryptoNetworkCreateAsETH (listener, uids, name, desc, false, confirmationPeriodInSeconds, ethNetworkTestnet);
    else if (0 == strcmp ("rinkeby", desc))
        return cryptoNetworkCreateAsETH (listener, uids, name, desc, false, confirmationPeriodInSeconds, ethNetworkRinkeby);
    else {
        assert (false); return NULL;
    }
}

static void
cryptoNetworkReleaseETH (BRCryptoNetwork network) {
    BRCryptoNetworkETH networkETH = cryptoNetworkCoerce (network);
    (void) networkETH;
}

extern BRCryptoAddress
cryptoAddressCreateFromStringAsETH (const char *address);

static BRCryptoAddress
cryptoNetworkCreateAddressETH (BRCryptoNetwork network,
                               const char *addressAsString) {
    BRCryptoNetworkETH networkETH = cryptoNetworkCoerce (network);
    (void) networkETH;

    return cryptoAddressCreateFromStringAsETH (addressAsString);
}

static BRCryptoBlockNumber
cryptoNetworkGetBlockNumberAtOrBeforeTimestampETH (BRCryptoNetwork network,
                                                   BRCryptoTimestamp timestamp) {
    BRCryptoNetworkETH networkETH = cryptoNetworkCoerce (network);
    const BREthereumBlockCheckpoint *checkpoint = blockCheckpointLookupByTimestamp (networkETH->eth, timestamp);
    return (NULL == checkpoint ? 0 : checkpoint->number);
}


static BRCryptoBoolean
cryptoNetworkIsAccountInitializedETH (BRCryptoNetwork network,
                                      BRCryptoAccount account) {
    BRCryptoNetworkETH networkETH = cryptoNetworkCoerce (network);
    (void) networkETH;

    return CRYPTO_TRUE;
}


static uint8_t *
cryptoNetworkGetAccountInitializationDataETH (BRCryptoNetwork network,
                                              BRCryptoAccount account,
                                              size_t *bytesCount) {
    BRCryptoNetworkETH networkETH = cryptoNetworkCoerce (network);
    (void) networkETH;

    return NULL;
}

static void
cryptoNetworkInitializeAccountETH (BRCryptoNetwork network,
                                   BRCryptoAccount account,
                                   const uint8_t *bytes,
                                   size_t bytesCount) {
    BRCryptoNetworkETH networkETH = cryptoNetworkCoerce (network);
    (void) networkETH;

    return;
}

private_extern BREthereumGasPrice
cryptoNetworkFeeAsETH (BRCryptoNetworkFee fee) {
    BRCryptoAmount     amount   = cryptoNetworkFeeGetPricePerCostFactor (fee);
    BREthereumGasPrice gasPrice = ethGasPriceCreate (ethEtherCreate (cryptoAmountGetValue(amount)));
    cryptoAmountGive (amount);

    return gasPrice;
}

static BRCryptoHash
cryptoNetworkCreateHashFromStringETH (BRCryptoNetwork network,
                                      const char *string) {
    return cryptoHashCreateAsETH (ethHashCreate (string));
}

static char *
cryptoNetworkEncodeHashETH (BRCryptoHash hash) {
    return cryptoHashStringAsHex (hash);
}

// MARK: -

BRCryptoNetworkHandlers cryptoNetworkHandlersETH = {
    cyptoNetworkCreateETH,
    cryptoNetworkReleaseETH,
    cryptoNetworkCreateAddressETH,
    cryptoNetworkGetBlockNumberAtOrBeforeTimestampETH,
    cryptoNetworkIsAccountInitializedETH,
    cryptoNetworkGetAccountInitializationDataETH,
    cryptoNetworkInitializeAccountETH,
    cryptoNetworkCreateHashFromStringETH,
    cryptoNetworkEncodeHashETH
};
