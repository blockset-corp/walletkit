//
//  WKTransferXTZ.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXTZ.h"
#include "walletkit/WKAmountP.h"
#include "walletkit/WKHashP.h"
#include "tezos/BRTezosTransaction.h"
#include "tezos/BRTezosFeeBasis.h"
#include "support/util/BRUtilMath.h"

static WKTransferDirection
wkTransferGetDirectionFromXTZ (BRTezosAccount account,
                               BRTezosAddress source,
                               BRTezosAddress target) {
    int isSource = tezosAccountHasAddress (account, source);
    int isTarget = tezosAccountHasAddress (account, target);

    return (isSource && isTarget
            ? WK_TRANSFER_RECOVERED
            : (isSource
               ? WK_TRANSFER_SENT
               : WK_TRANSFER_RECEIVED));
}

extern WKTransferXTZ
wkTransferCoerceXTZ (WKTransfer transfer) {
    assert (WK_NETWORK_TYPE_XTZ == transfer->type);
    return (WKTransferXTZ) transfer;
}

typedef struct {
    BRTezosHash xtzHash;
    BRTezosUnitMutez xtzAmount;
    BRTezosTransaction xtzTransaction;
} WKTransferCreateContextXTZ;

static void
wkTransferCreateCallbackXTZ (WKTransferCreateContext context,
                                    WKTransfer transfer) {
    WKTransferCreateContextXTZ *contextXTZ = (WKTransferCreateContextXTZ*) context;
    WKTransferXTZ transferXTZ = wkTransferCoerceXTZ (transfer);

    transferXTZ->hash = contextXTZ->xtzHash;
    transferXTZ->amount = contextXTZ->xtzAmount;
    transferXTZ->originatingTransaction = contextXTZ->xtzTransaction;
}

extern WKTransfer
wkTransferCreateAsXTZ (WKTransferListener listener,
                       const char *uids,
                       WKUnit unit,
                       WKUnit unitForFee,
                       WKFeeBasis feeBasisEstimated,
                       WKAmount   amount,
                       WKAddress  source,
                       WKAddress  target,
                       WKTransferState state,
                       OwnershipKept BRTezosAccount xtzAccount,
                       BRTezosHash xtzHash,
                       OwnershipGiven BRTezosTransaction xtzTransaction) {
    WKTransferCreateContextXTZ contextXTZ = {
        xtzHash,
        0, // xtzAmount,
        xtzTransaction
    };

    WKTransferDirection direction = wkTransferGetDirectionFromXTZ (xtzAccount,
                                                                   wkAddressAsXTZ(source),
                                                                   wkAddressAsXTZ(target));

    return wkTransferAllocAndInit (sizeof (struct WKTransferXTZRecord),
                                   WK_NETWORK_TYPE_XTZ,
                                   listener,
                                   uids,
                                   unit,
                                   unitForFee,
                                   feeBasisEstimated,
                                   amount,
                                   direction,
                                   source,
                                   target,
                                   state,
                                   &contextXTZ,
                                   wkTransferCreateCallbackXTZ);
}

static void
wkTransferReleaseXTZ (WKTransfer transfer) {
    WKTransferXTZ transferXTZ = wkTransferCoerceXTZ(transfer);
    if (NULL != transferXTZ->originatingTransaction)
        tezosTransactionFree (transferXTZ->originatingTransaction);
}

static WKHash
wkTransferGetHashXTZ (WKTransfer transfer) {
    WKTransferXTZ transferXTZ = wkTransferCoerceXTZ(transfer);
    return (tezosHashIsEmpty (transferXTZ->hash)
            ? NULL
            : wkHashCreateAsXTZ (transferXTZ->hash));
}

static uint8_t *
wkTransferSerializeXTZ (WKTransfer transfer,
                            WKNetwork network,
                            WKBoolean  requireSignature,
                            size_t *serializationCount) {
    WKTransferXTZ transferXTZ = wkTransferCoerceXTZ (transfer);

    *serializationCount = 0;

    return (NULL == transferXTZ->originatingTransaction
            ? NULL
            : tezosTransactionGetSignedBytes (transferXTZ->originatingTransaction, serializationCount));
}

static int
wkTransferIsEqualXTZ (WKTransfer tb1, WKTransfer tb2) {
    if (tb1 == tb2) return 1;
    
    WKTransferXTZ tz1 = wkTransferCoerceXTZ (tb1);
    WKTransferXTZ tz2 = wkTransferCoerceXTZ (tb2);

    return (NULL != tz1->originatingTransaction &&
            NULL != tz2->originatingTransaction &&
            tezosTransactionEqual (tz1->originatingTransaction,
                                   tz2->originatingTransaction));
}

WKTransferHandlers wkTransferHandlersXTZ = {
    wkTransferReleaseXTZ,
    wkTransferGetHashXTZ,
    NULL, // setHash
    NULL, // updateIdentifier
    wkTransferSerializeXTZ,
    NULL, // getBytesForFeeEstimate
    wkTransferIsEqualXTZ
};
