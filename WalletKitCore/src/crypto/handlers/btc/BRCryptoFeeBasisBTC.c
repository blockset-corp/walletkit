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
    uint64_t feePerKB;
    uint32_t sizeInByte;
} BRCryptoFeeBasisCreateContextBTC;

static void
cryptoFeeBasisCreateCallbackBTC (BRCryptoFeeBasisCreateContext context,
                                 BRCryptoFeeBasis feeBasis) {
    BRCryptoFeeBasisCreateContextBTC *contextBTC = (BRCryptoFeeBasisCreateContextBTC*) context;
    BRCryptoFeeBasisBTC feeBasisBTC = cryptoFeeBasisCoerce (feeBasis);
    
    feeBasisBTC->feePerKB = contextBTC->feePerKB;
    feeBasisBTC->sizeInByte = contextBTC->sizeInByte;
}

private_extern BRCryptoFeeBasis
cryptoFeeBasisCreateAsBTC (BRCryptoUnit unit,
                           uint64_t feePerKB,
                           uint32_t sizeInByte) {
    BRCryptoFeeBasisCreateContextBTC contextBTC = {
        feePerKB,
        sizeInByte
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

static double
cryptoFeeBasisGetCostFactorBTC (BRCryptoFeeBasis feeBasis) {
    BRCryptoFeeBasisBTC btcFeeBasis = cryptoFeeBasisCoerce (feeBasis);
    return ((double) btcFeeBasis->sizeInByte) / 1000.0;
}

static BRCryptoAmount
cryptoFeeBasisGetPricePerCostFactorBTC (BRCryptoFeeBasis feeBasis) {
    BRCryptoFeeBasisBTC btcFeeBasis = cryptoFeeBasisCoerce (feeBasis);
    return cryptoAmountCreate (feeBasis->unit,
                               CRYPTO_FALSE,
                               uint256Create(btcFeeBasis->feePerKB));
}

static BRCryptoAmount
cryptoFeeBasisGetFeeBTC (BRCryptoFeeBasis feeBasis) {
    BRCryptoFeeBasisBTC btcFeeBasis = cryptoFeeBasisCoerce (feeBasis);
    double fee = (((double) btcFeeBasis->feePerKB) * btcFeeBasis->sizeInByte) / 1000.0;
    return cryptoAmountCreate (feeBasis->unit,
                               CRYPTO_FALSE,
                               uint256Create ((uint64_t) lround (fee)));
}

static BRCryptoBoolean
cryptoFeeBasisIsEqualBTC (BRCryptoFeeBasis feeBasis1, BRCryptoFeeBasis feeBasis2) {
    BRCryptoFeeBasisBTC fb1 = cryptoFeeBasisCoerce (feeBasis1);
    BRCryptoFeeBasisBTC fb2 = cryptoFeeBasisCoerce (feeBasis2);

    return (fb1->feePerKB == fb2->feePerKB &&
            fb1->sizeInByte == fb2->sizeInByte);
}

// MARK: - Handlers

BRCryptoFeeBasisHandlers cryptoFeeBasisHandlersBTC = {
    cryptoFeeBasisReleaseBTC,
    cryptoFeeBasisGetCostFactorBTC,
    cryptoFeeBasisGetPricePerCostFactorBTC,
    cryptoFeeBasisGetFeeBTC,
    cryptoFeeBasisIsEqualBTC
};
