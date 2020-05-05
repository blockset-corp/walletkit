//
//  BRCryptoFeeBasis.c
//  Core
//
//  Created by Ed Gamble on 5/15/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <math.h>

#include "BRCryptoFeeBasisP.h"
#include "BRCryptoAmountP.h"
#include "ethereum/util/BRUtilMath.h"

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoFeeBasis, cryptoFeeBasis)

extern BRCryptoFeeBasis
cryptoFeeBasisCreate (BRCryptoAmount pricePerCostFactor,
                      double costFactor) {
    assert (costFactor >= 0.0);

    BRCryptoFeeBasis feeBasis = malloc (sizeof (struct BRCryptoFeeBasisRecord));

    feeBasis->pricePerCostFactor = cryptoAmountTake (pricePerCostFactor);
    feeBasis->costFactor =costFactor;
    feeBasis->ref  = CRYPTO_REF_ASSIGN (cryptoFeeBasisRelease);

    return feeBasis;
}

static void
cryptoFeeBasisRelease (BRCryptoFeeBasis feeBasis) {
    cryptoAmountGive (feeBasis->pricePerCostFactor);
    memset (feeBasis, 0, sizeof(*feeBasis));
    free (feeBasis);
}


extern BRCryptoAmount
cryptoFeeBasisGetPricePerCostFactor (BRCryptoFeeBasis feeBasis) {
    return cryptoAmountTake (feeBasis->pricePerCostFactor);
}

extern double
cryptoFeeBasisGetCostFactor (BRCryptoFeeBasis feeBasis) {
    return feeBasis->costFactor;
}

extern BRCryptoAmount
cryptoFeeBasisGetFee (BRCryptoFeeBasis feeBasis) {
    int overflow, negative;
    double rem;

    UInt256 feeValue = uint256Mul_Double (cryptoAmountGetValue(feeBasis->pricePerCostFactor),
                                          feeBasis->costFactor,
                                          &overflow,
                                          &negative,
                                          &rem);

    assert (!overflow); assert (!negative);

    BRCryptoUnit   unit = cryptoAmountGetUnit (feeBasis->pricePerCostFactor);
    BRCryptoAmount fee  = cryptoAmountCreate (unit, CRYPTO_FALSE, feeValue);
    cryptoUnitGive(unit);

    return fee;
}

extern BRCryptoBoolean
cryptoFeeBasisIsEqual (BRCryptoFeeBasis feeBasis1,
                       BRCryptoFeeBasis feeBasis2) {
    return AS_CRYPTO_BOOLEAN (feeBasis1 == feeBasis2 ||
                              (CRYPTO_COMPARE_EQ == cryptoAmountCompare (feeBasis1->pricePerCostFactor, feeBasis2->pricePerCostFactor) &&
                               feeBasis1->costFactor == feeBasis2->costFactor));
}

extern BRCryptoUnit
cryptoFeeBasisGetPricePerCostFactorUnit (BRCryptoFeeBasis feeBasis) {
    return cryptoAmountGetUnit (feeBasis->pricePerCostFactor);
}

#ifdef REFACTOR

static UInt256
cryptoFeeBasisGetPricePerCostFactorAsUInt256 (BRCryptoFeeBasis feeBasis) {
    return cryptoAmountGetValue (feeBasis->pricePerCostFactor);
}


extern BRCryptoBoolean
cryptoFeeBasisIsIdentical (BRCryptoFeeBasis feeBasis1,
                           BRCryptoFeeBasis feeBasis2) {
    if (feeBasis1 == feeBasis2) return CRYPTO_TRUE;
    if (feeBasis1->type != feeBasis2->type) return CRYPTO_FALSE;
    if (CRYPTO_FALSE == cryptoUnitIsCompatible (feeBasis1->unit, feeBasis2->unit)) return CRYPTO_FALSE;

    switch (feeBasis1->type) {
        case BLOCK_CHAIN_TYPE_BTC:
            return AS_CRYPTO_BOOLEAN (feeBasis1->u.btc.feePerKB   == feeBasis2->u.btc.feePerKB &&
                                      feeBasis1->u.btc.sizeInByte == feeBasis2->u.btc.sizeInByte);

        case BLOCK_CHAIN_TYPE_ETH:
            return AS_CRYPTO_BOOLEAN (ETHEREUM_BOOLEAN_IS_TRUE (ethFeeBasisEqual (&feeBasis1->u.eth, &feeBasis2->u.eth)));

        case BLOCK_CHAIN_TYPE_GEN:
            return AS_CRYPTO_BOOLEAN (genFeeBasisIsEqual (&feeBasis1->u.gen, &feeBasis2->u.gen));
    }
}

static BRCryptoFeeBasis
cryptoFeeBasisCreateInternal (BRCryptoBlockChainType type,
                              BRCryptoUnit unit) {
    BRCryptoFeeBasis feeBasis = malloc (sizeof (struct BRCryptoFeeBasisRecord));

    feeBasis->unit = cryptoUnitTake (unit);
    feeBasis->ref  = CRYPTO_REF_ASSIGN (cryptoFeeBasisRelease);

    return feeBasis;
}

extern BRCryptoAmount
cryptoFeeBasisGetPricePerCostFactor (BRCryptoFeeBasis feeBasis) {
    return cryptoAmountCreateInternal (feeBasis->unit,
                                       CRYPTO_FALSE,
                                       cryptoFeeBasisGetPricePerCostFactorAsUInt256 (feeBasis),
                                       1);
}

extern double
cryptoFeeBasisGetCostFactor (BRCryptoFeeBasis feeBasis) {
    switch (feeBasis->type) {
        case BLOCK_CHAIN_TYPE_BTC:
            return ((double) feeBasis->u.btc.sizeInByte) / 1000.0;

        case BLOCK_CHAIN_TYPE_ETH:
            return feeBasis->u.eth.u.gas.limit.amountOfGas;

        case BLOCK_CHAIN_TYPE_GEN:
            return genFeeBasisGetCostFactor (&feeBasis->u.gen);
    }
}


private_extern BRCryptoFeeBasis
cryptoFeeBasisCreateAsETH (BRCryptoUnit unit,
                           BREthereumGas gas,
                           BREthereumGasPrice gasPrice) {
    BRCryptoFeeBasis feeBasis = cryptoFeeBasisCreateInternal (BLOCK_CHAIN_TYPE_ETH, unit);
    feeBasis->u.eth = (BREthereumFeeBasis) {
        FEE_BASIS_GAS,
        { .gas = { gas, gasPrice }}
    };

    return feeBasis;
}

private_extern BRCryptoFeeBasis
cryptoFeeBasisCreateAsGEN (BRCryptoUnit unit,
                           OwnershipGiven BRGenericFeeBasis bid) {
    BRCryptoFeeBasis feeBasis = cryptoFeeBasisCreateInternal (BLOCK_CHAIN_TYPE_GEN, unit);
    feeBasis->u.gen = bid;

    return feeBasis;
}


private_extern uint64_t // SAT-per-KB
cryptoFeeBasisAsBTC (BRCryptoFeeBasis feeBasis) {
    assert (BLOCK_CHAIN_TYPE_BTC == feeBasis->type);
    return feeBasis->u.btc.feePerKB;
}

private_extern BREthereumFeeBasis
cryptoFeeBasisAsETH (BRCryptoFeeBasis feeBasis) {
    assert (BLOCK_CHAIN_TYPE_ETH == feeBasis->type);
    return feeBasis->u.eth;
}

private_extern BRGenericFeeBasis
cryptoFeeBasisAsGEN (BRCryptoFeeBasis feeBasis) {
    assert (BLOCK_CHAIN_TYPE_GEN == feeBasis->type);
    return feeBasis->u.gen;
}
#endif
