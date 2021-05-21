//
//  WKWalletXLM.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXLM.h"
#include "WKBase.h"
#include "walletkit/WKWalletP.h"
#include "walletkit/WKAmountP.h"
#include "stellar/BRStellar.h"
#include "support/BRSet.h"
#include "support/util/BRUtilMath.h"

#include <stdio.h>
#include <errno.h>


static WKWalletXLM
wkWalletCoerce (WKWallet wallet) {
    assert (WK_NETWORK_TYPE_XLM == wallet->type);
    return (WKWalletXLM) wallet;
}

typedef struct {
    BRStellarAccount xlmAccount;
} WKWalletCreateContextXLM;

static void
wkWalletCreateCallbackXLM (WKWalletCreateContext context,
                               WKWallet wallet) {
    WKWalletCreateContextXLM *contextXLM = (WKWalletCreateContextXLM*) context;
    WKWalletXLM walletXLM = wkWalletCoerce (wallet);

    walletXLM->xlmAccount = contextXLM->xlmAccount;
}

private_extern WKWallet
wkWalletCreateAsXLM (WKWalletListener listener,
                         WKUnit unit,
                         WKUnit unitForFee,
                         BRStellarAccount xlmAccount) {
    int hasMinBalance;
    int hasMaxBalance;
    BRStellarAmount minBalanceXLM = stellarAccountGetBalanceLimit (xlmAccount, 0, &hasMinBalance);
    BRStellarAmount maxBalanceXLM = stellarAccountGetBalanceLimit (xlmAccount, 1, &hasMaxBalance);

    WKAmount minBalance = hasMinBalance ? wkAmountCreateAsXLM(unit, WK_FALSE, minBalanceXLM) : NULL;
    WKAmount maxBalance = hasMaxBalance ? wkAmountCreateAsXLM(unit, WK_FALSE, maxBalanceXLM) : NULL;

    BRStellarFeeBasis feeBasisXLM = stellarAccountGetDefaultFeeBasis (xlmAccount);
    WKFeeBasis feeBasis    = wkFeeBasisCreateAsXLM (unitForFee, feeBasisXLM.pricePerCostFactor);

    WKWalletCreateContextXLM contextXLM = {
        xlmAccount
    };

    WKWallet wallet = wkWalletAllocAndInit (sizeof (struct WKWalletXLMRecord),
                                                      WK_NETWORK_TYPE_XLM,
                                                      listener,
                                                      unit,
                                                      unitForFee,
                                                      minBalance,
                                                      maxBalance,
                                                      feeBasis,
                                                      &contextXLM,
                                                      wkWalletCreateCallbackXLM);

    wkFeeBasisGive(feeBasis);
    wkAmountGive (maxBalance);
    wkAmountGive (minBalance);

    return wallet;
}

static void
wkWalletReleaseXLM (WKWallet wallet) {
    WKWalletXLM walletXLM = wkWalletCoerce (wallet);
    stellarAccountFree(walletXLM->xlmAccount);
}

static WKAddress
wkWalletGetAddressXLM (WKWallet wallet,
                           WKAddressScheme addressScheme) {
    assert (WK_ADDRESS_SCHEME_NATIVE == addressScheme);
    WKWalletXLM walletXLM = wkWalletCoerce (wallet);
    return wkAddressCreateAsXLM (stellarAccountGetAddress(walletXLM->xlmAccount));
}

static bool
wkWalletHasAddressXLM (WKWallet wallet,
                           WKAddress address) {
    WKWalletXLM walletXLM = wkWalletCoerce (wallet);
    BRStellarAddress xlmAddress = wkAddressAsXLM (address);
    
    return stellarAccountHasAddress (walletXLM->xlmAccount, xlmAddress);
}

extern size_t
wkWalletGetTransferAttributeCountXLM (WKWallet wallet,
                                          WKAddress target) {
    BRStellarAddress xlmTarget = (NULL == target) ? NULL : wkAddressAsXLM (target);
    
    size_t countRequired, countOptional;
    stellarAddressGetTransactionAttributeKeys (xlmTarget, 1, &countRequired);
    stellarAddressGetTransactionAttributeKeys (xlmTarget, 0, &countOptional);
    return countRequired + countOptional;
}

extern WKTransferAttribute
wkWalletGetTransferAttributeAtXLM (WKWallet wallet,
                                       WKAddress target,
                                       size_t index) {
    BRStellarAddress xlmTarget = (NULL == target) ? NULL : wkAddressAsXLM (target);
    
    size_t countRequired, countOptional;
    const char **keysRequired = stellarAddressGetTransactionAttributeKeys (xlmTarget, 1, &countRequired);
    const char **keysOptional = stellarAddressGetTransactionAttributeKeys (xlmTarget, 0, &countOptional);

    assert (index < (countRequired + countOptional));

    WKBoolean isRequired = AS_WK_BOOLEAN (index < countRequired);
    const char **keys      = (isRequired ? keysRequired : keysOptional);
    size_t       keysIndex = (isRequired ? index : (index - countRequired));

    return wkTransferAttributeCreate(keys[keysIndex], NULL, isRequired);
}

extern WKTransferAttributeValidationError
wkWalletValidateTransferAttributeXLM (WKWallet wallet,
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

    if (stellarCompareFieldOption (key, FIELD_OPTION_DESTINATION_TAG)) {
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
    else if (stellarCompareFieldOption (key, FIELD_OPTION_INVOICE_ID)) {
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
wkWalletCreateTransferXLM (WKWallet  wallet,
                               WKAddress target,
                               WKAmount  amount,
                               WKFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept WKTransferAttribute *attributes,
                               WKCurrency currency,
                               WKUnit unit,
                               WKUnit unitForFee) {
    WKWalletXLM walletXLM = wkWalletCoerce (wallet);

    UInt256 value = wkAmountGetValue (amount);
    
    BRStellarAddress source  = stellarAccountGetAddress(walletXLM->xlmAccount);
    BRStellarAmount amountXLM = (double)value.u64[0];

    // TODO - Carl - I think this is when we create a new transaction to submit
    BRStellarTransaction xlmTransaction = stellarTransactionCreate (source,
                                                                  wkAddressAsXLM(target),
                                                                  amountXLM,
                                                                  wkFeeBasisAsXLM(estimatedFeeBasis));

    if (NULL == xlmTransaction)
        return NULL;

    // TODO - attributes?
    for (size_t index = 0; index < attributesCount; index++) {
        WKTransferAttribute attribute = attributes[index];
        if (NULL != wkTransferAttributeGetValue(attribute)) {
            /*
            if (stellarCompareFieldOption (wkTransferAttributeGetKey(attribute), FIELD_OPTION_DESTINATION_TAG)) {
                BRCoreParseStatus tag;
                sscanf (wkTransferAttributeGetValue(attribute), "%u", &tag);
                stellarTransactionSetDestinationTag (xlmTransaction, tag);
            }
            else if (stellarCompareFieldOption (wkTransferAttributeGetKey(attribute), FIELD_OPTION_INVOICE_ID)) {
                // TODO: Handle INVOICE_ID (note: not used in BRD App)
            }
            else {
                // TODO: Impossible if validated?
            }
             */
        }
    }

    stellarAddressFree(source);

    WKTransferState state    = wkTransferStateInit(WK_TRANSFER_STATE_CREATED);
    WKTransfer      transfer = wkTransferCreateAsXLM (wallet->listenerTransfer,
                                                                unit,
                                                                unitForFee,
                                                                state,
                                                                walletXLM->xlmAccount,
                                                                xlmTransaction);


    wkTransferSetAttributes (transfer, attributesCount, attributes);
    wkTransferStateGive(state);

    return transfer;
}

extern WKTransfer
wkWalletCreateTransferMultipleXLM (WKWallet wallet,
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
wkWalletGetAddressesForRecoveryXLM (WKWallet wallet) {
    BRSetOf(WKAddress) addresses = wkAddressSetCreate (1);

    WKWalletXLM walletXLM = wkWalletCoerce(wallet);

    BRSetAdd (addresses, wkAddressCreateAsXLM (stellarAccountGetAddress (walletXLM->xlmAccount)));

    return addresses;
}

static void
wkWalletAnnounceTransferXLM (WKWallet wallet,
                                 WKTransfer transfer,
                                 WKWalletEventType type) {
    WKWalletXLM walletXLM = wkWalletCoerce (wallet);

    // Now update the account's sequence id
    BRStellarSequence sequence = 0;

    // The address for comparison with `transfer` source and target addresses.
    BRStellarAddress accountAddress = stellarAccountGetAddress (walletXLM->xlmAccount);

    // We need to keep track of the first block where this account shows up due to a
    // change in how stellar assigns the sequence number to new accounts
    // TODO - Carl - do we need to do anything with the sequence number here

    stellarAddressFree (accountAddress);
}

static bool
wkWalletIsEqualXLM (WKWallet wb1, WKWallet wb2) {
    if (wb1 == wb2) return true;

    WKWalletXLM w1 = wkWalletCoerce(wb1);
    WKWalletXLM w2 = wkWalletCoerce(wb2);
    return w1->xlmAccount == w2->xlmAccount;
}

WKWalletHandlers wkWalletHandlersXLM = {
    wkWalletReleaseXLM,
    wkWalletGetAddressXLM,
    wkWalletHasAddressXLM,
    wkWalletGetTransferAttributeCountXLM,
    wkWalletGetTransferAttributeAtXLM,
    wkWalletValidateTransferAttributeXLM,
    wkWalletCreateTransferXLM,
    wkWalletCreateTransferMultipleXLM,
    wkWalletGetAddressesForRecoveryXLM,
    wkWalletAnnounceTransferXLM,
    wkWalletIsEqualXLM
};


#if defined (NEVER_DEFINED)  // Keep as an example
static void stellarWalletUpdateSequence (BRStellarWallet wallet,
                                        OwnershipKept BRStellarAddress accountAddress) {
    // Now update the account's sequence id
    BRStellarSequence sequence = 0;
    // We need to keep track of the first block where this account shows up due to a
    // change in how stellar assigns the sequence number to new accounts
    uint64_t minBlockHeight = UINT64_MAX;
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        BRStellarTransfer transfer = wallet->transfers[index];
        BRStellarAddress targetAddress = stellarTransferGetTarget(transfer);
        if (stellarTransferHasError(transfer) == 0
            && stellarAddressEqual(accountAddress, targetAddress)) {
            // We trying to find the lowest block number where we were sent
            // currency successful - basically this is the block where our account
            // was created *** ignore failed transfers TO us since we end up seeing
            // items before our account is actually created.
            uint64_t blockHeight = stellarTransferGetBlockHeight(transfer);
            minBlockHeight = blockHeight < minBlockHeight ? blockHeight : minBlockHeight;
        }
        stellarAddressFree(targetAddress);
        if (stellarTransferHasSource (wallet->transfers[index], accountAddress))
            sequence += 1;
    }

    stellarAccountSetBlockNumberAtCreation(wallet->account, minBlockHeight);
    stellarAccountSetSequence (wallet->account, sequence);
}

extern void stellarWalletAddTransfer (BRStellarWallet wallet,
                                     OwnershipKept BRStellarTransfer transfer)
{
    assert(wallet);
    assert(transfer);
    pthread_mutex_lock (&wallet->lock);
    if (!walletHasTransfer(wallet, transfer)) {
        // We'll add `transfer` to `wallet->transfers`; since we don't own `transfer` we must copy.
        transfer = stellarTransferClone(transfer);
        array_add(wallet->transfers, transfer);

        // Update the balance
        BRStellarUnitDrops amount = (stellarTransferHasError(transfer)
                                    ? 0
                                    : stellarTransferGetAmount(transfer));
        BRStellarUnitDrops fee    = stellarTransferGetFee(transfer);

        BRStellarAddress accountAddress = stellarAccountGetAddress(wallet->account);
        BRStellarAddress source = stellarTransferGetSource(transfer);
        BRStellarAddress target = stellarTransferGetTarget(transfer);

        int isSource = stellarAccountHasAddress (wallet->account, source);
        int isTarget = stellarAccountHasAddress (wallet->account, target);

        if (isSource && isTarget)
            wallet->balance -= fee;
        else if (isSource)
            wallet->balance -= (amount + fee);
        else if (isTarget)
            wallet->balance += amount;
        else {
            // something is seriously wrong
        }
        stellarAddressFree (source);
        stellarAddressFree (target);

        stellarWalletUpdateSequence(wallet, accountAddress);
        stellarAddressFree (accountAddress);
    }
    pthread_mutex_unlock (&wallet->lock);
    // Now update the balance
}

extern void stellarWalletRemTransfer (BRStellarWallet wallet,
                                     OwnershipKept BRStellarTransfer transfer)
{
    assert(wallet);
    assert(transfer);
    pthread_mutex_lock (&wallet->lock);
    if (walletHasTransfer(wallet, transfer)) {
        for (size_t index = 0; index < array_count(wallet->transfers); index++)
            if (stellarTransferEqual (transfer, wallet->transfers[index])) {
                stellarTransferFree(wallet->transfers[index]);
                array_rm (wallet->transfers, index);
                break;
            }

        // Update the balance
        BRStellarUnitDrops amount = (stellarTransferHasError(transfer)
                                    ? 0
                                    : stellarTransferGetAmount(transfer));

        BRStellarUnitDrops fee    = stellarTransferGetFee(transfer);

        BRStellarAddress accountAddress = stellarAccountGetAddress(wallet->account);
        BRStellarAddress source = stellarTransferGetSource(transfer);
        BRStellarAddress target = stellarTransferGetTarget(transfer);

        int isSource = stellarAccountHasAddress (wallet->account, source);
        int isTarget = stellarAccountHasAddress (wallet->account, target);

        if (isSource && isTarget)
            wallet->balance += fee;
        else if (isSource)
            wallet->balance += (amount + fee);
        else if (isTarget)
            wallet->balance -= amount;
        else {
            // something is seriously wrong
        }
        stellarAddressFree (source);
        stellarAddressFree (target);

        stellarWalletUpdateSequence(wallet, accountAddress);
        stellarAddressFree (accountAddress);
    }
    pthread_mutex_unlock (&wallet->lock);
    // Now update the balance
}
#endif
