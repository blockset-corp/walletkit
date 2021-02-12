//
//  BRCryptoNetworkXRP.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoXRP.h"
#include "crypto/BRCryptoAccountP.h"
#include "crypto/BRCryptoHashP.h"

static BRCryptoNetworkXRP
cryptoNetworkCoerce (BRCryptoNetwork network) {
    assert (CRYPTO_NETWORK_TYPE_XRP == network->type);
    return (BRCryptoNetworkXRP) network;
}

static BRCryptoNetwork
cyptoNetworkCreateXRP (BRCryptoNetworkListener listener,
                       const char *uids,
                       const char *name,
                       const char *desc,
                       bool isMainnet,
                       uint32_t confirmationPeriodInSeconds,
                       BRCryptoAddressScheme defaultAddressScheme,
                       BRCryptoSyncMode defaultSyncMode,
                       BRCryptoCurrency nativeCurrency) {
    assert (0 == strcmp (desc, (isMainnet ? "mainnet" : "testnet")));

    return cryptoNetworkAllocAndInit (sizeof (struct BRCryptoNetworkRecord),
                                      CRYPTO_NETWORK_TYPE_XRP,
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
cryptoNetworkReleaseXRP (BRCryptoNetwork network) {
    BRCryptoNetworkXRP networkXRP = cryptoNetworkCoerce (network);
    (void) networkXRP;
}

static BRCryptoAddress
cryptoNetworkCreateAddressXRP (BRCryptoNetwork network,
                               const char *addressAsString) {
    return cryptoAddressCreateFromStringAsXRP (addressAsString);
}

static BRCryptoBlockNumber
cryptoNetworkGetBlockNumberAtOrBeforeTimestampXRP (BRCryptoNetwork network,
                                                   BRCryptoTimestamp timestamp) {
    // not supported (used for p2p sync checkpoints)
    return 0;
}

// MARK: Account Initialization

static BRCryptoBoolean
cryptoNetworkIsAccountInitializedXRP (BRCryptoNetwork network,
                                      BRCryptoAccount account) {
    BRCryptoNetworkXRP networkXRP = cryptoNetworkCoerce (network);
    (void) networkXRP;

    BRRippleAccount xrpAccount = cryptoAccountAsXRP (account);
    assert (NULL != xrpAccount);
    return AS_CRYPTO_BOOLEAN (true);
}


static uint8_t *
cryptoNetworkGetAccountInitializationDataXRP (BRCryptoNetwork network,
                                              BRCryptoAccount account,
                                              size_t *bytesCount) {
    BRCryptoNetworkXRP networkXRP = cryptoNetworkCoerce (network);
    (void) networkXRP;

    BRRippleAccount xrpAccount = cryptoAccountAsXRP (account);
    assert (NULL != xrpAccount);
    if (NULL != bytesCount) *bytesCount = 0;
    return NULL;
}

static void
cryptoNetworkInitializeAccountXRP (BRCryptoNetwork network,
                                   BRCryptoAccount account,
                                   const uint8_t *bytes,
                                   size_t bytesCount) {
    BRCryptoNetworkXRP networkXRP = cryptoNetworkCoerce (network);
    (void) networkXRP;

    BRRippleAccount xrpAccount = cryptoAccountAsXRP (account);
    assert (NULL != xrpAccount);
    return;
}

static BRCryptoHash
cryptoNetworkCreateHashFromStringXRP (BRCryptoNetwork network,
                                      const char *string) {
    BRRippleTransactionHash hash = rippleHashCreateFromString (string);
    return cryptoHashCreateAsXRP (hash);
}

static char *
cryptoNetworkEncodeHashXRP (BRCryptoHash hash) {
    return cryptoHashStringAsHex (hash, false);
}

// MARK: - Handlers

BRCryptoNetworkHandlers cryptoNetworkHandlersXRP = {
    cyptoNetworkCreateXRP,
    cryptoNetworkReleaseXRP,
    cryptoNetworkCreateAddressXRP,
    cryptoNetworkGetBlockNumberAtOrBeforeTimestampXRP,
    cryptoNetworkIsAccountInitializedXRP,
    cryptoNetworkGetAccountInitializationDataXRP,
    cryptoNetworkInitializeAccountXRP,
    cryptoNetworkCreateHashFromStringXRP,
    cryptoNetworkEncodeHashXRP
};

