//
//  WKFeeBasisXLM.c
//  WalletKitCore
//
//  Created by Carl Cherry on 2021-05-21.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXLM.h"
#include "walletkit/WKFeeBasisP.h"
#include "stellar/BRStellar.h"

static WKFeeBasisXLM
wkFeeBasisCoerce (WKFeeBasis feeBasis) {
    assert (WK_NETWORK_TYPE_XLM == feeBasis->type);
    return (WKFeeBasisXLM) feeBasis;
}

typedef struct {
    BRStellarFeeBasis xlmFeeBasis;
} WKFeeBasisCreateContextXLM;

static void
wkFeeBasisCreateCallbackXLM (WKFeeBasisCreateContext context,
                             WKFeeBasis feeBasis) {
    WKFeeBasisCreateContextXLM *contextXLM = (WKFeeBasisCreateContextXLM*) context;
    WKFeeBasisXLM feeBasisXLM = wkFeeBasisCoerce (feeBasis);
    
    feeBasisXLM->xlmFeeBasis = contextXLM->xlmFeeBasis;
}

private_extern WKFeeBasis
wkFeeBasisCreateAsXLM (WKUnit unit, BRStellarAmount fee) {
    BRStellarFeeBasis xlmFeeBasis;
    xlmFeeBasis.costFactor = 1;
    xlmFeeBasis.pricePerCostFactor = fee;
    
    WKFeeBasisCreateContextXLM contextXLM = {
        xlmFeeBasis
    };
    
    return wkFeeBasisAllocAndInit (sizeof (struct WKFeeBasisXLMRecord),
                                       WK_NETWORK_TYPE_XLM,
                                       unit,
                                       &contextXLM,
                                       wkFeeBasisCreateCallbackXLM);
}

private_extern BRStellarFeeBasis
wkFeeBasisAsXLM (WKFeeBasis feeBasis) {
    WKFeeBasisXLM feeBasisXLM = wkFeeBasisCoerce (feeBasis);
    return feeBasisXLM->xlmFeeBasis;
}

static void
wkFeeBasisReleaseXLM (WKFeeBasis feeBasis) {
}

static double
wkFeeBasisGetCostFactorXLM (WKFeeBasis feeBasis) {
    return (double) wkFeeBasisCoerce (feeBasis)->xlmFeeBasis.costFactor;
}

static WKAmount
wkFeeBasisGetPricePerCostFactorXLM (WKFeeBasis feeBasis) {
    BRStellarFeeBasis xlmFeeBasis = wkFeeBasisCoerce (feeBasis)->xlmFeeBasis;
    return wkAmountCreateAsXLM (feeBasis->unit, WK_FALSE, xlmFeeBasis.pricePerCostFactor);
}

static WKAmount
wkFeeBasisGetFeeXLM (WKFeeBasis feeBasis) {
    return wkFeeBasisGetPricePerCostFactor (feeBasis);
}

static WKBoolean
wkFeeBasisIsEqualXLM (WKFeeBasis feeBasis1, WKFeeBasis feeBasis2) {
    WKFeeBasisXLM fb1 = wkFeeBasisCoerce (feeBasis1);
    WKFeeBasisXLM fb2 = wkFeeBasisCoerce (feeBasis2);

    return stellarFeeBasisIsEqual (&fb1->xlmFeeBasis, &fb2->xlmFeeBasis);
}

// MARK: - Handlers

WKFeeBasisHandlers wkFeeBasisHandlersXLM = {
    wkFeeBasisReleaseXLM,
    wkFeeBasisGetCostFactorXLM,
    wkFeeBasisGetPricePerCostFactorXLM,
    wkFeeBasisGetFeeXLM,
    wkFeeBasisIsEqualXLM
};
