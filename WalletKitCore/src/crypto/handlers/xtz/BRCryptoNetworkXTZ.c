//
//  BRCryptoNetworkXTZ.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoXTZ.h"
#include "crypto/BRCryptoAccountP.h"
#include "crypto/BRCryptoHashP.h"
#include "support/BRBase58.h"

static BRCryptoNetworkXTZ
cryptoNetworkCoerce (BRCryptoNetwork network) {
    assert (CRYPTO_NETWORK_TYPE_XTZ == network->type);
    return (BRCryptoNetworkXTZ) network;
}

static BRCryptoNetwork
cyptoNetworkCreateXTZ (BRCryptoNetworkListener listener,
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
                                      CRYPTO_NETWORK_TYPE_XTZ,
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
cryptoNetworkReleaseXTZ (BRCryptoNetwork network) {
    BRCryptoNetworkXTZ networkXTZ = cryptoNetworkCoerce (network);
    (void) networkXTZ;
}

static BRCryptoAddress
cryptoNetworkCreateAddressXTZ (BRCryptoNetwork network,
                               const char *addressAsString) {
    BRCryptoNetworkXTZ networkXTZ = cryptoNetworkCoerce (network);
    (void) networkXTZ;

    return cryptoAddressCreateFromStringAsXTZ (addressAsString);
}

static BRCryptoBlockNumber
cryptoNetworkGetBlockNumberAtOrBeforeTimestampXTZ (BRCryptoNetwork network,
                                                   BRCryptoTimestamp timestamp) {
    // not supported (used for p2p sync checkpoints)
    return 0;
}

// MARK: Account Initialization

static BRCryptoBoolean
cryptoNetworkIsAccountInitializedXTZ (BRCryptoNetwork network,
                                      BRCryptoAccount account) {
    BRCryptoNetworkXTZ networkXTZ = cryptoNetworkCoerce (network);
    (void) networkXTZ;

    BRTezosAccount xtzAccount = cryptoAccountAsXTZ (account);
    assert (NULL != xtzAccount);
    return AS_CRYPTO_BOOLEAN (true);
}


static uint8_t *
cryptoNetworkGetAccountInitializationDataXTZ (BRCryptoNetwork network,
                                              BRCryptoAccount account,
                                              size_t *bytesCount) {
    BRCryptoNetworkXTZ networkXTZ = cryptoNetworkCoerce (network);
    (void) networkXTZ;

    BRTezosAccount xtzAccount = cryptoAccountAsXTZ (account);
    assert (NULL != xtzAccount);
    if (NULL != bytesCount) *bytesCount = 0;
    return NULL;
}

static void
cryptoNetworkInitializeAccountXTZ (BRCryptoNetwork network,
                                   BRCryptoAccount account,
                                   const uint8_t *bytes,
                                   size_t bytesCount) {
    BRCryptoNetworkXTZ networkXTZ = cryptoNetworkCoerce (network);
    (void) networkXTZ;

    BRTezosAccount xtzAccount = cryptoAccountAsXTZ (account);
    assert (NULL != xtzAccount);
    return;
}

static BRCryptoHash
cryptoNetworkCreateHashFromStringXTZ (BRCryptoNetwork network,
                                      const char *string) {
    return cryptoHashCreateFromStringAsXTZ (string);
}

static char *
cryptoNetworkEncodeHashXTZ (BRCryptoHash hash) {
    size_t len = BRBase58CheckEncode (NULL, 0, hash->bytes, TEZOS_HASH_BYTES);
    if (0 == len) return NULL;

    char * string = calloc (1, len);
    BRBase58CheckEncode (string, len, hash->bytes, TEZOS_HASH_BYTES);
    return string;
}

// MARK: - Handlers

BRCryptoNetworkHandlers cryptoNetworkHandlersXTZ = {
    cyptoNetworkCreateXTZ,
    cryptoNetworkReleaseXTZ,
    cryptoNetworkCreateAddressXTZ,
    cryptoNetworkGetBlockNumberAtOrBeforeTimestampXTZ,
    cryptoNetworkIsAccountInitializedXTZ,
    cryptoNetworkGetAccountInitializationDataXTZ,
    cryptoNetworkInitializeAccountXTZ,
    cryptoNetworkCreateHashFromStringXTZ,
    cryptoNetworkEncodeHashXTZ
};

