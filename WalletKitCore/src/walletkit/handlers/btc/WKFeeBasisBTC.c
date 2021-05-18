//
//  WKFeeBasisBTC.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-09-04.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKBTC.h"
#include "walletkit/WKFeeBasisP.h"
#include "walletkit/WKAmountP.h"
#include "support/util/BRUtil.h"

#include <math.h>

static WKFeeBasisBTC
wkFeeBasisCoerce (WKFeeBasis feeBasis) {
    assert (WK_NETWORK_TYPE_BTC == feeBasis->type);
    return (WKFeeBasisBTC) feeBasis;
}

typedef struct {
    uint64_t fee;
    uint64_t feePerKB;
    double   sizeInKB;
} WKFeeBasisCreateContextBTC;

static void
wkFeeBasisCreateCallbackBTC (WKFeeBasisCreateContext context,
                                 WKFeeBasis feeBasis) {
    WKFeeBasisCreateContextBTC *contextBTC = (WKFeeBasisCreateContextBTC*) context;
    WKFeeBasisBTC feeBasisBTC = wkFeeBasisCoerce (feeBasis);

    feeBasisBTC->fee = contextBTC->fee;
    feeBasisBTC->feePerKB = contextBTC->feePerKB;
    feeBasisBTC->sizeInKB = contextBTC->sizeInKB;
}

private_extern WKFeeBasis
wkFeeBasisCreateAsBTC (WKUnit unit,
                           uint64_t fee,
                           uint64_t feePerKB,
                           uint32_t sizeInByte) {
    // Only one of these can be unknownn
    assert (((fee        == WK_FEE_BASIS_BTC_FEE_UNKNOWN        ? 1 : 0) +
             (feePerKB   == WK_FEE_BASIS_BTC_FEE_PER_KB_UNKNOWN ? 1 : 0) +
             (sizeInByte == WK_FEE_BASIS_BTC_SIZE_UNKNOWN       ? 1 : 0)) <= 1);

    WKFeeBasisCreateContextBTC contextBTC = {
        // Fee
        (fee == WK_FEE_BASIS_BTC_FEE_UNKNOWN
         ? (uint64_t) lround (((double) feePerKB) * sizeInByte / 1000.0)
         : fee),

        // Fee-Per-KB
        (feePerKB == WK_FEE_BASIS_BTC_FEE_PER_KB_UNKNOWN
         ? (((1000 * fee) + (sizeInByte/2)) / sizeInByte)
         : feePerKB),

        // Size-In-KB
        (sizeInByte == WK_FEE_BASIS_BTC_SIZE_UNKNOWN
         ? (((double) fee) / feePerKB)
         : (((double) sizeInByte) / 1000))
    };
    
    return wkFeeBasisAllocAndInit (sizeof (struct WKFeeBasisBTCRecord),
                                       WK_NETWORK_TYPE_BTC,
                                       unit,
                                       &contextBTC,
                                       wkFeeBasisCreateCallbackBTC);
}

private_extern uint64_t // SAT-per-KB
wkFeeBasisAsBTC (WKFeeBasis feeBasis) {
    WKFeeBasisBTC btcFeeBasis = wkFeeBasisCoerce (feeBasis);
    return btcFeeBasis->feePerKB;
}

static void
wkFeeBasisReleaseBTC (WKFeeBasis feeBasis) {
}

static double // as sizeInKB
wkFeeBasisGetCostFactorBTC (WKFeeBasis feeBasis) {
    WKFeeBasisBTC btcFeeBasis = wkFeeBasisCoerce (feeBasis);
    return btcFeeBasis->sizeInKB;
}

static WKAmount
wkFeeBasisGetPricePerCostFactorBTC (WKFeeBasis feeBasis) {
    return wkAmountCreate (feeBasis->unit,
                               WK_FALSE,
                               uint256Create (wkFeeBasisAsBTC (feeBasis)));
}

static WKAmount
wkFeeBasisGetFeeBTC (WKFeeBasis feeBasis) {
    WKFeeBasisBTC btcFeeBasis = wkFeeBasisCoerce (feeBasis);
    return wkAmountCreate (feeBasis->unit,
                               WK_FALSE,
                               uint256Create (btcFeeBasis->fee));
}

static WKBoolean
wkFeeBasisIsEqualBTC (WKFeeBasis feeBasis1, WKFeeBasis feeBasis2) {
    WKFeeBasisBTC fb1 = wkFeeBasisCoerce (feeBasis1);
    WKFeeBasisBTC fb2 = wkFeeBasisCoerce (feeBasis2);

    return (fb1->fee == fb2->fee &&
            fb1->feePerKB == fb2->feePerKB &&
            lround (1000.0 * fb1->sizeInKB) == lround (1000.0 * fb2->sizeInKB));
}

// MARK: - Handlers

WKFeeBasisHandlers wkFeeBasisHandlersBTC = {
    wkFeeBasisReleaseBTC,
    wkFeeBasisGetCostFactorBTC,
    wkFeeBasisGetPricePerCostFactorBTC,
    wkFeeBasisGetFeeBTC,
    wkFeeBasisIsEqualBTC
};
