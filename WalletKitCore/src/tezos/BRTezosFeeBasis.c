//
//  BRTezosFeeBasis.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include <stdint.h>
#include "BRTezosFeeBasis.h"
#include "ethereum/util/BRUtilMath.h"
#include "crypto/BRCryptoBaseP.h"
#include <stdio.h>
#include <assert.h>

#if !defined (MAX)
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

// https://tezos.gitlab.io/protocols/004_Pt24m4xi.html#gas-and-fees
#define TEZOS_MINIMAL_FEE_MUTEZ 100
#define TEZOS_MUTEZ_PER_GAS_UNIT 0.1
#define TEZOS_DEFAULT_FEE_MUTEZ 1420
#define TEZOS_MINIMAL_STORAGE_LIMIT 300 // sending to inactive accounts


extern BRTezosFeeBasis
tezosDefaultFeeBasis(BRTezosUnitMutez mutezPerKByte) {
    return (BRTezosFeeBasis) {
        FEE_BASIS_ESTIMATE,
        { .estimate = {
            mutezPerKByte,
            0,
            // https://github.com/TezTech/eztz/blob/master/PROTO_004_FEES.md
            1040000, // hard gas limit, actual will be given by node estimate_fee
            60000, // hard limit, actual will be given by node estimate_fee
            0 // counter will be given by estimate_fee
        } }
    };
}

extern BRTezosFeeBasis
tezosFeeBasisCreateEstimate(BRTezosUnitMutez mutezPerKByte,
                            double sizeInKBytes,
                            int64_t gasLimit,
                            int64_t storageLimit,
                            int64_t counter) {
    return (BRTezosFeeBasis) {
        FEE_BASIS_ESTIMATE,
        { .estimate = {
            mutezPerKByte,
            sizeInKBytes,
            gasLimit,
            MAX(storageLimit, TEZOS_MINIMAL_STORAGE_LIMIT),
            counter
        } }
    };
}

extern BRTezosFeeBasis
tezosFeeBasisCreateActual(BRTezosUnitMutez fee) {
    return (BRTezosFeeBasis) {
        FEE_BASIS_ACTUAL,
        { .actual = {
            fee
        } }
    };
}

extern BRTezosUnitMutez
tezosFeeBasisGetFee (BRTezosFeeBasis *feeBasis) {
    if (FEE_BASIS_ESTIMATE == feeBasis->type) {
        if (0 == feeBasis->u.estimate.sizeInKBytes) {
            // unserialized transaction
            return TEZOS_DEFAULT_FEE_MUTEZ;
        } else {
            // storage is burned and not part of the fee
            BRTezosUnitMutez minimalFee = TEZOS_MINIMAL_FEE_MUTEZ
                                          + (int64_t)(TEZOS_MUTEZ_PER_GAS_UNIT * feeBasis->u.estimate.gasLimit)
                                          + (int64_t)(feeBasis->u.estimate.mutezPerKByte * feeBasis->u.estimate.sizeInKBytes);
            // add a 5% padding to the estimated minimum to improve chance of acceptance by network
            return  (BRTezosUnitMutez)(minimalFee * 1.05);
        }
    } else {
        return feeBasis->u.actual.fee;
    }
}

extern bool
tezosFeeBasisIsEqual (BRTezosFeeBasis *fb1, BRTezosFeeBasis *fb2) {
    assert(fb1);
    assert(fb2);
    
    if (fb1->type != fb2->type) return false;
    
    switch (fb1->type) {
        case FEE_BASIS_ESTIMATE:
            return (fb1->u.estimate.mutezPerKByte == fb2->u.estimate.mutezPerKByte &&
                    fb1->u.estimate.gasLimit == fb2->u.estimate.gasLimit &&
                    fb1->u.estimate.storageLimit == fb2->u.estimate.storageLimit);
            
        case FEE_BASIS_ACTUAL:
            return (fb1->u.actual.fee == fb2->u.actual.fee);
            
        default:
            return false;
    }
}
