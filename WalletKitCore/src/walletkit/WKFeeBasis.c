//
//  WKFeeBasis.c
//  WalletKitCore
//
//  Created by Ed Gamble on 5/15/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <math.h>

#include "WKFeeBasisP.h"
#include "WKAmountP.h"
#include "WKHandlersP.h"
#include "support/util/BRUtilMath.h"

IMPLEMENT_WK_GIVE_TAKE (WKFeeBasis, wkFeeBasis)

extern WKFeeBasis
wkFeeBasisAllocAndInit (size_t sizeInBytes,
                            WKNetworkType type,
                            WKUnit unit,
                            WKFeeBasisCreateContext  createContext,
                            WKFeeBasisCreateCallback createCallback) {
    assert (sizeInBytes >= sizeof (struct WKFeeBasisRecord));
    WKFeeBasis feeBasis = calloc (1, sizeInBytes);
    
    feeBasis->type = type;
    feeBasis->handlers = wkHandlersLookup (type)->feeBasis;
    feeBasis->sizeInBytes = sizeInBytes;
    feeBasis->ref  = WK_REF_ASSIGN (wkFeeBasisRelease);
    
    feeBasis->unit = wkUnitTake (unit);
    
    if (NULL != createCallback) createCallback (createContext, feeBasis);

    return feeBasis;
}

static void
wkFeeBasisRelease (WKFeeBasis feeBasis) {
    feeBasis->handlers->release (feeBasis);
    
    wkUnitGive (feeBasis->unit);
    
    memset (feeBasis, 0, feeBasis->sizeInBytes);
    free (feeBasis);
}

private_extern WKNetworkType
wkFeeBasisGetType (WKFeeBasis feeBasis) {
    return feeBasis->type;
}

extern WKAmount
wkFeeBasisGetPricePerCostFactor (WKFeeBasis feeBasis) {
    return feeBasis->handlers->getPricePerCostFactor (feeBasis);
}

extern double
wkFeeBasisGetCostFactor (WKFeeBasis feeBasis) {
    return feeBasis->handlers->getCostFactor (feeBasis);
}

extern WKAmount
wkFeeBasisGetFee (WKFeeBasis feeBasis) {
    return feeBasis->handlers->getFee (feeBasis);
}

extern WKBoolean
wkFeeBasisIsEqual (WKFeeBasis feeBasis1,
                       WKFeeBasis feeBasis2) {
    return AS_WK_BOOLEAN (feeBasis1 == feeBasis2 ||
                              (feeBasis1->type == feeBasis2->type &&
                               feeBasis1->handlers->isEqual (feeBasis1, feeBasis2)));
}
