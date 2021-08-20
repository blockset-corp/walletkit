//
//  BRAvalancheTransaction.c
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRAvalancheTransaction.h"
#include "ed25519/ed25519.h"
#include "blake2/blake2b.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>

// MARK: - Attributes

extern const char **
avalancheTransactionGetAttributeKeys (bool asRequired, size_t *count) {
    static size_t requiredCount = 0;
    static const char **requiredNames = NULL;

    static size_t optionalCount = 3;
    static const char **optionalNames = NULL;
#if 0
    {
        FIELD_OPTION_DELEGATION_OP,
        FIELD_OPTION_DELEGATE,
        FIELD_OPTION_OPERATION_TYPE
    };
#endif

    if (asRequired) { *count = requiredCount; return requiredNames; }
    else {            *count = optionalCount; return optionalNames; }
}


// MARK - Transaction
static BRAvalancheTransaction
avalancheTransactionCreateTransactionInternal (BRAvalancheAddress source,
                                              BRAvalancheAddress target,
                                              BRAvalancheAmount amount,
                                              BRAvalancheFeeBasis feeBasis,
                                              BRAvalancheHash hash,
                                              BRAvalancheSignature signature) {
    BRAvalancheTransaction transaction = calloc (1, sizeof(struct BRAvalancheTransactionRecord));

    transaction->source = source;
    transaction->target = target;

    transaction->amount   = amount;
    transaction->feeBasis = feeBasis;

    transaction->hash = AVALANCHE_HASH_EMPTY;
    transaction->signature = ((BRAvalancheSignature) { 0 });

    return transaction;
}

extern BRAvalancheTransaction
avalancheTransactionCreate (BRAvalancheAddress source,
                           BRAvalancheAddress target,
                           BRAvalancheAmount amount,
                           BRAvalancheFeeBasis feeBasis) {
    return avalancheTransactionCreateTransactionInternal (source,
                                                         target,
                                                         amount,
                                                         feeBasis,
                                                         ((BRAvalancheHash) { 0 }),
                                                         ((BRAvalancheSignature) { 0 }));
}

extern BRAvalancheTransaction
avalancheTransactionClone (BRAvalancheTransaction transaction) {
    assert(transaction);

    return avalancheTransactionCreateTransactionInternal (transaction->source,
                                                         transaction->target,
                                                         transaction->amount,
                                                         transaction->feeBasis,
                                                         transaction->hash,
                                                         transaction->signature);
}

extern void
avalancheTransactionFree (BRAvalancheTransaction transaction)
{
    assert (transaction);

    ASSERT_UNIMPLEMENTED;

    memset (transaction, 0, sizeof (struct BRAvalancheTransactionRecord));
    free (transaction);
}


extern OwnershipGiven uint8_t *
avalancheTransactionSerializeForFeeEstimation (BRAvalancheTransaction transaction,
                                              BRAvalancheAccount account,
                                              size_t *count) {
    ASSERT_UNIMPLEMENTED;

    *count = 0;
    return NULL;
}

extern OwnershipGiven uint8_t *
avalancheTransactionSerializeForSubmission (BRAvalancheTransaction transaction,
                                           BRAvalancheAccount account,
                                           UInt512 seed,
                                           size_t *count) {
    ASSERT_UNIMPLEMENTED;

     // Fill in the serialization

    // Fill in the hash, if possible

   *count = 0;
    return NULL;
}

extern  OwnershipGiven uint8_t *
avalancheTransactionGetSerialization (BRAvalancheTransaction transaction,
                                     size_t *count) {
    uint8_t *serialization = NULL;

    if (NULL != transaction->serialization) {
        serialization = malloc (transaction->serializationCount);
        memcpy (serialization, transaction->serialization, transaction->serializationCount);
    }

    *count = transaction->serializationCount;
    return serialization;

}

extern BRAvalancheHash
avalancheTransactionGetHash(BRAvalancheTransaction transaction){
    assert(transaction);
    return transaction->hash;
}

extern BRAvalancheFeeBasis
avalancheTransactionGetFeeBasis(BRAvalancheTransaction transaction) {
    assert(transaction);
    return transaction->feeBasis;
}

extern void
avalancheTransactionSetFeeBasis(BRAvalancheTransaction transaction,
                                        BRAvalancheFeeBasis feeBasis) {
    assert(transaction);
    transaction->feeBasis = feeBasis;
}

extern BRAvalancheAmount
avalancheTransactionGetAmount(BRAvalancheTransaction transaction){
    assert(transaction);
    return transaction->amount;
}

extern BRAvalancheAddress
avalancheTransactionGetSource(BRAvalancheTransaction transaction){
    assert(transaction);
    return transaction->source;
}

extern BRAvalancheAddress
avalancheTransactionGetTarget(BRAvalancheTransaction transaction){
    assert(transaction);
    return transaction->target;
}

extern bool
avalancheTransactionEqual (BRAvalancheTransaction t1, BRAvalancheTransaction t2) {
    assert(t1);
    assert(t2);
    // Equal means the same transaction id, source, target
    ASSERT_UNIMPLEMENTED;
    return false;
}

