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

#define __NAME___SIGNATURE_BYTES 64

WKData
__name__SerializeTransaction (BR__Name__Transaction tx) {
    WKData data = {0};
    return data;
}

WKData
__name__SerializeOperationList (
        BR__Name__Transaction * tx,
        size_t txCount,
        BR__Name__Hash blockHash) {
    WKData data = {0};
    return data;
}

struct BR__Name__TransactionRecord {
    
    BR__Name__Hash hash;
    BR__Name__Address source;
    BR__Name__FeeBasis feeBasis;
    
    BR__Name__OperationData operation;
    
    uint64_t blockHeight;
    int64_t counter; // nonce
    
    const char * protocol; // protocol name
    const char * branch; // hash of head block
    
    WKData signedBytes;
};

struct BR__Name__SignatureRecord {
    uint8_t signature[__NAME___SIGNATURE_BYTES];
};

static BR__Name__Transaction
createTransactionObject() {
    BR__Name__Transaction transaction = calloc (1, sizeof(struct BR__Name__TransactionRecord));
    assert(transaction);
    return transaction;
}

extern BR__Name__Transaction
__name__TransactionCreateTransaction (BR__Name__Address source,
                                   BR__Name__Address target,
                                   BR__Name__UnitMutez amount,
                                   BR__Name__FeeBasis feeBasis,
                                   int64_t counter) {
    BR__Name__Transaction transaction = createTransactionObject();
    transaction->hash   = __NAME___HASH_EMPTY;
    transaction->source = __name__AddressClone (source);
    transaction->operation.kind = __NAME___OP_TRANSACTION;
    transaction->operation.u.transaction.amount = amount;
    transaction->operation.u.transaction.target = __name__AddressClone (target);
    transaction->feeBasis = feeBasis;
    transaction->blockHeight = 0;
    transaction->counter = counter;
    return transaction;
}

extern BR__Name__Transaction
__name__TransactionCreateDelegation (BR__Name__Address source,
                                  BR__Name__Address target,
                                  BR__Name__FeeBasis feeBasis,
                                  int64_t counter) {
    BR__Name__Transaction transaction = createTransactionObject();
    transaction->hash   = __NAME___HASH_EMPTY;
    transaction->source = __name__AddressClone (source);
    transaction->operation.kind = __NAME___OP_DELEGATION;
    bool endDelegation = __name__AddressEqual(source, target);
    transaction->operation.u.delegation.target = endDelegation ? NULL : __name__AddressClone (target);
    transaction->feeBasis = feeBasis;
    transaction->blockHeight = 0;
    transaction->counter = counter;
    return transaction;
}

extern BR__Name__Transaction
__name__TransactionCreateReveal (BR__Name__Address source,
                              uint8_t * pubKey,
                              BR__Name__FeeBasis feeBasis,
                              int64_t counter) {
    BR__Name__Transaction transaction = createTransactionObject();
    transaction->hash   = __NAME___HASH_EMPTY;
    transaction->source = __name__AddressClone (source);
    transaction->operation.kind = __NAME___OP_REVEAL;
    memcpy(transaction->operation.u.reveal.publicKey, pubKey, __NAME___PUBLIC_KEY_SIZE);
    transaction->feeBasis = feeBasis;
    transaction->blockHeight = 0;
    transaction->counter = counter;
    return transaction;
}

extern BR__Name__Transaction
__name__TransactionClone (BR__Name__Transaction transaction) {
    assert(transaction);
    BR__Name__Transaction newTransaction = calloc (1, sizeof(struct BR__Name__TransactionRecord));
    newTransaction->source = __name__TransactionGetSource(transaction);
    newTransaction->operation = __name__TransactionGetOperationData (transaction);
    newTransaction->feeBasis = transaction->feeBasis;
    newTransaction->hash = transaction->hash;
    newTransaction->blockHeight = transaction->blockHeight;
    return newTransaction;
}

extern void __name__TransactionFree (BR__Name__Transaction transaction)
{
    assert (transaction);
    __name__AddressFree (transaction->source);
    wkDataFree(transaction->signedBytes);
    switch (transaction->operation.kind) {
        case __NAME___OP_REVEAL:
            break;
            
        case __NAME___OP_TRANSACTION:
            __name__AddressFree(transaction->operation.u.transaction.target);
            break;
            
        case __NAME___OP_DELEGATION:
            __name__AddressFree(transaction->operation.u.delegation.target);
            break;
        default:
            break;
    }
    free (transaction);
}

static void
createTransactionHash(BR__Name__Transaction tx) {
    assert(tx->signedBytes.size);
    
    uint8_t hash[32];
    blake2b(hash, sizeof(hash), NULL, 0, tx->signedBytes.bytes, tx->signedBytes.size);
    
    uint8_t prefix[] = { 5, 116 }; // operation prefix
    memcpy(tx->hash.bytes, prefix, sizeof(prefix));
    memcpy(&(tx->hash.bytes[sizeof(prefix)]), hash, sizeof(hash));
}

static WKData
__name__TransactionSerialize (BR__Name__Transaction transaction,
                           BR__Name__Account account,
                           BR__Name__Hash lastBlockHash,
                           bool needsReveal) {
    BR__Name__Transaction opList[2];
    size_t opCount = 0;
    
    // add a reveal operation to the operation list if needed
    if (needsReveal) {
        BRKey publicKey = __name__AccountGetPublicKey(account);

        // The 'reveal' operation takes the transaction's feeBasis.
        BR__Name__FeeBasis revealFeeBasis = transaction->feeBasis;

        // If this is a submit, we need to modifiy the reveal operation's feeBasis.
        if (FEE_BASIS_ESTIMATE == transaction->feeBasis.type) {
            assert (0 != transaction->feeBasis.u.estimate.gasLimit);

            // When a 'reveal' operation is needed, we are going to double the transaction's
            // gasLimit *and* calculated fee.  This is 'belt and suspenders' - ensuring the
            // operation succeeds.
            transaction->feeBasis.u.estimate.gasLimit      *= 2;
            transaction->feeBasis.u.estimate.calculatedFee *= 2;

            // The 'reveal' operation itself will duplicate the transaction's feeBasis except the
            // fee will be set to zero.  The 'fee split' should be 0/100 for reveal/transaction.
            revealFeeBasis.u.estimate.gasLimit     *= 2;
            revealFeeBasis.u.estimate.calculatedFee = 0;
        }

        BR__Name__Transaction reveal = __name__TransactionCreateReveal(transaction->source,
                                                                 publicKey.pubKey,
                                                                 revealFeeBasis,
                                                                 transaction->counter);
        transaction->counter += 1;
        
        opList[opCount++] = reveal;
    }
    
    opList[opCount++] = transaction;

    return __name__SerializeOperationList(opList, opCount, lastBlockHash);
}

extern size_t
__name__TransactionSerializeForFeeEstimation (BR__Name__Transaction transaction,
                                           BR__Name__Account account,
                                           BR__Name__Hash lastBlockHash,
                                           bool needsReveal) {
    assert (transaction);
    assert (account);
    
    wkDataFree(transaction->signedBytes);
    
    WKData unsignedBytes = __name__TransactionSerialize(transaction, account, lastBlockHash, needsReveal);
    WKData signature = wkDataNew(__NAME___SIGNATURE_BYTES); // empty signature
    
    WKData serializedBytes = wkDataNew(unsignedBytes.size + signature.size);
    memcpy(serializedBytes.bytes, unsignedBytes.bytes, unsignedBytes.size);
    memcpy(&serializedBytes.bytes[unsignedBytes.size], signature.bytes, signature.size);

    wkDataFree (unsignedBytes);
    wkDataFree (signature);
    
    transaction->signedBytes = serializedBytes;
    assert (FEE_BASIS_INITIAL == transaction->feeBasis.type);
    transaction->feeBasis.u.initial.sizeInKBytes = (double) serializedBytes.size / 1000;
    
    if (transaction->signedBytes.size > 0) {
        createTransactionHash(transaction);
    }
    
    return transaction->signedBytes.size;
}

extern size_t
__name__TransactionSerializeAndSign (BR__Name__Transaction transaction,
                                  BR__Name__Account account,
                                  UInt512 seed,
                                  BR__Name__Hash lastBlockHash,
                                  bool needsReveal) {
    assert (transaction);
    assert (account);
    
    wkDataFree(transaction->signedBytes);
    
    WKData unsignedBytes = __name__TransactionSerialize (transaction, account, lastBlockHash, needsReveal);
    
    WKData signature = __name__AccountSignData(account, unsignedBytes, seed);
    assert(__NAME___SIGNATURE_BYTES == signature.size);
    
    WKData signedBytes = wkDataNew(unsignedBytes.size + signature.size);
    memcpy(signedBytes.bytes, unsignedBytes.bytes, unsignedBytes.size);
    memcpy(&signedBytes.bytes[unsignedBytes.size], signature.bytes, signature.size);
    
    transaction->signedBytes = signedBytes;

    if (transaction->signedBytes.size > 0) {
        createTransactionHash(transaction);
    }
    
    return transaction->signedBytes.size;
}

extern uint8_t *
__name__TransactionGetSignedBytes (BR__Name__Transaction transaction, size_t *size) {
    if (transaction->signedBytes.size > 0) {
        *size = transaction->signedBytes.size;
        return transaction->signedBytes.bytes;
    } else {
        *size = 0;
        return NULL;
    }
}

extern BR__Name__Hash __name__TransactionGetHash(BR__Name__Transaction transaction){
    assert(transaction);
    return transaction->hash;
}

extern int64_t __name__TransactionGetCounter(BR__Name__Transaction transaction) {
    assert(transaction);
    return transaction->counter;
}

extern BR__Name__FeeBasis __name__TransactionGetFeeBasis(BR__Name__Transaction transaction) {
    assert(transaction);
    return transaction->feeBasis;
}

extern void __name__TransactionSetFeeBasis(BR__Name__Transaction transaction,
                                        BR__Name__FeeBasis feeBasis) {
    assert(transaction);
    transaction->feeBasis = feeBasis;
}

extern BR__Name__UnitMutez __name__TransactionGetFee(BR__Name__Transaction transaction){
    assert(transaction);
    return __name__FeeBasisGetFee (&transaction->feeBasis);
}

extern BR__Name__UnitMutez __name__TransactionGetAmount(BR__Name__Transaction transaction){
    assert(transaction);
    assert(__NAME___OP_TRANSACTION == transaction->operation.kind);
    return transaction->operation.u.transaction.amount;
}

extern BR__Name__Address __name__TransactionGetSource(BR__Name__Transaction transaction){
    assert(transaction);
    return __name__AddressClone (transaction->source);
}

extern BR__Name__Address __name__TransactionGetTarget(BR__Name__Transaction transaction){
    assert(transaction);
    assert(__NAME___OP_TRANSACTION == transaction->operation.kind);
    return __name__AddressClone (transaction->operation.u.transaction.target);
}

extern BR__Name__OperationKind
__name__TransactionGetOperationKind(BR__Name__Transaction transaction) {
    assert(transaction);
    return transaction->operation.kind;
}

extern BR__Name__OperationData __name__TransactionGetOperationData(BR__Name__Transaction transaction){
    assert(transaction);
    BR__Name__OperationData clone;
    clone.kind = transaction->operation.kind;
    switch (transaction->operation.kind) {
        case __NAME___OP_TRANSACTION:
            clone.u.transaction.amount = transaction->operation.u.transaction.amount;
            clone.u.transaction.target = __name__AddressClone (transaction->operation.u.transaction.target);
            break;
        case __NAME___OP_REVEAL:
            memcpy(clone.u.reveal.publicKey, transaction->operation.u.reveal.publicKey, __NAME___PUBLIC_KEY_SIZE);
            break;
        case __NAME___OP_DELEGATION:
            clone.u.delegation.target = __name__AddressClone (transaction->operation.u.delegation.target);
            break;
        default:
            break;
    }
    return clone;
}

extern void
__name__TransactionFreeOperationData (BR__Name__OperationData opData) {
    switch (opData.kind) {
        case __NAME___OP_TRANSACTION:
            __name__AddressFree (opData.u.transaction.target);
            break;
        case __NAME___OP_REVEAL:
            break;
        case __NAME___OP_DELEGATION:
            __name__AddressFree (opData.u.delegation.target);
            break;
        default:
            break;
    }
}

extern bool __name__TransactionEqual (BR__Name__Transaction t1, BR__Name__Transaction t2){
    assert(t1);
    assert(t2);
    // Equal means the same transaction id, source, target
    bool result = false;
    // Transaction IDs are not available - use the hash
    BR__Name__Hash hash1 = __name__TransactionGetHash(t1);
    BR__Name__Hash hash2 = __name__TransactionGetHash(t2);
    if (memcmp(hash1.bytes, hash2.bytes, sizeof(hash1.bytes)) == 0) {
        // Hash is the same - compare the source
        BR__Name__Address source1 = __name__TransactionGetSource(t1);
        BR__Name__Address source2 = __name__TransactionGetSource(t2);
        if (1 == __name__AddressEqual(source1, source2)) {
            // OK - compare the target
            BR__Name__Address target1 = __name__TransactionGetTarget(t1);
            BR__Name__Address target2 = __name__TransactionGetTarget(t2);
            if (1 == __name__AddressEqual(target1, target2)) {
                result = true;
            }
            __name__AddressFree(target1);
            __name__AddressFree(target2);
        }
        __name__AddressFree (source1);
        __name__AddressFree (source2);
    }
    return result;
}

