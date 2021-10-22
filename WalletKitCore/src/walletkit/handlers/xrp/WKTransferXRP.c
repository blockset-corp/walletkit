//
//  WKTransferXRP.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXRP.h"
#include "walletkit/WKAmountP.h"
#include "walletkit/WKHashP.h"
#include "ripple/BRRippleTransaction.h"
#include "support/util/BRUtilMath.h"

static WKTransferDirection
transferGetDirectionFromXRP (BRRippleTransaction transaction,
                             BRRippleAccount account);

extern WKTransferXRP
wkTransferCoerceXRP (WKTransfer transfer) {
    assert (WK_NETWORK_TYPE_XRP == transfer->type);
    return (WKTransferXRP) transfer;
}

typedef struct {
    BRRippleTransaction xrpTransaction;
} WKTransferCreateContextXRP;

extern BRRippleTransaction
wkTransferAsXRP (WKTransfer transfer) {
    WKTransferXRP transferXRP = wkTransferCoerceXRP (transfer);
    return transferXRP->xrpTransaction;
}

static void
wkTransferCreateCallbackXRP (WKTransferCreateContext context,
                                    WKTransfer transfer) {
    WKTransferCreateContextXRP *contextXRP = (WKTransferCreateContextXRP*) context;
    WKTransferXRP transferXRP = wkTransferCoerceXRP (transfer);

    transferXRP->xrpTransaction = contextXRP->xrpTransaction;
}

extern WKTransfer
wkTransferCreateAsXRP (WKTransferListener listener,
                           const char *uids,
                           WKUnit unit,
                           WKUnit unitForFee,
                           WKTransferState state,
                           OwnershipKept BRRippleAccount xrpAccount,
                           OwnershipGiven BRRippleTransaction xrpTransfer) {
    
    WKTransferDirection direction = transferGetDirectionFromXRP (xrpTransfer, xrpAccount);
    
    WKAmount amount = wkAmountCreateAsXRP (unit,
                                                     WK_FALSE,
                                                     rippleTransactionGetAmount(xrpTransfer));

    WKFeeBasis feeBasisEstimated = wkFeeBasisCreateAsXRP (unitForFee, (WK_TRANSFER_RECEIVED == direction ? 0 : rippleTransactionGetFee(xrpTransfer)));
    
    WKAddress sourceAddress = wkAddressCreateAsXRP (rippleTransactionGetSource(xrpTransfer));
    WKAddress targetAddress = wkAddressCreateAsXRP (rippleTransactionGetTarget(xrpTransfer));

    WKTransferCreateContextXRP contextXRP = {
        xrpTransfer
    };

    WKTransfer transfer = wkTransferAllocAndInit (sizeof (struct WKTransferXRPRecord),
                                                            WK_NETWORK_TYPE_XRP,
                                                            listener,
                                                            uids,
                                                            unit,
                                                            unitForFee,
                                                            feeBasisEstimated,
                                                            amount,
                                                            direction,
                                                            sourceAddress,
                                                            targetAddress,
                                                            state,
                                                            &contextXRP,
                                                            wkTransferCreateCallbackXRP);

    wkFeeBasisGive (feeBasisEstimated);
    wkAddressGive (sourceAddress);
    wkAddressGive (targetAddress);

    return transfer;
}

static void
wkTransferReleaseXRP (WKTransfer transfer) {
    WKTransferXRP transferXRP = wkTransferCoerceXRP(transfer);
    rippleTransactionFree (transferXRP->xrpTransaction);
}

static WKHash
wkTransferGetHashXRP (WKTransfer transfer) {
    WKTransferXRP transferXRP = wkTransferCoerceXRP(transfer);
    BRRippleTransactionHash hash = rippleTransactionGetHash(transferXRP->xrpTransaction);
    return (rippleTransactionHashIsEmpty (hash)
            ? NULL
            : wkHashCreateAsXRP (hash));
}

static OwnershipGiven uint8_t *
wkTransferSerializeXRP (WKTransfer transfer,
                            WKNetwork network,
                            WKBoolean  requireSignature,
                            size_t *serializationCount) {
    assert (WK_TRUE == requireSignature);
    WKTransferXRP transferXRP = wkTransferCoerceXRP (transfer);

    uint8_t *serialization = NULL;
    *serializationCount = 0;
    BRRippleTransaction transaction = transferXRP->xrpTransaction;
    if (transaction) {
        serialization = rippleTransactionSerialize (transaction, serializationCount);
    }
    
    return serialization;
}

static int
wkTransferIsEqualXRP (WKTransfer tb1, WKTransfer tb2) {
    if (tb1 == tb2) return 1;

    WKHash h1 = wkTransferGetHashXRP (tb1);
    WKHash h2 = wkTransferGetHashXRP (tb2);

    int result = (WK_TRUE == wkHashEqual (h1, h2));

    wkHashGive (h2);
    wkHashGive (h1);

    return result;
}

static WKTransferDirection
transferGetDirectionFromXRP (BRRippleTransaction transaction,
                             BRRippleAccount account) {
    BRRippleAddress address = rippleAccountGetAddress(account);

    int isSource = rippleTransactionHasSource(transaction, address);
    int isTarget = rippleTransactionHasTarget(transaction, address);

    rippleAddressFree(address);

    return (isSource && isTarget
            ? WK_TRANSFER_RECOVERED
            : (isSource
               ? WK_TRANSFER_SENT
               : WK_TRANSFER_RECEIVED));
}

WKTransferHandlers wkTransferHandlersXRP = {
    wkTransferReleaseXRP,
    wkTransferGetHashXRP,
    NULL, // setHash
    NULL,
    wkTransferSerializeXRP,
    NULL, // getBytesForFeeEstimate
    wkTransferIsEqualXRP
};
