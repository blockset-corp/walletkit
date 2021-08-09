//
//  BRTezosFeeBasis.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include <stdint.h>
#include <assert.h>
//#include <stdio.h>

#include "support/util/BRUtilMath.h"

#include "BRTezosFeeBasis.h"
#include "BRTezosOperation.h"
//#include "walletkit/WKBaseP.h"

static BRTezosFeeBasis
tezosFeeBasisCreateInternal (BRTezosUnitMutez mutezPerKByte,
                             BRTezosOperationFeeBasis primaryOperationFeeBasis,
                             bool hasRevealOperationFeeBasis,
                             BRTezosOperationFeeBasis revealOperationFeeBasis) {
    return (BRTezosFeeBasis) {
        mutezPerKByte,
        primaryOperationFeeBasis,
        hasRevealOperationFeeBasis,
        revealOperationFeeBasis
    };
}

extern BRTezosFeeBasis
tezosFeeBasisCreate (BRTezosUnitMutez mutezPerKByte,
                     BRTezosOperationFeeBasis primaryOperationFeeBasis) {
    return tezosFeeBasisCreateInternal (mutezPerKByte,
                                        primaryOperationFeeBasis,
                                        false,
                                        tezosOperationFeeBasisCreateEmpty (TEZOS_OP_REVEAL));
}


extern BRTezosFeeBasis
tezosFeeBasisCreateWithReveal (BRTezosUnitMutez mutezPerKByte,
                               BRTezosOperationFeeBasis primaryOperationFeeBasis,
                               BRTezosOperationFeeBasis revealOperationFeeBasis) {
    return tezosFeeBasisCreateInternal (mutezPerKByte,
                                        primaryOperationFeeBasis,
                                        true,
                                        revealOperationFeeBasis);
}

extern BRTezosFeeBasis
tezosFeeBasisCreateWithFee (BRTezosOperationKind kind,
                            BRTezosUnitMutez fee) {
    return tezosFeeBasisCreate (tezosOperationFeeBasisInvertFee (fee),
                                tezosOperationFeeBasisActual (kind, fee, 0));
}

extern BRTezosFeeBasis
tezosFeeBasisCreateDefault (BRTezosUnitMutez mutexPerKByte, bool isDelegate, bool needsReveal) {
    return tezosFeeBasisCreateInternal (mutexPerKByte,
                                        tezosOperationFeeBasisCreateDefault (isDelegate ? TEZOS_OP_DELEGATION : TEZOS_OP_TRANSACTION),
                                        needsReveal,
                                        (needsReveal
                                         ? tezosOperationFeeBasisCreateDefault (TEZOS_OP_REVEAL)
                                         : tezosOperationFeeBasisCreateEmpty (TEZOS_OP_REVEAL)));
}

#if 0
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
#endif

extern BRTezosUnitMutez
tezosFeeBasisGetFee (BRTezosFeeBasis feeBasis) {
    return (tezosOperationFeeBasisGetTotalFee (&feeBasis.primaryOperationFeeBasis) +
            (feeBasis.hasRevealOperationFeeBasis
             ? tezosOperationFeeBasisGetTotalFee (&feeBasis.revealOperationFeeBasis)
             : 0));
#if 0
    switch (feeBasis.type) {
        case FEE_BASIS_INITIAL:
            return TEZOS_DEFAULT_FEE_MUTEZ;
        case FEE_BASIS_ESTIMATE:
            return feeBasis.u.estimate.calculatedFee;
        case FEE_BASIS_ACTUAL:
            return feeBasis.u.actual.fee;
    }
#endif
}

extern bool
tezosFeeBasisIsEqual (BRTezosFeeBasis *fb1, BRTezosFeeBasis *fb2) {
    assert(fb1);
    assert(fb2);

    return (fb1 == fb2 ||
            (fb1->mutezPerKByte == fb2->mutezPerKByte &&
             tezosOperationFeeBasisEqual (&fb1->primaryOperationFeeBasis, &fb2->primaryOperationFeeBasis) &&
             fb1->hasRevealOperationFeeBasis == fb2->hasRevealOperationFeeBasis &&
             (!fb1->hasRevealOperationFeeBasis ||
              tezosOperationFeeBasisEqual (&fb1->revealOperationFeeBasis, &fb2->revealOperationFeeBasis))));
#if 0
    return (fb1 == fb2 ||
            (fb1->type          == fb2->type          &&
             fb1->fee           == fb2->fee           &&
             fb1->mutezPerKByte == fb2->mutezPerKByte &&
             fb1->sizeInBytes   == fb2->sizeInBytes   &&
             fb1->gasLimit      == fb2->gasLimit      &&
             fb1->storageLimit  == fb2->storageLimit  &&
             fb1->counter       == fb2->counter));
#endif
}

#include <stdio.h>

private_extern void
tezosFeeBasisShow (BRTezosFeeBasis feeBasis, const char *prefix) {
    printf ("%s: FeeBasis\n", prefix);
#if 0
    printf ("%s:    Type         : %s\n", prefix,
            (FEE_BASIS_INITIAL == feeBasis.type
             ? "Initial"
             : (FEE_BASIS_ESTIMATE == feeBasis.type
                ? "Estimate"
                : "Actual")));

    printf ("%s:    Fee          : %llu\n", prefix, feeBasis.fee);
    printf ("%s:    mutexPerKByte: %llu\n", prefix, feeBasis.mutezPerKByte);
    printf ("%s:    sizeInBytes  : %zu\n",  prefix, feeBasis.sizeInBytes);
    printf ("%s:    gasLimit     : %llu\n", prefix, feeBasis.gasLimit);
    printf ("%s:    storageLimit : %llu\n", prefix, feeBasis.storageLimit);
    printf ("%s:    counter      : %llu\n", prefix, feeBasis.counter);
#endif
}


#if 0
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
#endif
