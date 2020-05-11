//
//  BRCryptoNetworkXRP.c
//  
//
//  Created by Ed Gamble on 4/24/20.
//

#include "BRCryptoXRP.h"

#include "bitcoin/BRChainParams.h"

struct BRCryptoNetworkXRPRecord {
    struct BRCryptoNetworkRecord base;
    // ...
};

static BRCryptoNetwork
cryptoNetworkCreateAsXRP (const char *uids,
                          const char *name,
                          bool isMainnet) {
    BRCryptoNetwork networkBase = cryptoNetworkAllocAndInit (sizeof (struct BRCryptoNetworkBTCRecord),
                                                             CRYPTO_NETWORK_TYPE_XRP,
                                                             uids,
                                                             name,
                                                             isMainnet);
//    BRCryptoNetworkXRP network = cryptoNetworkCoerce(networkBase);
//    network->params = params;

    return networkBase;
}


static BRCryptoNetwork
cyptoNetworkCreateXRP (const char *uids,
                                const char *name,
                                const char *network,
                       bool isMainnet) {
    if      (0 == strcmp ("mainnet", network))
        return cryptoNetworkCreateAsXRP (uids, name, true);
    else if (0 == strcmp ("testnet", network))
        return cryptoNetworkCreateAsXRP (uids, name, false);
    else {
        assert (false); return NULL;
    }
}

static void
cryptoNetworkReleaseXRP (BRCryptoNetwork network) {
}


extern BRCryptoAddress
cryptoAddressCreateFromStringAsXRP (BRAddressParams params, const char *btcAddress);

extern BRCryptoAddress
cryptoAddressCreateFromStringAsBCH (BRAddressParams params, const char *bchAddress);

static BRCryptoAddress
cryptoNetworkCreateAddressXRP (BRCryptoNetwork networkBase,
                                const char *addressAsString) {
    BRCryptoNetworkXRP network = (BRCryptoNetworkXRP) networkBase;

    return (BRChainParamsIsBitcoin (network->params)
            ? cryptoAddressCreateFromStringAsXRP (network->params->addrParams, addressAsString)
            : cryptoAddressCreateFromStringAsBCH (network->params->addrParams, addressAsString));
}


// MARK: Account Initialization

static BRCryptoBoolean
cryptoNetworkIsAccountInitializedXRP (BRCryptoNetwork network,
                                   BRCryptoAccount account) {
    BRGenericAccount genAccount = cryptoAccountAsGEN (account, network->canonicalType);
    assert (NULL != genAccount);
    return AS_CRYPTO_BOOLEAN (genAccountIsInitialized(genAccount));
}


static uint8_t *
cryptoNetworkGetAccountInitializationDataXRP (BRCryptoNetwork network,
                                           BRCryptoAccount account,
                                           size_t *bytesCount) {
    BRGenericAccount genAccount = cryptoAccountAsGEN (account, network->canonicalType);
    assert (NULL != genAccount);
    return genAccountGetInitializationData (genAccount, bytesCount);
}

static void
cryptoNetworkInitializeAccountXRP (BRCryptoNetwork network,
                                BRCryptoAccount account,
                                const uint8_t *bytes,
                                size_t bytesCount) {
    BRGenericAccount genAccount = cryptoAccountAsGEN (account, network->canonicalType);
    assert (NULL != genAccount);
    genAccountInitialize(genAccount, bytes, bytesCount);
}

BRCryptoNetworkHandlers cryptoNetworkHandlersXRP = {
    cyptoNetworkCreateXRP,
    cryptoNetworkReleaseXRP,
    cryptoNetworkCreateAddressXRP,
    cryptoNetworkIsAccountInitializedBTC,
    cryptoNetworkGetAccountInitializationDataBTC,
    cryptoNetworkInitializeAccountBTC
};

