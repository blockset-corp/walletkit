//
//  WKTransferAVAX.c
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKAVAX.h"
#include "walletkit/WKAmountP.h"
#include "walletkit/WKHashP.h"

#include "support/util/BRUtilMath.h"

static WKTransferDirection
avalancheTransactionGetDirection (BRAvalancheTransaction transfer,
                                 BRAvalancheAccount account);

typedef struct {
    BRAvalancheTransaction avaxTransaction;
} WKTransferCreateContextAVAX;

static void
wkTransferCreateCallbackAVAX (WKTransferCreateContext context,
                                    WKTransfer transfer) {
    WKTransferCreateContextAVAX *contextAVAX = (WKTransferCreateContextAVAX*) context;
    WKTransferAVAX transferAVAX = wkTransferCoerceAVAX (transfer);

    transferAVAX->avaxTransaction = contextAVAX->avaxTransaction;
}

extern WKTransfer
wkTransferCreateAsAVAX (WKTransferListener listener,
                              const char *uids,
                              WKUnit unit,
                              WKUnit unitForFee,
                              WKTransferState state,
                              OwnershipKept  BRAvalancheAccount     avaxAccount,
                              OwnershipGiven BRAvalancheTransaction avaxTransaction) {
    
    WKTransferDirection direction = avalancheTransactionGetDirection (avaxTransaction, avaxAccount);

    BRAvalancheAmount avaxAmount = avalancheTransactionGetAmount (avaxTransaction);
    WKAmount amount = wkAmountCreateAsAVAX (unit, WK_FALSE, avaxAmount);

    BRAvalancheFeeBasis avaxFeeBasis = avalancheFeeBasisCreate ();
    WKFeeBasis feeBasis = wkFeeBasisCreateAsAVAX (unitForFee, avaxFeeBasis);
    
    WKAddress sourceAddress = wkAddressCreateAsAVAX (avalancheTransactionGetSource (avaxTransaction));
    WKAddress targetAddress = wkAddressCreateAsAVAX (avalancheTransactionGetTarget (avaxTransaction));

    WKTransferCreateContextAVAX contextAVAX = {
        avaxTransaction
    };

    WKTransfer transfer = wkTransferAllocAndInit (sizeof (struct WKTransferAVAXRecord),
                                                  WK_NETWORK_TYPE_AVAX,
                                                  listener,
                                                  uids,
                                                  unit,
                                                  unitForFee,
                                                  feeBasis,
                                                  amount,
                                                  direction,
                                                  sourceAddress,
                                                  targetAddress,
                                                  state,
                                                  &contextAVAX,
                                                  wkTransferCreateCallbackAVAX);
    
    wkFeeBasisGive (feeBasis);
    wkAddressGive (sourceAddress);
    wkAddressGive (targetAddress);

    return transfer;
}

static void
wkTransferReleaseAVAX (WKTransfer transfer) {
    WKTransferAVAX transferAVAX = wkTransferCoerceAVAX(transfer);
    avalancheTransactionFree (transferAVAX->avaxTransaction);
}

static WKHash
wkTransferGetHashAVAX (WKTransfer transfer) {
    WKTransferAVAX transferAVAX = wkTransferCoerceAVAX(transfer);

    BRAvalancheHash hash = avalancheTransactionGetHash (transferAVAX->avaxTransaction);
    return (avalancheHashIsEmpty(hash)
            ? NULL
            : wkHashCreateAsAVAX (hash));
}

static uint8_t *
wkTransferSerializeAVAX (WKTransfer transfer,
                               WKNetwork network,
                               WKBoolean  requireSignature,
                               size_t *serializationCount) {
    WKTransferAVAX transferAVAX = wkTransferCoerceAVAX (transfer);

    *serializationCount = 0;

    return (NULL == transferAVAX->avaxTransaction
            ? NULL
            : avalancheTransactionGetSerialization (transferAVAX->avaxTransaction, serializationCount));
}

static int
wkTransferIsEqualAVAX (WKTransfer tb1, WKTransfer tb2) {
    if (tb1 == tb2) return 1;
    
    WKTransferAVAX tz1 = wkTransferCoerceAVAX (tb1);
    WKTransferAVAX tz2 = wkTransferCoerceAVAX (tb2);
    
    return avalancheTransactionEqual (tz1->avaxTransaction, tz2->avaxTransaction);
}

static WKTransferDirection
avalancheTransactionGetDirection (BRAvalancheTransaction transfer,
                                    BRAvalancheAccount account) {
    BRAvalancheAddress source = avalancheTransactionGetSource (transfer);
    BRAvalancheAddress target = avalancheTransactionGetTarget (transfer);
    
    int isSource = avalancheAccountHasAddress (account, source);
    int isTarget = avalancheAccountHasAddress (account, target);
        
    return (isSource && isTarget
            ? WK_TRANSFER_RECOVERED
            : (isSource
               ? WK_TRANSFER_SENT
               : WK_TRANSFER_RECEIVED));
}

WKTransferHandlers wkTransferHandlersAVAX = {
    wkTransferReleaseAVAX,
    wkTransferGetHashAVAX,
    NULL, // setHash
    NULL, // updateIdentifier
    wkTransferSerializeAVAX,
    NULL, // getBytesForFeeEstimate
    wkTransferIsEqualAVAX
};
