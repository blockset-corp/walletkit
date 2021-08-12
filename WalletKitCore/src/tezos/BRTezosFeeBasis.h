//
//  BRTezosFeeBasis.h
//  WalletKitCore
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
#include "BRTezosOperation.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TEZOS_DEFAULT_MUTEZ_PER_BYTE 1

typedef struct
{
    BRTezosUnitMutez    mutezPerKByte;

    BRTezosOperationFeeBasis primaryOperationFeeBasis;

    bool hasRevealOperationFeeBasis;
    BRTezosOperationFeeBasis revealOperationFeeBasis;
} BRTezosFeeBasis;

extern BRTezosFeeBasis
tezosFeeBasisCreate (BRTezosUnitMutez mutezPerKByte,
                     BRTezosOperationFeeBasis primaryOperationFeeBasis);

extern BRTezosFeeBasis
tezosFeeBasisCreateWithReveal (BRTezosUnitMutez mutezPerKByte,
                               BRTezosOperationFeeBasis primaryOperationFeeBasis,
                               BRTezosOperationFeeBasis revealOperationFeeBasis);

extern BRTezosFeeBasis
tezosFeeBasisCreateWithFee (BRTezosOperationKind kind,
                            BRTezosUnitMutez fee);

extern BRTezosFeeBasis
tezosFeeBasisCreateDefault (BRTezosUnitMutez mutexPerKByte, bool isDelegate, bool needsReveal);

extern BRTezosUnitMutez
tezosFeeBasisGetFee(BRTezosFeeBasis feeBasis);

#if 0
private_extern int64_t
tezosFeeBasisGetGasLimit(BRTezosFeeBasis feeBasis);

private_extern int64_t
tezosFeeBasisGetStorageLimit(BRTezosFeeBasis feeBasis);
#endif

extern bool
tezosFeeBasisIsEqual(BRTezosFeeBasis *fb1, BRTezosFeeBasis *fb2);

#ifdef __cplusplus
}
#endif

#endif /* BRTezosFeeBasis_h */
