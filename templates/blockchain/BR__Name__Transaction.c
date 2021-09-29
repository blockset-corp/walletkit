//
//  BR__Name__Transaction.c
//  WalletKitCore
//
//  Created by __USER__ on __DATE__.
//  Copyright © __YEAR__ Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BR__Name__Transaction.h"
#include "ed25519/ed25519.h"
#include "blake2/blake2b.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>

// MARK: - Attributes

extern const char **
__name__TransactionGetAttributeKeys (bool asRequired, size_t *count) {
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
static BR__Name__Transaction
__name__TransactionCreateTransactionInternal (BR__Name__Address source,
                                              BR__Name__Address target,
                                              BR__Name__Amount amount,
                                              BR__Name__FeeBasis feeBasis,
                                              BR__Name__Hash hash,
                                              BR__Name__Signature signature) {
    BR__Name__Transaction transaction = calloc (1, sizeof(struct BR__Name__TransactionRecord));

    transaction->source = __name__AddressClone (source);
    transaction->target = __name__AddressClone (target);

    transaction->amount   = amount;
    transaction->feeBasis = feeBasis;

    transaction->hash = __NAME___HASH_EMPTY;
    transaction->signature = ((BR__Name__Signature) { 0 });

    return transaction;
}

extern BR__Name__Transaction
__name__TransactionCreate (BR__Name__Address source,
                           BR__Name__Address target,
                           BR__Name__Amount amount,
                           BR__Name__FeeBasis feeBasis) {
    return __name__TransactionCreateTransactionInternal (source,
                                                         target,
                                                         amount,
                                                         feeBasis,
                                                         ((BR__Name__Hash) { 0 }),
                                                         ((BR__Name__Signature) { 0 }));
}

extern BR__Name__Transaction
__name__TransactionClone (BR__Name__Transaction transaction) {
    assert(transaction);

    return __name__TransactionCreateTransactionInternal (transaction->source,
                                                         transaction->target,
                                                         transaction->amount,
                                                         transaction->feeBasis,
                                                         transaction->hash,
                                                         transaction->signature);
}

extern void
__name__TransactionFree (BR__Name__Transaction transaction)
{
    assert (transaction);

    __name__AddressFree (transaction->source);
    __name__AddressFree (transaction->target);

    ASSERT_UNIMPLEMENTED;

    memset (transaction, 0, sizeof (struct BR__Name__TransactionRecord));
    free (transaction);
}


extern OwnershipGiven uint8_t *
__name__TransactionSerializeForFeeEstimation (BR__Name__Transaction transaction,
                                              BR__Name__Account account,
                                              size_t *count) {
    ASSERT_UNIMPLEMENTED;

    *count = 0;
    return NULL;
}

extern OwnershipGiven uint8_t *
__name__TransactionSerializeForSubmission (BR__Name__Transaction transaction,
                                           BR__Name__Account account,
                                           UInt512 seed,
                                           size_t *count) {
    ASSERT_UNIMPLEMENTED;

     // Fill in the serialization

    // Fill in the hash, if possible

   *count = 0;
    return NULL;
}

extern  OwnershipGiven uint8_t *
__name__TransactionGetSerialization (BR__Name__Transaction transaction,
                                     size_t *count) {
    uint8_t *serialization = NULL;

    if (NULL != transaction->serialization) {
        serialization = malloc (transaction->serializationCount);
        memcpy (serialization, transaction->serialization, transaction->serializationCount);
    }

    *count = transaction->serializationCount;
    return serialization;

}

extern BR__Name__Hash
__name__TransactionGetHash(BR__Name__Transaction transaction){
    assert(transaction);
    return transaction->hash;
}

extern BR__Name__FeeBasis
__name__TransactionGetFeeBasis(BR__Name__Transaction transaction) {
    assert(transaction);
    return transaction->feeBasis;
}

extern void
__name__TransactionSetFeeBasis(BR__Name__Transaction transaction,
                                        BR__Name__FeeBasis feeBasis) {
    assert(transaction);
    transaction->feeBasis = feeBasis;
}

extern BR__Name__Amount
__name__TransactionGetAmount(BR__Name__Transaction transaction){
    assert(transaction);
    return transaction->amount;
}

extern BR__Name__Address
__name__TransactionGetSource(BR__Name__Transaction transaction){
    assert(transaction);
    return __name__AddressClone (transaction->source);
}

extern BR__Name__Address
__name__TransactionGetTarget(BR__Name__Transaction transaction){
    assert(transaction);
    return __name__AddressClone (transaction->target);
}

extern bool
__name__TransactionEqual (BR__Name__Transaction t1, BR__Name__Transaction t2) {
    assert(t1);
    assert(t2);
    // Equal means the same transaction id, source, target
    ASSERT_UNIMPLEMENTED;
    return false;
}

