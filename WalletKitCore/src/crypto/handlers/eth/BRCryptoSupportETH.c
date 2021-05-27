//
//  BRCryptoSupportETH.c
//  Core
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoETH.h"
#include "crypto/BRCryptoHashP.h"
#include "crypto/BRCryptoAmountP.h"

#include <math.h>


private_extern BRCryptoHash
cryptoHashCreateAsETH (BREthereumHash eth) {
    return cryptoHashCreateInternal ((uint32_t) ethHashSetValue (&eth),
                                     ETHEREUM_HASH_BYTES,
                                     eth.bytes,
                                     CRYPTO_NETWORK_TYPE_ETH);
}

private_extern BREthereumHash
cryptoHashAsETH (BRCryptoHash hash) {
    assert (ETHEREUM_HASH_BYTES == hash->bytesCount);
    BREthereumHash eth;
    memcpy (eth.bytes, hash->bytes, ETHEREUM_HASH_BYTES);
    return eth;
}

private_extern BRCryptoCurrency
cryptoNetworkGetCurrencyforTokenETH (BRCryptoNetwork network,
                                     BREthereumToken token) {
    BRCryptoCurrency tokenCurrency = NULL;
    pthread_mutex_lock (&network->lock);
    for (size_t index = 0; index < array_count(network->associations); index++) {
        BRCryptoCurrency currency = network->associations[index].currency;
        const char *address = cryptoCurrencyGetIssuer (currency);

        if (NULL != address && ETHEREUM_BOOLEAN_IS_TRUE (ethTokenHasAddress (token, address))) {
            tokenCurrency = cryptoCurrencyTake (currency);
            break;
        }
    }
    pthread_mutex_unlock (&network->lock);
    return tokenCurrency;
}
