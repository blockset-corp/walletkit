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

#include "support/util/BRUtilMath.h"

static WKTransferDirection
__name__TransactionGetDirection (BR__Name__Transaction transfer,
                                 BR__Name__Account account);

typedef struct {
    BR__Name__Transaction __symbol__Transaction;
} WKTransferCreateContext__SYMBOL__;

static void
wkTransferCreateCallback__SYMBOL__ (WKTransferCreateContext context,
                                    WKTransfer transfer) {
    WKTransferCreateContext__SYMBOL__ *context__SYMBOL__ = (WKTransferCreateContext__SYMBOL__*) context;
    WKTransfer__SYMBOL__ transfer__SYMBOL__ = wkTransferCoerce__SYMBOL__ (transfer);

    transfer__SYMBOL__->__symbol__Transaction = context__SYMBOL__->__symbol__Transaction;
}

extern WKTransfer
wkTransferCreateAs__SYMBOL__ (WKTransferListener listener,
                              const char *uids,
                              WKUnit unit,
                              WKUnit unitForFee,
                              WKTransferState state,
                              OwnershipKept  BR__Name__Account     __symbol__Account,
                              OwnershipGiven BR__Name__Transaction __symbol__Transaction) {
    
    WKTransferDirection direction = __name__TransactionGetDirection (__symbol__Transaction, __symbol__Account);

    BR__Name__Amount __symbol__Amount = __name__TransactionGetAmount (__symbol__Transaction);
    WKAmount amount = wkAmountCreateAs__SYMBOL__ (unit, WK_FALSE, __symbol__Amount);

    BR__Name__FeeBasis __symbol__FeeBasis = __name__FeeBasisCreate ();
    WKFeeBasis feeBasis = wkFeeBasisCreateAs__SYMBOL__ (unitForFee, __symbol__FeeBasis);
    
    WKAddress sourceAddress = wkAddressCreateAs__SYMBOL__ (__name__TransactionGetSource (__symbol__Transaction));
    WKAddress targetAddress = wkAddressCreateAs__SYMBOL__ (__name__TransactionGetTarget (__symbol__Transaction));

    WKTransferCreateContext__SYMBOL__ context__SYMBOL__ = {
        __symbol__Transaction
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
    __name__TransactionFree (transfer__SYMBOL__->__symbol__Transaction);
}

static WKHash
wkTransferGetHash__SYMBOL__ (WKTransfer transfer) {
    WKTransfer__SYMBOL__ transfer__SYMBOL__ = wkTransferCoerce__SYMBOL__(transfer);

    BR__Name__Hash hash = __name__TransactionGetHash (transfer__SYMBOL__->__symbol__Transaction);
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

    *serializationCount = 0;

    return (NULL == transfer__SYMBOL__->__symbol__Transaction
            ? NULL
            : __name__TransactionGetSerialization (transfer__SYMBOL__->__symbol__Transaction, serializationCount));
}

static int
wkTransferIsEqual__SYMBOL__ (WKTransfer tb1, WKTransfer tb2) {
    if (tb1 == tb2) return 1;
    
    WKTransfer__SYMBOL__ tz1 = wkTransferCoerce__SYMBOL__ (tb1);
    WKTransfer__SYMBOL__ tz2 = wkTransferCoerce__SYMBOL__ (tb2);
    
    return __name__TransactionEqual (tz1->__symbol__Transaction, tz2->__symbol__Transaction);
}

static WKTransferDirection
__name__TransactionGetDirection (BR__Name__Transaction transfer,
                                    BR__Name__Account account) {
    BR__Name__Address source = __name__TransactionGetSource (transfer);
    BR__Name__Address target = __name__TransactionGetTarget (transfer);
    
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
