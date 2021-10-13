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
#include <stdio.h>

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

extern BRTezosUnitMutez
tezosFeeBasisGetFee (BRTezosFeeBasis feeBasis) {
    return (tezosOperationFeeBasisGetTotalFee (&feeBasis.primaryOperationFeeBasis) +
            (feeBasis.hasRevealOperationFeeBasis
             ? tezosOperationFeeBasisGetTotalFee (&feeBasis.revealOperationFeeBasis)
             : 0));
}

extern BRTezosFeeBasis
tezosFeeBasisGiveTezosAGift (BRTezosFeeBasis feeBasis, unsigned int marginPercentage) {
    BRTezosOperationFeeBasis primaryOperationFeeBasis = feeBasis.primaryOperationFeeBasis;
    primaryOperationFeeBasis.fee = (primaryOperationFeeBasis.fee * (marginPercentage + 100)) / 100;

    return (BRTezosFeeBasis) {
        feeBasis.mutezPerKByte,
        primaryOperationFeeBasis,
        feeBasis.hasRevealOperationFeeBasis,
        feeBasis.revealOperationFeeBasis
    };
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

