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
#include "BRCryptoHandlersP.h"
#include "ethereum/util/BRUtilMath.h"

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoFeeBasis, cryptoFeeBasis)

extern BRCryptoFeeBasis
cryptoFeeBasisAllocAndInit (size_t sizeInBytes,
                            BRCryptoBlockChainType type,
                            BRCryptoUnit unit,
                            BRCryptoFeeBasisCreateContext  createContext,
                            BRCryptoFeeBasisCreateCallback createCallback) {
    assert (sizeInBytes >= sizeof (struct BRCryptoFeeBasisRecord));
    BRCryptoFeeBasis feeBasis = calloc (1, sizeInBytes);
    
    feeBasis->type = type;
    feeBasis->handlers = cryptoHandlersLookup (type)->feeBasis;
    feeBasis->sizeInBytes = sizeInBytes;
    feeBasis->ref  = CRYPTO_REF_ASSIGN (cryptoFeeBasisRelease);
    
    feeBasis->unit = cryptoUnitTake (unit);
    
    if (NULL != createCallback) createCallback (createContext, feeBasis);

    return feeBasis;
}

static void
cryptoFeeBasisRelease (BRCryptoFeeBasis feeBasis) {
    feeBasis->handlers->release (feeBasis);
    
    cryptoUnitGive (feeBasis->unit);
    
    memset (feeBasis, 0, feeBasis->sizeInBytes);
    free (feeBasis);
}

private_extern BRCryptoBlockChainType
cryptoFeeBasisGetType (BRCryptoFeeBasis feeBasis) {
    return feeBasis->type;
}

extern BRCryptoAmount
cryptoFeeBasisGetPricePerCostFactor (BRCryptoFeeBasis feeBasis) {
    return feeBasis->handlers->getPricePerCostFactor (feeBasis);
}

extern double
cryptoFeeBasisGetCostFactor (BRCryptoFeeBasis feeBasis) {
    return feeBasis->handlers->getCostFactor (feeBasis);
}

extern BRCryptoAmount
cryptoFeeBasisGetFee (BRCryptoFeeBasis feeBasis) {
    return feeBasis->handlers->getFee (feeBasis);
}

extern BRCryptoBoolean
cryptoFeeBasisIsEqual (BRCryptoFeeBasis feeBasis1,
                       BRCryptoFeeBasis feeBasis2) {
    return AS_CRYPTO_BOOLEAN (feeBasis1 == feeBasis2 ||
                              (feeBasis1->type == feeBasis2->type &&
                               feeBasis1->handlers->isEqual (feeBasis1, feeBasis2)));
}
