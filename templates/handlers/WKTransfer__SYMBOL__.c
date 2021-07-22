//
//  WKTransfer__SYMBOL__.c
//  WalletKitCore
//
//  Created by __USER__ on __DATE__.
//  Copyright © __YEAR__ Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WK__SYMBOL__.h"
#include "walletkit/WKAmountP.h"
#include "walletkit/WKHashP.h"
#include "__name__/BR__Name__Transfer.h"
#include "__name__/BR__Name__FeeBasis.h"
#include "support/util/BRUtilMath.h"

static WKTransferDirection
transferGetDirectionFrom__SYMBOL__ (BR__Name__Transfer transfer,
                             BR__Name__Account account);

extern WKTransfer__SYMBOL__
wkTransferCoerce__SYMBOL__ (WKTransfer transfer) {
    assert (WK_NETWORK_TYPE___SYMBOL__ == transfer->type);
    return (WKTransfer__SYMBOL__) transfer;
}

typedef struct {
    BR__Name__Transfer __symbol__Transfer;
} WKTransferCreateContext__SYMBOL__;

static void
wkTransferCreateCallback__SYMBOL__ (WKTransferCreateContext context,
                                    WKTransfer transfer) {
    WKTransferCreateContext__SYMBOL__ *context__SYMBOL__ = (WKTransferCreateContext__SYMBOL__*) context;
    WKTransfer__SYMBOL__ transfer__SYMBOL__ = wkTransferCoerce__SYMBOL__ (transfer);

    transfer__SYMBOL__->__symbol__Transfer = context__SYMBOL__->__symbol__Transfer;
}

extern WKTransfer
wkTransferCreateAs__SYMBOL__ (WKTransferListener listener,
                           const char *uids,
                           WKUnit unit,
                           WKUnit unitForFee,
                           WKTransferState state,
                           OwnershipKept BR__Name__Account __symbol__Account,
                           OwnershipGiven BR__Name__Transfer __symbol__Transfer) {
    
    WKTransferDirection direction = transferGetDirectionFrom__SYMBOL__ (__symbol__Transfer, __symbol__Account);
    
    WKAmount amount = wkAmountCreateAs__SYMBOL__ (unit,
                                                     WK_FALSE,
                                                     __name__TransferGetAmount (__symbol__Transfer));
    
    BR__Name__FeeBasis __symbol__FeeBasis = __name__FeeBasisCreateActual (WK_TRANSFER_RECEIVED == direction ? 0 : __name__TransferGetFee(__symbol__Transfer));
    WKFeeBasis feeBasis = wkFeeBasisCreateAs__SYMBOL__ (unitForFee,
                                                           __symbol__FeeBasis);
    
    WKAddress sourceAddress = wkAddressCreateAs__SYMBOL__ (__name__TransferGetSource (__symbol__Transfer));
    WKAddress targetAddress = wkAddressCreateAs__SYMBOL__ (__name__TransferGetTarget (__symbol__Transfer));

    WKTransferCreateContext__SYMBOL__ context__SYMBOL__ = {
        __symbol__Transfer
    };

    WKTransfer transfer = wkTransferAllocAndInit (sizeof (struct WKTransfer__SYMBOL__Record),
                                                            WK_NETWORK_TYPE___SYMBOL__,
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
                                                            &context__SYMBOL__,
                                                            wkTransferCreateCallback__SYMBOL__);
    
    wkFeeBasisGive (feeBasis);
    wkAddressGive (sourceAddress);
    wkAddressGive (targetAddress);

    return transfer;
}

static void
wkTransferRelease__SYMBOL__ (WKTransfer transfer) {
    WKTransfer__SYMBOL__ transfer__SYMBOL__ = wkTransferCoerce__SYMBOL__(transfer);
    __name__TransferFree (transfer__SYMBOL__->__symbol__Transfer);
}

static WKHash
wkTransferGetHash__SYMBOL__ (WKTransfer transfer) {
    WKTransfer__SYMBOL__ transfer__SYMBOL__ = wkTransferCoerce__SYMBOL__(transfer);
    BR__Name__Hash hash = __name__TransferGetTransactionId (transfer__SYMBOL__->__symbol__Transfer);
    return (__name__HashIsEmpty(hash)
            ? NULL
            : wkHashCreateAs__SYMBOL__ (hash));
}

static uint8_t *
wkTransferSerialize__SYMBOL__ (WKTransfer transfer,
                            WKNetwork network,
                            WKBoolean  requireSignature,
                            size_t *serializationCount) {
    WKTransfer__SYMBOL__ transfer__SYMBOL__ = wkTransferCoerce__SYMBOL__ (transfer);

    uint8_t *serialization = NULL;
    *serializationCount = 0;
    BR__Name__Transaction transaction = __name__TransferGetTransaction (transfer__SYMBOL__->__symbol__Transfer);
    if (transaction) {
        serialization = __name__TransactionGetSignedBytes (transaction, serializationCount);
    }
    
    return serialization;
}

static int
wkTransferIsEqual__SYMBOL__ (WKTransfer tb1, WKTransfer tb2) {
    if (tb1 == tb2) return 1;
    
    WKTransfer__SYMBOL__ tz1 = wkTransferCoerce__SYMBOL__ (tb1);
    WKTransfer__SYMBOL__ tz2 = wkTransferCoerce__SYMBOL__ (tb2);
    
    return __name__TransferIsEqual (tz1->__symbol__Transfer, tz2->__symbol__Transfer);
}

static WKTransferDirection
transferGetDirectionFrom__SYMBOL__ (BR__Name__Transfer transfer,
                             BR__Name__Account account) {
    BR__Name__Address source = __name__TransferGetSource (transfer);
    BR__Name__Address target = __name__TransferGetTarget (transfer);
    
    int isSource = __name__AccountHasAddress (account, source);
    int isTarget = __name__AccountHasAddress (account, target);
    
    __name__AddressFree (target);
    __name__AddressFree (source);
    
    return (isSource && isTarget
            ? WK_TRANSFER_RECOVERED
            : (isSource
               ? WK_TRANSFER_SENT
               : WK_TRANSFER_RECEIVED));
}

WKTransferHandlers wkTransferHandlers__SYMBOL__ = {
    wkTransferRelease__SYMBOL__,
    wkTransferGetHash__SYMBOL__,
    NULL, // setHash
    NULL, // updateIdentifier
    wkTransferSerialize__SYMBOL__,
    NULL, // getBytesForFeeEstimate
    wkTransferIsEqual__SYMBOL__
};
