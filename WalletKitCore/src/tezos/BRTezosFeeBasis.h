//
//  BRTezosFeeBasis.h
//  Core
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRTezosFeeBasis_h
#define BRTezosFeeBasis_h

#include <stdint.h>
#include "BRTezosBase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    BRTezosUnitMutez    fee;
    int64_t             gasLimit;
    int64_t             storageLimit; // not used for fee calculation but needed for tx serialization
} BRTezosFeeBasis;

extern BRTezosFeeBasis tezosDefaultFeeBasis();

extern BRTezosUnitMutez tezosFeeBasisGetPricePerCostFactor(BRTezosFeeBasis *feeBasis);
extern uint64_t tezosFeeBasisGetCostFactor(BRTezosFeeBasis *feeBasis);
extern uint32_t tezosFeeBasisIsEqual(BRTezosFeeBasis *fb1, BRTezosFeeBasis *fb2);

extern BRTezosUnitMutez tezosFeeBasisGetFee(BRTezosFeeBasis *feeBasis);


#ifdef __cplusplus
}
#endif

#endif /* BRTezosFeeBasis_h */
