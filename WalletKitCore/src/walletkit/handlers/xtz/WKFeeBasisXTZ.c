//
//  WKFeeBasisXTZ.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-09-04.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXTZ.h"
#include "walletkit/WKFeeBasisP.h"
#include "tezos/BRTezos.h"

private_extern WKFeeBasisXTZ
wkFeeBasisCoerceXTZ (WKFeeBasis feeBasis) {
    assert (WK_NETWORK_TYPE_XTZ == feeBasis->type);
    return (WKFeeBasisXTZ) feeBasis;
}

typedef struct {
    BRTezosFeeBasis xtzFeeBasis;
} WKFeeBasisCreateContextXTZ;

static void
wkFeeBasisCreateCallbackXTZ (WKFeeBasisCreateContext context,
                             WKFeeBasis feeBasis) {
    WKFeeBasisCreateContextXTZ *contextXTZ = (WKFeeBasisCreateContextXTZ*) context;
    WKFeeBasisXTZ feeBasisXTZ = wkFeeBasisCoerceXTZ (feeBasis);
    
    feeBasisXTZ->xtzFeeBasis = contextXTZ->xtzFeeBasis;
}

private_extern WKFeeBasis
wkFeeBasisCreateAsXTZ (WKUnit unit,
                       BRTezosFeeBasis xtzFeeBasis) {
    WKFeeBasisCreateContextXTZ contextXTZ = {
        xtzFeeBasis
    };
    
    return wkFeeBasisAllocAndInit (sizeof (struct WKFeeBasisXTZRecord),
                                   WK_NETWORK_TYPE_XTZ,
                                   unit,
                                   &contextXTZ,
                                   wkFeeBasisCreateCallbackXTZ);
}

static void
wkFeeBasisReleaseXTZ (WKFeeBasis feeBasis) {
}

private_extern BRTezosFeeBasis
wkFeeBasisAsXTZ (WKFeeBasis feeBasis) {
    WKFeeBasisXTZ feeBasisXTZ = wkFeeBasisCoerceXTZ(feeBasis);
    return feeBasisXTZ->xtzFeeBasis;
}

static double
wkFeeBasisGetCostFactorXTZ (WKFeeBasis feeBasis) {
    BRTezosFeeBasis xtzFeeBasis = wkFeeBasisCoerceXTZ (feeBasis)->xtzFeeBasis;

    BRTezosUnitMutez fee = tezosFeeBasisGetFee (xtzFeeBasis);
    return 1000 * fee / ((double) xtzFeeBasis.mutezPerKByte);  // byte
}

static WKAmount // mutez/byte
wkFeeBasisGetPricePerCostFactorXTZ (WKFeeBasis feeBasis) {
    BRTezosFeeBasis xtzFeeBasis = wkFeeBasisCoerceXTZ (feeBasis)->xtzFeeBasis;
    return wkAmountCreateAsXTZ (feeBasis->unit, WK_FALSE, xtzFeeBasis.mutezPerKByte / 1000);
}

static WKAmount
wkFeeBasisGetFeeXTZ (WKFeeBasis feeBasis) {
    BRTezosFeeBasis xtzFeeBasis = wkFeeBasisCoerceXTZ (feeBasis)->xtzFeeBasis;
    return wkAmountCreateAsXTZ (feeBasis->unit, WK_FALSE, tezosFeeBasisGetFee (xtzFeeBasis));
}

static WKBoolean
wkFeeBasisIsEqualXTZ (WKFeeBasis feeBasis1, WKFeeBasis feeBasis2) {
    WKFeeBasisXTZ fb1 = wkFeeBasisCoerceXTZ (feeBasis1);
    WKFeeBasisXTZ fb2 = wkFeeBasisCoerceXTZ (feeBasis2);
    
    return AS_WK_BOOLEAN (tezosFeeBasisIsEqual (&fb1->xtzFeeBasis, &fb2->xtzFeeBasis));
}

// MARK: - Handlers

WKFeeBasisHandlers wkFeeBasisHandlersXTZ = {
    wkFeeBasisReleaseXTZ,
    wkFeeBasisGetCostFactorXTZ,
    wkFeeBasisGetPricePerCostFactorXTZ,
    wkFeeBasisGetFeeXTZ,
    wkFeeBasisIsEqualXTZ
};

