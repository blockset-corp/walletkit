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
#include "crypto/BRCryptoHashP.h"

static BRCryptoNetworkHBAR
cryptoNetworkCoerce (BRCryptoNetwork network) {
    assert (CRYPTO_NETWORK_TYPE_HBAR == network->type);
    return (BRCryptoNetworkHBAR) network;
}

static BRCryptoNetwork
cyptoNetworkCreateHBAR (BRCryptoNetworkListener listener,
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
                                      CRYPTO_NETWORK_TYPE_HBAR,
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
cryptoNetworkReleaseHBAR (BRCryptoNetwork network) {
    BRCryptoNetworkHBAR networkHBAR = cryptoNetworkCoerce (network);
    (void) networkHBAR;
}

static BRCryptoAddress
cryptoNetworkCreateAddressHBAR (BRCryptoNetwork network,
                                const char *addressAsString) {
    return cryptoAddressCreateFromStringAsHBAR (addressAsString);
}

static BRCryptoBlockNumber
cryptoNetworkGetBlockNumberAtOrBeforeTimestampHBAR (BRCryptoNetwork network,
                                                    BRCryptoTimestamp timestamp) {
    // not supported (used for p2p sync checkpoints)
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

static BRCryptoHash
cryptoNetworkCreateHashFromStringHBAR (BRCryptoNetwork network,
                                      const char *string) {
    BRHederaTransactionHash hash = hederaHashCreateFromString(string);
    return cryptoHashCreateAsHBAR (hash);
}

static char *
cryptoNetworkEncodeHashHBAR (BRCryptoHash hash) {
    return cryptoHashStringAsHex (hash, false);
}

// MARK: -

BRCryptoNetworkHandlers cryptoNetworkHandlersHBAR = {
    cyptoNetworkCreateHBAR,
    cryptoNetworkReleaseHBAR,
    cryptoNetworkCreateAddressHBAR,
    cryptoNetworkGetBlockNumberAtOrBeforeTimestampHBAR,
    cryptoNetworkIsAccountInitializedHBAR,
    cryptoNetworkGetAccountInitializationDataHBAR,
    cryptoNetworkInitializeAccountHBAR,
    cryptoNetworkCreateHashFromStringHBAR,
    cryptoNetworkEncodeHashHBAR
};

