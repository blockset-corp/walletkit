//
//  WKWalletXTZ.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXTZ.h"
#include "WKBase.h"
#include "walletkit/WKWalletP.h"
#include "walletkit/WKAmountP.h"
#include "walletkit/WKFeeBasisP.h"
#include "tezos/BRTezos.h"
#include "support/BRSet.h"
#include "support/util/BRUtilMath.h"

#include <stdio.h>
#include <errno.h>

static WKWalletXTZ
wkWalletCoerce (WKWallet wallet) {
    assert (WK_NETWORK_TYPE_XTZ == wallet->type);
    return (WKWalletXTZ) wallet;
}

typedef struct {
    BRTezosAccount xtzAccount;
} WKWalletCreateContextXTZ;

static void
wkWalletCreateCallbackXTZ (WKWalletCreateContext context,
                               WKWallet wallet) {
    WKWalletCreateContextXTZ *contextXTZ = (WKWalletCreateContextXTZ*) context;
    WKWalletXTZ walletXTZ = wkWalletCoerce (wallet);

    walletXTZ->xtzAccount = contextXTZ->xtzAccount;
}

private_extern WKWallet
wkWalletCreateAsXTZ (WKWalletListener listener,
                         WKUnit unit,
                         WKUnit unitForFee,
                         BRTezosAccount xtzAccount) {
    int hasMinBalance;
    int hasMaxBalance;
    BRTezosUnitMutez minBalanceXTZ = tezosAccountGetBalanceLimit (xtzAccount, 0, &hasMinBalance);
    BRTezosUnitMutez maxBalanceXTZ = tezosAccountGetBalanceLimit (xtzAccount, 1, &hasMaxBalance);

    WKAmount minBalance = hasMinBalance ? wkAmountCreateAsXTZ(unit, WK_FALSE, minBalanceXTZ) : NULL;
    WKAmount maxBalance = hasMaxBalance ? wkAmountCreateAsXTZ(unit, WK_FALSE, maxBalanceXTZ) : NULL;

    BRTezosFeeBasis feeBasisXTZ = tezosDefaultFeeBasis (TEZOS_DEFAULT_MUTEZ_PER_BYTE);
    WKFeeBasis feeBasis   = wkFeeBasisCreateAsXTZ (unitForFee, feeBasisXTZ);

    WKWalletCreateContextXTZ contextXTZ = {
        xtzAccount
    };

    WKWallet wallet = wkWalletAllocAndInit (sizeof (struct WKWalletXTZRecord),
                                                      WK_NETWORK_TYPE_XTZ,
                                                      listener,
                                                      unit,
                                                      unitForFee,
                                                      minBalance,
                                                      maxBalance,
                                                      feeBasis,
                                                      &contextXTZ,
                                                      wkWalletCreateCallbackXTZ);

    wkFeeBasisGive(feeBasis);
    wkAmountGive (maxBalance);
    wkAmountGive (minBalance);

    return wallet;
}

static void
wkWalletReleaseXTZ (WKWallet wallet) {
    WKWalletXTZ walletXTZ = wkWalletCoerce (wallet);
    tezosAccountFree(walletXTZ->xtzAccount);
}

static WKAddress
wkWalletGetAddressXTZ (WKWallet wallet,
                           WKAddressScheme addressScheme) {
    assert (WK_ADDRESS_SCHEME_NATIVE == addressScheme);
    WKWalletXTZ walletXTZ = wkWalletCoerce (wallet);
    return wkAddressCreateAsXTZ (tezosAccountGetAddress(walletXTZ->xtzAccount));
}

static bool
wkWalletHasAddressXTZ (WKWallet wallet,
                           WKAddress address) {
    WKWalletXTZ walletXTZ = wkWalletCoerce (wallet);
    BRTezosAddress xtzAddress = wkAddressAsXTZ (address);
    
    return tezosAccountHasAddress (walletXTZ->xtzAccount, xtzAddress);
}

private_extern bool
wkWalletNeedsRevealXTZ (WKWallet wallet) {
    assert(wallet);
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        // reveal is needed before the first outgoing transfer
        WKTransferDirection direction = wkTransferGetDirection (wallet->transfers[index]);
        if (WK_TRANSFER_SENT == direction) return false;
    }
    return true;
}

private_extern WKTransfer
wkWalletGetTransferByHashOrUIDSAndTargetXTZ (WKWallet wallet,
                                             WKHash hashToMatch,
                                             const char *uids,
                                             WKAddress targetToMatch) {
    WKTransfer transfer = NULL;
    
    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; NULL == transfer && index < array_count(wallet->transfers); index++) {
        WKHash hash = wkTransferGetHash (wallet->transfers[index]);
        if (((NULL != uids && NULL != wallet->transfers[index]->uids && 0 == strcmp (uids, wallet->transfers[index]->uids))
             || WK_TRUE == wkHashEqual(hash, hashToMatch)) &&

            wkAddressIsEqual(wallet->transfers[index]->targetAddress, targetToMatch))
            transfer = wallet->transfers[index];
        wkHashGive(hash);
    }
    pthread_mutex_unlock (&wallet->lock);
    
    return wkTransferTake (transfer);
}

extern size_t
wkWalletGetTransferAttributeCountXTZ (WKWallet wallet,
                                          WKAddress target) {
    size_t countRequired, countOptional;
    tezosGetTransactionAttributeKeys (1, &countRequired);
    tezosGetTransactionAttributeKeys (0, &countOptional);
    return countRequired + countOptional;
}

extern WKTransferAttribute
wkWalletGetTransferAttributeAtXTZ (WKWallet wallet,
                                       WKAddress target,
                                       size_t index) {
    size_t countRequired, countOptional;
    const char **keysRequired = tezosGetTransactionAttributeKeys (1, &countRequired);
    const char **keysOptional = tezosGetTransactionAttributeKeys (0, &countOptional);

    assert (index < (countRequired + countOptional));

    WKBoolean isRequired = AS_WK_BOOLEAN (index < countRequired);
    const char **keys      = (isRequired ? keysRequired : keysOptional);
    size_t       keysIndex = (isRequired ? index : (index - countRequired));

    return wkTransferAttributeCreate(keys[keysIndex], NULL, isRequired);
}

static int // 1 if equal, 0 if not.
tezosCompareFieldOption (const char *t1, const char *t2) {
    return 0 == strcasecmp (t1, t2);
}

extern WKTransferAttributeValidationError
wkWalletValidateTransferAttributeXTZ (WKWallet wallet,
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

    if (tezosCompareFieldOption (key, FIELD_OPTION_DELEGATION_OP)) {
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
    else if (tezosCompareFieldOption (key, FIELD_OPTION_DELEGATE)) {
        // expect string
        *validates = WK_TRUE;
    }
    else if (tezosCompareFieldOption (key, FIELD_OPTION_OPERATION_TYPE)) {
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
wkWalletCreateTransferXTZ (WKWallet  wallet,
                               WKAddress target,
                               WKAmount  amount,
                               WKFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept WKTransferAttribute *attributes,
                               WKCurrency currency,
                               WKUnit unit,
                               WKUnit unitForFee) {
    WKWalletXTZ walletXTZ = wkWalletCoerce (wallet);
    
    BRTezosAddress source  = tezosAccountGetAddress (walletXTZ->xtzAccount);
    BRTezosAddress xtzTarget  = wkAddressAsXTZ (target);
    BRTezosUnitMutez mutez = tezosMutezCreate (amount);
    BRTezosFeeBasis feeBasis = wkFeeBasisCoerceXTZ (estimatedFeeBasis)->xtzFeeBasis;
    int64_t counter = (FEE_BASIS_ESTIMATE == feeBasis.type) ? feeBasis.u.estimate.counter : 0;
    
    bool delegationOp = false;
    
    for (size_t index = 0; index < attributesCount; index++) {
        WKTransferAttribute attribute = attributes[index];
        if (NULL != wkTransferAttributeGetValue(attribute)) {
            if (tezosCompareFieldOption (wkTransferAttributeGetKey(attribute), FIELD_OPTION_DELEGATION_OP)) {
                int op;
                sscanf (wkTransferAttributeGetValue(attribute), "%d", &op);
                delegationOp = (op == 1);
            }
        }
    }
    
    BRTezosTransfer xtzTransfer = tezosTransferCreateNew (source,
                                                          xtzTarget,
                                                          mutez,
                                                          feeBasis,
                                                          counter,
                                                          delegationOp);

    tezosAddressFree (source);

    WKTransferState state    = wkTransferStateInit (WK_TRANSFER_STATE_CREATED);
    WKTransfer      transfer = wkTransferCreateAsXTZ (wallet->listenerTransfer,
                                                                NULL,
                                                                unit,
                                                                unitForFee,
                                                                state,
                                                                walletXTZ->xtzAccount,
                                                                xtzTransfer);
    
    wkTransferSetAttributes (transfer, attributesCount, attributes);
    wkTransferStateGive (state);
    
    return transfer;
}

extern WKTransfer
wkWalletCreateTransferMultipleXTZ (WKWallet wallet,
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
wkWalletGetAddressesForRecoveryXTZ (WKWallet wallet) {
    BRSetOf(WKAddress) addresses = wkAddressSetCreate (1);

    WKWalletXTZ walletXTZ = wkWalletCoerce(wallet);

    BRSetAdd (addresses, wkAddressCreateAsXTZ (tezosAccountGetAddress (walletXTZ->xtzAccount)));

    return addresses;
}

static bool
wkWalletIsEqualXTZ (WKWallet wb1, WKWallet wb2) {
    if (wb1 == wb2) return true;

    WKWalletXTZ w1 = wkWalletCoerce(wb1);
    WKWalletXTZ w2 = wkWalletCoerce(wb2);
    return w1->xtzAccount == w2->xtzAccount;
}

WKWalletHandlers wkWalletHandlersXTZ = {
    wkWalletReleaseXTZ,
    wkWalletGetAddressXTZ,
    wkWalletHasAddressXTZ,
    wkWalletGetTransferAttributeCountXTZ,
    wkWalletGetTransferAttributeAtXTZ,
    wkWalletValidateTransferAttributeXTZ,
    wkWalletCreateTransferXTZ,
    wkWalletCreateTransferMultipleXTZ,
    wkWalletGetAddressesForRecoveryXTZ,
    NULL,//WKWalletAnnounceTransfer
    wkWalletIsEqualXTZ
};
