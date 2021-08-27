//
//  BRTezosEncoder.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-07-22.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "support/util/BRUtilMath.h"
#include "support/BRBase58.h"

#include "BRTezosOperation.h"

#if !defined (MAX)
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

// MARK: - Tezos Fee Basis


// https://tezos.gitlab.io/protocols/004_Pt24m4xi.html#gas-and-fees
#define TEZOS_FEE_BASELINE      ((BRTezosUnitMutez)  100)
#define TEZOS_FEE_DEFAULT       ((BRTezosUnitMutez) 1420)

#define TEZOS_TX_SIZE_MARGIN_PERCENTAGE  10
#define TEZOS_TX_SIZE_FEE_RATE           ((BRTezosUnitMutez) 1000)  // mutez/kbyte
#define TEZOS_TX_SIZE_DEFAULT           225                         // bytes - hackily

#define TEZOS_GAS_LIMIT_MARGIN_PERCENTAGE        10
#define TEZOS_GAS_LIMIT_MINIMUM                1000
#define TEZOS_GAS_LIMIT_MAXIMUM             1040000         // gas
#define TEZOS_GAS_LIMIT_DEFAULT_FEE_RATE        0.1         // mutez/gas

#define TEZOS_STORAGE_LIMIT_MARGIN_PERCENTAGE          10
#define TEZOS_STORAGE_LIMIT_MINIMUM                   300                 // sending to inactive accounts
#define TEZOS_STORAGE_LIMIT_MAXIMUM                 60000
#define TEZOS_STORAGE_LIMIT_DEFAULT_FEE_RATE          0.0         // mutez/storage


static inline int64_t
applyMargin (int64_t value, uint8_t marginInPercent) {
    return ((100 + marginInPercent) * value) / 100;
}
 
static BRTezosUnitMutez
tezosOperationFeeBasisComputeFee (BRTezosUnitMutez mutezPerKByte,
                                  size_t sizeInBytes,
                                  int64_t gasLimit,
                                  int64_t storageLimit) {
    BRTezosUnitMutez feeForGasLimit = (int64_t) (TEZOS_GAS_LIMIT_DEFAULT_FEE_RATE     * gasLimit);
    BRTezosUnitMutez feeForStoLimit = (int64_t) (TEZOS_STORAGE_LIMIT_DEFAULT_FEE_RATE * storageLimit);
    BRTezosUnitMutez feeForTxBytes  = (int64_t) (MAX (TEZOS_TX_SIZE_FEE_RATE, mutezPerKByte) * ((ssize_t) sizeInBytes) / 1000);

    // Not really a minimum.  If so, `MAX (MINIMUM, fee)` should be used; but that leads to a fee
    // that Tezos nodes don't effectively accept - rejected from mempool (> 30 minutes).
    return TEZOS_FEE_BASELINE + feeForGasLimit + feeForStoLimit + feeForTxBytes;
}

extern BRTezosUnitMutez // mutezPerKByte
tezosOperationFeeBasisInvertFee (BRTezosUnitMutez fee) {
    return (1000 * fee) / TEZOS_TX_SIZE_DEFAULT;
}

static BRTezosUnitMutez
tezosOperationFeeBasisGetDefaultFee (BRTezosOperationKind kind) {
    switch (kind) {
        case TEZOS_OP_ENDORESEMENT:  return 0;
        case TEZOS_OP_REVEAL:        return TEZOS_FEE_DEFAULT;
        case TEZOS_OP_TRANSACTION:   return TEZOS_FEE_DEFAULT;
        case TEZOS_OP_DELEGATION:    return TEZOS_FEE_DEFAULT;
    }
}

static void
tezosOperationFeeBasisShow (BRTezosOperationFeeBasis feeBasis) {
    printf ("XTZ: FeeBasis\n");
    printf ("    Kind: %d\n", feeBasis.kind);
    printf ("    Fee : %"PRIu64"\n", feeBasis.fee);
    printf ("    GasL: %"PRIu64"\n", feeBasis.gasLimit);
    printf ("    StoL: %"PRIu64"\n", feeBasis.storageLimit);
    printf ("    Cntr: %"PRIu64"\n", feeBasis.counter);
    printf ("    FeeX: %"PRIu64"\n", feeBasis.feeExtra);
}

extern BRTezosOperationFeeBasis
tezosOperationFeeBasisCreate (BRTezosOperationKind kind,
                              BRTezosUnitMutez fee,
                              int64_t gasLimit,
                              int64_t storageLimit,
                              int64_t counter,
                              BRTezosUnitMutez feeExtra) {
    return (BRTezosOperationFeeBasis) {
        kind,
        fee,
        MAX (gasLimit,     TEZOS_GAS_LIMIT_MINIMUM),
        MAX (storageLimit, TEZOS_STORAGE_LIMIT_MINIMUM),
        counter,
        feeExtra
    };
}

extern BRTezosOperationFeeBasis
tezosOperationFeeBasisActual (BRTezosOperationKind kind,
                              BRTezosUnitMutez fee,
                              BRTezosUnitMutez feeExtra) {
    return tezosOperationFeeBasisCreate (kind, fee, 0, 0, 0, feeExtra);
}

extern BRTezosOperationFeeBasis
tezosOperationFeeBasisCreateEmpty (BRTezosOperationKind kind) {
    return tezosOperationFeeBasisCreate (kind, 0, 0, 0, 0, 0);
}

extern BRTezosOperationFeeBasis
tezosOperationFeeBasisCreateDefault (BRTezosOperationKind kind) {
    // Used in the fee estimation as a starting point.
    return tezosOperationFeeBasisCreate (kind,
                                         tezosOperationFeeBasisGetDefaultFee (kind),
                                         TEZOS_GAS_LIMIT_MAXIMUM,
                                         TEZOS_STORAGE_LIMIT_MAXIMUM,
                                         0,
                                         0);
}

extern BRTezosOperationFeeBasis
tezosOperationFeeBasisApplyMargin (BRTezosOperationFeeBasis feeBasis,
                                   BRTezosUnitMutez mutezPerKByte,
                                   size_t sizeInByte,
                                   unsigned int marginInPercentage) {
    // Given a valid, best-estimate `feeBasis`, apply to margin

    int64_t mutezPerKByteWithMargin = applyMargin (mutezPerKByte,         MAX (marginInPercentage, TEZOS_TX_SIZE_MARGIN_PERCENTAGE));
    int64_t gasLimitWithMargin      = applyMargin (feeBasis.gasLimit,     MAX (marginInPercentage, TEZOS_GAS_LIMIT_MARGIN_PERCENTAGE));
    int64_t storageLimitWithMargin  = applyMargin (feeBasis.storageLimit, MAX (marginInPercentage, TEZOS_STORAGE_LIMIT_MARGIN_PERCENTAGE));

    return tezosOperationFeeBasisCreate (feeBasis.kind,
                                         tezosOperationFeeBasisComputeFee (mutezPerKByteWithMargin,
                                                                           sizeInByte,
                                                                           gasLimitWithMargin,
                                                                           storageLimitWithMargin),
                                         gasLimitWithMargin,
                                         feeBasis.storageLimit,
                                         feeBasis.counter,
                                         feeBasis.feeExtra);
}

extern BRTezosUnitMutez
tezosOperationFeeBasisGetTotalFee (const BRTezosOperationFeeBasis *feeBasis) {
    return feeBasis->fee + feeBasis->feeExtra;
}


// MARK: - Tezos Operation

static BRTezosOperation
tezosOperationCreate (BRTezosOperationKind kind,
                      OwnershipGiven BRTezosAddress source,
                      BRTezosOperationFeeBasis feeBasis) {
    BRTezosOperation operation = calloc (1, sizeof (struct BRTezosOperationRecord));

    operation->kind     = kind;
    operation->source   = source;
    operation->feeBasis = feeBasis;

    return operation;
}

extern BRTezosOperation
tezosOperationCreateTransaction (BRTezosAddress source,
                                 BRTezosAddress target,
                                 BRTezosOperationFeeBasis feeBasis,
                                 BRTezosUnitMutez amount) {
    assert (TEZOS_OP_TRANSACTION == feeBasis.kind);
    BRTezosOperation operation = tezosOperationCreate (TEZOS_OP_TRANSACTION,
                                                       tezosAddressClone (source),
                                                       feeBasis);

    operation->data.transaction.target = tezosAddressClone (target);
    operation->data.transaction.amount = amount;

    return operation;
}

extern BRTezosOperation
tezosOperationCreateDelegation (BRTezosAddress source,
                                BRTezosAddress target,
                                BRTezosOperationFeeBasis feeBasis) {
    assert (TEZOS_OP_DELEGATION == feeBasis.kind);
    BRTezosOperation operation = tezosOperationCreate (TEZOS_OP_DELEGATION,
                                                       tezosAddressClone (source),
                                                       feeBasis);

    operation->data.delegation.target = (tezosAddressEqual (source, target) ? NULL : tezosAddressClone (target));

    return operation;
}

extern BRTezosOperation
tezosOperationCreateReveal (BRTezosAddress source,
                            BRTezosAddress target,
                            BRTezosOperationFeeBasis feeBasis,
                            BRTezosPublicKey publicKey) {
    assert (TEZOS_OP_REVEAL == feeBasis.kind);
    BRTezosOperation operation = tezosOperationCreate (TEZOS_OP_REVEAL,
                                                       tezosAddressClone (source),
                                                       feeBasis);

    operation->data.reveal.publicKey = publicKey;

    return operation;
}

extern BRTezosOperation
tezosOperationClone (BRTezosOperation operation) {
    if (NULL == operation) return NULL;

    BRTezosOperation clone = tezosOperationCreate (operation->kind,
                                                   tezosAddressClone(operation->source),
                                                   operation->feeBasis);
    switch (operation->kind) {
        case TEZOS_OP_ENDORESEMENT:
            assert (false);
            break;

        case TEZOS_OP_REVEAL:
            clone->data.reveal.publicKey = operation->data.reveal.publicKey;
            break;

        case TEZOS_OP_TRANSACTION:
            clone->data.transaction.amount = operation->data.transaction.amount;
            clone->data.transaction.target = tezosAddressClone (operation->data.transaction.target);
            break;

        case TEZOS_OP_DELEGATION:
            clone->data.delegation.target = tezosAddressClone (operation->data.delegation.target);
            break;
    }

    return clone;
}

extern void
tezosOperationFree (BRTezosOperation operation) {
    if (NULL == operation) return;
    
    tezosAddressFree (operation->source);

    switch (operation->kind) {
        case TEZOS_OP_ENDORESEMENT:
            break;

        case TEZOS_OP_REVEAL:
            break;

        case TEZOS_OP_TRANSACTION:
            tezosAddressFree (operation->data.transaction.target);
            break;

        case TEZOS_OP_DELEGATION:
            tezosAddressFree (operation->data.delegation.target);
            break;
    }

    memset (operation, 0, sizeof (struct BRTezosOperationRecord));
    free (operation);
}

//extern void
//tezosOperationSetCounter (BRTezosOperation operation, int64_t counter) {
//    operation->counter = counter;
//}

static bool
tezosOperationDataEqual (BRTezosOperation op1,
                         BRTezosOperation op2) {
    switch (op1->kind) {
        case TEZOS_OP_ENDORESEMENT:
            return true;

        case TEZOS_OP_REVEAL:
            return tezosPublicKeyEqual (&op1->data.reveal.publicKey, &op2->data.reveal.publicKey);

        case TEZOS_OP_TRANSACTION:
            return (tezosAddressEqual (op1->data.transaction.target, op2->data.transaction.target) &&
                    op1->data.transaction.amount == op2->data.transaction.amount);

        case TEZOS_OP_DELEGATION:
            return (op1->data.delegation.target == op2->data.delegation.target ||
                    (NULL != op1->data.delegation.target &&
                     NULL != op2->data.delegation.target &&
                     tezosAddressEqual (op1->data.delegation.target,
                                        op2->data.delegation.target)));
    }
}

extern bool
tezosOperationEqual (BRTezosOperation op1,
                     BRTezosOperation op2) {
    return (op1 == op2 ||
            (NULL != op1 &&
             NULL != op2 &&
             op1->kind    == op2->kind    &&
             tezosAddressEqual           ( op1->source,    op2->source)   &&
             tezosOperationFeeBasisEqual (&op1->feeBasis, &op2->feeBasis) &&
             tezosOperationDataEqual (op1, op2)));
}

// MARK: - Tezos Serialization

static BRData
encodeAddress (BRTezosAddress address) {
    // address bytes with 3-byte TZx prefix replaced with a 1-byte prefix

    assert (address);
    assert (tezosAddressIsImplicit (address));

    size_t len = tezosAddressGetRawSize (address);
    uint8_t bytes[len];
    tezosAddressGetRawBytes (address, bytes, len);

    uint8_t prefix;

    if (0 == memcmp(bytes, TZ1_PREFIX, sizeof(TZ1_PREFIX)))
        prefix = 0x00;
    else if (0 == memcmp(bytes, TZ2_PREFIX, sizeof(TZ2_PREFIX)))
        prefix = 0x01;
    else if (0 == memcmp(bytes, TZ3_PREFIX, sizeof(TZ1_PREFIX)))
        prefix = 0x02;
    else
        assert(0);

    uint8_t encoded[len-2];
    memcpy(&encoded[0], &prefix, 1);
    memcpy(&encoded[1], &bytes[3], len-3);

    return dataCopy (encoded, sizeof(encoded));
}

static BRData
encodePublicKey (BRTezosPublicKey pubKey) {
    uint8_t prefix = 0x0; // ed25519
    BRData encoded = dataNew(TEZOS_PUBLIC_KEY_SIZE + 1);
    memcpy(encoded.bytes, &prefix, 1);
    memcpy(&encoded.bytes[1], pubKey.bytes, TEZOS_PUBLIC_KEY_SIZE);
    return encoded;
}

extern BRData
encodeZarith (int64_t value) {
    assert (value >= 0);
    uint8_t result[32] = {0};
    size_t resultSize = 0;

    uint64_t input = (uint64_t)value;
    while (input >= 0x80) {
        result[resultSize++] = (uint8_t)((input & 0xff) | 0x80);
        input >>= 7;
    }
    result[resultSize++] = (uint8_t)input;

    return dataCopy(&result[0], resultSize);
}

static BRData
encodeBool (bool value) {
    BRData encoded = dataNew(1);
    encoded.bytes[0] = value ? 0xff : 0x00;
    return encoded;
}

static BRData
encodeOperationKind (BRTezosOperationKind kind) {
    uint8_t bytes[1] = { kind };
    return dataCopy (bytes, 1);
}

static BRData
encodeBranch (BRTezosHash blockHash) {
    // omit prefix
    size_t numPrefixBytes = 2;
    size_t branchSize = sizeof(blockHash.bytes) - numPrefixBytes;

    BRData branchData = dataNew(branchSize);
    memcpy(branchData.bytes, &blockHash.bytes[numPrefixBytes], branchSize);

    return branchData;
}


extern BRData
tezosOperationSerialize (BRTezosOperation op) {
    assert (op);

    size_t maxFields = 10;
    BRData fields[maxFields];
    size_t numFields = 0;
    fields[numFields++] = encodeOperationKind (op->kind);
    fields[numFields++] = encodeAddress (op->source);
    fields[numFields++] = encodeZarith  (op->feeBasis.fee);
    fields[numFields++] = encodeZarith  (op->feeBasis.counter);
    fields[numFields++] = encodeZarith  (op->feeBasis.gasLimit);
    fields[numFields++] = encodeZarith  (op->feeBasis.storageLimit);

#if 1
    printf ("XTZ: Serialize Tx\n");
    printf ("XTZ:    Type    : %s\n",   tezosOperationKindDescription(op->kind));
    tezosOperationFeeBasisShow (op->feeBasis);
#endif
    switch (op->kind) {
        case TEZOS_OP_ENDORESEMENT:
            assert (false);
            break;

        case TEZOS_OP_REVEAL:
            fields[numFields++] = encodePublicKey(op->data.reveal.publicKey);
            break;

        case TEZOS_OP_TRANSACTION:
            fields[numFields++] = encodeZarith   (op->data.transaction.amount);
            fields[numFields++] = encodeBool    (false); // TODO: Support KT Address; false -> not originated (KT) address
            fields[numFields++] = encodeAddress (op->data.transaction.target);
            fields[numFields++] = encodeBool    (false); // contract execution params (0x0 for no params)
            break;

        case TEZOS_OP_DELEGATION:
            if (NULL != op->data.delegation.target) {
                fields[numFields++] = encodeBool    (true); // set delegate
                fields[numFields++] = encodeAddress (op->data.delegation.target);
            }
            else {
                fields[numFields++] = encodeBool(false); // remove delegate
            }
            break;
    }

    BRData serialized = dataConcat(fields, numFields);
    
    for (int i=0; i < numFields; i++) {
        dataFree(fields[i]);
    }
    
    return serialized;
}

extern BRData
tezosOperationSerializeList (BRTezosOperation * ops,
                             size_t opsCount,
                             BRTezosHash blockHash) {
    
    BRData fields[opsCount + 1];
    size_t numFields = 0;
    
    // operation list = branch + [reveal op bytes] + transaction/delegation op bytes
    
    fields[numFields++] = encodeBranch(blockHash);

    printf ("XTZ: Serialize Operation List\n");
    for (int i=0; i < opsCount; i++) {
        fields[numFields++] = tezosOperationSerialize (ops[i]);
    }
    
    BRData serialized = dataConcat(fields, numFields);
    
    for (int i=0; i < numFields; i++) {
        dataFree(fields[i]);
    }

    printf ("XTZ: Serialize TX Count: %zu\n", opsCount);
    
    return serialized;
}
