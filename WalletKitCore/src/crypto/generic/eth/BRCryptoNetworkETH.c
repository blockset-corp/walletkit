//
//  BRCryptoNetworkETH.c
//  
//
//  Created by Ed Gamble on 4/24/20.
//

#include "BRCryptoETH.h"
#include "ethereum/blockchain/BREthereumNetwork.h"

struct BRCryptoNetworkETHRecord {
    struct BRCryptoNetworkRecord base;

    BREthereumNetwork eth;
};

static BRCryptoNetworkBTC
cryptoNetworkCoerce (BRCryptoNetwork network) {
    assert (CRYPTO_NETWORK_TYPE_ETH == network->type);
    return (BRCryptoNetworkETH) network;
}

static BRCryptoNetwork
cryptoNetworkCreateAsETH (const char *uids,
                          const char *name,
                          bool isMainnet,
                          BREthereumNetwork eth) {
    BRCryptoNetwork networkBase = cryptoNetworkAllocAndInit (sizeof (struct BRCryptoNetworkBTCRecord),
                                                             CRYPTO_NETWORK_TYPE_ETH,
                                                             uids,
                                                             name,
                                                             isMainnet);
    BRCryptoNetworkETH network = cryptoNetworkCoerce(networkBase);
    network->eth = eth;

    return networkBase;
}

static BRCryptoNetwork
cyptoNetworkCreateETH (const char *uids,
                       const char *name,
                       const char *network,
                       bool isMainnet) {
    if      (0 == strcmp ("mainnet", network))
        return cryptoNetworkCreateAsETH (uids, name, true, ethNetworkMainnet);
    else if (0 == strcmp ("testnet", network))
        return cryptoNetworkCreateAsETH (uids, name, false, ethNetworkTestnet);
    else if (0 == strcmp ("rinkeby", network))
        return cryptoNetworkCreateAsETH (uids, name, false, ethNetworkRinkeby);
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

BRCryptoNetworkHandlers cryptoNetworkHandlersETH = {
    cryptoNetworkReleaseETH,
    cryptoNetworkCreateAddressETH,
    cryptoNetworkIsAccountInitializedETH,
    cryptoNetworkGetAccountInitializationDataETH,
    cryptoNetworkInitializeAccountETH
};
