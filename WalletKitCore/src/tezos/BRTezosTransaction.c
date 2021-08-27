//
//  BRTezosTransaction.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRTezosTransaction.h"
#include "ed25519/ed25519.h"
#include "blake2/blake2b.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>

#define TEZOS_SIGNATURE_BYTES 64

struct BRTezosTransactionRecord {
    
    BRTezosHash hash;

    BRTezosOperation primaryOperation;
    Nullable BRTezosOperation revealOperation;

    const char * protocol; // protocol name
    const char * branch; // hash of head block
    
    BRData signedBytes;
};

struct BRTezosSignatureRecord {
    uint8_t signature[TEZOS_SIGNATURE_BYTES];
};

static BRTezosTransaction
tezosTransactionCreateInternal (BRTezosHash hash,
                                OwnershipGiven BRTezosOperation primaryOperation,
                                Nullable OwnershipGiven BRTezosOperation revealOperation) {
    assert (NULL == revealOperation || TEZOS_OP_REVEAL == revealOperation->kind);

    BRTezosTransaction transaction = calloc (1, sizeof (struct BRTezosTransactionRecord));

    transaction->hash = hash;
    transaction->primaryOperation = primaryOperation;
    transaction->revealOperation  = revealOperation;

    transaction->protocol = NULL;
    transaction->branch   = NULL;

    transaction->signedBytes = dataCreateEmpty();
    
    return transaction;
}

extern BRTezosTransaction
tezosTransactionCreate (OwnershipGiven BRTezosOperation operation) {
    return tezosTransactionCreateInternal (TEZOS_HASH_EMPTY, operation, NULL);
}

extern BRTezosTransaction
tezosTransactionCreateWithReveal (OwnershipGiven BRTezosOperation operation,
                                  OwnershipGiven BRTezosOperation revel) {
    return tezosTransactionCreateInternal(TEZOS_HASH_EMPTY, operation, revel);
}


extern BRTezosTransaction
tezosTransactionClone (BRTezosTransaction transaction) {
    BRTezosTransaction result  = tezosTransactionCreateInternal (transaction->hash,
                                                                 tezosOperationClone (transaction->primaryOperation),
                                                                 tezosOperationClone (transaction->revealOperation));

    result->protocol = transaction->protocol;
    result->branch   = transaction->branch;

    result->signedBytes = dataClone (transaction->signedBytes);

    return result;
}

extern void tezosTransactionFree (BRTezosTransaction transaction)
{
    tezosOperationFree (transaction->primaryOperation);
    tezosOperationFree (transaction->revealOperation);

    transaction->protocol = NULL;
    transaction->branch   = NULL;

    dataFree (transaction->signedBytes);

    memset (transaction, 0, sizeof (struct BRTezosTransactionRecord));
    free (transaction);
}

extern OwnershipKept BRTezosOperation
tezosTransactionGetPrimaryOperation (BRTezosTransaction transaction) {
    return transaction->primaryOperation;
}

extern OwnershipKept BRTezosOperation
tezosTransactionGetRevealOperation (BRTezosTransaction transaction) {
    return transaction->revealOperation;
}

extern bool
tezosTransactionHasReveal (BRTezosTransaction transaction) {
    return NULL != transaction->revealOperation;
}

static void
tezosTransactionAssignHash(BRTezosTransaction tx) {
    assert(tx->signedBytes.size);
    
    uint8_t hash[32];
    blake2b(hash, sizeof(hash), NULL, 0, tx->signedBytes.bytes, tx->signedBytes.size);
    
    uint8_t prefix[] = { 5, 116 }; // operation prefix
    memcpy(tx->hash.bytes, prefix, sizeof(prefix));
    memcpy(&(tx->hash.bytes[sizeof(prefix)]), hash, sizeof(hash));
}

static BRData
tezosTransactionSerialize (BRTezosTransaction transaction,
                           BRTezosHash lastBlockHash) {
    BRTezosOperation operations[2] = {
        transaction->revealOperation,           // must be first, if used
        transaction->primaryOperation,
    };
    size_t operationsCount = (NULL != transaction->revealOperation ? 2 : 1);
    size_t operationsIndex = (NULL != transaction->revealOperation ? 0 : 1);

    return tezosOperationSerializeList (&operations[operationsIndex], operationsCount, lastBlockHash);
}

extern void
tezosTransactionSerializeForFeeEstimation (BRTezosTransaction transaction,
                                           BRTezosAccount account,
                                           BRTezosHash lastBlockHash) {
    assert (transaction);
    assert (account);
    
    dataFree(transaction->signedBytes);
    
    BRData unsignedBytes = tezosTransactionSerialize(transaction, lastBlockHash);
    BRData signature     = dataNew(TEZOS_SIGNATURE_BYTES); // empty signature

    transaction->signedBytes = dataConcatTwo (unsignedBytes, signature);

    dataFree (unsignedBytes);
    dataFree (signature);

    //    tezosFeeBasisShow (transaction->feeBasis, "XTZ SerializeForFeeEstimation");
}

extern void
tezosTransactionSerializeAndSign (BRTezosTransaction transaction,
                                  BRTezosAccount account,
                                  UInt512 seed,
                                  BRTezosHash lastBlockHash) {
    assert (transaction);
    assert (account);
    
    dataFree(transaction->signedBytes);
    
    BRData unsignedBytes = tezosTransactionSerialize (transaction, lastBlockHash);
    BRData signature     = tezosAccountSignData(account, unsignedBytes, seed);
    assert(TEZOS_SIGNATURE_BYTES == signature.size);

    transaction->signedBytes = dataConcatTwo (unsignedBytes, signature);

    if (transaction->signedBytes.size > 0) {
        tezosTransactionAssignHash(transaction);
    }
}

extern OwnershipKept uint8_t *
tezosTransactionGetSignedBytes (BRTezosTransaction transaction, size_t *size) {
    *size = transaction->signedBytes.size;
    return transaction->signedBytes.bytes;
}

extern size_t
tezosTransactionGetSignedBytesCount (BRTezosTransaction transaction) {
    return transaction->signedBytes.size;
}

extern BRTezosHash tezosTransactionGetHash(BRTezosTransaction transaction){
    assert(transaction);
    return transaction->hash;
}

extern bool
tezosTransactionRequiresReveal (BRTezosTransaction transaction) {
    switch (transaction->primaryOperation->kind) {
        case TEZOS_OP_ENDORESEMENT: return false;  // unsupported
        case TEZOS_OP_REVEAL:       return false;
        case TEZOS_OP_TRANSACTION:  return true;
        case TEZOS_OP_DELEGATION:   return true;
    }
}

extern bool tezosTransactionEqual (BRTezosTransaction t1, BRTezosTransaction t2){
    assert(t1);
    assert(t2);

    return tezosOperationEqual (t1->primaryOperation,
                                t2->primaryOperation);
#if 0
    // Equal means the same transaction id, source, target
    bool result = false;


    // Transaction IDs are not available - use the hash
    BRTezosHash hash1 = tezosTransactionGetHash(t1);
    BRTezosHash hash2 = tezosTransactionGetHash(t2);
    if (memcmp(hash1.bytes, hash2.bytes, sizeof(hash1.bytes)) == 0) {
        // Hash is the same - compare the source
        BRTezosAddress source1 = tezosTransactionGetSource(t1);
        BRTezosAddress source2 = tezosTransactionGetSource(t2);
        if (1 == tezosAddressEqual(source1, source2)) {
            // OK - compare the target
            BRTezosAddress target1 = tezosTransactionGetTarget(t1);
            BRTezosAddress target2 = tezosTransactionGetTarget(t2);
            if (1 == tezosAddressEqual(target1, target2)) {
                result = true;
            }
            tezosAddressFree(target1);
            tezosAddressFree(target2);
        }
        tezosAddressFree (source1);
        tezosAddressFree (source2);
    }
    return result;
#endif
}

