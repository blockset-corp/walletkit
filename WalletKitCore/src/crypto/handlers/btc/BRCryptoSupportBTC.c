//
//  BRCryptoSupportBTC.c
//  Core
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRCryptoBTC.h"
#include "crypto/BRCryptoHashP.h"
#include "crypto/BRCryptoAmountP.h"
#include "ethereum/util/BRUtilMath.h"

private_extern BRCryptoFeeBasis
cryptoFeeBasisCreateAsBTC (BRCryptoUnit unit,
                           uint32_t feePerKB,
                           uint32_t sizeInByte) {
    return cryptoFeeBasisCreate (cryptoAmountCreate (unit,
                                                     CRYPTO_FALSE,
                                                     uint256Create(feePerKB)),
                                 sizeInByte / 1000.0);
}

private_extern uint64_t // SAT-per-KB
cryptoFeeBasisAsBTC (BRCryptoFeeBasis feeBasis) {
#ifdef REFACTOR
    assert (BLOCK_CHAIN_TYPE_BTC == feeBasis->type);
#endif
    BRCryptoBoolean overflow;
    BRCryptoAmount costPerPriceFactor = cryptoFeeBasisGetPricePerCostFactor(feeBasis);
    uint64_t satPerKb = cryptoAmountGetIntegerRaw(costPerPriceFactor, &overflow);

    assert (CRYPTO_FALSE == overflow);
    return satPerKb;
}

private_extern BRCryptoHash
cryptoHashCreateAsBTC (UInt256 btc) {
    UInt256 revBtc = UInt256Reverse (btc);

    return cryptoHashCreateInternal (btc.u32[0],
                                     sizeof (revBtc.u8),
                                     revBtc.u8);
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

