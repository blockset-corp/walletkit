//
//  WKFeeBasisAVAX.c
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKAVAX.h"
#include "walletkit/WKFeeBasisP.h"
#include "avalanche/BRAvalanche.h"

typedef struct {
    BRAvalancheFeeBasis avaxFeeBasis;
} WKFeeBasisCreateContextAVAX;

static void
wkFeeBasisCreateCallbackAVAX (WKFeeBasisCreateContext context,
                                    WKFeeBasis feeBasis) {
    WKFeeBasisCreateContextAVAX *contextAVAX = (WKFeeBasisCreateContextAVAX*) context;
    WKFeeBasisAVAX feeBasisAVAX = wkFeeBasisCoerceAVAX (feeBasis);
    
    feeBasisAVAX->avaxFeeBasis = contextAVAX->avaxFeeBasis;
}

private_extern WKFeeBasis
wkFeeBasisCreateAsAVAX (WKUnit unit,
                              BRAvalancheFeeBasis avaxFeeBasis) {
    WKFeeBasisCreateContextAVAX contextAVAX = {
        avaxFeeBasis
    };
    
    return wkFeeBasisAllocAndInit (sizeof (struct WKFeeBasisAVAXRecord),
                                   WK_NETWORK_TYPE_AVAX,
                                   unit,
                                   &contextAVAX,
                                   wkFeeBasisCreateCallbackAVAX);
}

static void
wkFeeBasisReleaseAVAX (WKFeeBasis feeBasis) {
    // If BRAvalancheFeeBasis is a value type, nothing needed
}

static double
wkFeeBasisGetCostFactorAVAX (WKFeeBasis feeBasis) {
    BRAvalancheFeeBasis avaxFeeBasis = wkFeeBasisCoerceAVAX (feeBasis)->avaxFeeBasis;

    ASSERT_UNIMPLEMENTED; (void) avaxFeeBasis;
    return 1.0;
#if 0
    switch (avaxFeeBasis.type) {
        case FEE_BASIS_INITIAL:
            return (double) avaxFeeBasis.u.initial.sizeInKBytes;
        case FEE_BASIS_ESTIMATE:
            return 1.0;
        case FEE_BASIS_ACTUAL:
            return 1.0;
    }
#endif
}

static WKAmount
wkFeeBasisGetPricePerCostFactorAVAX (WKFeeBasis feeBasis) {
    BRAvalancheFeeBasis avaxFeeBasis = wkFeeBasisCoerceAVAX (feeBasis)->avaxFeeBasis;

    ASSERT_UNIMPLEMENTED;  (void) avaxFeeBasis;
    return wkAmountCreateAsAVAX (feeBasis->unit, WK_FALSE, 1);
#if 0
    switch (avaxFeeBasis.type) {
        case FEE_BASIS_INITIAL:
            return wkAmountCreateAsAVAX (feeBasis->unit, WK_FALSE, avaxFeeBasis.u.initial.mutezPerKByte);
        case FEE_BASIS_ESTIMATE:
            return wkAmountCreateAsAVAX (feeBasis->unit, WK_FALSE, avaxFeeBasis.u.estimate.calculatedFee);
        case FEE_BASIS_ACTUAL:
            return wkAmountCreateAsAVAX (feeBasis->unit, WK_FALSE, avaxFeeBasis.u.actual.fee);
    }
#endif
}

static WKAmount
wkFeeBasisGetFeeAVAX (WKFeeBasis feeBasis) {
    BRAvalancheFeeBasis avaxFeeBasis = wkFeeBasisCoerceAVAX (feeBasis)->avaxFeeBasis;
    BRAvalancheAmount fee = avalancheFeeBasisGetFee (&avaxFeeBasis);
    return wkAmountCreateAsAVAX (feeBasis->unit, WK_FALSE, fee);
}

static WKBoolean
wkFeeBasisIsEqualAVAX (WKFeeBasis feeBasis1, WKFeeBasis feeBasis2) {
    WKFeeBasisAVAX fb1 = wkFeeBasisCoerceAVAX (feeBasis1);
    WKFeeBasisAVAX fb2 = wkFeeBasisCoerceAVAX (feeBasis2);
    
    return AS_WK_BOOLEAN (avalancheFeeBasisIsEqual (&fb1->avaxFeeBasis, &fb2->avaxFeeBasis));
}

// MARK: - Handlers

WKFeeBasisHandlers wkFeeBasisHandlersAVAX = {
    wkFeeBasisReleaseAVAX,
    wkFeeBasisGetCostFactorAVAX,
    wkFeeBasisGetPricePerCostFactorAVAX,
    wkFeeBasisGetFeeAVAX,
    wkFeeBasisIsEqualAVAX
};

