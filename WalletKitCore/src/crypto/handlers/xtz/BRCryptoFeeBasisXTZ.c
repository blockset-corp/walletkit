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
    return (double) cryptoFeeBasisCoerceXTZ (feeBasis)->xtzFeeBasis.gasLimit;
}

static BRCryptoAmount
cryptoFeeBasisGetPricePerCostFactorXTZ (BRCryptoFeeBasis feeBasis) {
    BRTezosFeeBasis xtzFeeBasis = cryptoFeeBasisCoerceXTZ (feeBasis)->xtzFeeBasis;
    return cryptoAmountCreateAsXTZ (feeBasis->unit, CRYPTO_FALSE, xtzFeeBasis.fee);
}

static BRCryptoAmount
cryptoFeeBasisGetFeeXTZ (BRCryptoFeeBasis feeBasis) {
    return cryptoFeeBasisGetPricePerCostFactor (feeBasis);
}

static BRCryptoBoolean
cryptoFeeBasisIsEqualXTZ (BRCryptoFeeBasis feeBasis1, BRCryptoFeeBasis feeBasis2) {
    BRCryptoFeeBasisXTZ fb1 = cryptoFeeBasisCoerceXTZ (feeBasis1);
    BRCryptoFeeBasisXTZ fb2 = cryptoFeeBasisCoerceXTZ (feeBasis2);
    
    return tezosFeeBasisIsEqual (&fb1->xtzFeeBasis, &fb2->xtzFeeBasis);
}

// MARK: - Handlers

BRCryptoFeeBasisHandlers cryptoFeeBasisHandlersXTZ = {
    cryptoFeeBasisReleaseXTZ,
    cryptoFeeBasisGetCostFactorXTZ,
    cryptoFeeBasisGetPricePerCostFactorXTZ,
    cryptoFeeBasisGetFeeXTZ,
    cryptoFeeBasisIsEqualXTZ
};

