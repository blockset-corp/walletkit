//
//  BRCryptoFeeBasisHBAR.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-09-04.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoHBAR.h"
#include "crypto/BRCryptoFeeBasisP.h"
#include "hedera/BRHedera.h"

private_extern BRCryptoFeeBasisHBAR
cryptoFeeBasisCoerceHBAR (BRCryptoFeeBasis feeBasis) {
    assert (CRYPTO_NETWORK_TYPE_HBAR == feeBasis->type);
    return (BRCryptoFeeBasisHBAR) feeBasis;
}

typedef struct {
    BRHederaFeeBasis hbarFeeBasis;
} BRCryptoFeeBasisCreateContextHBAR;

static void
cryptoFeeBasisCreateCallbackHBAR (BRCryptoFeeBasisCreateContext context,
                                 BRCryptoFeeBasis feeBasis) {
    BRCryptoFeeBasisCreateContextHBAR *contextHBAR = (BRCryptoFeeBasisCreateContextHBAR*) context;
    BRCryptoFeeBasisHBAR feeBasisHBAR = cryptoFeeBasisCoerceHBAR (feeBasis);
    
    feeBasisHBAR->hbarFeeBasis = contextHBAR->hbarFeeBasis;
}

private_extern BRHederaFeeBasis
cryptoFeeBasisAsHBAR (BRCryptoFeeBasis feeBasis) {
    BRCryptoFeeBasisHBAR feeBasisHBAR = cryptoFeeBasisCoerceHBAR (feeBasis);
    return feeBasisHBAR->hbarFeeBasis;
}

private_extern BRCryptoFeeBasis
cryptoFeeBasisCreateAsHBAR (BRCryptoUnit unit,
                            BRHederaFeeBasis hbarFeeBasis) {
    BRCryptoFeeBasisCreateContextHBAR contextHBAR = {
        hbarFeeBasis
    };
    
    return cryptoFeeBasisAllocAndInit (sizeof (struct BRCryptoFeeBasisHBARRecord),
                                       CRYPTO_NETWORK_TYPE_HBAR,
                                       unit,
                                       &contextHBAR,
                                       cryptoFeeBasisCreateCallbackHBAR);
}

static void
cryptoFeeBasisReleaseHBAR (BRCryptoFeeBasis feeBasis) {
}

static double
cryptoFeeBasisGetCostFactorHBAR (BRCryptoFeeBasis feeBasis) {
    return (double) cryptoFeeBasisCoerceHBAR (feeBasis)->hbarFeeBasis.costFactor;
}

static BRCryptoAmount
cryptoFeeBasisGetPricePerCostFactorHBAR (BRCryptoFeeBasis feeBasis) {
    BRHederaFeeBasis hbarFeeBasis = cryptoFeeBasisCoerceHBAR (feeBasis)->hbarFeeBasis;
    return cryptoAmountCreateAsHBAR (feeBasis->unit, CRYPTO_FALSE, hbarFeeBasis.pricePerCostFactor);
}

static BRCryptoAmount
cryptoFeeBasisGetFeeHBAR (BRCryptoFeeBasis feeBasis) {
    return cryptoFeeBasisGetPricePerCostFactor (feeBasis);
}

static BRCryptoBoolean
cryptoFeeBasisIsEqualHBAR (BRCryptoFeeBasis feeBasis1, BRCryptoFeeBasis feeBasis2) {
    BRCryptoFeeBasisHBAR fb1 = cryptoFeeBasisCoerceHBAR (feeBasis1);
    BRCryptoFeeBasisHBAR fb2 = cryptoFeeBasisCoerceHBAR (feeBasis2);

    return hederaFeeBasisIsEqual (&fb1->hbarFeeBasis, &fb2->hbarFeeBasis);
}

// MARK: - Handlers

BRCryptoFeeBasisHandlers cryptoFeeBasisHandlersHBAR = {
    cryptoFeeBasisReleaseHBAR,
    cryptoFeeBasisGetCostFactorHBAR,
    cryptoFeeBasisGetPricePerCostFactorHBAR,
    cryptoFeeBasisGetFeeHBAR,
    cryptoFeeBasisIsEqualHBAR
};

