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
#include <strings.h>

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

    BRTezosFeeBasis xtzFeeBasis = tezosFeeBasisCreateDefault(TEZOS_DEFAULT_MUTEZ_PER_BYTE, false, false);
    WKFeeBasis      feeBasis    = wkFeeBasisCreateAsXTZ (unitForFee, xtzFeeBasis);

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
    //
    // A 'reveal' is needed anytime the wallet's balance has hit zero and there has not been a
    // subsequent successful send.  Quoting 'Tezos Experts': "If you emptied your account, the
    // revealed public key is garbage-collected. You need to re-reveal it".  Quoting 'WalletKit
    // Experts': crap.
    //

    // We'll update this as we find a balance of zero and a subsequent send.
    bool needsReveal = true;

    // Keep a running total of the balance
    WKAmount balance = wkAmountCreateInteger (0, wallet->unit);

    // Transactions are ordered as oldest to newest.  Surely there is going to be a screw case with
    // two or more transactions in the same block... having an ambiguous order.
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        WKTransfer transfer = wallet->transfers[index];

        // If the type is 'ERRORED' then skip to the next transfer
        WKTransferStateType type = wkTransferGetStateType (transfer);
        if (WK_TRANSFER_STATE_ERRORED == type) continue;

        // If the direction is not 'RECEIVED' then we've already been revealed.
        WKTransferDirection direction = wkTransferGetDirection (transfer);
        if (WK_TRANSFER_RECEIVED != direction) needsReveal = false;

        // Update the balance
        WKAmount amount     = wkWalletGetTransferAmountDirectedNet (wallet, transfer);
        WKAmount newBalance = wkAmountAdd (balance, amount);

        wkAmountGive(amount);
        wkAmountGive(balance);

        balance = newBalance;

        // If we hit zero, a reveal is need.  A subsequent 'not received' will unset this.
        if (wkAmountIsZero(balance)) needsReveal = true;
    }

    return needsReveal;
}

// reveal is needed before the first outgoing transfer and if the type of any outgoing
// transfer is not WK_TRANSFER_STATE_ERRORED (a failed submit).

private_extern bool
wkWalletNeedsRevealForTransactionXTZ (WKWallet wallet,
                                      BRTezosTransaction transaction) {
    return tezosTransactionRequiresReveal(transaction) && wkWalletNeedsRevealXTZ (wallet);
 }

private_extern WKTransfer
wkWalletGetTransferByHashOrUIDSAndTargetXTZ (WKWallet wallet,
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

private_extern bool
wkWalletHasTransferAttributeForDelegationXTZ (WKWallet wallet,
                                              size_t attributesCount,
                                              OwnershipKept WKTransferAttribute *attributes) {
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

    return delegationOp;
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
    
    BRTezosAddress   xtzSource   = tezosAccountGetAddress (walletXTZ->xtzAccount);
    BRTezosAddress   xtzTarget   = wkAddressAsXTZ (target);
    BRTezosUnitMutez xtzAmount   = tezosMutezCreate (amount);
    BRTezosFeeBasis  xtzFeeBasis = wkFeeBasisCoerceXTZ (estimatedFeeBasis)->xtzFeeBasis;
    BRTezosHash      xtzHash     = TEZOS_HASH_EMPTY;

    BRTezosPublicKey xtzKey      = tezosAccountGetPublicKey(walletXTZ->xtzAccount);

    // If the xtzFeeBasis is for a reveal but the wallet doesn't needd a reveal we've got a problem.
    // Most likely some kind of race condition.  
    //
    // TODO: Need to handle this appropriately
    if (xtzFeeBasis.hasRevealOperationFeeBasis != wkWalletNeedsRevealXTZ(wallet))
        return NULL;

    bool delegationOp = wkWalletHasTransferAttributeForDelegationXTZ (wallet, attributesCount, attributes);


    // The primaryOperation is either a TRANSACTION of a DELEGATION
    BRTezosOperation xtzPrimaryOperation = (delegationOp
                                            ? tezosOperationCreateDelegation  (xtzSource, xtzTarget, xtzFeeBasis.primaryOperationFeeBasis)
                                            : tezosOperationCreateTransaction (xtzSource, xtzTarget, xtzFeeBasis.primaryOperationFeeBasis, xtzAmount));

    // A revealOperation is needed if this Account has not been seen; reveals the public key
    BRTezosOperation xtzRevealOperation = (!xtzFeeBasis.hasRevealOperationFeeBasis
                                           ? NULL
                                           : tezosOperationCreateReveal (xtzSource, xtzTarget, xtzFeeBasis.revealOperationFeeBasis, xtzKey));

    // The transaction is surely the primaryOperation with the optional revealOperation
    BRTezosTransaction xtzTransaction = tezosTransactionCreateWithReveal (xtzPrimaryOperation, xtzRevealOperation);

    WKAddress       source   = wkAddressCreateAsXTZ (xtzSource);
    WKTransferState state    = wkTransferStateInit (WK_TRANSFER_STATE_CREATED);
    WKTransfer      transfer = wkTransferCreateAsXTZ (wallet->listenerTransfer,
                                                      NULL,
                                                      unit,
                                                      unitForFee,
                                                      estimatedFeeBasis,
                                                      amount,
                                                      source,
                                                      target,
                                                      state,
                                                      walletXTZ->xtzAccount,
                                                      xtzHash,
                                                      xtzTransaction);
    
    wkTransferSetAttributes (transfer, attributesCount, attributes);
    wkTransferStateGive (state);
    wkAddressGive (source);
    
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
