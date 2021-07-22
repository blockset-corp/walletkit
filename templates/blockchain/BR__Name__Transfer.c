//
//  BR__Name__Transfer.c
//  WalletKitCore
//
//  Created by __USER__ on __DATE__.
//  Copyright © __YEAR__ Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include <stdlib.h>
#include <assert.h>

#include "BR__Name__Transfer.h"
#include "BR__Name__Account.h"
#include "BR__Name__Transaction.h"
#include "BR__Name__FeeBasis.h"

//const char * fee = "__fee__";

struct BR__Name__TransferRecord {
    BR__Name__Address sourceAddress;
    BR__Name__Address targetAddress;
    BR__Name__UnitMutez amount;
    BR__Name__UnitMutez fee;
    BR__Name__Hash transactionId;
    uint64_t timestamp;
    uint64_t blockHeight;
    int error;
    BR__Name__Transaction transaction;
};

extern BR__Name__Transfer /* caller must free - __name__TransferFree */
__name__TransferCreate(BR__Name__Address from,
                    BR__Name__Address to,
                    BR__Name__UnitMutez amount,
                    BR__Name__UnitMutez fee,
                    BR__Name__Hash hash,
                    uint64_t timestamp, uint64_t blockHeight, int error)
{
    BR__Name__Transfer transfer = (BR__Name__Transfer) calloc (1, sizeof (struct BR__Name__TransferRecord));
    transfer->sourceAddress = __name__AddressClone (from);
    transfer->targetAddress = __name__AddressClone (to);
    transfer->amount = amount;
    transfer->fee = fee;
    transfer->transactionId = hash;
    transfer->timestamp = timestamp;
    transfer->blockHeight = blockHeight;
    transfer->error = error;
    transfer->transaction = NULL;
    return transfer;
}

extern BR__Name__Transfer /* caller must free - __name__TransferFree */
__name__TransferCreateNew(BR__Name__Address from,
                       BR__Name__Address to,
                       BR__Name__UnitMutez amount,
                       BR__Name__FeeBasis feeBasis,
                       int64_t counter,
                       bool delegationOp)
{
    BR__Name__Transfer transfer = (BR__Name__Transfer) calloc (1, sizeof (struct BR__Name__TransferRecord));
    transfer->sourceAddress = __name__AddressClone (from);
    transfer->targetAddress = __name__AddressClone (to);
    transfer->amount = amount;
    transfer->fee = __name__FeeBasisGetFee (&feeBasis);
    transfer->transactionId = __NAME___HASH_EMPTY;
    if (delegationOp)
        transfer->transaction = __name__TransactionCreateDelegation (from, to, feeBasis, counter);
    else
        transfer->transaction = __name__TransactionCreateTransaction (from, to, amount, feeBasis, counter);
    return transfer;
}

extern BR__Name__Transfer
__name__TransferClone (BR__Name__Transfer transfer) {
    BR__Name__Transfer clone = (BR__Name__Transfer) calloc (1, sizeof (struct BR__Name__TransferRecord));
    memcpy (clone, transfer, sizeof (struct BR__Name__TransferRecord));
    
    if (transfer->sourceAddress)
        clone->sourceAddress = __name__AddressClone (transfer->sourceAddress);
    
    if (transfer->targetAddress)
        clone->targetAddress = __name__AddressClone (transfer->targetAddress);
    
    if (transfer->transaction)
        clone->transaction = __name__TransactionClone (transfer->transaction);
    
    return clone;
}

extern void __name__TransferFree(BR__Name__Transfer transfer)
{
    assert(transfer);
    if (transfer->sourceAddress) __name__AddressFree (transfer->sourceAddress);
    if (transfer->targetAddress) __name__AddressFree (transfer->targetAddress);
    if (transfer->transaction) {
        __name__TransactionFree(transfer->transaction);
    }
    free(transfer);
}

// Getters for all the values
extern BR__Name__Hash __name__TransferGetTransactionId(BR__Name__Transfer transfer)
{
    assert(transfer);
    if (transfer->transaction) {
        // If we have an embedded transaction that means that we created a new tx
        // which by now should have been serialized
        return __name__TransactionGetHash(transfer->transaction);
    } else {
        return transfer->transactionId;
    }
}

extern BR__Name__UnitMutez __name__TransferGetAmount(BR__Name__Transfer transfer)
{
    assert(transfer);
    return transfer->amount;
}

extern BR__Name__UnitMutez __name__TransferGetAmountDirected (BR__Name__Transfer transfer,
                                                        BR__Name__Address address,
                                                        int *negative) {
    BR__Name__UnitMutez amount = (__name__TransferHasError(transfer)
                               ? 0
                               : __name__TransferGetAmount(transfer));
    BR__Name__UnitMutez fee    = __name__TransferGetFee(transfer);
    
    int isSource = __name__TransferHasSource (transfer, address);
    int isTarget = __name__TransferHasTarget (transfer, address);
    
    if (isSource && isTarget) {
        *negative = 1;
        return fee;
    }
    else if (isSource) {
        *negative = 1;
        return amount + fee;
    }
    else if (isTarget) {
        *negative = 0;
        return amount;
    }
    else {
        assert (0);
    }
}

extern BR__Name__Address __name__TransferGetSource(BR__Name__Transfer transfer)
{
    assert(transfer);
    return __name__AddressClone (transfer->sourceAddress);
}

extern BR__Name__Address __name__TransferGetTarget(BR__Name__Transfer transfer)
{
    assert(transfer);
    return __name__AddressClone (transfer->targetAddress);
}

extern BR__Name__UnitMutez __name__TransferGetFee(BR__Name__Transfer transfer)
{
    assert(transfer);
    return transfer->fee;
}

extern BR__Name__FeeBasis __name__TransferGetFeeBasis (BR__Name__Transfer transfer) {
    assert(transfer);
    return (NULL == transfer->transaction)
    ? __name__FeeBasisCreateActual (transfer->fee)
    : __name__TransactionGetFeeBasis (transfer->transaction);
}

extern void __name__TransferSetFeeBasis (BR__Name__Transfer transfer,
                                      BR__Name__FeeBasis feeBasis) {
    assert(transfer);
    if (NULL == transfer->transaction) transfer->fee = __name__FeeBasisGetFee (&feeBasis);
    else __name__TransactionSetFeeBasis (transfer->transaction, feeBasis);
}

extern BR__Name__Transaction __name__TransferGetTransaction(BR__Name__Transfer transfer)
{
    assert(transfer);
    return transfer->transaction;
}

extern int
__name__TransferHasError(BR__Name__Transfer transfer) {
    return transfer->error;
}

extern int __name__TransferHasSource (BR__Name__Transfer transfer,
                                   BR__Name__Address source) {
    return __name__AddressEqual (transfer->sourceAddress, source);
}

extern int __name__TransferHasTarget (BR__Name__Transfer transfer,
                                   BR__Name__Address target) {
    return __name__AddressEqual (transfer->targetAddress, target);
}

extern uint64_t __name__TransferGetBlockHeight (BR__Name__Transfer transfer) {
    assert(transfer);
    return transfer->blockHeight;
}

extern int
__name__TransferIsEqual (BR__Name__Transfer t1, BR__Name__Transfer t2) {
    // burn transfers with same hash but different target address are not matched
    return  (t1 == t2) ||
            (__name__HashIsEqual (__name__TransferGetTransactionId (t1),
                               __name__TransferGetTransactionId (t2)) &&
             __name__AddressEqual(t1->targetAddress, t2->targetAddress));
}
