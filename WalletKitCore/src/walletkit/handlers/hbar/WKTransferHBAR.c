//
//  WKTransferHBAR.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKHBAR.h"
#include "walletkit/WKAmountP.h"
#include "walletkit/WKHashP.h"
#include "hedera/BRHederaTransaction.h"
#include "support/util/BRUtilMath.h"

static WKTransferDirection
transferGetDirectionFromHBAR (BRHederaTransaction transaction,
                              BRHederaAccount account);

extern WKTransferHBAR
wkTransferCoerceHBAR (WKTransfer transfer) {
    assert (WK_NETWORK_TYPE_HBAR == transfer->type);
    return (WKTransferHBAR) transfer;
}

typedef struct {
    BRHederaTransaction hbarTransaction;
} WKTransferCreateContextHBAR;

static void
wkTransferCreateCallbackHBAR (WKTransferCreateContext context,
                                    WKTransfer transfer) {
    WKTransferCreateContextHBAR *contextHBAR = (WKTransferCreateContextHBAR*) context;
    WKTransferHBAR transferHBAR = wkTransferCoerceHBAR (transfer);

    transferHBAR->hbarTransaction = contextHBAR->hbarTransaction;
}

extern WKTransfer
wkTransferCreateAsHBAR (WKTransferListener listener,
                            WKUnit unit,
                            WKUnit unitForFee,
                            WKTransferState state,
                            OwnershipKept BRHederaAccount hbarAccount,
                            OwnershipGiven BRHederaTransaction hbarTransaction) {
    
    WKTransferDirection direction = transferGetDirectionFromHBAR (hbarTransaction, hbarAccount);
    
    WKAmount amount = wkAmountCreateAsHBAR (unit,
                                                      WK_FALSE,
                                                      hederaTransactionGetAmount (hbarTransaction));
    
    BRHederaFeeBasis hbarFeeBasis = { (WK_TRANSFER_RECEIVED == direction ? 0 : hederaTransactionGetFee (hbarTransaction)), 1 };
    WKFeeBasis feeBasisEstimated = wkFeeBasisCreateAsHBAR (unitForFee, hbarFeeBasis);
    
    WKAddress sourceAddress = wkAddressCreateAsHBAR (hederaTransactionGetSource (hbarTransaction));
    WKAddress targetAddress = wkAddressCreateAsHBAR (hederaTransactionGetTarget (hbarTransaction));

    WKTransferCreateContextHBAR contextHBAR = {
        hbarTransaction
    };

    WKTransfer transfer = wkTransferAllocAndInit (sizeof (struct WKTransferHBARRecord),
                                                            WK_NETWORK_TYPE_HBAR,
                                                            listener,
                                                            unit,
                                                            unitForFee,
                                                            feeBasisEstimated,
                                                            amount,
                                                            direction,
                                                            sourceAddress,
                                                            targetAddress,
                                                            state,
                                                            &contextHBAR,
                                                            wkTransferCreateCallbackHBAR);

    wkFeeBasisGive (feeBasisEstimated);
    wkAddressGive (sourceAddress);
    wkAddressGive (targetAddress);
    
    return transfer;
}

static void
wkTransferReleaseHBAR (WKTransfer transfer) {
    WKTransferHBAR transferHBAR = wkTransferCoerceHBAR(transfer);
    hederaTransactionFree (transferHBAR->hbarTransaction);
}

static WKHash
wkTransferGetHashHBAR (WKTransfer transfer) {
    WKTransferHBAR transferHBAR = wkTransferCoerceHBAR(transfer);
    return (! hederaTransactionHashExists (transferHBAR->hbarTransaction)
            ? NULL
            : wkHashCreateAsHBAR (hederaTransactionGetHash (transferHBAR->hbarTransaction)));
}

extern bool
wkTransferSetHashHBAR (WKTransfer transfer,
                           WKHash hash) {
    WKTransferHBAR transferHBAR = wkTransferCoerceHBAR(transfer);
    BRHederaTransactionHash hashHBAR  = wkHashAsHBAR(hash);

    return 1 == hederaTransactionUpdateHash (transferHBAR->hbarTransaction, hashHBAR);
}

static void
crptoTransferUpdateIdentifierHBAR (WKTransfer transfer) {
    if (NULL != transfer->identifier) return;

    WKTransferHBAR transferHBAR = wkTransferCoerceHBAR(transfer);
    transfer->identifier = hederaTransactionGetTransactionId (transferHBAR->hbarTransaction);
}

static uint8_t *
wkTransferSerializeHBAR (WKTransfer transfer,
                             WKNetwork network,
                             WKBoolean  requireSignature,
                             size_t *serializationCount) {
    assert (WK_TRUE == requireSignature);
    WKTransferHBAR transferHBAR = wkTransferCoerceHBAR (transfer);
    return hederaTransactionSerialize (transferHBAR->hbarTransaction, serializationCount);
}

static int
wkTransferIsEqualHBAR (WKTransfer t1, WKTransfer t2) {
    if (t1 == t2) return 1;

    WKTransferHBAR th1 = wkTransferCoerceHBAR (t1);
    WKTransferHBAR th2 = wkTransferCoerceHBAR (t2);

    if (th1->hbarTransaction == th2->hbarTransaction) return 1;

    return hederaTransactionHashIsEqual (hederaTransactionGetHash (th1->hbarTransaction),
                                         hederaTransactionGetHash (th2->hbarTransaction));
}

static WKTransferDirection
transferGetDirectionFromHBAR (BRHederaTransaction transaction,
                              BRHederaAccount account) {
    BRHederaAddress source = hederaTransactionGetSource (transaction);
    BRHederaAddress target = hederaTransactionGetTarget (transaction);
    
    int isSource = hederaAccountHasAddress (account, source);
    int isTarget = hederaAccountHasAddress (account, target);

    hederaAddressFree (target);
    hederaAddressFree (source);
    
    return (isSource && isTarget
            ? WK_TRANSFER_RECOVERED
            : (isSource
               ? WK_TRANSFER_SENT
               : WK_TRANSFER_RECEIVED));
}

WKTransferHandlers wkTransferHandlersHBAR = {
    wkTransferReleaseHBAR,
    wkTransferGetHashHBAR,
    wkTransferSetHashHBAR,
    crptoTransferUpdateIdentifierHBAR,
    wkTransferSerializeHBAR,
    NULL, // getBytesForFeeEstimate
    wkTransferIsEqualHBAR
};
