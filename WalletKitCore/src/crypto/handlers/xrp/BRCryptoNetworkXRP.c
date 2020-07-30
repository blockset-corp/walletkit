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

static BRCryptoNetworkXRP
cryptoNetworkCoerce (BRCryptoNetwork network) {
    assert (CRYPTO_NETWORK_TYPE_XRP == network->type);
    return (BRCryptoNetworkXRP) network;
}

static BRCryptoNetwork
cryptoNetworkCreateAsXRP (const char *uids,
                          const char *name,
                          const char *desc,
                          bool isMainnet,
                          uint32_t confirmationPeriodInSeconds) {
    BRCryptoNetwork network = cryptoNetworkAllocAndInit (sizeof (struct BRCryptoNetworkRecord),
                                                         CRYPTO_NETWORK_TYPE_XRP,
                                                         uids,
                                                         name,
                                                         desc,
                                                         isMainnet,
                                                         confirmationPeriodInSeconds);
    
    return network;
}

static BRCryptoNetwork
cyptoNetworkCreateXRP (const char *uids,
                       const char *name,
                       const char *desc,
                       bool isMainnet,
                       uint32_t confirmationPeriodInSeconds) {
    if      (0 == strcmp ("mainnet", desc))
        return cryptoNetworkCreateAsXRP (uids, name, desc, true, confirmationPeriodInSeconds);
    else if (0 == strcmp ("testnet", desc))
        return cryptoNetworkCreateAsXRP (uids, name, desc, false, confirmationPeriodInSeconds);
    else {
        assert (false); return NULL;
    }
}

static void
cryptoNetworkReleaseXRP (BRCryptoNetwork network) {
    BRCryptoNetworkXRP networkXRP = cryptoNetworkCoerce (network);
    (void) networkXRP;
}

//TODO:XRP make common? remove network param?
static BRCryptoAddress
cryptoNetworkCreateAddressXRP (BRCryptoNetwork network,
                               const char *addressAsString) {
    BRCryptoNetworkXRP networkXRP = cryptoNetworkCoerce (network);
    (void) networkXRP;

    return cryptoAddressCreateFromStringAsXRP (addressAsString);
}

static BRCryptoBlockNumber
cryptoNetworkGetBlockNumberAtOrBeforeTimestampXRP (BRCryptoNetwork network,
                                                   BRCryptoTimestamp timestamp) {
    BRCryptoNetworkXRP networkXRP = cryptoNetworkCoerce (network);
    (void) networkXRP;

    //TODO:XRP
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

// MARK: - Handlers

BRCryptoNetworkHandlers cryptoNetworkHandlersXRP = {
    cyptoNetworkCreateXRP,
    cryptoNetworkReleaseXRP,
    cryptoNetworkCreateAddressXRP,
    cryptoNetworkGetBlockNumberAtOrBeforeTimestampXRP,
    cryptoNetworkIsAccountInitializedXRP,
    cryptoNetworkGetAccountInitializationDataXRP,
    cryptoNetworkInitializeAccountXRP
};

