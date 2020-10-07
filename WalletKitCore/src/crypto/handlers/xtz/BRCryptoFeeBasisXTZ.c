//
//  BRCryptoFeeBasisXTZ.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-09-04.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoXTZ.h"
#include "crypto/BRCryptoFeeBasisP.h"
#include "tezos/BRTezos.h"

private_extern BRCryptoFeeBasisXTZ
cryptoFeeBasisCoerceXTZ (BRCryptoFeeBasis feeBasis) {
    assert (CRYPTO_NETWORK_TYPE_XTZ == feeBasis->type);
    return (BRCryptoFeeBasisXTZ) feeBasis;
}

typedef struct {
    BRTezosFeeBasis xtzFeeBasis;
} BRCryptoFeeBasisCreateContextXTZ;

static void
cryptoFeeBasisCreateCallbackXTZ (BRCryptoFeeBasisCreateContext context,
                                 BRCryptoFeeBasis feeBasis) {
    BRCryptoFeeBasisCreateContextXTZ *contextXTZ = (BRCryptoFeeBasisCreateContextXTZ*) context;
    BRCryptoFeeBasisXTZ feeBasisXTZ = cryptoFeeBasisCoerceXTZ (feeBasis);
    
    feeBasisXTZ->xtzFeeBasis = contextXTZ->xtzFeeBasis;
}

private_extern BRCryptoFeeBasis
cryptoFeeBasisCreateAsXTZ (BRCryptoUnit unit,
                           BRTezosFeeBasis xtzFeeBasis) {
    BRCryptoFeeBasisCreateContextXTZ contextXTZ = {
        xtzFeeBasis
    };
    
    return cryptoFeeBasisAllocAndInit (sizeof (struct BRCryptoFeeBasisXTZRecord),
                                       CRYPTO_NETWORK_TYPE_XTZ,
                                       unit,
                                       &contextXTZ,
                                       cryptoFeeBasisCreateCallbackXTZ);
}

static void
cryptoFeeBasisReleaseXTZ (BRCryptoFeeBasis feeBasis) {
}

static double
cryptoFeeBasisGetCostFactorXTZ (BRCryptoFeeBasis feeBasis) {
    BRTezosFeeBasis xtzFeeBasis = cryptoFeeBasisCoerceXTZ (feeBasis)->xtzFeeBasis;
    switch (xtzFeeBasis.type) {
        case FEE_BASIS_ESTIMATE:
            return (double) xtzFeeBasis.u.estimate.sizeInBytes;
        case FEE_BASIS_ACTUAL:
            return 1.0;
    }
}

static BRCryptoAmount
cryptoFeeBasisGetPricePerCostFactorXTZ (BRCryptoFeeBasis feeBasis) {
    BRTezosFeeBasis xtzFeeBasis = cryptoFeeBasisCoerceXTZ (feeBasis)->xtzFeeBasis;
    switch (xtzFeeBasis.type) {
        case FEE_BASIS_ESTIMATE:
            return cryptoAmountCreateAsXTZ (feeBasis->unit, CRYPTO_FALSE, xtzFeeBasis.u.estimate.mutezPerByte);
        case FEE_BASIS_ACTUAL:
            return cryptoAmountCreateAsXTZ (feeBasis->unit, CRYPTO_FALSE, xtzFeeBasis.u.actual.fee);
    }
}

static BRCryptoAmount
cryptoFeeBasisGetFeeXTZ (BRCryptoFeeBasis feeBasis) {
    BRTezosFeeBasis xtzFeeBasis = cryptoFeeBasisCoerceXTZ (feeBasis)->xtzFeeBasis;
    BRTezosUnitMutez fee = tezosFeeBasisGetFee (&xtzFeeBasis);
    return cryptoAmountCreateAsXTZ (feeBasis->unit, CRYPTO_FALSE, fee);
}

static BRCryptoBoolean
cryptoFeeBasisIsEqualXTZ (BRCryptoFeeBasis feeBasis1, BRCryptoFeeBasis feeBasis2) {
    BRCryptoFeeBasisXTZ fb1 = cryptoFeeBasisCoerceXTZ (feeBasis1);
    BRCryptoFeeBasisXTZ fb2 = cryptoFeeBasisCoerceXTZ (feeBasis2);
    
    return AS_CRYPTO_BOOLEAN (tezosFeeBasisIsEqual (&fb1->xtzFeeBasis, &fb2->xtzFeeBasis));
}

// MARK: - Handlers

BRCryptoFeeBasisHandlers cryptoFeeBasisHandlersXTZ = {
    cryptoFeeBasisReleaseXTZ,
    cryptoFeeBasisGetCostFactorXTZ,
    cryptoFeeBasisGetPricePerCostFactorXTZ,
    cryptoFeeBasisGetFeeXTZ,
    cryptoFeeBasisIsEqualXTZ
};

