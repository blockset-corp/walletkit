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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>


struct BRTezosTransactionRecord {
    
    BRTezosTransactionHash hash;
    BRTezosAddress source;
    BRTezosUnitMutez fee; //TODO: move into feeBasis?
    BRTezosFeeBasis feeBasis;
    
    BRTezosOperationData operation;
    
    uint64_t blockHeight;
    int64_t counter; // nonce
    
    const char * protocol; // protocol name
    const char * branch; // hash of head block
    
    BRTezosSerializedOperation signedBytes;
};

struct BRTezosSerializedOperationRecord {
    uint8_t * bytes;
    size_t size;
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
                                   int64_t counter)
{
    BRTezosTransaction transaction = createTransactionObject();
    transaction->source = tezosAddressClone (source);
    transaction->operation.kind = TEZOS_OP_TRANSACTION;
    transaction->operation.u.transaction.amount = amount;
    transaction->operation.u.transaction.target = tezosAddressClone (target);
    transaction->feeBasis = feeBasis;
    transaction->fee = tezosFeeBasisGetFee (&transaction->feeBasis);
    transaction->blockHeight = 0;
    transaction->counter = counter;
    return transaction;
}

extern BRTezosTransaction
tezosTransactionCreateDelegation (BRTezosAddress source,
                                  BRTezosAddress target,
                                  BRTezosFeeBasis feeBasis,
                                  int64_t counter)
{
    BRTezosTransaction transaction = createTransactionObject();
    transaction->source = tezosAddressClone (source);
    transaction->operation.kind = TEZOS_OP_DELEGATION;
    transaction->operation.u.delegation.target = tezosAddressClone (target);
    transaction->feeBasis = feeBasis;
    transaction->fee = tezosFeeBasisGetFee (&transaction->feeBasis);
    transaction->blockHeight = 0;
    transaction->counter = counter;
    return transaction;
}

//
//extern BRTezosTransaction tezosTransactionCreate (BRTezosAddress source,
//                                                    BRTezosAddress target,
//                                                    BRTezosUnitMutez amount,
//                                                    BRTezosUnitMutez fee,
//                                                    const char * txID,
//                                                    BRTezosTransactionHash hash,
//                                                    uint64_t timestamp, uint64_t blockHeight,
//                                                    int error)
//{
//    // This is an existing transaction - it must have a transaction ID
//    assert(source);
//    assert(target);
//    BRTezosTransaction transaction = calloc (1, sizeof(struct BRTezosTransactionRecord));
//    transaction->source = tezosAddressClone (source);
//    transaction->target = tezosAddressClone (target);
//    transaction->amount = amount;
//    transaction->fee = fee;
//    transaction->feeBasis.pricePerCostFactor = fee;
//    transaction->feeBasis.costFactor = 1;
//
//    // Parse the transactionID
//    if (txID && strlen(txID) > 1) {
//        transaction->transactionId = (char*) calloc(1, strlen(txID) + 1);
//        strcpy(transaction->transactionId, txID);
//        // Get the timestamp from the transaction id
//        transaction->timeStamp = tezosParseTimeStamp(txID);
//    } else {
//        // It would great to be able to get the timestamp from the txID - but I guess
//        // we just have to use whatever came from blockset
//        transaction->timeStamp.seconds = (int64_t) timestamp;
//    }
//
//    transaction->hash = hash;
//    transaction->blockHeight = blockHeight;
//    transaction->error = error;
//
//    return transaction;
//}

extern BRTezosTransaction
tezosTransactionClone (BRTezosTransaction transaction) {
    assert(transaction);
    BRTezosTransaction newTransaction = calloc (1, sizeof(struct BRTezosTransactionRecord));
    newTransaction->source = tezosTransactionGetSource(transaction);
    newTransaction->operation = tezosTransactionGetOperationData (transaction);
    newTransaction->fee = transaction->fee;
    newTransaction->feeBasis = transaction->feeBasis;
    newTransaction->hash = transaction->hash;
    newTransaction->blockHeight = transaction->blockHeight;
    return newTransaction;
}

extern void tezosTransactionFree (BRTezosTransaction transaction)
{
    assert (transaction);
    tezosAddressFree (transaction->source);
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

extern size_t
tezosTransactionSignTransaction (BRTezosTransaction transaction,
                                  BRKey publicKey,
                                  UInt512 seed)
{
    assert (transaction);
    //TODO:TEZOS
    return 0;
}

extern uint8_t *
tezosTransactionSerialize (BRTezosTransaction transaction, size_t *bytesCount) {
    BRCryptoData data = tezosSerializeTransaction (transaction);
    *bytesCount = data.size;
    return data.bytes;
}

extern BRTezosTransactionHash tezosTransactionGetHash(BRTezosTransaction transaction)
{
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

extern BRTezosUnitMutez tezosTransactionGetFee(BRTezosTransaction transaction)
{
    assert(transaction);
    return transaction->fee;
}

extern BRTezosUnitMutez tezosTransactionGetAmount(BRTezosTransaction transaction)
{
    assert(transaction);
    assert(TEZOS_OP_TRANSACTION == transaction->operation.kind);
    return transaction->operation.u.transaction.amount;
}

extern BRTezosAddress tezosTransactionGetSource(BRTezosTransaction transaction)
{
    assert(transaction);
    return tezosAddressClone (transaction->source);
}

extern BRTezosAddress tezosTransactionGetTarget(BRTezosTransaction transaction)
{
    assert(transaction);
    assert(TEZOS_OP_TRANSACTION == transaction->operation.kind);
    return tezosAddressClone (transaction->operation.u.transaction.target);
}

extern BRTezosOperationData tezosTransactionGetOperationData(BRTezosTransaction transaction)
{
    assert(transaction);
    BRTezosOperationData clone;
    clone.kind = transaction->operation.kind;
    switch (transaction->operation.kind) {
        case TEZOS_OP_TRANSACTION:
            clone.u.transaction.amount = transaction->operation.u.transaction.amount;
            clone.u.transaction.target = tezosAddressClone (transaction->operation.u.transaction.target);
            break;
        case TEZOS_OP_REVEAL:
            //TODO:TEZOS clone public key
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
            if (opData.u.reveal.publicKey) free (opData.u.reveal.publicKey);
            break;
        case TEZOS_OP_DELEGATION:
            tezosAddressFree (opData.u.delegation.target);
            break;
        default:
            break;
    }
}

extern bool tezosTransactionEqual (BRTezosTransaction t1, BRTezosTransaction t2)
{
    assert(t1);
    assert(t2);
    // Equal means the same transaction id, source, target
    bool result = false;
    // Transaction IDs are not available - use the hash
    BRTezosTransactionHash hash1 = tezosTransactionGetHash(t1);
    BRTezosTransactionHash hash2 = tezosTransactionGetHash(t2);
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

