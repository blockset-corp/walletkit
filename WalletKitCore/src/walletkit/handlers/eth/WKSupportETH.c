//
//  WKSupportETH.c
//  WalletKitCore
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKETH.h"
#include "walletkit/WKHashP.h"
#include "walletkit/WKAmountP.h"

#include <math.h>


private_extern WKHash
wkHashCreateAsETH (BREthereumHash eth) {
    return wkHashCreateInternal ((uint32_t) ethHashSetValue (&eth),
                                     ETHEREUM_HASH_BYTES,
                                     eth.bytes,
                                     WK_NETWORK_TYPE_ETH);
}

private_extern BREthereumHash
wkHashAsETH (WKHash hash) {
    assert (ETHEREUM_HASH_BYTES == hash->bytesCount);
    BREthereumHash eth;
    memcpy (eth.bytes, hash->bytes, ETHEREUM_HASH_BYTES);
    return eth;
}

private_extern WKCurrency
wkNetworkGetCurrencyforTokenETH (WKNetwork network,
                                     BREthereumToken token) {
    WKCurrency tokenCurrency = NULL;
    pthread_mutex_lock (&network->lock);
    for (size_t index = 0; index < array_count(network->associations); index++) {
        WKCurrency currency = network->associations[index].currency;
        const char *address = wkCurrencyGetIssuer (currency);

        if (NULL != address && ETHEREUM_BOOLEAN_IS_TRUE (ethTokenHasAddress (token, address))) {
            tokenCurrency = wkCurrencyTake (currency);
            break;
        }
    }
    pthread_mutex_unlock (&network->lock);
    return tokenCurrency;
}
