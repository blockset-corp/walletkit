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
#include "tezos/BRTezosTransfer.h"
#include "tezos/BRTezosFeeBasis.h"
#include "support/util/BRUtilMath.h"

static WKTransferDirection
transferGetDirectionFromXTZ (BRTezosTransfer transfer,
                             BRTezosAccount account);

extern WKTransferXTZ
wkTransferCoerceXTZ (WKTransfer transfer) {
    assert (WK_NETWORK_TYPE_XTZ == transfer->type);
    return (WKTransferXTZ) transfer;
}

typedef struct {
    BRTezosTransfer xtzTransfer;
} WKTransferCreateContextXTZ;

static void
wkTransferCreateCallbackXTZ (WKTransferCreateContext context,
                                    WKTransfer transfer) {
    WKTransferCreateContextXTZ *contextXTZ = (WKTransferCreateContextXTZ*) context;
    WKTransferXTZ transferXTZ = wkTransferCoerceXTZ (transfer);

    transferXTZ->xtzTransfer = contextXTZ->xtzTransfer;
}

extern WKTransfer
wkTransferCreateAsXTZ (WKTransferListener listener,
                           const char *uids,
                           WKUnit unit,
                           WKUnit unitForFee,
                           WKTransferState state,
                           OwnershipKept BRTezosAccount xtzAccount,
                           OwnershipGiven BRTezosTransfer xtzTransfer) {
    
    WKTransferDirection direction = transferGetDirectionFromXTZ (xtzTransfer, xtzAccount);
    
    WKAmount amount = wkAmountCreateAsXTZ (unit,
                                                     WK_FALSE,
                                                     tezosTransferGetAmount (xtzTransfer));
    
    BRTezosFeeBasis xtzFeeBasis = tezosFeeBasisCreateActual (WK_TRANSFER_RECEIVED == direction ? 0 : tezosTransferGetFee(xtzTransfer));
    WKFeeBasis feeBasis = wkFeeBasisCreateAsXTZ (unitForFee,
                                                           xtzFeeBasis);
    
    WKAddress sourceAddress = wkAddressCreateAsXTZ (tezosTransferGetSource (xtzTransfer));
    WKAddress targetAddress = wkAddressCreateAsXTZ (tezosTransferGetTarget (xtzTransfer));

    WKTransferCreateContextXTZ contextXTZ = {
        xtzTransfer
    };

    WKTransfer transfer = wkTransferAllocAndInit (sizeof (struct WKTransferXTZRecord),
                                                            WK_NETWORK_TYPE_XTZ,
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
                                                            &contextXTZ,
                                                            wkTransferCreateCallbackXTZ);
    
    wkFeeBasisGive (feeBasis);
    wkAddressGive (sourceAddress);
    wkAddressGive (targetAddress);

    return transfer;
}

static void
wkTransferReleaseXTZ (WKTransfer transfer) {
    WKTransferXTZ transferXTZ = wkTransferCoerceXTZ(transfer);
    tezosTransferFree (transferXTZ->xtzTransfer);
}

static WKHash
wkTransferGetHashXTZ (WKTransfer transfer) {
    WKTransferXTZ transferXTZ = wkTransferCoerceXTZ(transfer);
    BRTezosHash hash = tezosTransferGetTransactionId (transferXTZ->xtzTransfer);
    return (tezosHashIsEmpty(hash)
            ? NULL
            : wkHashCreateAsXTZ (hash));
}

static uint8_t *
wkTransferSerializeXTZ (WKTransfer transfer,
                            WKNetwork network,
                            WKBoolean  requireSignature,
                            size_t *serializationCount) {
    WKTransferXTZ transferXTZ = wkTransferCoerceXTZ (transfer);

    uint8_t *serialization = NULL;
    *serializationCount = 0;
    BRTezosTransaction transaction = tezosTransferGetTransaction (transferXTZ->xtzTransfer);
    if (transaction) {
        serialization = tezosTransactionGetSignedBytes (transaction, serializationCount);
    }
    
    return serialization;
}

static int
wkTransferIsEqualXTZ (WKTransfer tb1, WKTransfer tb2) {
    if (tb1 == tb2) return 1;
    
    WKTransferXTZ tz1 = wkTransferCoerceXTZ (tb1);
    WKTransferXTZ tz2 = wkTransferCoerceXTZ (tb2);
    
    return tezosTransferIsEqual (tz1->xtzTransfer, tz2->xtzTransfer);
}

static WKTransferDirection
transferGetDirectionFromXTZ (BRTezosTransfer transfer,
                             BRTezosAccount account) {
    BRTezosAddress source = tezosTransferGetSource (transfer);
    BRTezosAddress target = tezosTransferGetTarget (transfer);
    
    int isSource = tezosAccountHasAddress (account, source);
    int isTarget = tezosAccountHasAddress (account, target);
    
    tezosAddressFree (target);
    tezosAddressFree (source);
    
    return (isSource && isTarget
            ? WK_TRANSFER_RECOVERED
            : (isSource
               ? WK_TRANSFER_SENT
               : WK_TRANSFER_RECEIVED));
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
