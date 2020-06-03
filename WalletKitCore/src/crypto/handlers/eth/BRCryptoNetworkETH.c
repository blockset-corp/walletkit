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
#include "ethereum/blockchain/BREthereumNetwork.h"
#include "ethereum/blockchain/BREthereumBlock.h"

static BRCryptoNetworkETH
cryptoNetworkCoerce (BRCryptoNetwork network) {
    assert (CRYPTO_NETWORK_TYPE_ETH == network->type);
    return (BRCryptoNetworkETH) network;
}

private_extern BREthereumNetwork
cryptoNetworkAsETH (BRCryptoNetwork networkBase) {
    BRCryptoNetworkETH network = cryptoNetworkCoerce(networkBase);
    return network->eth;
}

static BRCryptoNetwork
cryptoNetworkCreateAsETH (const char *uids,
                          const char *name,
                          const char *desc,
                          bool isMainnet,
                          uint32_t confirmationPeriodInSeconds,
                          BREthereumNetwork eth) {
    BRCryptoNetwork networkBase = cryptoNetworkAllocAndInit (sizeof (struct BRCryptoNetworkETHRecord),
                                                             CRYPTO_NETWORK_TYPE_ETH,
                                                             uids,
                                                             name,
                                                             desc,
                                                             isMainnet,
                                                             confirmationPeriodInSeconds);
    BRCryptoNetworkETH network = cryptoNetworkCoerce(networkBase);
    network->eth = eth;

    return networkBase;
}

static BRCryptoNetwork
cyptoNetworkCreateETH (const char *uids,
                       const char *name,
                       const char *desc,
                       bool isMainnet,
                       uint32_t confirmationPeriodInSeconds) {
    if      (0 == strcmp ("mainnet", desc))
        return cryptoNetworkCreateAsETH (uids, name, desc, true, confirmationPeriodInSeconds, ethNetworkMainnet);
    else if (0 == strcmp ("testnet", desc))
        return cryptoNetworkCreateAsETH (uids, name, desc, false, confirmationPeriodInSeconds, ethNetworkTestnet);
    else if (0 == strcmp ("rinkeby", desc))
        return cryptoNetworkCreateAsETH (uids, name, desc, false, confirmationPeriodInSeconds, ethNetworkRinkeby);
    else {
        assert (false); return NULL;
    }
}

static void
cryptoNetworkReleaseETH (BRCryptoNetwork network) {
}

extern BRCryptoAddress
cryptoAddressCreateFromStringAsETH (const char *address);

static BRCryptoAddress
cryptoNetworkCreateAddressETH (BRCryptoNetwork networkBase,
                                const char *addressAsString) {
    return cryptoAddressCreateFromStringAsETH (addressAsString);
}

static BRCryptoBlockNumber
cryptoNetworkGetBlockNumberAtOrBeforeTimestampETH (BRCryptoNetwork networkBase,
                                                   BRCryptoTimestamp timestamp) {
    BRCryptoNetworkETH network = cryptoNetworkCoerce (networkBase);
    const BREthereumBlockCheckpoint *checkpoint = blockCheckpointLookupByTimestamp (network->eth, timestamp);
    return (NULL == checkpoint ? 0 : checkpoint->number);
}


static BRCryptoBoolean
cryptoNetworkIsAccountInitializedETH (BRCryptoNetwork network,
                                   BRCryptoAccount account) {
    return CRYPTO_TRUE;
}


static uint8_t *
cryptoNetworkGetAccountInitializationDataETH (BRCryptoNetwork network,
                                           BRCryptoAccount account,
                                           size_t *bytesCount) {
    return NULL;
}

static void
cryptoNetworkInitializeAccountETH (BRCryptoNetwork network,
                                BRCryptoAccount account,
                                const uint8_t *bytes,
                                size_t bytesCount) {
    return;
}

private_extern BREthereumGasPrice
cryptoNetworkFeeAsETH (BRCryptoNetworkFee fee) {
    BRCryptoAmount     amount   = cryptoNetworkFeeGetPricePerCostFactor (fee);
    BREthereumGasPrice gasPrice = ethGasPriceCreate (ethEtherCreate (cryptoAmountGetValue(amount)));
    cryptoAmountGive (amount);

    return gasPrice;
}

BRCryptoNetworkHandlers cryptoNetworkHandlersETH = {
    cyptoNetworkCreateETH,
    cryptoNetworkReleaseETH,
    cryptoNetworkCreateAddressETH,
    cryptoNetworkGetBlockNumberAtOrBeforeTimestampETH,
    cryptoNetworkIsAccountInitializedETH,
    cryptoNetworkGetAccountInitializationDataETH,
    cryptoNetworkInitializeAccountETH
};
