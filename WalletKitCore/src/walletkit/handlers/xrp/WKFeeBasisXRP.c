//
//  WKFeeBasisXRP.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-09-04.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXRP.h"
#include "walletkit/WKFeeBasisP.h"
#include "ripple/BRRipple.h"

static WKFeeBasisXRP
wkFeeBasisCoerce (WKFeeBasis feeBasis) {
    assert (WK_NETWORK_TYPE_XRP == feeBasis->type);
    return (WKFeeBasisXRP) feeBasis;
}

typedef struct {
    BRRippleFeeBasis xrpFeeBasis;
} WKFeeBasisCreateContextXRP;

static void
wkFeeBasisCreateCallbackXRP (WKFeeBasisCreateContext context,
                                 WKFeeBasis feeBasis) {
    WKFeeBasisCreateContextXRP *contextXRP = (WKFeeBasisCreateContextXRP*) context;
    WKFeeBasisXRP feeBasisXRP = wkFeeBasisCoerce (feeBasis);
    
    feeBasisXRP->xrpFeeBasis = contextXRP->xrpFeeBasis;
}

private_extern WKFeeBasis
wkFeeBasisCreateAsXRP (WKUnit unit,
                           BRRippleUnitDrops fee) {
    BRRippleFeeBasis xrpFeeBasis;
    xrpFeeBasis.costFactor = 1;
    xrpFeeBasis.pricePerCostFactor = fee;
    
    WKFeeBasisCreateContextXRP contextXRP = {
        xrpFeeBasis
    };
    
    return wkFeeBasisAllocAndInit (sizeof (struct WKFeeBasisXRPRecord),
                                       WK_NETWORK_TYPE_XRP,
                                       unit,
                                       &contextXRP,
                                       wkFeeBasisCreateCallbackXRP);
}

private_extern BRRippleFeeBasis
wkFeeBasisAsXRP (WKFeeBasis feeBasis) {
    WKFeeBasisXRP feeBasisXRP = wkFeeBasisCoerce (feeBasis);
    return feeBasisXRP->xrpFeeBasis;
}

static void
wkFeeBasisReleaseXRP (WKFeeBasis feeBasis) {
}

static double
wkFeeBasisGetCostFactorXRP (WKFeeBasis feeBasis) {
    return (double) wkFeeBasisCoerce (feeBasis)->xrpFeeBasis.costFactor;
}

static WKAmount
wkFeeBasisGetPricePerCostFactorXRP (WKFeeBasis feeBasis) {
    BRRippleFeeBasis xrpFeeBasis = wkFeeBasisCoerce (feeBasis)->xrpFeeBasis;
    return wkAmountCreateAsXRP (feeBasis->unit, WK_FALSE, xrpFeeBasis.pricePerCostFactor);
}

static WKAmount
wkFeeBasisGetFeeXRP (WKFeeBasis feeBasis) {
    return wkFeeBasisGetPricePerCostFactor (feeBasis);
}

static WKBoolean
wkFeeBasisIsEqualXRP (WKFeeBasis feeBasis1, WKFeeBasis feeBasis2) {
    WKFeeBasisXRP fb1 = wkFeeBasisCoerce (feeBasis1);
    WKFeeBasisXRP fb2 = wkFeeBasisCoerce (feeBasis2);

    return rippleFeeBasisIsEqual (&fb1->xrpFeeBasis, &fb2->xrpFeeBasis);
}

// MARK: - Handlers

WKFeeBasisHandlers wkFeeBasisHandlersXRP = {
    wkFeeBasisReleaseXRP,
    wkFeeBasisGetCostFactorXRP,
    wkFeeBasisGetPricePerCostFactorXRP,
    wkFeeBasisGetFeeXRP,
    wkFeeBasisIsEqualXRP
};
