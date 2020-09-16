//
//  BRCryptoWalletXRP.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoXRP.h"
#include "BRCryptoBase.h"
#include "crypto/BRCryptoWalletP.h"
#include "crypto/BRCryptoAmountP.h"
#include "ripple/BRRipple.h"
#include "support/BRSet.h"
#include "ethereum/util/BRUtilMath.h"

#include <stdio.h>
#include <errno.h>


static BRCryptoWalletXRP
cryptoWalletCoerce (BRCryptoWallet wallet) {
    assert (CRYPTO_NETWORK_TYPE_XRP == wallet->type);
    return (BRCryptoWalletXRP) wallet;
}

typedef struct {
    BRRippleAccount xrpAccount;
} BRCryptoWalletCreateContextXRP;

static void
cryptoWalletCreateCallbackXRP (BRCryptoWalletCreateContext context,
                               BRCryptoWallet wallet) {
    BRCryptoWalletCreateContextXRP *contextXRP = (BRCryptoWalletCreateContextXRP*) context;
    BRCryptoWalletXRP walletXRP = cryptoWalletCoerce (wallet);

    walletXRP->xrpAccount = contextXRP->xrpAccount;
}

private_extern BRCryptoWallet
cryptoWalletCreateAsXRP (BRCryptoWalletListener listener,
                         BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BRRippleAccount xrpAccount) {
    int hasMinBalance;
    int hasMaxBalance;
    BRRippleUnitDrops minBalanceDrops = rippleAccountGetBalanceLimit (xrpAccount, 0, &hasMinBalance);
    BRRippleUnitDrops maxBalanceDrops = rippleAccountGetBalanceLimit (xrpAccount, 1, &hasMaxBalance);

    BRRippleFeeBasis feeBasisXRP = rippleAccountGetDefaultFeeBasis (xrpAccount);
    BRCryptoFeeBasis feeBasis    = cryptoFeeBasisCreateAsXRP (unitForFee, feeBasisXRP.pricePerCostFactor);

    BRCryptoWalletCreateContextXRP contextXRP = {
        xrpAccount
    };

    BRCryptoWallet wallet = cryptoWalletAllocAndInit (sizeof (struct BRCryptoWalletXRPRecord),
                                                      CRYPTO_NETWORK_TYPE_XRP,
                                                      listener,
                                                      unit,
                                                      unitForFee,
                                                      hasMinBalance ? cryptoAmountCreateAsXRP(unit, CRYPTO_FALSE, minBalanceDrops) : NULL,
                                                      hasMaxBalance ? cryptoAmountCreateAsXRP(unit, CRYPTO_FALSE, maxBalanceDrops) : NULL,
                                                      feeBasis,
                                                      &contextXRP,
                                                      cryptoWalletCreateCallbackXRP);
    cryptoFeeBasisGive(feeBasis);

    return wallet;
}

//private_extern BRRippleWallet
//cryptoWalletAsXRP (BRCryptoWallet wallet) {
//    BRCryptoWalletXRP walletXRP = cryptoWalletCoerce(wallet);
//    return walletXRP->wid;
//}

static void
cryptoWalletReleaseXRP (BRCryptoWallet wallet) {
    BRCryptoWalletXRP walletXRP = cryptoWalletCoerce (wallet);
    rippleAccountFree(walletXRP->xrpAccount);
}

static BRCryptoAddress
cryptoWalletGetAddressXRP (BRCryptoWallet wallet,
                           BRCryptoAddressScheme addressScheme) {
    assert (CRYPTO_ADDRESS_SCHEME_GEN_DEFAULT == addressScheme);
    BRCryptoWalletXRP walletXRP = cryptoWalletCoerce (wallet);
    return cryptoAddressCreateAsXRP (rippleAccountGetAddress(walletXRP->xrpAccount));
}

static bool
cryptoWalletHasAddressXRP (BRCryptoWallet wallet,
                           BRCryptoAddress address) {
    BRCryptoWalletXRP walletXRP = cryptoWalletCoerce (wallet);
    BRRippleAddress xrpAddress = cryptoAddressAsXRP (address);
    
    return rippleAccountHasAddress (walletXRP->xrpAccount, xrpAddress);
}

extern size_t
cryptoWalletGetTransferAttributeCountXRP (BRCryptoWallet wallet,
                                          BRCryptoAddress target) {
    BRRippleAddress xrpTarget = cryptoAddressAsXRP (target);
    
    size_t countRequired, countOptional;
    rippleAddressGetTransactionAttributeKeys (xrpTarget, 1, &countRequired);
    rippleAddressGetTransactionAttributeKeys (xrpTarget, 0, &countOptional);
    return countRequired + countOptional;
}

extern BRCryptoTransferAttribute
cryptoWalletGetTransferAttributeAtXRP (BRCryptoWallet wallet,
                                       BRCryptoAddress target,
                                       size_t index) {
    BRRippleAddress xrpTarget = cryptoAddressAsXRP (target);
    
    size_t countRequired, countOptional;
    const char **keysRequired = rippleAddressGetTransactionAttributeKeys (xrpTarget, 1, &countRequired);
    const char **keysOptional = rippleAddressGetTransactionAttributeKeys (xrpTarget, 0, &countOptional);

    assert (index < (countRequired + countOptional));

    BRCryptoBoolean isRequired = AS_CRYPTO_BOOLEAN (index < countRequired);
    const char **keys      = (isRequired ? keysRequired : keysOptional);
    size_t       keysIndex = (isRequired ? index : (index - countRequired));

    return cryptoTransferAttributeCreate(keys[keysIndex], NULL, isRequired);
}

extern BRCryptoTransferAttributeValidationError
cryptoWalletValidateTransferAttributeXRP (BRCryptoWallet wallet,
                                          OwnershipKept BRCryptoTransferAttribute attribute,
                                          BRCryptoBoolean *validates) {
    const char *key = cryptoTransferAttributeGetKey (attribute);
    const char *val = cryptoTransferAttributeGetValue (attribute);
    BRCryptoTransferAttributeValidationError error = 0;

    // If attribute.value is NULL, we validate unless the attribute.value is required.
    if (NULL == val) {
        if (cryptoTransferAttributeIsRequired(attribute)) {
            error = CRYPTO_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_REQUIRED_BUT_NOT_PROVIDED;
            *validates = CRYPTO_FALSE;
        } else {
            *validates = CRYPTO_TRUE;
        }
        return error;
    }

    if (rippleCompareFieldOption (key, FIELD_OPTION_DESTINATION_TAG)) {
        char *end = NULL;
        errno = 0;

        uintmax_t tag = strtoumax (val, &end, 10);
        if (ERANGE != errno && EINVAL != errno && '\0' == end[0] && tag <= UINT32_MAX) {
            *validates = CRYPTO_TRUE;
        } else {
            *validates = CRYPTO_FALSE;
            error = CRYPTO_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_MISMATCHED_TYPE;
        }
    }
    else if (rippleCompareFieldOption (key, FIELD_OPTION_INVOICE_ID)) {
        BRCoreParseStatus status;
        uint256CreateParse(val, 10, &status);
        if (status) {
            *validates = CRYPTO_TRUE;
        } else {
                *validates = CRYPTO_FALSE;
                error = CRYPTO_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_MISMATCHED_TYPE;
            }
    }
    else {
        error = CRYPTO_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_RELATIONSHIP_INCONSISTENCY;
        *validates = CRYPTO_FALSE;
    }
    
    return error;
}

extern BRCryptoTransfer
cryptoWalletCreateTransferXRP (BRCryptoWallet  wallet,
                               BRCryptoAddress target,
                               BRCryptoAmount  amount,
                               BRCryptoFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept BRCryptoTransferAttribute *attributes,
                               BRCryptoCurrency currency,
                               BRCryptoUnit unit,
                               BRCryptoUnit unitForFee) {
    BRCryptoWalletXRP walletXRP = cryptoWalletCoerce (wallet);

    UInt256 value = cryptoAmountGetValue (amount);
    
    BRRippleAddress source  = rippleAccountGetAddress(walletXRP->xrpAccount);
    BRRippleUnitDrops drops = value.u64[0];

    BRRippleTransfer xrpTransfer = rippleTransferCreateNew (source,
                                                            cryptoAddressAsXRP (target),
                                                            drops);
    if (NULL == xrpTransfer) {
        return NULL;
    }

    BRRippleTransaction transaction = rippleTransferGetTransaction (xrpTransfer);

    for (size_t index = 0; index < attributesCount; index++) {
        BRCryptoTransferAttribute attribute = attributes[index];
        if (NULL != cryptoTransferAttributeGetValue(attribute)) {
            if (rippleCompareFieldOption (cryptoTransferAttributeGetKey(attribute), FIELD_OPTION_DESTINATION_TAG)) {
                BRCoreParseStatus tag;
                sscanf (cryptoTransferAttributeGetValue(attribute), "%u", &tag);
                rippleTransactionSetDestinationTag (transaction, tag);
            }
            else if (rippleCompareFieldOption (cryptoTransferAttributeGetKey(attribute), FIELD_OPTION_INVOICE_ID)) {
                // TODO:
            }
            else {
                // TODO: Impossible if validated?
            }
        }
    }

    rippleAddressFree(source);
    
    BRCryptoTransfer transfer = cryptoTransferCreateAsXRP (wallet->listenerTransfer,
                                                           unit,
                                                           unitForFee,
                                                           walletXRP->xrpAccount,
                                                           xrpTransfer);
    cryptoTransferSetAttributes (transfer, attributes);
    
    return transfer;
}

extern BRCryptoTransfer
cryptoWalletCreateTransferMultipleXRP (BRCryptoWallet wallet,
                                       size_t outputsCount,
                                       BRCryptoTransferOutput *outputs,
                                       BRCryptoFeeBasis estimatedFeeBasis,
                                       BRCryptoCurrency currency,
                                       BRCryptoUnit unit,
                                       BRCryptoUnit unitForFee) {
    // not supported
    return NULL;
}

static OwnershipGiven BRSetOf(BRCryptoAddress)
cryptoWalletGetAddressesForRecoveryXRP (BRCryptoWallet wallet) {
    BRSetOf(BRCryptoAddress) addresses = cryptoAddressSetCreate (1);

    BRCryptoWalletXRP walletXRP = cryptoWalletCoerce(wallet);

    BRSetAdd (addresses, cryptoAddressCreateAsXRP (rippleAccountGetAddress (walletXRP->xrpAccount)));

    return addresses;
}

static void
cryptoWalletAnnounceTransferXRP (BRCryptoWallet wallet,
                                 BRCryptoTransfer transfer,
                                 BRCryptoWalletEventType type) {
    BRCryptoWalletXRP walletXRP = cryptoWalletCoerce (wallet);

    // Now update the account's sequence id
    BRRippleSequence sequence = 0;

    // The address for comparison with `transfer` source and target addresses.
    BRRippleAddress accountAddress = rippleAccountGetAddress (walletXRP->xrpAccount);

    // We need to keep track of the first block where this account shows up due to a
    // change in how ripple assigns the sequence number to new accounts
    uint64_t minBlockHeight = UINT64_MAX;
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        BRRippleTransfer xrpTransfer = cryptoTransferAsXRP (wallet->transfers[index]);
        BRRippleAddress targetAddress = rippleTransferGetTarget(xrpTransfer);

        if (rippleTransferHasError(xrpTransfer) == 0
            && rippleAddressEqual(accountAddress, targetAddress)) {
            // We trying to find the lowest block number where we were sent
            // currency successful - basically this is the block where our account
            // was created *** ignore failed transfers TO us since we end up seeing
            // items before our account is actually created.
            uint64_t blockHeight = rippleTransferGetBlockHeight(xrpTransfer);
            minBlockHeight = blockHeight < minBlockHeight ? blockHeight : minBlockHeight;
        }
        rippleAddressFree(targetAddress);
        if (rippleTransferHasSource (xrpTransfer, accountAddress))
            sequence += 1;
    }

    rippleAddressFree (accountAddress);

    rippleAccountSetBlockNumberAtCreation(walletXRP->xrpAccount, minBlockHeight);
    rippleAccountSetSequence (walletXRP->xrpAccount, sequence);
}

static bool
cryptoWalletIsEqualXRP (BRCryptoWallet wb1, BRCryptoWallet wb2) {
    if (wb1 == wb2) return true;

    BRCryptoWalletXRP w1 = cryptoWalletCoerce(wb1);
    BRCryptoWalletXRP w2 = cryptoWalletCoerce(wb2);
    return w1->xrpAccount == w2->xrpAccount;
}

BRCryptoWalletHandlers cryptoWalletHandlersXRP = {
    cryptoWalletReleaseXRP,
    cryptoWalletGetAddressXRP,
    cryptoWalletHasAddressXRP,
    cryptoWalletGetTransferAttributeCountXRP,
    cryptoWalletGetTransferAttributeAtXRP,
    cryptoWalletValidateTransferAttributeXRP,
    cryptoWalletCreateTransferXRP,
    cryptoWalletCreateTransferMultipleXRP,
    cryptoWalletGetAddressesForRecoveryXRP,
    cryptoWalletAnnounceTransferXRP,
    cryptoWalletIsEqualXRP
};


#if 0
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
