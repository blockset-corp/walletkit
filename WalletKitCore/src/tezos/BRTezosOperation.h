//
//  BRTezosEncoder.h
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-07-22.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRTezosEncoder_h
#define BRTezosEncoder_h

#include <stdint.h>
#include <assert.h>
#include "BRTezosBase.h"
#include "BRTezosAddress.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Tezos Operation Fee Basis

typedef struct {
    /// The kind deteremines, in part, which operation this feeBasis applies to.
    BRTezosOperationKind kind;

    /// The fee for including the operation in the blockchain.
    BRTezosUnitMutez fee;

    /// The gasLimit applicable to the operation; bounds the blockchain computation.
    int64_t gasLimit;

    /// The storageLimit applicable to the operations; bounds the blockchain storage.
    int64_t storageLimit;

    /// The counter (aka nonce) for the operation.
    int64_t counter;

    /// Any extra feess which for Tezos might be the 'burn_fee' (a fee the source pays for the
    /// pleasure of sending funds to a target w/o having received funds previously).  These fees
    /// are not included in the serialization of an operation; these fees are displayed to the
    /// User for confirmation prior to submit (see tezosOperationFeeBasisGetTotalFee())
    BRTezosUnitMutez feeExtra;

} BRTezosOperationFeeBasis;

static inline bool
tezosOperationFeeBasisEqual (const BRTezosOperationFeeBasis *fb1,
                             const BRTezosOperationFeeBasis *fb2) {
    return (fb1->kind         == fb2->kind         &&
            fb1->fee          == fb2->fee          &&
            fb1->gasLimit     == fb2->gasLimit     &&
            fb1->storageLimit == fb2->storageLimit &&
            fb1->counter      == fb2->counter      &&
            fb1->feeExtra     == fb2->feeExtra);
}

extern BRTezosOperationFeeBasis
tezosOperationFeeBasisCreate (BRTezosOperationKind kind,
                              BRTezosUnitMutez fee,
                              int64_t gasLimit,
                              int64_t storageLimit,
                              int64_t counter,
                              BRTezosUnitMutez feeExtra);

extern BRTezosOperationFeeBasis
tezosOperationFeeBasisActual (BRTezosOperationKind kind,
                              BRTezosUnitMutez fee,
                              BRTezosUnitMutez feeExtra);

extern BRTezosOperationFeeBasis
tezosOperationFeeBasisCreateEmpty (BRTezosOperationKind kind);

extern BRTezosOperationFeeBasis
tezosOperationFeeBasisCreateDefault (BRTezosOperationKind kind);

extern BRTezosOperationFeeBasis
tezosOperationFeeBasisApplyMargin (BRTezosOperationFeeBasis feeBasis,
                                   BRTezosUnitMutez mutezPerKByte,
                                   size_t sizeInByte,
                                   unsigned int marginInPercentage);

extern BRTezosUnitMutez
tezosOperationFeeBasisGetTotalFee (const BRTezosOperationFeeBasis *feeBasis);

extern BRTezosUnitMutez
tezosOperationFeeBasisComputeMinimalFee (BRTezosUnitMutez mutezPerKByte,
                                         size_t sizeInBytes,
                                         int64_t gasLimit);

extern BRTezosUnitMutez
tezosOperationFeeBasisApplyMarginToFee (BRTezosUnitMutez fee);

extern BRTezosUnitMutez // mutezPerKByte
tezosOperationFeeBasisInvertFee (BRTezosUnitMutez fee);

// MARK: Tezos Operation

typedef struct BRTezosOperationRecord {
    BRTezosOperationKind kind;

    BRTezosAddress source;
    BRTezosOperationFeeBasis feeBasis;
//    int64_t counter;

    union {
        struct {
            BRTezosAddress target;
            BRTezosUnitMutez amount;
        } transaction;

        struct {
            BRTezosAddress target;
        } delegation;

        struct {
            BRTezosPublicKey publicKey;
        } reveal;
    } data;
} *BRTezosOperation;

extern BRTezosOperation
tezosOperationCreateTransaction (BRTezosAddress source,
                                 BRTezosAddress target,
                                 BRTezosOperationFeeBasis feeBasis,
                                 BRTezosUnitMutez amount);

extern BRTezosOperation
tezosOperationCreateDelegation (BRTezosAddress source,
                                BRTezosAddress target,
                                BRTezosOperationFeeBasis feeBasis);

extern BRTezosOperation
tezosOperationCreateReveal (BRTezosAddress source,
                            BRTezosAddress target,
                            BRTezosOperationFeeBasis feeBasis,
                            BRTezosPublicKey publicKey);

extern void
tezosOperationFree (BRTezosOperation operation);

//extern void
//tezosOperationSetCounter (BRTezosOperation operation, int64_t counter);

extern WKData
tezosOperationSerialize (BRTezosOperation operation);

extern WKData
tezosOperationSerializeList (BRTezosOperation * operations,
                             size_t operationsCount,
                             BRTezosHash blockHash);

extern bool
tezosOperationEqual (BRTezosOperation op1,
                     BRTezosOperation op2);

#ifdef __cplusplus
}
#endif

#endif /* BRTezosEncoder_h */
