//
//  BRCryptoNetworkHBAR.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoHBAR.h"
#include "crypto/BRCryptoAccountP.h"

static BRCryptoNetworkHBAR
cryptoNetworkCoerce (BRCryptoNetwork network) {
    assert (CRYPTO_NETWORK_TYPE_HBAR == network->type);
    return (BRCryptoNetworkHBAR) network;
}

static BRCryptoNetwork
cryptoNetworkCreateAsHBAR (BRCryptoNetworkListener listener,
                           const char *uids,
                           const char *name,
                           const char *desc,
                           bool isMainnet,
                           uint32_t confirmationPeriodInSeconds) {
    BRCryptoNetwork network = cryptoNetworkAllocAndInit (sizeof (struct BRCryptoNetworkRecord),
                                                         CRYPTO_NETWORK_TYPE_HBAR,
                                                         listener,
                                                         uids,
                                                         name,
                                                         desc,
                                                         isMainnet,
                                                         confirmationPeriodInSeconds,
                                                         NULL,
                                                         NULL);
    
    return network;
}

static BRCryptoNetwork
cyptoNetworkCreateHBAR (BRCryptoNetworkListener listener,
                        const char *uids,
                        const char *name,
                        const char *desc,
                        bool isMainnet,
                        uint32_t confirmationPeriodInSeconds) {
    if      (0 == strcmp ("mainnet", desc))
        return cryptoNetworkCreateAsHBAR (listener, uids, name, desc, true, confirmationPeriodInSeconds);
    else if (0 == strcmp ("testnet", desc))
        return cryptoNetworkCreateAsHBAR (listener, uids, name, desc, false, confirmationPeriodInSeconds);
    else {
        assert (false); return NULL;
    }
}

static void
cryptoNetworkReleaseHBAR (BRCryptoNetwork network) {
    BRCryptoNetworkHBAR networkHBAR = cryptoNetworkCoerce (network);
    (void) networkHBAR;
}

//TODO:HBAR make common? remove network param?
static BRCryptoAddress
cryptoNetworkCreateAddressHBAR (BRCryptoNetwork network,
                                const char *addressAsString) {
    BRCryptoNetworkHBAR networkHBAR = cryptoNetworkCoerce (network);
    (void) networkHBAR;

    return cryptoAddressCreateFromStringAsHBAR (addressAsString);
}

static BRCryptoBlockNumber
cryptoNetworkGetBlockNumberAtOrBeforeTimestampHBAR (BRCryptoNetwork network,
                                                    BRCryptoTimestamp timestamp) {
    BRCryptoNetworkHBAR networkHBAR = cryptoNetworkCoerce (network);
    (void) networkHBAR;

    //TODO:HBAR
    return 0;
}

// MARK: Account Initialization

static BRCryptoBoolean
cryptoNetworkIsAccountInitializedHBAR (BRCryptoNetwork network,
                                       BRCryptoAccount account) {
    BRCryptoNetworkHBAR networkHBAR = cryptoNetworkCoerce (network);
    (void) networkHBAR;

    BRHederaAccount hbarAccount = cryptoAccountAsHBAR (account);
    assert (NULL != hbarAccount);

    return AS_CRYPTO_BOOLEAN (hederaAccountHasPrimaryAddress (hbarAccount));
}


static uint8_t *
cryptoNetworkGetAccountInitializationDataHBAR (BRCryptoNetwork network,
                                               BRCryptoAccount account,
                                               size_t *bytesCount) {
    BRCryptoNetworkHBAR networkHBAR = cryptoNetworkCoerce (network);
    (void) networkHBAR;

    BRHederaAccount hbarAccount = cryptoAccountAsHBAR (account);
    assert (NULL != hbarAccount);

    return hederaAccountGetPublicKeyBytes (hbarAccount, bytesCount);
}

static void
cryptoNetworkInitializeAccountHBAR (BRCryptoNetwork network,
                                    BRCryptoAccount account,
                                    const uint8_t *bytes,
                                    size_t bytesCount) {
    BRCryptoNetworkHBAR networkHBAR = cryptoNetworkCoerce (network);
    (void) networkHBAR;

    BRHederaAccount hbarAccount = cryptoAccountAsHBAR (account);
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

// MARK: -

BRCryptoNetworkHandlers cryptoNetworkHandlersHBAR = {
    cyptoNetworkCreateHBAR,
    cryptoNetworkReleaseHBAR,
    cryptoNetworkCreateAddressHBAR,
    cryptoNetworkGetBlockNumberAtOrBeforeTimestampHBAR,
    cryptoNetworkIsAccountInitializedHBAR,
    cryptoNetworkGetAccountInitializationDataHBAR,
    cryptoNetworkInitializeAccountHBAR
};

