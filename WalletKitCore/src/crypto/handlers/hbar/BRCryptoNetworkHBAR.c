//
//  BRCryptoNetworkHBAR.c
//
//
//  Created by Ehsan Rezaie on 2020-05-19.
//

#include "BRCryptoHBAR.h"
#include "../../BRCryptoAccountP.h"


static BRCryptoNetwork
cryptoNetworkCreateAsHBAR (const char *uids,
                           const char *name,
                           const char *desc,
                           bool isMainnet,
                           uint32_t confirmationPeriodInSeconds) {
    BRCryptoNetwork networkBase = cryptoNetworkAllocAndInit (sizeof (struct BRCryptoNetworkRecord),
                                                             CRYPTO_NETWORK_TYPE_HBAR,
                                                             uids,
                                                             name,
                                                             desc,
                                                             isMainnet,
                                                             confirmationPeriodInSeconds);
    
    return networkBase;
}

static BRCryptoNetwork
cyptoNetworkCreateHBAR (const char *uids,
                        const char *name,
                        const char *desc,
                        bool isMainnet,
                        uint32_t confirmationPeriodInSeconds) {
    if      (0 == strcmp ("mainnet", desc))
        return cryptoNetworkCreateAsHBAR (uids, name, desc, true, confirmationPeriodInSeconds);
    else if (0 == strcmp ("testnet", desc))
        return cryptoNetworkCreateAsHBAR (uids, name, desc, false, confirmationPeriodInSeconds);
    else {
        assert (false); return NULL;
    }
}

static void
cryptoNetworkReleaseHBAR (BRCryptoNetwork network) {
}

//TODO:HBAR make common? remove network param?
static BRCryptoAddress
cryptoNetworkCreateAddressHBAR (BRCryptoNetwork networkBase,
                                const char *addressAsString) {
    return cryptoAddressCreateFromStringAsHBAR (addressAsString);
}

static BRCryptoBlockNumber
cryptoNetworkGetBlockNumberAtOrBeforeTimestampHBAR (BRCryptoNetwork networkBase,
                                                    BRCryptoTimestamp timestamp) {
    //TODO:HBAR
    return 0;
}

// MARK: Account Initialization

static BRCryptoBoolean
cryptoNetworkIsAccountInitializedHBAR (BRCryptoNetwork network,
                                       BRCryptoAccount account) {
    BRHederaAccount hbarAccount = cryptoAccountAsHBAR (account);
    assert (NULL != hbarAccount);
    return AS_CRYPTO_BOOLEAN (true);
}


static uint8_t *
cryptoNetworkGetAccountInitializationDataHBAR (BRCryptoNetwork network,
                                               BRCryptoAccount account,
                                               size_t *bytesCount) {
    BRHederaAccount hbarAccount = cryptoAccountAsHBAR (account);
    assert (NULL != hbarAccount);
    if (NULL != bytesCount) *bytesCount = 0;
    return NULL;
}

static void
cryptoNetworkInitializeAccountHBAR (BRCryptoNetwork network,
                                    BRCryptoAccount account,
                                    const uint8_t *bytes,
                                    size_t bytesCount) {
    BRHederaAccount hbarAccount = cryptoAccountAsHBAR (account);
    assert (NULL != hbarAccount);
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

