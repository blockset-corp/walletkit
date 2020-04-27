//
//  BRCryptoAddress.c
//  BRCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoAddressP.h"
#include "BRCryptoNetworkP.h"
#include "BRCryptoGenericP.h"

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoAddress, cryptoAddress);

private_extern BRCryptoAddress
cryptoAddressAllocAndInit (size_t sizeInBytes,
                          BRCryptoBlockChainType type) {
    assert (sizeInBytes >= sizeof (struct BRCryptoAddressRecord));

    BRCryptoAddress address = calloc (1, sizeInBytes);

    address->type = type;
    address->handlers = cryptoGenericHandlersLookup(type)->address;
    address->ref = CRYPTO_REF_ASSIGN(cryptoAddressRelease);
    address->sizeInBytes = sizeInBytes;

    return address;
}

static void
cryptoAddressRelease (BRCryptoAddress address) {
    address->handlers->release (address);
    memset (address, 0, address->sizeInBytes);
    free (address);
}


private_extern BRCryptoBlockChainType
cryptoAddressGetType (BRCryptoAddress address) {
    return address->type;
}

//extern BRCryptoAddress
//cryptoAddressCreateFromString (BRCryptoNetwork network,
//                               const char *string) {
//    switch (cryptoNetworkGetType(network)) {
//        case BLOCK_CHAIN_TYPE_BTC: {
//            const BRChainParams *params = cryptoNetworkAsBTC (network);
//            return (BRChainParamsIsBitcoin (params)
//                    ? cryptoAddressCreateFromStringAsBTC (params->addrParams, string)
//                    : cryptoAddressCreateFromStringAsBCH (params->addrParams, string));
//        }
//        case BLOCK_CHAIN_TYPE_ETH:
//            return cryptoAddressCreateFromStringAsETH (string);
//
//        case BLOCK_CHAIN_TYPE_GEN: {
//            BRGenericNetwork gen = cryptoNetworkAsGEN (network);
//            return cryptoAddressCreateFromStringAsGEN (gen, string);
//        }
//    }
//}

extern char *
cryptoAddressAsString (BRCryptoAddress address) {
    return address->handlers->asString (address);
}

extern BRCryptoBoolean
cryptoAddressIsIdentical (BRCryptoAddress a1,
                          BRCryptoAddress a2) {
    return (a1 == a2 ||
            (a1->type == a2->type &&
             a1->handlers->isEqual (a1, a2)));
}
