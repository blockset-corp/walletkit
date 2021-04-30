//
//  WKFeeBasisHBAR.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-09-04.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKHBAR.h"
#include "walletkit/WKFeeBasisP.h"
#include "hedera/BRHedera.h"

private_extern WKFeeBasisHBAR
wkFeeBasisCoerceHBAR (WKFeeBasis feeBasis) {
    assert (WK_NETWORK_TYPE_HBAR == feeBasis->type);
    return (WKFeeBasisHBAR) feeBasis;
}

typedef struct {
    BRHederaFeeBasis hbarFeeBasis;
} WKFeeBasisCreateContextHBAR;

static void
wkFeeBasisCreateCallbackHBAR (WKFeeBasisCreateContext context,
                                 WKFeeBasis feeBasis) {
    WKFeeBasisCreateContextHBAR *contextHBAR = (WKFeeBasisCreateContextHBAR*) context;
    WKFeeBasisHBAR feeBasisHBAR = wkFeeBasisCoerceHBAR (feeBasis);
    
    feeBasisHBAR->hbarFeeBasis = contextHBAR->hbarFeeBasis;
}

private_extern BRHederaFeeBasis
wkFeeBasisAsHBAR (WKFeeBasis feeBasis) {
    WKFeeBasisHBAR feeBasisHBAR = wkFeeBasisCoerceHBAR (feeBasis);
    return feeBasisHBAR->hbarFeeBasis;
}

private_extern WKFeeBasis
wkFeeBasisCreateAsHBAR (WKUnit unit,
                            BRHederaFeeBasis hbarFeeBasis) {
    WKFeeBasisCreateContextHBAR contextHBAR = {
        hbarFeeBasis
    };
    
    return wkFeeBasisAllocAndInit (sizeof (struct WKFeeBasisHBARRecord),
                                       WK_NETWORK_TYPE_HBAR,
                                       unit,
                                       &contextHBAR,
                                       wkFeeBasisCreateCallbackHBAR);
}

static void
wkFeeBasisReleaseHBAR (WKFeeBasis feeBasis) {
}

static double
wkFeeBasisGetCostFactorHBAR (WKFeeBasis feeBasis) {
    return (double) wkFeeBasisCoerceHBAR (feeBasis)->hbarFeeBasis.costFactor;
}

static WKAmount
wkFeeBasisGetPricePerCostFactorHBAR (WKFeeBasis feeBasis) {
    BRHederaFeeBasis hbarFeeBasis = wkFeeBasisCoerceHBAR (feeBasis)->hbarFeeBasis;
    return wkAmountCreateAsHBAR (feeBasis->unit, WK_FALSE, hbarFeeBasis.pricePerCostFactor);
}

static WKAmount
wkFeeBasisGetFeeHBAR (WKFeeBasis feeBasis) {
    return wkFeeBasisGetPricePerCostFactor (feeBasis);
}

static WKBoolean
wkFeeBasisIsEqualHBAR (WKFeeBasis feeBasis1, WKFeeBasis feeBasis2) {
    WKFeeBasisHBAR fb1 = wkFeeBasisCoerceHBAR (feeBasis1);
    WKFeeBasisHBAR fb2 = wkFeeBasisCoerceHBAR (feeBasis2);

    return hederaFeeBasisIsEqual (&fb1->hbarFeeBasis, &fb2->hbarFeeBasis);
}

// MARK: - Handlers

WKFeeBasisHandlers wkFeeBasisHandlersHBAR = {
    wkFeeBasisReleaseHBAR,
    wkFeeBasisGetCostFactorHBAR,
    wkFeeBasisGetPricePerCostFactorHBAR,
    wkFeeBasisGetFeeHBAR,
    wkFeeBasisIsEqualHBAR
};

