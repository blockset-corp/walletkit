//
//  WKWalletHBAR.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKHBAR.h"
#include "WKBase.h"
#include "walletkit/WKWalletP.h"
#include "walletkit/WKAmountP.h"
#include "walletkit/WKFeeBasisP.h"
#include "hedera/BRHederaTransaction.h"
#include "support/BRSet.h"
#include "support/util/BRUtilMath.h"
#include <stdio.h>

static WKWalletHBAR
wkWalletCoerce (WKWallet wallet) {
    assert (WK_NETWORK_TYPE_HBAR == wallet->type);
    return (WKWalletHBAR) wallet;
}

typedef struct {
    BRHederaAccount hbarAccount;
} WKWalletCreateContextHBAR;

static void
wkWalletCreateCallbackHBAR (WKWalletCreateContext context,
                               WKWallet wallet) {
    WKWalletCreateContextHBAR *contextHBAR = (WKWalletCreateContextHBAR*) context;
    WKWalletHBAR walletHBAR = wkWalletCoerce (wallet);

    walletHBAR->hbarAccount = contextHBAR->hbarAccount;
}

private_extern WKWallet
wkWalletCreateAsHBAR (WKWalletListener listener,
                          WKUnit unit,
                          WKUnit unitForFee,
                          BRHederaAccount hbarAccount) {
    int hasMinBalance;
    int hasMaxBalance;
    BRHederaUnitTinyBar minBalanceHBAR = hederaAccountGetBalanceLimit (hbarAccount, 0, &hasMinBalance);
    BRHederaUnitTinyBar maxBalanceHBAR = hederaAccountGetBalanceLimit (hbarAccount, 1, &hasMaxBalance);

    WKAmount minBalance = hasMinBalance ? wkAmountCreateAsHBAR(unit, WK_FALSE, minBalanceHBAR) : NULL;
    WKAmount maxBalance = hasMaxBalance ? wkAmountCreateAsHBAR(unit, WK_FALSE, maxBalanceHBAR) : NULL;

    BRHederaFeeBasis feeBasisHBAR = hederaAccountGetDefaultFeeBasis (hbarAccount);
    WKFeeBasis feeBasis     = wkFeeBasisCreateAsHBAR (unitForFee, feeBasisHBAR);

    WKWalletCreateContextHBAR contextHBAR = {
        hbarAccount
    };

    WKWallet wallet = wkWalletAllocAndInit (sizeof (struct WKWalletHBARRecord),
                                                      WK_NETWORK_TYPE_HBAR,
                                                      listener,
                                                      unit,
                                                      unitForFee,
                                                      minBalance,
                                                      maxBalance,
                                                      feeBasis,
                                                      &contextHBAR,
                                                      wkWalletCreateCallbackHBAR);

    wkFeeBasisGive(feeBasis);
    wkAmountGive (maxBalance);
    wkAmountGive (minBalance);

    return wallet;
}

static void
wkWalletReleaseHBAR (WKWallet wallet) {
    WKWalletHBAR walletHBAR = wkWalletCoerce (wallet);
    hederaAccountFree(walletHBAR->hbarAccount);
}

static WKAddress
wkWalletGetAddressHBAR (WKWallet wallet,
                            WKAddressScheme addressScheme) {
    assert (WK_ADDRESS_SCHEME_NATIVE == addressScheme);
    WKWalletHBAR walletHBAR = wkWalletCoerce (wallet);
    return wkAddressCreateAsHBAR (hederaAccountGetAddress (walletHBAR->hbarAccount));
}

static bool
wkWalletHasAddressHBAR (WKWallet wallet,
                            WKAddress address) {
    WKWalletHBAR walletHBAR = wkWalletCoerce (wallet);
    
    BRHederaAddress hbarAddress = wkAddressAsHBAR (address);
    
    return hederaAccountHasAddress (walletHBAR->hbarAccount, hbarAddress);
}

extern size_t
wkWalletGetTransferAttributeCountHBAR (WKWallet wallet,
                                           WKAddress target) {
    BRHederaAddress hbarTarget = (NULL == target) ? NULL : wkAddressAsHBAR (target);
    
    size_t countRequired, countOptional;
    hederaWalletGetTransactionAttributeKeys (hbarTarget, 1, &countRequired);
    hederaWalletGetTransactionAttributeKeys (hbarTarget, 0, &countOptional);
    return countRequired + countOptional;
}

extern WKTransferAttribute
wkWalletGetTransferAttributeAtHBAR (WKWallet wallet,
                                        WKAddress target,
                                        size_t index) {
    BRHederaAddress hbarTarget = (NULL == target) ? NULL : wkAddressAsHBAR (target);
    
    size_t countRequired, countOptional;
    const char **keysRequired = hederaWalletGetTransactionAttributeKeys (hbarTarget, 1, &countRequired);
    const char **keysOptional = hederaWalletGetTransactionAttributeKeys (hbarTarget, 0, &countOptional);
    
    assert (index < (countRequired + countOptional));
    
    WKBoolean isRequired = AS_WK_BOOLEAN (index < countRequired);
    const char **keys      = (isRequired ? keysRequired : keysOptional);
    size_t       keysIndex = (isRequired ? index : (index - countRequired));
    
    return wkTransferAttributeCreate(keys[keysIndex], NULL, isRequired);
}

extern WKTransferAttributeValidationError
wkWalletValidateTransferAttributeHBAR (WKWallet wallet,
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
    
    if (hederaCompareAttribute (key, TRANSFER_ATTRIBUTE_MEMO_TAG)) {
        // There is no constraint on the form of the 'memo' field.
        *validates = WK_TRUE;
    }
    else {
        error = WK_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_RELATIONSHIP_INCONSISTENCY;
        *validates = WK_FALSE;
    }
    
    return error;
}

extern WKTransfer
wkWalletCreateTransferHBAR (WKWallet  wallet,
                                WKAddress target,
                                WKAmount  amount,
                                WKFeeBasis estimatedFeeBasis,
                                size_t attributesCount,
                                OwnershipKept WKTransferAttribute *attributes,
                                WKCurrency currency,
                                WKUnit unit,
                                WKUnit unitForFee) {
    WKWalletHBAR walletHBAR = wkWalletCoerce (wallet);

    BRHederaAddress source = hederaAccountGetAddress(walletHBAR->hbarAccount);
    UInt256 value = wkAmountGetValue (amount);
    BRHederaUnitTinyBar thbar = (BRHederaUnitTinyBar) value.u64[0];
    BRHederaFeeBasis hbarFeeBasis = wkFeeBasisCoerceHBAR (estimatedFeeBasis)->hbarFeeBasis;
    BRHederaTransaction hbarTransaction = hederaTransactionCreateNew (source,
                                                                      wkAddressAsHBAR (target),
                                                                      thbar,
                                                                      hbarFeeBasis,
                                                                      NULL);
    if (NULL == hbarTransaction)
        return NULL;
    
    for (size_t index = 0; index < attributesCount; index++) {
        WKTransferAttribute attribute = attributes[index];
        if (NULL != wkTransferAttributeGetValue(attribute)) {
            if (hederaCompareAttribute (wkTransferAttributeGetKey(attribute), TRANSFER_ATTRIBUTE_MEMO_TAG)) {
                hederaTransactionSetMemo (hbarTransaction, wkTransferAttributeGetValue(attribute));
            }
            else {
                // TODO: Impossible if validated?
            }
        }
    }
    
    hederaAddressFree (source);

    WKTransferState state    = wkTransferStateInit (WK_TRANSFER_STATE_CREATED);
    WKTransfer      transfer = wkTransferCreateAsHBAR (wallet->listenerTransfer,
                                                                 NULL,
                                                                 unit,
                                                                 unitForFee,
                                                                 state,
                                                                 walletHBAR->hbarAccount,
                                                                 hbarTransaction);
    
    wkTransferSetAttributes (transfer, attributesCount, attributes);
    wkTransferStateGive (state);

    return transfer;
}

extern WKTransfer
wkWalletCreateTransferMultipleHBAR (WKWallet wallet,
                                        size_t outputsCount,
                                        WKTransferOutput *outputs,
                                        WKFeeBasis estimatedFeeBasis,
                                        WKCurrency currency,
                                        WKUnit unit,
                                        WKUnit unitForFee) {
    return NULL;
}

static OwnershipGiven BRSetOf(WKAddress)
wkWalletGetAddressesForRecoveryHBAR (WKWallet wallet) {
    BRSetOf(WKAddress) addresses = wkAddressSetCreate (1);
    
    WKWalletHBAR walletHBAR = wkWalletCoerce(wallet);

    BRSetAdd (addresses, wkAddressCreateAsHBAR (hederaAccountGetAddress(walletHBAR->hbarAccount)));
    
    return addresses;
}

static bool
wkWalletIsEqualHBAR (WKWallet wb1, WKWallet wb2) {
    if (wb1 == wb2) return true;
    WKWalletHBAR w1 = wkWalletCoerce(wb1);
    WKWalletHBAR w2 = wkWalletCoerce(wb2);
    return w1->hbarAccount == w2->hbarAccount;
}

WKWalletHandlers wkWalletHandlersHBAR = {
    wkWalletReleaseHBAR,
    wkWalletGetAddressHBAR,
    wkWalletHasAddressHBAR,
    wkWalletGetTransferAttributeCountHBAR,
    wkWalletGetTransferAttributeAtHBAR,
    wkWalletValidateTransferAttributeHBAR,
    wkWalletCreateTransferHBAR,
    wkWalletCreateTransferMultipleHBAR,
    wkWalletGetAddressesForRecoveryHBAR,
    NULL,
    wkWalletIsEqualHBAR
};
