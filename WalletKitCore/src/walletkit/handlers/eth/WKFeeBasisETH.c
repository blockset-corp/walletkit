//
//  WKFeeBasisETH.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-09-04.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKETH.h"
#include "walletkit/WKFeeBasisP.h"
#include "walletkit/WKAmountP.h"

static WKFeeBasisETH
wkFeeBasisCoerce (WKFeeBasis feeBasis) {
    assert (WK_NETWORK_TYPE_ETH == feeBasis->type);
    return (WKFeeBasisETH) feeBasis;
}

typedef struct {
    BREthereumFeeBasis ethFeeBasis;
} WKFeeBasisCreateContextETH;

static void
wkFeeBasisCreateCallbackETH (WKFeeBasisCreateContext context,
                                 WKFeeBasis feeBasis) {
    WKFeeBasisCreateContextETH *contextETH = (WKFeeBasisCreateContextETH*) context;
    WKFeeBasisETH feeBasisETH = wkFeeBasisCoerce (feeBasis);
    
    feeBasisETH->ethFeeBasis = contextETH->ethFeeBasis;
}

private_extern WKFeeBasis
wkFeeBasisCreateAsETH (WKUnit unit,
                           BREthereumFeeBasis ethFeeBasis) {
    WKFeeBasisCreateContextETH contextETH = {
        ethFeeBasis
    };
    
    return wkFeeBasisAllocAndInit (sizeof (struct WKFeeBasisETHRecord),
                                       WK_NETWORK_TYPE_ETH,
                                       unit,
                                       &contextETH,
                                       wkFeeBasisCreateCallbackETH);
}

private_extern BREthereumFeeBasis
wkFeeBasisAsETH (WKFeeBasis feeBasis) {
    WKFeeBasisETH ethFeeBasis = wkFeeBasisCoerce (feeBasis);
    return ethFeeBasis->ethFeeBasis;
}

static void
wkFeeBasisReleaseETH (WKFeeBasis feeBasis) {
}

static double
wkFeeBasisGetCostFactorETH (WKFeeBasis feeBasis) {
    WKFeeBasisETH ethFeeBasis = wkFeeBasisCoerce (feeBasis);
    return ethFeeBasis->ethFeeBasis.u.gas.limit.amountOfGas;
}

static WKAmount
wkFeeBasisGetPricePerCostFactorETH (WKFeeBasis feeBasis) {
    BREthereumFeeBasis ethFeeBasis = wkFeeBasisCoerce (feeBasis)->ethFeeBasis;
    return wkAmountCreate (feeBasis->unit,
                               WK_FALSE,
                               ethFeeBasis.u.gas.price.etherPerGas.valueInWEI);
}

static WKAmount
wkFeeBasisGetFeeETH (WKFeeBasis feeBasis) {
    BREthereumFeeBasis ethFeeBasis = wkFeeBasisCoerce (feeBasis)->ethFeeBasis;
    UInt256 gasPrice = ethFeeBasis.u.gas.price.etherPerGas.valueInWEI;
    double  gasAmount = wkFeeBasisGetCostFactor (feeBasis);
    
    int overflow = 0, negative = 0;
    double rem;
    
    UInt256 value = uint256Mul_Double (gasPrice, gasAmount, &overflow, &negative, &rem);
    
    return (overflow
            ? NULL
            : wkAmountCreate (feeBasis->unit, WK_FALSE, value));
}

static WKBoolean
wkFeeBasisIsEqualETH (WKFeeBasis feeBasis1, WKFeeBasis feeBasis2) {
    WKFeeBasisETH fb1 = wkFeeBasisCoerce (feeBasis1);
    WKFeeBasisETH fb2 = wkFeeBasisCoerce (feeBasis2);

    return AS_WK_BOOLEAN (ETHEREUM_BOOLEAN_TRUE == ethFeeBasisEqual (&fb1->ethFeeBasis, &fb2->ethFeeBasis));
}

// MARK: - Handlers

WKFeeBasisHandlers wkFeeBasisHandlersETH = {
    wkFeeBasisReleaseETH,
    wkFeeBasisGetCostFactorETH,
    wkFeeBasisGetPricePerCostFactorETH,
    wkFeeBasisGetFeeETH,
    wkFeeBasisIsEqualETH
};
