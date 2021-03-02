//
//  BRTezosTransfer.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include <stdlib.h>
#include <assert.h>

#include "BRTezosTransfer.h"
#include "BRTezosAccount.h"
#include "BRTezosTransaction.h"
#include "BRTezosFeeBasis.h"

//const char * fee = "__fee__";

struct BRTezosTransferRecord {
    BRTezosAddress sourceAddress;
    BRTezosAddress targetAddress;
    BRTezosUnitMutez amount;
    BRTezosUnitMutez fee;
    BRTezosHash transactionId;
    uint64_t timestamp;
    uint64_t blockHeight;
    int error;
    BRTezosTransaction transaction;
};

extern BRTezosTransfer /* caller must free - tezosTransferFree */
tezosTransferCreate(BRTezosAddress from,
                    BRTezosAddress to,
                    BRTezosUnitMutez amount,
                    BRTezosUnitMutez fee,
                    BRTezosHash hash,
                    uint64_t timestamp, uint64_t blockHeight, int error)
{
    BRTezosTransfer transfer = (BRTezosTransfer) calloc (1, sizeof (struct BRTezosTransferRecord));
    transfer->sourceAddress = tezosAddressClone (from);
    transfer->targetAddress = tezosAddressClone (to);
    transfer->amount = amount;
    transfer->fee = fee;
    transfer->transactionId = hash;
    transfer->timestamp = timestamp;
    transfer->blockHeight = blockHeight;
    transfer->error = error;
    transfer->transaction = NULL;
    return transfer;
}

extern BRTezosTransfer /* caller must free - tezosTransferFree */
tezosTransferCreateNew(BRTezosAddress from,
                       BRTezosAddress to,
                       BRTezosUnitMutez amount,
                       BRTezosFeeBasis feeBasis,
                       int64_t counter,
                       bool delegationOp)
{
    BRTezosTransfer transfer = (BRTezosTransfer) calloc (1, sizeof (struct BRTezosTransferRecord));
    transfer->sourceAddress = tezosAddressClone (from);
    transfer->targetAddress = tezosAddressClone (to);
    transfer->amount = amount;
    transfer->fee = tezosFeeBasisGetFee (&feeBasis);
    if (delegationOp)
        transfer->transaction = tezosTransactionCreateDelegation (from, to, feeBasis, counter);
    else
        transfer->transaction = tezosTransactionCreateTransaction (from, to, amount, feeBasis, counter);
    return transfer;
}

extern BRTezosTransfer
tezosTransferClone (BRTezosTransfer transfer) {
    BRTezosTransfer clone = (BRTezosTransfer) calloc (1, sizeof (struct BRTezosTransferRecord));
    memcpy (clone, transfer, sizeof (struct BRTezosTransferRecord));
    
    if (transfer->sourceAddress)
        clone->sourceAddress = tezosAddressClone (transfer->sourceAddress);
    
    if (transfer->targetAddress)
        clone->targetAddress = tezosAddressClone (transfer->targetAddress);
    
    if (transfer->transaction)
        clone->transaction = tezosTransactionClone (transfer->transaction);
    
    return clone;
}

extern void tezosTransferFree(BRTezosTransfer transfer)
{
    assert(transfer);
    if (transfer->sourceAddress) tezosAddressFree (transfer->sourceAddress);
    if (transfer->targetAddress) tezosAddressFree (transfer->targetAddress);
    if (transfer->transaction) {
        tezosTransactionFree(transfer->transaction);
    }
    free(transfer);
}

// Getters for all the values
extern BRTezosHash tezosTransferGetTransactionId(BRTezosTransfer transfer)
{
    assert(transfer);
    if (transfer->transaction) {
        // If we have an embedded transaction that means that we created a new tx
        // which by now should have been serialized
        return tezosTransactionGetHash(transfer->transaction);
    } else {
        return transfer->transactionId;
    }
}

extern BRTezosUnitMutez tezosTransferGetAmount(BRTezosTransfer transfer)
{
    assert(transfer);
    return transfer->amount;
}

extern BRTezosUnitMutez tezosTransferGetAmountDirected (BRTezosTransfer transfer,
                                                        BRTezosAddress address,
                                                        int *negative) {
    BRTezosUnitMutez amount = (tezosTransferHasError(transfer)
                               ? 0
                               : tezosTransferGetAmount(transfer));
    BRTezosUnitMutez fee    = tezosTransferGetFee(transfer);
    
    int isSource = tezosTransferHasSource (transfer, address);
    int isTarget = tezosTransferHasTarget (transfer, address);
    
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

extern BRTezosAddress tezosTransferGetSource(BRTezosTransfer transfer)
{
    assert(transfer);
    return tezosAddressClone (transfer->sourceAddress);
}

extern BRTezosAddress tezosTransferGetTarget(BRTezosTransfer transfer)
{
    assert(transfer);
    return tezosAddressClone (transfer->targetAddress);
}

extern BRTezosUnitMutez tezosTransferGetFee(BRTezosTransfer transfer)
{
    assert(transfer);
    return transfer->fee;
}

extern BRTezosFeeBasis tezosTransferGetFeeBasis (BRTezosTransfer transfer) {
    assert(transfer);
    return (NULL == transfer->transaction)
    ? tezosFeeBasisCreateActual (transfer->fee)
    : tezosTransactionGetFeeBasis (transfer->transaction);
}

extern void tezosTransferSetFeeBasis (BRTezosTransfer transfer,
                                      BRTezosFeeBasis feeBasis) {
    assert(transfer);
    if (NULL == transfer->transaction) transfer->fee = tezosFeeBasisGetFee (&feeBasis);
    else tezosTransactionSetFeeBasis (transfer->transaction, feeBasis);
}

extern BRTezosTransaction tezosTransferGetTransaction(BRTezosTransfer transfer)
{
    assert(transfer);
    return transfer->transaction;
}

extern int
tezosTransferHasError(BRTezosTransfer transfer) {
    return transfer->error;
}

extern int tezosTransferHasSource (BRTezosTransfer transfer,
                                   BRTezosAddress source) {
    return tezosAddressEqual (transfer->sourceAddress, source);
}

extern int tezosTransferHasTarget (BRTezosTransfer transfer,
                                   BRTezosAddress target) {
    return tezosAddressEqual (transfer->targetAddress, target);
}

extern uint64_t tezosTransferGetBlockHeight (BRTezosTransfer transfer) {
    assert(transfer);
    return transfer->blockHeight;
}

extern int
tezosTransferIsEqual (BRTezosTransfer t1, BRTezosTransfer t2) {
    // burn transfers with same hash but different target address are not matched
    return  (t1 == t2) ||
            (tezosHashIsEqual (tezosTransferGetTransactionId (t1),
                               tezosTransferGetTransactionId (t2)) &&
             tezosAddressEqual(t1->targetAddress, t2->targetAddress));
}
