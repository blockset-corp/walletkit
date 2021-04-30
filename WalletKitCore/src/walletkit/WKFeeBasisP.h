//
//  WKFeeBasisP.h
//  WalletKitCore
//
//  Created by Ed Gamble on 11/22/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKFeeBasisP_h
#define WKFeeBasisP_h

#include "WKFeeBasis.h"
#include "WKBaseP.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void
(*WKFeeBasisReleaseHandler) (WKFeeBasis feeBasis);

typedef double
(*WKFeeBasisGetCostFactorHandler) (WKFeeBasis feeBasis);

typedef WKAmount
(*WKFeeBasisGetPricePerCostFactorHandler) (WKFeeBasis feeBasis);

typedef WKAmount
(*WKFeeBasisGetFeeHandler) (WKFeeBasis feeBasis);

typedef WKBoolean
(*WKFeeBasisIsEqualHandler) (WKFeeBasis feeBasis1,
                                   WKFeeBasis feeBasis2);

typedef struct {
    WKFeeBasisReleaseHandler release;
    WKFeeBasisGetCostFactorHandler getCostFactor;
    WKFeeBasisGetPricePerCostFactorHandler getPricePerCostFactor;
    WKFeeBasisGetFeeHandler getFee;
    WKFeeBasisIsEqualHandler isEqual;
} WKFeeBasisHandlers;

struct WKFeeBasisRecord {
    WKNetworkType type;
    const WKFeeBasisHandlers *handlers;
    WKRef ref;
    size_t sizeInBytes;
    
    WKUnit unit;
};

typedef void *WKFeeBasisCreateContext;
typedef void (*WKFeeBasisCreateCallback) (WKFeeBasisCreateContext context,
                                                WKFeeBasis feeBasis);

private_extern WKFeeBasis
wkFeeBasisAllocAndInit (size_t sizeInBytes,
                            WKNetworkType type,
                            WKUnit unit,
                            WKFeeBasisCreateContext  createContext,
                            WKFeeBasisCreateCallback createCallback);

private_extern WKNetworkType
wkFeeBasisGetType (WKFeeBasis feeBasis);


#ifdef __cplusplus
}
#endif

#endif /* WKFeeBasisP_h */
