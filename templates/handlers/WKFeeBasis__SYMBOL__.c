//
//  WKFeeBasis__SYMBOL__.c
//  WalletKitCore
//
//  Created by __USER__ on __DATE__.
//  Copyright © __YEAR__ Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WK__SYMBOL__.h"
#include "walletkit/WKFeeBasisP.h"
#include "__name__/BR__Name__.h"

typedef struct {
    BR__Name__FeeBasis __symbol__FeeBasis;
} WKFeeBasisCreateContext__SYMBOL__;

static void
wkFeeBasisCreateCallback__SYMBOL__ (WKFeeBasisCreateContext context,
                                    WKFeeBasis feeBasis) {
    WKFeeBasisCreateContext__SYMBOL__ *context__SYMBOL__ = (WKFeeBasisCreateContext__SYMBOL__*) context;
    WKFeeBasis__SYMBOL__ feeBasis__SYMBOL__ = wkFeeBasisCoerce__SYMBOL__ (feeBasis);
    
    feeBasis__SYMBOL__->__symbol__FeeBasis = context__SYMBOL__->__symbol__FeeBasis;
}

private_extern WKFeeBasis
wkFeeBasisCreateAs__SYMBOL__ (WKUnit unit,
                              BR__Name__FeeBasis __symbol__FeeBasis) {
    WKFeeBasisCreateContext__SYMBOL__ context__SYMBOL__ = {
        __symbol__FeeBasis
    };
    
    return wkFeeBasisAllocAndInit (sizeof (struct WKFeeBasis__SYMBOL__Record),
                                   WK_NETWORK_TYPE___SYMBOL__,
                                   unit,
                                   &context__SYMBOL__,
                                   wkFeeBasisCreateCallback__SYMBOL__);
}

static void
wkFeeBasisRelease__SYMBOL__ (WKFeeBasis feeBasis) {
    // If BR__Name__FeeBasis is a value type, nothing needed
}

static double
wkFeeBasisGetCostFactor__SYMBOL__ (WKFeeBasis feeBasis) {
    BR__Name__FeeBasis __symbol__FeeBasis = wkFeeBasisCoerce__SYMBOL__ (feeBasis)->__symbol__FeeBasis;

    ASSERT_UNIMPLEMENTED; (void) __symbol__FeeBasis;
    return 1.0;
#if 0
    switch (__symbol__FeeBasis.type) {
        case FEE_BASIS_INITIAL:
            return (double) __symbol__FeeBasis.u.initial.sizeInKBytes;
        case FEE_BASIS_ESTIMATE:
            return 1.0;
        case FEE_BASIS_ACTUAL:
            return 1.0;
    }
#endif
}

static WKAmount
wkFeeBasisGetPricePerCostFactor__SYMBOL__ (WKFeeBasis feeBasis) {
    BR__Name__FeeBasis __symbol__FeeBasis = wkFeeBasisCoerce__SYMBOL__ (feeBasis)->__symbol__FeeBasis;

    ASSERT_UNIMPLEMENTED;  (void) __symbol__FeeBasis;
    return wkAmountCreateAs__SYMBOL__ (feeBasis->unit, WK_FALSE, 1);
#if 0
    switch (__symbol__FeeBasis.type) {
        case FEE_BASIS_INITIAL:
            return wkAmountCreateAs__SYMBOL__ (feeBasis->unit, WK_FALSE, __symbol__FeeBasis.u.initial.mutezPerKByte);
        case FEE_BASIS_ESTIMATE:
            return wkAmountCreateAs__SYMBOL__ (feeBasis->unit, WK_FALSE, __symbol__FeeBasis.u.estimate.calculatedFee);
        case FEE_BASIS_ACTUAL:
            return wkAmountCreateAs__SYMBOL__ (feeBasis->unit, WK_FALSE, __symbol__FeeBasis.u.actual.fee);
    }
#endif
}

static WKAmount
wkFeeBasisGetFee__SYMBOL__ (WKFeeBasis feeBasis) {
    BR__Name__FeeBasis __symbol__FeeBasis = wkFeeBasisCoerce__SYMBOL__ (feeBasis)->__symbol__FeeBasis;
    BR__Name__Amount fee = __name__FeeBasisGetFee (&__symbol__FeeBasis);
    return wkAmountCreateAs__SYMBOL__ (feeBasis->unit, WK_FALSE, fee);
}

static WKBoolean
wkFeeBasisIsEqual__SYMBOL__ (WKFeeBasis feeBasis1, WKFeeBasis feeBasis2) {
    WKFeeBasis__SYMBOL__ fb1 = wkFeeBasisCoerce__SYMBOL__ (feeBasis1);
    WKFeeBasis__SYMBOL__ fb2 = wkFeeBasisCoerce__SYMBOL__ (feeBasis2);
    
    return AS_WK_BOOLEAN (__name__FeeBasisIsEqual (&fb1->__symbol__FeeBasis, &fb2->__symbol__FeeBasis));
}

// MARK: - Handlers

WKFeeBasisHandlers wkFeeBasisHandlers__SYMBOL__ = {
    wkFeeBasisRelease__SYMBOL__,
    wkFeeBasisGetCostFactor__SYMBOL__,
    wkFeeBasisGetPricePerCostFactor__SYMBOL__,
    wkFeeBasisGetFee__SYMBOL__,
    wkFeeBasisIsEqual__SYMBOL__
};

