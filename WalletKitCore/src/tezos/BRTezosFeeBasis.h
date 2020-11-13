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

#define TEZOS_DEFAULT_MUTEZ_PER_BYTE 1

typedef enum {
    FEE_BASIS_ESTIMATE,
    FEE_BASIS_ACTUAL
} BRTezosFeeBasisType;

typedef struct
{
    BRTezosFeeBasisType type;
    union {
        struct {
            BRTezosUnitMutez    mutezPerKByte; // cost per byte is given as nanotez and can be less than one mutez
            double              sizeInKBytes;
            int64_t             gasLimit;
            int64_t             storageLimit;
            int64_t             counter;
        } estimate;
        
        struct {
            BRTezosUnitMutez fee;
        } actual;
    } u;
} BRTezosFeeBasis;


extern BRTezosFeeBasis
tezosDefaultFeeBasis(BRTezosUnitMutez mutezPerKByte);

extern BRTezosFeeBasis
tezosFeeBasisCreateEstimate(BRTezosUnitMutez mutezPerKByte,
                            double sizeInBytes,
                            int64_t gasLimit,
                            int64_t storageLimit,
                            int64_t counter);

extern BRTezosFeeBasis
tezosFeeBasisCreateActual(BRTezosUnitMutez fee);

extern BRTezosUnitMutez
tezosFeeBasisGetFee(BRTezosFeeBasis *feeBasis);

extern bool
tezosFeeBasisIsEqual(BRTezosFeeBasis *fb1, BRTezosFeeBasis *fb2);


#ifdef __cplusplus
}
#endif

#endif /* BRTezosFeeBasis_h */
