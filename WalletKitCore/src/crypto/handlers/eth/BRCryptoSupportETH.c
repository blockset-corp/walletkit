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

private_extern BRCryptoFeeBasis
cryptoFeeBasisCreateAsETH (BRCryptoUnit unit,
                           BREthereumFeeBasis feeBasis) {
    double costFactor = (double) feeBasis.u.gas.limit.amountOfGas;
    BRCryptoAmount pricePerCostFactor = cryptoAmountCreate (unit,
                                                            CRYPTO_FALSE,
                                                            feeBasis.u.gas.price.etherPerGas.valueInWEI);

    return cryptoFeeBasisCreate (pricePerCostFactor, costFactor);
}

private_extern BREthereumFeeBasis
cryptoFeeBasisAsETH (BRCryptoFeeBasis feeBasis) {
    double         costFactor         = cryptoFeeBasisGetCostFactor(feeBasis);
    BRCryptoAmount pricePerCostFactor = cryptoFeeBasisGetPricePerCostFactor(feeBasis);

    BREthereumGas ethGas = ethGasCreate ((uint64_t) lround (costFactor));
    BREthereumGasPrice ethGasPrice = ethGasPriceCreate (ethEtherCreate (cryptoAmountGetValue(pricePerCostFactor)));

    return (BREthereumFeeBasis) {
        FEE_BASIS_GAS,
        { .gas = { ethGas, ethGasPrice }}
    };
}


private_extern BRCryptoHash
cryptoHashCreateAsETH (BREthereumHash eth) {
    return cryptoHashCreateInternal ((uint32_t) ethHashSetValue (&eth),
                                     ETHEREUM_HASH_BYTES,
                                     eth.bytes);
}

private_extern BREthereumHash
cryptoHashAsETH (BRCryptoHash hash) {
    assert (ETHEREUM_HASH_BYTES == hash->bytesCount);
    BREthereumHash eth;
    memcpy (eth.bytes, hash->bytes, ETHEREUM_HASH_BYTES);
    return eth;
}

#ifdef REFACTOR
    extern char *
    cryptoHashString (BRCryptoHash hash) {
    switch (hash->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            UInt256 reversedHash = UInt256Reverse (hash->u.btc);
            return _cryptoHashAddPrefix (hexEncodeCreate(NULL, reversedHash.u8, sizeof(reversedHash.u8)), 1);
        }
        case BLOCK_CHAIN_TYPE_ETH: {
            return ethHashAsString (hash->u.eth);
        }
        case BLOCK_CHAIN_TYPE_GEN: {
            return _cryptoHashAddPrefix (genericHashAsString(hash->u.gen), 1);
        }
    }
}

extern int
cryptoHashGetHashValue (BRCryptoHash hash) {
switch (hash->type) {
        case BLOCK_CHAIN_TYPE_BTC:
            return (int) hash->u.btc.u32[0];

        case BLOCK_CHAIN_TYPE_ETH:
            return ethHashSetValue (&hash->u.eth);

        case BLOCK_CHAIN_TYPE_GEN:
            return (int) genericHashSetValue (hash->u.gen);
    }
}
#endif

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
