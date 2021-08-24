//
//  BRAvalancheFeeBasis.h
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRAvalancheFeeBasis_h
#define BRAvalancheFeeBasis_h

#include <stdint.h>
#include "BRAvalancheBase.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AVALANCHE_DEFAULT_AVAX_PER_TRANSACTION   100000

typedef struct
{
    BRAvalancheAmount avaxPerTransaction;
} BRAvalancheFeeBasis;

extern BRAvalancheFeeBasis
avalancheFeeBasisCreate (BRAvalancheAmount avaxPerTransaction);

extern BRAvalancheAmount
avalancheFeeBasisGetFee(BRAvalancheFeeBasis *feeBasis);

extern bool
avalancheFeeBasisIsEqual(BRAvalancheFeeBasis *fb1, BRAvalancheFeeBasis *fb2);

#ifdef __cplusplus
}
#endif

#endif /* BRAvalancheFeeBasis_h */
