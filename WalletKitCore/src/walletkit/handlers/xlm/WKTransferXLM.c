//
//  WKTransferXLM.c
//  WalletKitCore
//
//  Created by Carl Cherry on 2021-05-21.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXLM.h"
#include "walletkit/WKAmountP.h"
#include "walletkit/WKHashP.h"
#include "stellar/BRStellarTransaction.h"
#include "support/util/BRUtilMath.h"

static WKTransferDirection
transferGetDirectionFromXLM (BRStellarTransaction transaction,
                             BRStellarAccount account);

extern WKTransferXLM
wkTransferCoerceXLM (WKTransfer transfer) {
    assert (WK_NETWORK_TYPE_XLM == transfer->type);
    return (WKTransferXLM) transfer;
}

typedef struct {
    BRStellarTransaction xlmTransaction;
} WKTransferCreateContextXLM;

extern BRStellarTransaction
wkTransferAsXLM (WKTransfer transfer) {
    WKTransferXLM transferXLM = wkTransferCoerceXLM (transfer);
    return transferXLM->xlmTransaction;
}

static void
wkTransferCreateCallbackXLM (WKTransferCreateContext context,
                                    WKTransfer transfer) {
    WKTransferCreateContextXLM *contextXLM = (WKTransferCreateContextXLM*) context;
    WKTransferXLM transferXLM = wkTransferCoerceXLM (transfer);

    transferXLM->xlmTransaction = contextXLM->xlmTransaction;
}

extern WKTransfer
wkTransferCreateAsXLM (WKTransferListener listener,
                           const char *uids,
                           WKUnit unit,
                           WKUnit unitForFee,
                           WKTransferState state,
                           OwnershipKept BRStellarAccount xlmAccount,
                           OwnershipGiven BRStellarTransaction xlmTransfer) {
    
    WKTransferDirection direction = transferGetDirectionFromXLM (xlmTransfer, xlmAccount);
    
    WKAmount amount = wkAmountCreateAsXLM (unit,
                                                     WK_FALSE,
                                                     stellarTransactionGetAmount(xlmTransfer));

    WKFeeBasis feeBasisEstimated = wkFeeBasisCreateAsXLM (unitForFee, (WK_TRANSFER_RECEIVED == direction ? 0 : stellarTransactionGetFee(xlmTransfer)));
    
    WKAddress sourceAddress = wkAddressCreateAsXLM (stellarTransactionGetSource(xlmTransfer));
    WKAddress targetAddress = wkAddressCreateAsXLM (stellarTransactionGetTarget(xlmTransfer));

    WKTransferCreateContextXLM contextXLM = {
        xlmTransfer
    };

    WKTransfer transfer = wkTransferAllocAndInit (sizeof (struct WKTransferXLMRecord),
                                                            WK_NETWORK_TYPE_XLM,
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
                                                            &contextXLM,
                                                            wkTransferCreateCallbackXLM);

    wkFeeBasisGive (feeBasisEstimated);
    wkAddressGive (sourceAddress);
    wkAddressGive (targetAddress);

    return transfer;
}

static void
wkTransferReleaseXLM (WKTransfer transfer) {
    WKTransferXLM transferXLM = wkTransferCoerceXLM(transfer);
    stellarTransactionFree (transferXLM->xlmTransaction);
}

static WKHash
wkTransferGetHashXLM (WKTransfer transfer) {
    WKTransferXLM transferXLM = wkTransferCoerceXLM(transfer);
    BRStellarTransactionHash hash = stellarTransactionGetHash(transferXLM->xlmTransaction);
    return wkHashCreateAsXLM (hash);
}

static uint8_t *
wkTransferSerializeXLM (WKTransfer transfer,
                            WKNetwork network,
                            WKBoolean  requireSignature,
                            size_t *serializationCount) {
    assert (WK_TRUE == requireSignature);
    WKTransferXLM transferXLM = wkTransferCoerceXLM (transfer);

    uint8_t *serialization = NULL;
    *serializationCount = 0;
    BRStellarTransaction transaction = transferXLM->xlmTransaction;
    if (transaction) {
        serialization = stellarTransactionSerialize (transaction, serializationCount);
    }
    
    return serialization;
}

static int
wkTransferIsEqualXLM (WKTransfer tb1, WKTransfer tb2) {
    if (tb1 == tb2) return 1;

    WKHash h1 = wkTransferGetHashXLM (tb1);
    WKHash h2 = wkTransferGetHashXLM (tb2);

    int result = (WK_TRUE == wkHashEqual (h1, h2));

    wkHashGive (h2);
    wkHashGive (h1);

    return result;
}

static WKTransferDirection
transferGetDirectionFromXLM (BRStellarTransaction transaction,
                             BRStellarAccount account) {
    BRStellarAddress address = stellarAccountGetAddress(account);

    int isSource = stellarTransactionHasSource(transaction, address);
    int isTarget = stellarTransactionHasTarget(transaction, address);

    stellarAddressFree(address);

    return (isSource && isTarget
            ? WK_TRANSFER_RECOVERED
            : (isSource
               ? WK_TRANSFER_SENT
               : WK_TRANSFER_RECEIVED));
}

WKTransferHandlers wkTransferHandlersXLM = {
    wkTransferReleaseXLM,
    wkTransferGetHashXLM,
    NULL, // setHash
    NULL,
    wkTransferSerializeXLM,
    NULL, // getBytesForFeeEstimate
    wkTransferIsEqualXLM
};
