//
//  WKWallet__SYMBOL__.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WK__SYMBOL__.h"
#include "WKBase.h"
#include "walletkit/WKWalletP.h"
#include "walletkit/WKAmountP.h"
#include "walletkit/WKFeeBasisP.h"
#include "__name__/BR__Name__.h"
#include "support/BRSet.h"
#include "support/util/BRUtilMath.h"

#include <stdio.h>
#include <errno.h>

static WKWallet__SYMBOL__
wkWalletCoerce (WKWallet wallet) {
    assert (WK_NETWORK_TYPE___SYMBOL__ == wallet->type);
    return (WKWallet__SYMBOL__) wallet;
}

typedef struct {
    BR__Name__Account __symbol__Account;
} WKWalletCreateContext__SYMBOL__;

static void
wkWalletCreateCallback__SYMBOL__ (WKWalletCreateContext context,
                               WKWallet wallet) {
    WKWalletCreateContext__SYMBOL__ *context__SYMBOL__ = (WKWalletCreateContext__SYMBOL__*) context;
    WKWallet__SYMBOL__ wallet__SYMBOL__ = wkWalletCoerce (wallet);

    wallet__SYMBOL__->__symbol__Account = context__SYMBOL__->__symbol__Account;
}

private_extern WKWallet
wkWalletCreateAs__SYMBOL__ (WKWalletListener listener,
                         WKUnit unit,
                         WKUnit unitForFee,
                         BR__Name__Account __symbol__Account) {
    int hasMinBalance;
    int hasMaxBalance;
    BR__Name__UnitMutez minBalance__SYMBOL__ = __name__AccountGetBalanceLimit (__symbol__Account, 0, &hasMinBalance);
    BR__Name__UnitMutez maxBalance__SYMBOL__ = __name__AccountGetBalanceLimit (__symbol__Account, 1, &hasMaxBalance);

    WKAmount minBalance = hasMinBalance ? wkAmountCreateAs__SYMBOL__(unit, WK_FALSE, minBalance__SYMBOL__) : NULL;
    WKAmount maxBalance = hasMaxBalance ? wkAmountCreateAs__SYMBOL__(unit, WK_FALSE, maxBalance__SYMBOL__) : NULL;

    BR__Name__FeeBasis feeBasis__SYMBOL__ = __name__DefaultFeeBasis (__NAME___DEFAULT_MUTEZ_PER_BYTE);
    WKFeeBasis feeBasis   = wkFeeBasisCreateAs__SYMBOL__ (unitForFee, feeBasis__SYMBOL__);

    WKWalletCreateContext__SYMBOL__ context__SYMBOL__ = {
        __symbol__Account
    };

    WKWallet wallet = wkWalletAllocAndInit (sizeof (struct WKWallet__SYMBOL__Record),
                                                      WK_NETWORK_TYPE___SYMBOL__,
                                                      listener,
                                                      unit,
                                                      unitForFee,
                                                      minBalance,
                                                      maxBalance,
                                                      feeBasis,
                                                      &context__SYMBOL__,
                                                      wkWalletCreateCallback__SYMBOL__);

    wkFeeBasisGive(feeBasis);
    wkAmountGive (maxBalance);
    wkAmountGive (minBalance);

    return wallet;
}

static void
wkWalletRelease__SYMBOL__ (WKWallet wallet) {
    WKWallet__SYMBOL__ wallet__SYMBOL__ = wkWalletCoerce (wallet);
    __name__AccountFree(wallet__SYMBOL__->__symbol__Account);
}

static WKAddress
wkWalletGetAddress__SYMBOL__ (WKWallet wallet,
                           WKAddressScheme addressScheme) {
    assert (WK_ADDRESS_SCHEME_NATIVE == addressScheme);
    WKWallet__SYMBOL__ wallet__SYMBOL__ = wkWalletCoerce (wallet);
    return wkAddressCreateAs__SYMBOL__ (__name__AccountGetAddress(wallet__SYMBOL__->__symbol__Account));
}

static bool
wkWalletHasAddress__SYMBOL__ (WKWallet wallet,
                           WKAddress address) {
    WKWallet__SYMBOL__ wallet__SYMBOL__ = wkWalletCoerce (wallet);
    BR__Name__Address __symbol__Address = wkAddressAs__SYMBOL__ (address);
    
    return __name__AccountHasAddress (wallet__SYMBOL__->__symbol__Account, __symbol__Address);
}

private_extern bool
wkWalletNeedsReveal__SYMBOL__ (WKWallet wallet) {
    assert(wallet);
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        // reveal is needed before the first outgoing transfer
        WKTransferDirection direction = wkTransferGetDirection (wallet->transfers[index]);
        if (WK_TRANSFER_SENT == direction) return false;
    }
    return true;
}

private_extern WKTransfer
wkWalletGetTransferByHashOrUIDSAndTarget__SYMBOL__ (WKWallet wallet,
                                             WKHash hashToMatch,
                                             const char *uids,
                                             WKAddress targetToMatch) {
    WKTransfer transfer = NULL;

    // Do the 'special match' based on hash and/or uids
    transfer = wkWalletGetTransferByHashOrUIDS (wallet, hashToMatch, uids);

    // Confirmed with the address
    if (NULL != transfer && !wkAddressIsEqual (transfer->targetAddress, targetToMatch)) {
        wkTransferGive(transfer);
        transfer = NULL;
    }

    return transfer;
}

extern size_t
wkWalletGetTransferAttributeCount__SYMBOL__ (WKWallet wallet,
                                          WKAddress target) {
    size_t countRequired, countOptional;
    __name__GetTransactionAttributeKeys (1, &countRequired);
    __name__GetTransactionAttributeKeys (0, &countOptional);
    return countRequired + countOptional;
}

extern WKTransferAttribute
wkWalletGetTransferAttributeAt__SYMBOL__ (WKWallet wallet,
                                       WKAddress target,
                                       size_t index) {
    size_t countRequired, countOptional;
    const char **keysRequired = __name__GetTransactionAttributeKeys (1, &countRequired);
    const char **keysOptional = __name__GetTransactionAttributeKeys (0, &countOptional);

    assert (index < (countRequired + countOptional));

    WKBoolean isRequired = AS_WK_BOOLEAN (index < countRequired);
    const char **keys      = (isRequired ? keysRequired : keysOptional);
    size_t       keysIndex = (isRequired ? index : (index - countRequired));

    return wkTransferAttributeCreate(keys[keysIndex], NULL, isRequired);
}

static int // 1 if equal, 0 if not.
__name__CompareFieldOption (const char *t1, const char *t2) {
    return 0 == strcasecmp (t1, t2);
}

extern WKTransferAttributeValidationError
wkWalletValidateTransferAttribute__SYMBOL__ (WKWallet wallet,
                                          OwnershipKept WKTransferAttribute attribute,
                                          WKBoolean *validates) {
    const char *key = wkTransferAttributeGetKey (attribute);
    const char *val = wkTransferAttributeGetValue (attribute);
    WKTransferAttributeValidationError error = 0;

    // If attribute.value is NULL, we validate unless the attribute.value is required.
    if (NULL == val) {
        if (wkTransferAttributeIsRequired(attribute)) {
            error = WK_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_REQUIRED_BUT_NOT_PROVIDED;
            *validates = WK_FALSE;
        } else {
            *validates = WK_TRUE;
        }
        return error;
    }

    if (__name__CompareFieldOption (key, FIELD_OPTION_DELEGATION_OP)) {
        // expect 0 or 1
        char *end = NULL;
        errno = 0;

        uintmax_t flag = strtoumax (val, &end, 10);
        if (ERANGE != errno && EINVAL != errno && '\0' == end[0] && flag >= 0 && flag <= 1) {
            *validates = WK_TRUE;
        } else {
            *validates = WK_FALSE;
            error = WK_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_MISMATCHED_TYPE;
        }
    }
    else if (__name__CompareFieldOption (key, FIELD_OPTION_DELEGATE)) {
        // expect string
        *validates = WK_TRUE;
    }
    else if (__name__CompareFieldOption (key, FIELD_OPTION_OPERATION_TYPE)) {
        // expect string
        *validates = WK_TRUE;
    }

    else {
        error = WK_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_RELATIONSHIP_INCONSISTENCY;
        *validates = WK_FALSE;
    }
    
    return error;
}

// create for send
extern WKTransfer
wkWalletCreateTransfer__SYMBOL__ (WKWallet  wallet,
                               WKAddress target,
                               WKAmount  amount,
                               WKFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept WKTransferAttribute *attributes,
                               WKCurrency currency,
                               WKUnit unit,
                               WKUnit unitForFee) {
    WKWallet__SYMBOL__ wallet__SYMBOL__ = wkWalletCoerce (wallet);
    
    BR__Name__Address source  = __name__AccountGetAddress (wallet__SYMBOL__->__symbol__Account);
    BR__Name__Address __symbol__Target  = wkAddressAs__SYMBOL__ (target);
    BR__Name__UnitMutez mutez = __name__MutezCreate (amount);
    BR__Name__FeeBasis feeBasis = wkFeeBasisCoerce__SYMBOL__ (estimatedFeeBasis)->__symbol__FeeBasis;
    int64_t counter = (FEE_BASIS_ESTIMATE == feeBasis.type) ? feeBasis.u.estimate.counter : 0;
    
    bool delegationOp = false;
    
    for (size_t index = 0; index < attributesCount; index++) {
        WKTransferAttribute attribute = attributes[index];
        if (NULL != wkTransferAttributeGetValue(attribute)) {
            if (__name__CompareFieldOption (wkTransferAttributeGetKey(attribute), FIELD_OPTION_DELEGATION_OP)) {
                int op;
                sscanf (wkTransferAttributeGetValue(attribute), "%d", &op);
                delegationOp = (op == 1);
            }
        }
    }
    
    BR__Name__Transfer __symbol__Transfer = __name__TransferCreateNew (source,
                                                          __symbol__Target,
                                                          mutez,
                                                          feeBasis,
                                                          counter,
                                                          delegationOp);

    __name__AddressFree (source);

    WKTransferState state    = wkTransferStateInit (WK_TRANSFER_STATE_CREATED);
    WKTransfer      transfer = wkTransferCreateAs__SYMBOL__ (wallet->listenerTransfer,
                                                                NULL,
                                                                unit,
                                                                unitForFee,
                                                                state,
                                                                wallet__SYMBOL__->__symbol__Account,
                                                                __symbol__Transfer);
    
    wkTransferSetAttributes (transfer, attributesCount, attributes);
    wkTransferStateGive (state);
    
    return transfer;
}

extern WKTransfer
wkWalletCreateTransferMultiple__SYMBOL__ (WKWallet wallet,
                                       size_t outputsCount,
                                       WKTransferOutput *outputs,
                                       WKFeeBasis estimatedFeeBasis,
                                       WKCurrency currency,
                                       WKUnit unit,
                                       WKUnit unitForFee) {
    // not supported
    return NULL;
}

static OwnershipGiven BRSetOf(WKAddress)
wkWalletGetAddressesForRecovery__SYMBOL__ (WKWallet wallet) {
    BRSetOf(WKAddress) addresses = wkAddressSetCreate (1);

    WKWallet__SYMBOL__ wallet__SYMBOL__ = wkWalletCoerce(wallet);

    BRSetAdd (addresses, wkAddressCreateAs__SYMBOL__ (__name__AccountGetAddress (wallet__SYMBOL__->__symbol__Account)));

    return addresses;
}

static bool
wkWalletIsEqual__SYMBOL__ (WKWallet wb1, WKWallet wb2) {
    if (wb1 == wb2) return true;

    WKWallet__SYMBOL__ w1 = wkWalletCoerce(wb1);
    WKWallet__SYMBOL__ w2 = wkWalletCoerce(wb2);
    return w1->__symbol__Account == w2->__symbol__Account;
}

WKWalletHandlers wkWalletHandlers__SYMBOL__ = {
    wkWalletRelease__SYMBOL__,
    wkWalletGetAddress__SYMBOL__,
    wkWalletHasAddress__SYMBOL__,
    wkWalletGetTransferAttributeCount__SYMBOL__,
    wkWalletGetTransferAttributeAt__SYMBOL__,
    wkWalletValidateTransferAttribute__SYMBOL__,
    wkWalletCreateTransfer__SYMBOL__,
    wkWalletCreateTransferMultiple__SYMBOL__,
    wkWalletGetAddressesForRecovery__SYMBOL__,
    NULL,//WKWalletAnnounceTransfer
    wkWalletIsEqual__SYMBOL__
};
