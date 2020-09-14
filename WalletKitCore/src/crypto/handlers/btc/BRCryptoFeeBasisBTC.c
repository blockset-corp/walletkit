//
//  BRCryptoFeeBasisBTC.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-09-04.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoBTC.h"
#include "crypto/BRCryptoFeeBasisP.h"
#include "crypto/BRCryptoAmountP.h"
#include "ethereum/util/BRUtil.h"

#include <math.h>

static BRCryptoFeeBasisBTC
cryptoFeeBasisCoerce (BRCryptoFeeBasis feeBasis) {
    assert (CRYPTO_NETWORK_TYPE_BTC == feeBasis->type);
    return (BRCryptoFeeBasisBTC) feeBasis;
}

typedef struct {
    uint64_t fee;
    uint64_t feePerKB;
    double   sizeInKB;
} BRCryptoFeeBasisCreateContextBTC;

static void
cryptoFeeBasisCreateCallbackBTC (BRCryptoFeeBasisCreateContext context,
                                 BRCryptoFeeBasis feeBasis) {
    BRCryptoFeeBasisCreateContextBTC *contextBTC = (BRCryptoFeeBasisCreateContextBTC*) context;
    BRCryptoFeeBasisBTC feeBasisBTC = cryptoFeeBasisCoerce (feeBasis);

    feeBasisBTC->fee = contextBTC->fee;
    feeBasisBTC->feePerKB = contextBTC->feePerKB;
    feeBasisBTC->sizeInKB = contextBTC->sizeInKB;
}

private_extern BRCryptoFeeBasis
cryptoFeeBasisCreateAsBTC (BRCryptoUnit unit,
                           uint64_t fee,
                           uint64_t feePerKB,
                           uint32_t sizeInByte) {
    // Only one of these can be unknownn
    assert (((fee        == CRYPTO_FEE_BASIS_BTC_FEE_UNKNOWN        ? 1 : 0) +
             (feePerKB   == CRYPTO_FEE_BASIS_BTC_FEE_PER_KB_UNKNOWN ? 1 : 0) +
             (sizeInByte == CRYPTO_FEE_BASIS_BTC_SIZE_UNKNOWN       ? 1 : 0)) <= 1);

    BRCryptoFeeBasisCreateContextBTC contextBTC = {
        // Fee
        (fee == CRYPTO_FEE_BASIS_BTC_FEE_UNKNOWN
         ? (uint64_t) lround (((double) feePerKB) * sizeInByte / 1000.0)
         : fee),

        // Fee-Per-KB
        (feePerKB == CRYPTO_FEE_BASIS_BTC_FEE_PER_KB_UNKNOWN
         ? (((1000 * fee) + (sizeInByte/2)) / sizeInByte)
         : feePerKB),

        // Size-In-KB
        (sizeInByte == CRYPTO_FEE_BASIS_BTC_SIZE_UNKNOWN
         ? (((double) fee) / feePerKB)
         : (((double) sizeInByte) / 1000))
    };
    
    return cryptoFeeBasisAllocAndInit (sizeof (struct BRCryptoFeeBasisBTCRecord),
                                       CRYPTO_NETWORK_TYPE_BTC,
                                       unit,
                                       &contextBTC,
                                       cryptoFeeBasisCreateCallbackBTC);
}

private_extern uint64_t // SAT-per-KB
cryptoFeeBasisAsBTC (BRCryptoFeeBasis feeBasis) {
    BRCryptoFeeBasisBTC btcFeeBasis = cryptoFeeBasisCoerce (feeBasis);
    return btcFeeBasis->feePerKB;
}

static void
cryptoFeeBasisReleaseBTC (BRCryptoFeeBasis feeBasis) {
}

static double // as sizeInKB
cryptoFeeBasisGetCostFactorBTC (BRCryptoFeeBasis feeBasis) {
    BRCryptoFeeBasisBTC btcFeeBasis = cryptoFeeBasisCoerce (feeBasis);
    return btcFeeBasis->sizeInKB;
}

static BRCryptoAmount
cryptoFeeBasisGetPricePerCostFactorBTC (BRCryptoFeeBasis feeBasis) {
    return cryptoAmountCreate (feeBasis->unit,
                               CRYPTO_FALSE,
                               uint256Create (cryptoFeeBasisAsBTC (feeBasis)));
}

static BRCryptoAmount
cryptoFeeBasisGetFeeBTC (BRCryptoFeeBasis feeBasis) {
    BRCryptoFeeBasisBTC btcFeeBasis = cryptoFeeBasisCoerce (feeBasis);
    return cryptoAmountCreate (feeBasis->unit,
                               CRYPTO_FALSE,
                               uint256Create (btcFeeBasis->fee));
}

static BRCryptoBoolean
cryptoFeeBasisIsEqualBTC (BRCryptoFeeBasis feeBasis1, BRCryptoFeeBasis feeBasis2) {
    BRCryptoFeeBasisBTC fb1 = cryptoFeeBasisCoerce (feeBasis1);
    BRCryptoFeeBasisBTC fb2 = cryptoFeeBasisCoerce (feeBasis2);

    return (fb1->fee == fb2->fee &&
            fb1->feePerKB == fb2->feePerKB &&
            lround (1000.0 * fb1->sizeInKB) == lround (1000.0 * fb2->sizeInKB));
}

// MARK: - Handlers

BRCryptoFeeBasisHandlers cryptoFeeBasisHandlersBTC = {
    cryptoFeeBasisReleaseBTC,
    cryptoFeeBasisGetCostFactorBTC,
    cryptoFeeBasisGetPricePerCostFactorBTC,
    cryptoFeeBasisGetFeeBTC,
    cryptoFeeBasisIsEqualBTC
};
