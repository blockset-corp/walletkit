//
//  WKWalletAVAX.c
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKAVAX.h"
#include "WKBase.h"
#include "walletkit/WKWalletP.h"
#include "walletkit/WKAmountP.h"
#include "walletkit/WKFeeBasisP.h"
#include "avalanche/BRAvalanche.h"
#include "support/BRSet.h"
#include "support/util/BRUtilMath.h"

#include <stdio.h>
#include <errno.h>

static WKWalletAVAX
wkWalletCoerce (WKWallet wallet) {
    assert (WK_NETWORK_TYPE_AVAX == wallet->type);
    return (WKWalletAVAX) wallet;
}

typedef struct {
    BRAvalancheAccount avaxAccount;
    BRAvalancheNetwork avaxNetwork;
} WKWalletCreateContextAVAX;

static void
wkWalletCreateCallbackAVAX (WKWalletCreateContext context,
                               WKWallet wallet) {
    WKWalletCreateContextAVAX *contextAVAX = (WKWalletCreateContextAVAX*) context;
    WKWalletAVAX walletAVAX = wkWalletCoerce (wallet);

    walletAVAX->avaxAccount = contextAVAX->avaxAccount;
    walletAVAX->avaxNetwork = contextAVAX->avaxNetwork;
}

private_extern WKWallet
wkWalletCreateAsAVAX (WKWalletListener listener,
                      WKUnit unit,
                      WKUnit unitForFee,
                      BRAvalancheAccount avaxAccount,
                      BRAvalancheNetwork avaxNetwork) {
    int hasMinBalance;
    int hasMaxBalance;
    BRAvalancheAmount minBalanceAVAX = avalancheAccountGetBalanceLimit (avaxAccount, 0, &hasMinBalance);
    BRAvalancheAmount maxBalanceAVAX = avalancheAccountGetBalanceLimit (avaxAccount, 1, &hasMaxBalance);
    
    WKAmount minBalance = hasMinBalance ? wkAmountCreateAsAVAX(unit, WK_FALSE, minBalanceAVAX) : NULL;
    WKAmount maxBalance = hasMaxBalance ? wkAmountCreateAsAVAX(unit, WK_FALSE, maxBalanceAVAX) : NULL;
    
    BRAvalancheFeeBasis avalancheFeeBasis = avalancheFeeBasisCreate (AVALANCHE_DEFAULT_AVAX_PER_TRANSACTION);
    WKFeeBasis feeBasis   = wkFeeBasisCreateAsAVAX (unitForFee, avalancheFeeBasis);
    
    WKWalletCreateContextAVAX contextAVAX = {
        avaxAccount,
        avaxNetwork
    };

    WKWallet wallet = wkWalletAllocAndInit (sizeof (struct WKWalletAVAXRecord),
                                                      WK_NETWORK_TYPE_AVAX,
                                                      listener,
                                                      unit,
                                                      unitForFee,
                                                      minBalance,
                                                      maxBalance,
                                                      feeBasis,
                                                      &contextAVAX,
                                                      wkWalletCreateCallbackAVAX);

    wkFeeBasisGive(feeBasis);
    wkAmountGive (maxBalance);
    wkAmountGive (minBalance);

    return wallet;
}

static void
wkWalletReleaseAVAX (WKWallet wallet) {
    WKWalletAVAX walletAVAX = wkWalletCoerce (wallet);
    avalancheAccountFree(walletAVAX->avaxAccount);
}

static WKAddress
wkWalletGetAddressAVAX (WKWallet wallet,
                           WKAddressScheme addressScheme) {
    assert (WK_ADDRESS_SCHEME_NATIVE == addressScheme);
    WKWalletAVAX walletAVAX = wkWalletCoerce (wallet);
    return wkAddressCreateAsAVAX (avalancheAccountGetAddress(walletAVAX->avaxAccount, AVALANCHE_CHAIN_TYPE_X), walletAVAX->avaxNetwork);
}

static bool
wkWalletHasAddressAVAX (WKWallet wallet,
                           WKAddress address) {
    WKWalletAVAX walletAVAX = wkWalletCoerce (wallet);
    BRAvalancheAddress avaxAddress = wkAddressAsAVAX (address);
    
    return avalancheAccountHasAddress (walletAVAX->avaxAccount, avaxAddress);
}


extern size_t
wkWalletGetTransferAttributeCountAVAX (WKWallet wallet,
                                          WKAddress target) {
    size_t countRequired, countOptional;
    avalancheTransactionGetAttributeKeys (true,  &countRequired);
    avalancheTransactionGetAttributeKeys (false, &countOptional);
    return countRequired + countOptional;
}

extern WKTransferAttribute
wkWalletGetTransferAttributeAtAVAX (WKWallet wallet,
                                       WKAddress target,
                                       size_t index) {
    size_t countRequired, countOptional;
    const char **keysRequired = avalancheTransactionGetAttributeKeys (true,  &countRequired);
    const char **keysOptional = avalancheTransactionGetAttributeKeys (false, &countOptional);

    assert (index < (countRequired + countOptional));

    WKBoolean isRequired = AS_WK_BOOLEAN (index < countRequired);
    const char **keys      = (isRequired ? keysRequired : keysOptional);
    size_t       keysIndex = (isRequired ? index : (index - countRequired));

    return wkTransferAttributeCreate(keys[keysIndex], NULL, isRequired);
}

#if 0
static int // 1 if equal, 0 if not.
avalancheCompareFieldOption (const char *t1, const char *t2) {
    return 0 == strcasecmp (t1, t2);
}
#endif

extern WKTransferAttributeValidationError
wkWalletValidateTransferAttributeAVAX (WKWallet wallet,
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

#if 0
    if (avalancheCompareFieldOption (key, FIELD_OPTION_DELEGATION_OP)) {
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
    else if (avalancheCompareFieldOption (key, FIELD_OPTION_DELEGATE)) {
        // expect string
        *validates = WK_TRUE;
    }
    else if (avalancheCompareFieldOption (key, FIELD_OPTION_OPERATION_TYPE)) {
        // expect string
        *validates = WK_TRUE;
    }

    else {
        error = WK_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_RELATIONSHIP_INCONSISTENCY;
        *validates = WK_FALSE;
    }
#endif

    (void) key;
    *validates = WK_TRUE;
    return error;
}

// create for send
extern WKTransfer
wkWalletCreateTransferAVAX (WKWallet  wallet,
                            WKAddress target,
                            WKAmount  amount,
                            WKFeeBasis estimatedFeeBasis,
                            size_t attributesCount,
                            OwnershipKept WKTransferAttribute *attributes,
                            WKCurrency currency,
                            WKUnit unit,
                            WKUnit unitForFee) {
    WKWalletAVAX walletAVAX = wkWalletCoerce (wallet);

    BRAvalancheAccount avaxAccount = walletAVAX->avaxAccount;
    BRAvalancheNetwork avaxNetwork = walletAVAX->avaxNetwork;
    
    BRAvalancheAddress avaxSource  = avalancheAccountGetAddress (walletAVAX->avaxAccount, AVALANCHE_CHAIN_TYPE_X);
    BRAvalancheAddress avaxTarget  = wkAddressAsAVAX (target);

    WKBoolean amountIsNegative = WK_FALSE;
    BRAvalancheAmount avaxAmount = wkAmountAsAVAX (amount, &amountIsNegative);
    assert (WK_FALSE == amountIsNegative);

    BRAvalancheFeeBasis avaxFeeBasis = wkFeeBasisCoerceAVAX (estimatedFeeBasis)->avaxFeeBasis;
    
    BRAvalancheTransaction avaxTransaction = avalancheTransactionCreate (avaxSource,
                                                                         avaxTarget,
                                                                         avaxAmount,
                                                                         avaxFeeBasis);

    WKTransferState state    = wkTransferStateInit (WK_TRANSFER_STATE_CREATED);
    WKTransfer      transfer = wkTransferCreateAsAVAX (wallet->listenerTransfer,
                                                       NULL,
                                                       unit,
                                                       unitForFee,
                                                       state,
                                                       avaxAccount,
                                                       avaxNetwork,
                                                       avaxTransaction);
    
    wkTransferSetAttributes (transfer, attributesCount, attributes);
    wkTransferStateGive (state);
    
    return transfer;
}

extern WKTransfer
wkWalletCreateTransferMultipleAVAX (WKWallet wallet,
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
wkWalletGetAddressesForRecoveryAVAX (WKWallet wallet) {
    BRSetOf(WKAddress) addresses = wkAddressSetCreate (1);

    WKWalletAVAX walletAVAX = wkWalletCoerce(wallet);
    BRSetAdd (addresses, wkAddressCreateAsAVAX (avalancheAccountGetAddress (walletAVAX->avaxAccount, AVALANCHE_CHAIN_TYPE_X), walletAVAX->avaxNetwork));

    return addresses;
}

static bool
wkWalletIsEqualAVAX (WKWallet wb1, WKWallet wb2) {
    if (wb1 == wb2) return true;

    WKWalletAVAX w1 = wkWalletCoerce(wb1);
    WKWalletAVAX w2 = wkWalletCoerce(wb2);
    return w1->avaxAccount == w2->avaxAccount;
}

WKWalletHandlers wkWalletHandlersAVAX = {
    wkWalletReleaseAVAX,
    wkWalletGetAddressAVAX,
    wkWalletHasAddressAVAX,
    wkWalletGetTransferAttributeCountAVAX,
    wkWalletGetTransferAttributeAtAVAX,
    wkWalletValidateTransferAttributeAVAX,
    wkWalletCreateTransferAVAX,
    wkWalletCreateTransferMultipleAVAX,
    wkWalletGetAddressesForRecoveryAVAX,
    NULL,//WKWalletAnnounceTransfer
    wkWalletIsEqualAVAX
};
