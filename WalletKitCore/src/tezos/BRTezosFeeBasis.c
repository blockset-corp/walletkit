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
#define TEZOS_MINIMAL_MUTEZ_PER_KBYTE 1000 // equivalent to 1000 nanotez per byte


extern BRTezosFeeBasis
tezosDefaultFeeBasis(BRTezosUnitMutez mutezPerKByte) {
    return (BRTezosFeeBasis) {
        FEE_BASIS_INITIAL,
        { .initial = {
            mutezPerKByte,
            0,
            // https://github.com/TezTech/eztz/blob/master/PROTO_004_FEES.md
            1040000, // hard gas limit, actual will be given by node estimate_fee
            60000, // hard limit, actual will be given by node estimate_fee
        } }
    };
}

extern BRTezosFeeBasis
tezosFeeBasisCreateEstimate(BRTezosUnitMutez mutezPerKByte,
                            double sizeInKBytes,
                            int64_t gasLimit,
                            int64_t storageLimit,
                            int64_t counter) {
    // storage is burned and not part of the fee
    BRTezosUnitMutez minimalFee = TEZOS_MINIMAL_FEE_MUTEZ
                                  + (int64_t)(TEZOS_MUTEZ_PER_GAS_UNIT * gasLimit)
                                  + (int64_t)(MAX(TEZOS_MINIMAL_MUTEZ_PER_KBYTE, mutezPerKByte) * sizeInKBytes);
    // add a 5% padding to the estimated minimum to improve chance of acceptance by network
    BRTezosUnitMutez fee = (BRTezosUnitMutez)(minimalFee * 1.05);

    return (BRTezosFeeBasis) {
        FEE_BASIS_ESTIMATE,
        { .estimate = {
            fee,
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
    switch (feeBasis->type) {
        case FEE_BASIS_INITIAL:
            return TEZOS_DEFAULT_FEE_MUTEZ;
        case FEE_BASIS_ESTIMATE:
            return feeBasis->u.estimate.calculatedFee;
        case FEE_BASIS_ACTUAL:
            return feeBasis->u.actual.fee;
    }
}

extern bool
tezosFeeBasisIsEqual (BRTezosFeeBasis *fb1, BRTezosFeeBasis *fb2) {
    assert(fb1);
    assert(fb2);
    
    if (fb1->type != fb2->type) return false;
    
    switch (fb1->type) {
        case FEE_BASIS_INITIAL:
            return (fb1->u.initial.mutezPerKByte == fb2->u.initial.mutezPerKByte &&
                    fb1->u.initial.sizeInKBytes == fb2->u.initial.sizeInKBytes &&
                    fb1->u.initial.gasLimit == fb2->u.initial.gasLimit &&
                    fb1->u.initial.storageLimit == fb2->u.initial.storageLimit);
            
        case FEE_BASIS_ESTIMATE:
            return (fb1->u.estimate.calculatedFee == fb2->u.estimate.calculatedFee &&
                    fb1->u.estimate.gasLimit == fb2->u.estimate.gasLimit &&
                    fb1->u.estimate.storageLimit == fb2->u.estimate.storageLimit &&
                    fb1->u.estimate.counter == fb2->u.estimate.counter);
            
        case FEE_BASIS_ACTUAL:
            return (fb1->u.actual.fee == fb2->u.actual.fee);
            
        default:
            return false;
    }
}

private_extern int64_t
tezosFeeBasisGetGasLimit(BRTezosFeeBasis feeBasis) {
    switch (feeBasis.type) {
        case FEE_BASIS_INITIAL:
            return feeBasis.u.initial.gasLimit;
        case FEE_BASIS_ESTIMATE:
            return feeBasis.u.estimate.gasLimit;
        default:
            assert(0);
            return 0;
    }
}

private_extern int64_t
tezosFeeBasisGetStorageLimit(BRTezosFeeBasis feeBasis) {
    switch (feeBasis.type) {
        case FEE_BASIS_INITIAL:
            return feeBasis.u.initial.storageLimit;
        case FEE_BASIS_ESTIMATE:
            return feeBasis.u.estimate.storageLimit;
        default:
            assert(0);
            return 0;
    }
}
