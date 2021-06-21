//
//  WKWalletXRP.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXRP.h"
#include "WKBase.h"
#include "walletkit/WKWalletP.h"
#include "walletkit/WKAmountP.h"
#include "ripple/BRRipple.h"
#include "support/BRSet.h"
#include "support/util/BRUtilMath.h"

#include <stdio.h>
#include <errno.h>


static WKWalletXRP
wkWalletCoerce (WKWallet wallet) {
    assert (WK_NETWORK_TYPE_XRP == wallet->type);
    return (WKWalletXRP) wallet;
}

typedef struct {
    BRRippleAccount xrpAccount;
} WKWalletCreateContextXRP;

static void
wkWalletCreateCallbackXRP (WKWalletCreateContext context,
                               WKWallet wallet) {
    WKWalletCreateContextXRP *contextXRP = (WKWalletCreateContextXRP*) context;
    WKWalletXRP walletXRP = wkWalletCoerce (wallet);

    walletXRP->xrpAccount = contextXRP->xrpAccount;
}

private_extern WKWallet
wkWalletCreateAsXRP (WKWalletListener listener,
                         WKUnit unit,
                         WKUnit unitForFee,
                         BRRippleAccount xrpAccount) {
    int hasMinBalance;
    int hasMaxBalance;
    BRRippleUnitDrops minBalanceDrops = rippleAccountGetBalanceLimit (xrpAccount, 0, &hasMinBalance);
    BRRippleUnitDrops maxBalanceDrops = rippleAccountGetBalanceLimit (xrpAccount, 1, &hasMaxBalance);

    WKAmount minBalance = hasMinBalance ? wkAmountCreateAsXRP(unit, WK_FALSE, minBalanceDrops) : NULL;
    WKAmount maxBalance = hasMaxBalance ? wkAmountCreateAsXRP(unit, WK_FALSE, maxBalanceDrops) : NULL;

    BRRippleFeeBasis feeBasisXRP = rippleAccountGetDefaultFeeBasis (xrpAccount);
    WKFeeBasis feeBasis    = wkFeeBasisCreateAsXRP (unitForFee, feeBasisXRP.pricePerCostFactor);

    WKWalletCreateContextXRP contextXRP = {
        xrpAccount
    };

    WKWallet wallet = wkWalletAllocAndInit (sizeof (struct WKWalletXRPRecord),
                                                      WK_NETWORK_TYPE_XRP,
                                                      listener,
                                                      unit,
                                                      unitForFee,
                                                      minBalance,
                                                      maxBalance,
                                                      feeBasis,
                                                      &contextXRP,
                                                      wkWalletCreateCallbackXRP);

    wkFeeBasisGive(feeBasis);
    wkAmountGive (maxBalance);
    wkAmountGive (minBalance);

    return wallet;
}

static void
wkWalletReleaseXRP (WKWallet wallet) {
    WKWalletXRP walletXRP = wkWalletCoerce (wallet);
    rippleAccountFree(walletXRP->xrpAccount);
}

static WKAddress
wkWalletGetAddressXRP (WKWallet wallet,
                           WKAddressScheme addressScheme) {
    assert (WK_ADDRESS_SCHEME_NATIVE == addressScheme);
    WKWalletXRP walletXRP = wkWalletCoerce (wallet);
    return wkAddressCreateAsXRP (rippleAccountGetAddress(walletXRP->xrpAccount));
}

static bool
wkWalletHasAddressXRP (WKWallet wallet,
                           WKAddress address) {
    WKWalletXRP walletXRP = wkWalletCoerce (wallet);
    BRRippleAddress xrpAddress = wkAddressAsXRP (address);
    
    return rippleAccountHasAddress (walletXRP->xrpAccount, xrpAddress);
}

extern size_t
wkWalletGetTransferAttributeCountXRP (WKWallet wallet,
                                          WKAddress target) {
    BRRippleAddress xrpTarget = (NULL == target) ? NULL : wkAddressAsXRP (target);
    
    size_t countRequired, countOptional;
    rippleAddressGetTransactionAttributeKeys (xrpTarget, 1, &countRequired);
    rippleAddressGetTransactionAttributeKeys (xrpTarget, 0, &countOptional);
    return countRequired + countOptional;
}

extern WKTransferAttribute
wkWalletGetTransferAttributeAtXRP (WKWallet wallet,
                                       WKAddress target,
                                       size_t index) {
    BRRippleAddress xrpTarget = (NULL == target) ? NULL : wkAddressAsXRP (target);
    
    size_t countRequired, countOptional;
    const char **keysRequired = rippleAddressGetTransactionAttributeKeys (xrpTarget, 1, &countRequired);
    const char **keysOptional = rippleAddressGetTransactionAttributeKeys (xrpTarget, 0, &countOptional);

    assert (index < (countRequired + countOptional));

    WKBoolean isRequired = AS_WK_BOOLEAN (index < countRequired);
    const char **keys      = (isRequired ? keysRequired : keysOptional);
    size_t       keysIndex = (isRequired ? index : (index - countRequired));

    return wkTransferAttributeCreate(keys[keysIndex], NULL, isRequired);
}

extern WKTransferAttributeValidationError
wkWalletValidateTransferAttributeXRP (WKWallet wallet,
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

    if (rippleCompareFieldOption (key, FIELD_OPTION_DESTINATION_TAG)) {
        char *end = NULL;
        errno = 0;

        uintmax_t tag = strtoumax (val, &end, 10);
        if (ERANGE != errno && EINVAL != errno && '\0' == end[0] && tag <= UINT32_MAX) {
            *validates = WK_TRUE;
        } else {
            *validates = WK_FALSE;
            error = WK_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_MISMATCHED_TYPE;
        }
    }
    else if (rippleCompareFieldOption (key, FIELD_OPTION_INVOICE_ID)) {
        BRCoreParseStatus status;
        uint256CreateParse(val, 10, &status);
        if (status) {
            *validates = WK_TRUE;
        } else {
                *validates = WK_FALSE;
                error = WK_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_MISMATCHED_TYPE;
            }
    }
    else {
        error = WK_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_RELATIONSHIP_INCONSISTENCY;
        *validates = WK_FALSE;
    }
    
    return error;
}

extern WKTransfer
wkWalletCreateTransferXRP (WKWallet  wallet,
                               WKAddress target,
                               WKAmount  amount,
                               WKFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept WKTransferAttribute *attributes,
                               WKCurrency currency,
                               WKUnit unit,
                               WKUnit unitForFee) {
    WKWalletXRP walletXRP = wkWalletCoerce (wallet);

    UInt256 value = wkAmountGetValue (amount);
    
    BRRippleAddress source  = rippleAccountGetAddress(walletXRP->xrpAccount);
    BRRippleUnitDrops drops = value.u64[0];

    BRRippleTransaction xrpTransaction = rippleTransactionCreate (source,
                                                                  wkAddressAsXRP(target),
                                                                  drops,
                                                                  wkFeeBasisAsXRP(estimatedFeeBasis));

    if (NULL == xrpTransaction)
        return NULL;

    for (size_t index = 0; index < attributesCount; index++) {
        WKTransferAttribute attribute = attributes[index];
        if (NULL != wkTransferAttributeGetValue(attribute)) {
            if (rippleCompareFieldOption (wkTransferAttributeGetKey(attribute), FIELD_OPTION_DESTINATION_TAG)) {
                BRCoreParseStatus tag;
                sscanf (wkTransferAttributeGetValue(attribute), "%u", &tag);
                rippleTransactionSetDestinationTag (xrpTransaction, tag);
            }
            else if (rippleCompareFieldOption (wkTransferAttributeGetKey(attribute), FIELD_OPTION_INVOICE_ID)) {
                // TODO: Handle INVOICE_ID (note: not used in BRD App)
            }
            else {
                // TODO: Impossible if validated?
            }
        }
    }

    rippleAddressFree(source);

    WKTransferState state    = wkTransferStateInit(WK_TRANSFER_STATE_CREATED);
    WKTransfer      transfer = wkTransferCreateAsXRP (wallet->listenerTransfer,
                                                                NULL,
                                                                unit,
                                                                unitForFee,
                                                                state,
                                                                walletXRP->xrpAccount,
                                                                xrpTransaction);


    wkTransferSetAttributes (transfer, attributesCount, attributes);
    wkTransferStateGive(state);

    return transfer;
}

extern WKTransfer
wkWalletCreateTransferMultipleXRP (WKWallet wallet,
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
wkWalletGetAddressesForRecoveryXRP (WKWallet wallet) {
    BRSetOf(WKAddress) addresses = wkAddressSetCreate (1);

    WKWalletXRP walletXRP = wkWalletCoerce(wallet);

    BRSetAdd (addresses, wkAddressCreateAsXRP (rippleAccountGetAddress (walletXRP->xrpAccount)));

    return addresses;
}

static void
wkWalletAnnounceTransferXRP (WKWallet wallet,
                                 WKTransfer transfer,
                                 WKWalletEventType type) {
    WKWalletXRP walletXRP = wkWalletCoerce (wallet);

    // Now update the account's sequence id
    BRRippleSequence sequence = 0;

    // The address for comparison with `transfer` source and target addresses.
    BRRippleAddress accountAddress = rippleAccountGetAddress (walletXRP->xrpAccount);

    // We need to keep track of the first block where this account shows up due to a
    // change in how ripple assigns the sequence number to new accounts
    uint64_t minBlockHeight = UINT64_MAX;
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        BRRippleTransaction xrpTransfer = wkTransferAsXRP (wallet->transfers[index]);

        // If we are the source of the transfer then we might want to update our sequence number
        if (rippleTransactionHasSource (xrpTransfer, accountAddress)) {
            // Update the sequence number if in a block OR successful
            if (rippleTransactionIsInBlock(xrpTransfer) || !rippleTransactionHasError(xrpTransfer))
                sequence += 1;
        } else if (!rippleTransactionHasError(xrpTransfer) && rippleTransactionHasTarget (xrpTransfer, accountAddress)) {
            // We are the target of the transfer - so we need to find the very first (successful) transfer where
            // our account received some XRP as this can affect our beginning sequence number. Ignore failed
            // transfers as Bockset could create a failed transfer for us before our account is created
            uint64_t blockHeight = rippleTransactionGetBlockHeight(xrpTransfer);
            minBlockHeight = blockHeight < minBlockHeight ? blockHeight : minBlockHeight;
        }
    }

    rippleAddressFree (accountAddress);

    rippleAccountSetBlockNumberAtCreation(walletXRP->xrpAccount, minBlockHeight);
    rippleAccountSetSequence (walletXRP->xrpAccount, sequence);
}

static bool
wkWalletIsEqualXRP (WKWallet wb1, WKWallet wb2) {
    if (wb1 == wb2) return true;

    WKWalletXRP w1 = wkWalletCoerce(wb1);
    WKWalletXRP w2 = wkWalletCoerce(wb2);
    return w1->xrpAccount == w2->xrpAccount;
}

WKWalletHandlers wkWalletHandlersXRP = {
    wkWalletReleaseXRP,
    wkWalletGetAddressXRP,
    wkWalletHasAddressXRP,
    wkWalletGetTransferAttributeCountXRP,
    wkWalletGetTransferAttributeAtXRP,
    wkWalletValidateTransferAttributeXRP,
    wkWalletCreateTransferXRP,
    wkWalletCreateTransferMultipleXRP,
    wkWalletGetAddressesForRecoveryXRP,
    wkWalletAnnounceTransferXRP,
    wkWalletIsEqualXRP
};


#if defined (NEVER_DEFINED)  // Keep as an example
static void rippleWalletUpdateSequence (BRRippleWallet wallet,
                                        OwnershipKept BRRippleAddress accountAddress) {
    // Now update the account's sequence id
    BRRippleSequence sequence = 0;
    // We need to keep track of the first block where this account shows up due to a
    // change in how ripple assigns the sequence number to new accounts
    uint64_t minBlockHeight = UINT64_MAX;
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        BRRippleTransfer transfer = wallet->transfers[index];
        BRRippleAddress targetAddress = rippleTransferGetTarget(transfer);
        if (rippleTransferHasError(transfer) == 0
            && rippleAddressEqual(accountAddress, targetAddress)) {
            // We trying to find the lowest block number where we were sent
            // currency successful - basically this is the block where our account
            // was created *** ignore failed transfers TO us since we end up seeing
            // items before our account is actually created.
            uint64_t blockHeight = rippleTransferGetBlockHeight(transfer);
            minBlockHeight = blockHeight < minBlockHeight ? blockHeight : minBlockHeight;
        }
        rippleAddressFree(targetAddress);
        if (rippleTransferHasSource (wallet->transfers[index], accountAddress))
            sequence += 1;
    }

    rippleAccountSetBlockNumberAtCreation(wallet->account, minBlockHeight);
    rippleAccountSetSequence (wallet->account, sequence);
}

extern void rippleWalletAddTransfer (BRRippleWallet wallet,
                                     OwnershipKept BRRippleTransfer transfer)
{
    assert(wallet);
    assert(transfer);
    pthread_mutex_lock (&wallet->lock);
    if (!walletHasTransfer(wallet, transfer)) {
        // We'll add `transfer` to `wallet->transfers`; since we don't own `transfer` we must copy.
        transfer = rippleTransferClone(transfer);
        array_add(wallet->transfers, transfer);

        // Update the balance
        BRRippleUnitDrops amount = (rippleTransferHasError(transfer)
                                    ? 0
                                    : rippleTransferGetAmount(transfer));
        BRRippleUnitDrops fee    = rippleTransferGetFee(transfer);

        BRRippleAddress accountAddress = rippleAccountGetAddress(wallet->account);
        BRRippleAddress source = rippleTransferGetSource(transfer);
        BRRippleAddress target = rippleTransferGetTarget(transfer);

        int isSource = rippleAccountHasAddress (wallet->account, source);
        int isTarget = rippleAccountHasAddress (wallet->account, target);

        if (isSource && isTarget)
            wallet->balance -= fee;
        else if (isSource)
            wallet->balance -= (amount + fee);
        else if (isTarget)
            wallet->balance += amount;
        else {
            // something is seriously wrong
        }
        rippleAddressFree (source);
        rippleAddressFree (target);

        rippleWalletUpdateSequence(wallet, accountAddress);
        rippleAddressFree (accountAddress);
    }
    pthread_mutex_unlock (&wallet->lock);
    // Now update the balance
}

extern void rippleWalletRemTransfer (BRRippleWallet wallet,
                                     OwnershipKept BRRippleTransfer transfer)
{
    assert(wallet);
    assert(transfer);
    pthread_mutex_lock (&wallet->lock);
    if (walletHasTransfer(wallet, transfer)) {
        for (size_t index = 0; index < array_count(wallet->transfers); index++)
            if (rippleTransferEqual (transfer, wallet->transfers[index])) {
                rippleTransferFree(wallet->transfers[index]);
                array_rm (wallet->transfers, index);
                break;
            }

        // Update the balance
        BRRippleUnitDrops amount = (rippleTransferHasError(transfer)
                                    ? 0
                                    : rippleTransferGetAmount(transfer));

        BRRippleUnitDrops fee    = rippleTransferGetFee(transfer);

        BRRippleAddress accountAddress = rippleAccountGetAddress(wallet->account);
        BRRippleAddress source = rippleTransferGetSource(transfer);
        BRRippleAddress target = rippleTransferGetTarget(transfer);

        int isSource = rippleAccountHasAddress (wallet->account, source);
        int isTarget = rippleAccountHasAddress (wallet->account, target);

        if (isSource && isTarget)
            wallet->balance += fee;
        else if (isSource)
            wallet->balance += (amount + fee);
        else if (isTarget)
            wallet->balance -= amount;
        else {
            // something is seriously wrong
        }
        rippleAddressFree (source);
        rippleAddressFree (target);

        rippleWalletUpdateSequence(wallet, accountAddress);
        rippleAddressFree (accountAddress);
    }
    pthread_mutex_unlock (&wallet->lock);
    // Now update the balance
}
#endif
