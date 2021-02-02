//
//  BRTezosTransaction.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRTezosTransaction.h"
#include "BRTezosEncoder.h"
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
    BRTezosAddress source;
    BRTezosFeeBasis feeBasis;
    
    BRTezosOperationData operation;
    
    uint64_t blockHeight;
    int64_t counter; // nonce
    
    const char * protocol; // protocol name
    const char * branch; // hash of head block
    
    BRCryptoData signedBytes;
};

struct BRTezosSignatureRecord {
    uint8_t signature[TEZOS_SIGNATURE_BYTES];
};

static BRTezosTransaction
createTransactionObject() {
    BRTezosTransaction transaction = calloc (1, sizeof(struct BRTezosTransactionRecord));
    assert(transaction);
    return transaction;
}

extern BRTezosTransaction
tezosTransactionCreateTransaction (BRTezosAddress source,
                                   BRTezosAddress target,
                                   BRTezosUnitMutez amount,
                                   BRTezosFeeBasis feeBasis,
                                   int64_t counter) {
    BRTezosTransaction transaction = createTransactionObject();
    transaction->source = tezosAddressClone (source);
    transaction->operation.kind = TEZOS_OP_TRANSACTION;
    transaction->operation.u.transaction.amount = amount;
    transaction->operation.u.transaction.target = tezosAddressClone (target);
    transaction->feeBasis = feeBasis;
    transaction->blockHeight = 0;
    transaction->counter = counter;
    return transaction;
}

extern BRTezosTransaction
tezosTransactionCreateDelegation (BRTezosAddress source,
                                  BRTezosAddress target,
                                  BRTezosFeeBasis feeBasis,
                                  int64_t counter) {
    BRTezosTransaction transaction = createTransactionObject();
    transaction->source = tezosAddressClone (source);
    transaction->operation.kind = TEZOS_OP_DELEGATION;
    bool endDelegation = tezosAddressEqual(source, target);
    transaction->operation.u.delegation.target = endDelegation ? NULL : tezosAddressClone (target);
    transaction->feeBasis = feeBasis;
    transaction->blockHeight = 0;
    transaction->counter = counter;
    return transaction;
}

extern BRTezosTransaction
tezosTransactionCreateReveal (BRTezosAddress source,
                              uint8_t * pubKey,
                              BRTezosFeeBasis feeBasis,
                              int64_t counter) {
    BRTezosTransaction transaction = createTransactionObject();
    transaction->source = tezosAddressClone (source);
    transaction->operation.kind = TEZOS_OP_REVEAL;
    memcpy(transaction->operation.u.reveal.publicKey, pubKey, TEZOS_PUBLIC_KEY_SIZE);
    transaction->feeBasis = feeBasis;
    transaction->blockHeight = 0;
    transaction->counter = counter;
    return transaction;
}

extern BRTezosTransaction
tezosTransactionClone (BRTezosTransaction transaction) {
    assert(transaction);
    BRTezosTransaction newTransaction = calloc (1, sizeof(struct BRTezosTransactionRecord));
    newTransaction->source = tezosTransactionGetSource(transaction);
    newTransaction->operation = tezosTransactionGetOperationData (transaction);
    newTransaction->feeBasis = transaction->feeBasis;
    newTransaction->hash = transaction->hash;
    newTransaction->blockHeight = transaction->blockHeight;
    return newTransaction;
}

extern void tezosTransactionFree (BRTezosTransaction transaction)
{
    assert (transaction);
    tezosAddressFree (transaction->source);
    cryptoDataFree(transaction->signedBytes);
    switch (transaction->operation.kind) {
        case TEZOS_OP_REVEAL:
            break;
            
        case TEZOS_OP_TRANSACTION:
            tezosAddressFree(transaction->operation.u.transaction.target);
            break;
            
        case TEZOS_OP_DELEGATION:
            tezosAddressFree(transaction->operation.u.delegation.target);
            break;
        default:
            break;
    }
    free (transaction);
}

static void
createTransactionHash(BRTezosTransaction tx) {
    assert(tx->signedBytes.size);
    
    uint8_t hash[32];
    blake2b(hash, sizeof(hash), NULL, 0, tx->signedBytes.bytes, tx->signedBytes.size);
    
    uint8_t prefix[] = { 5, 116 }; // operation prefix
    memcpy(tx->hash.bytes, prefix, sizeof(prefix));
    memcpy(&(tx->hash.bytes[sizeof(prefix)]), hash, sizeof(hash));
}

static BRCryptoData
tezosTransactionSerialize (BRTezosTransaction transaction,
                           BRTezosAccount account,
                           BRTezosHash lastBlockHash,
                           bool needsReveal) {
    BRTezosTransaction opList[2];
    size_t opCount = 0;
    
    // add a reveal operation to the operation list if needed
    if (needsReveal) {
        BRKey publicKey = tezosAccountGetPublicKey(account);
        BRTezosTransaction reveal = tezosTransactionCreateReveal(transaction->source,
                                                                 publicKey.pubKey,
                                                                 transaction->feeBasis,
                                                                 transaction->counter);
        transaction->counter += 1;
        
        opList[opCount++] = reveal;
    }
    
    opList[opCount++] = transaction;
    
    return tezosSerializeOperationList(opList, opCount, lastBlockHash);
}

extern size_t
tezosTransactionSerializeForFeeEstimation (BRTezosTransaction transaction,
                                           BRTezosAccount account,
                                           BRTezosHash lastBlockHash,
                                           bool needsReveal) {
    assert (transaction);
    assert (account);
    
    cryptoDataFree(transaction->signedBytes);
    
    BRCryptoData unsignedBytes = tezosTransactionSerialize(transaction, account, lastBlockHash, needsReveal);
    BRCryptoData signature = cryptoDataNew(TEZOS_SIGNATURE_BYTES); // empty signature
    
    BRCryptoData serializedBytes = cryptoDataNew(unsignedBytes.size + signature.size);
    memcpy(serializedBytes.bytes, unsignedBytes.bytes, unsignedBytes.size);
    memcpy(&serializedBytes.bytes[unsignedBytes.size], signature.bytes, signature.size);

    cryptoDataFree (unsignedBytes);
    cryptoDataFree (signature);
    
    transaction->signedBytes = serializedBytes;
    assert (FEE_BASIS_INITIAL == transaction->feeBasis.type);
    transaction->feeBasis.u.initial.sizeInKBytes = (double) serializedBytes.size / 1000;
    
    if (transaction->signedBytes.size > 0) {
        createTransactionHash(transaction);
    }
    
    return transaction->signedBytes.size;
}

extern size_t
tezosTransactionSerializeAndSign (BRTezosTransaction transaction,
                                  BRTezosAccount account,
                                  UInt512 seed,
                                  BRTezosHash lastBlockHash,
                                  bool needsReveal) {
    assert (transaction);
    assert (account);
    
    cryptoDataFree(transaction->signedBytes);
    
    BRCryptoData unsignedBytes = tezosTransactionSerialize (transaction, account, lastBlockHash, needsReveal);
    
    BRCryptoData signature = tezosAccountSignData(account, unsignedBytes, seed);
    assert(TEZOS_SIGNATURE_BYTES == signature.size);
    
    BRCryptoData signedBytes = cryptoDataNew(unsignedBytes.size + signature.size);
    memcpy(signedBytes.bytes, unsignedBytes.bytes, unsignedBytes.size);
    memcpy(&signedBytes.bytes[unsignedBytes.size], signature.bytes, signature.size);
    
    transaction->signedBytes = signedBytes;
    
    if (transaction->signedBytes.size > 0) {
        createTransactionHash(transaction);
    }
    
    return transaction->signedBytes.size;
}

extern uint8_t *
tezosTransactionGetSignedBytes (BRTezosTransaction transaction, size_t *size) {
    if (transaction->signedBytes.size > 0) {
        *size = transaction->signedBytes.size;
        return transaction->signedBytes.bytes;
    } else {
        *size = 0;
        return NULL;
    }
}

extern BRTezosHash tezosTransactionGetHash(BRTezosTransaction transaction){
    assert(transaction);
    return transaction->hash;
}

extern int64_t tezosTransactionGetCounter(BRTezosTransaction transaction) {
    assert(transaction);
    return transaction->counter;
}

extern BRTezosFeeBasis tezosTransactionGetFeeBasis(BRTezosTransaction transaction) {
    assert(transaction);
    return transaction->feeBasis;
}

extern void tezosTransactionSetFeeBasis(BRTezosTransaction transaction,
                                        BRTezosFeeBasis feeBasis) {
    assert(transaction);
    transaction->feeBasis = feeBasis;
}

extern BRTezosUnitMutez tezosTransactionGetFee(BRTezosTransaction transaction){
    assert(transaction);
    return tezosFeeBasisGetFee (&transaction->feeBasis);
}

extern BRTezosUnitMutez tezosTransactionGetAmount(BRTezosTransaction transaction){
    assert(transaction);
    assert(TEZOS_OP_TRANSACTION == transaction->operation.kind);
    return transaction->operation.u.transaction.amount;
}

extern BRTezosAddress tezosTransactionGetSource(BRTezosTransaction transaction){
    assert(transaction);
    return tezosAddressClone (transaction->source);
}

extern BRTezosAddress tezosTransactionGetTarget(BRTezosTransaction transaction){
    assert(transaction);
    assert(TEZOS_OP_TRANSACTION == transaction->operation.kind);
    return tezosAddressClone (transaction->operation.u.transaction.target);
}

extern BRTezosOperationKind
tezosTransactionGetOperationKind(BRTezosTransaction transaction) {
    assert(transaction);
    return transaction->operation.kind;
}

extern BRTezosOperationData tezosTransactionGetOperationData(BRTezosTransaction transaction){
    assert(transaction);
    BRTezosOperationData clone;
    clone.kind = transaction->operation.kind;
    switch (transaction->operation.kind) {
        case TEZOS_OP_TRANSACTION:
            clone.u.transaction.amount = transaction->operation.u.transaction.amount;
            clone.u.transaction.target = tezosAddressClone (transaction->operation.u.transaction.target);
            break;
        case TEZOS_OP_REVEAL:
            memcpy(clone.u.reveal.publicKey, transaction->operation.u.reveal.publicKey, TEZOS_PUBLIC_KEY_SIZE);
            break;
        case TEZOS_OP_DELEGATION:
            clone.u.delegation.target = tezosAddressClone (transaction->operation.u.delegation.target);
            break;
        default:
            break;
    }
    return clone;
}

extern void
tezosTransactionFreeOperationData (BRTezosOperationData opData) {
    switch (opData.kind) {
        case TEZOS_OP_TRANSACTION:
            tezosAddressFree (opData.u.transaction.target);
            break;
        case TEZOS_OP_REVEAL:
            break;
        case TEZOS_OP_DELEGATION:
            tezosAddressFree (opData.u.delegation.target);
            break;
        default:
            break;
    }
}

extern bool tezosTransactionEqual (BRTezosTransaction t1, BRTezosTransaction t2){
    assert(t1);
    assert(t2);
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
}

