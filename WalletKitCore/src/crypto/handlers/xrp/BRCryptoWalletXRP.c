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
#include "ripple/BRRippleWallet.h"
#include "ripple/BRRippleTransaction.h"
#include "support/BRSet.h"
#include "ethereum/util/BRUtilMath.h"

#include <stdio.h>
#include <errno.h>


static BRCryptoWalletXRP
cryptoWalletCoerce (BRCryptoWallet wallet) {
    assert (CRYPTO_NETWORK_TYPE_XRP == wallet->type);
    return (BRCryptoWalletXRP) wallet;
}

private_extern BRCryptoWallet
cryptoWalletCreateAsXRP (BRCryptoWalletListener listener,
                         BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BRRippleWallet wid) {
    int hasMinBalance;
    int hasMaxBalance;
    BRRippleUnitDrops minBalanceDrops = rippleWalletGetBalanceLimit (wid, 0, &hasMinBalance);
    BRRippleUnitDrops maxBalanceDrops = rippleWalletGetBalanceLimit (wid, 1, &hasMaxBalance);

    BRCryptoWallet walletBase = cryptoWalletAllocAndInit (sizeof (struct BRCryptoWalletXRPRecord),
                                                          CRYPTO_NETWORK_TYPE_XRP,
                                                          listener,
                                                          unit,
                                                          unitForFee,
                                                          hasMinBalance ? cryptoAmountCreateAsXRP(unit, CRYPTO_FALSE, minBalanceDrops) : NULL,
                                                          hasMaxBalance ? cryptoAmountCreateAsXRP(unit, CRYPTO_FALSE, maxBalanceDrops) : NULL);

    BRCryptoWalletXRP wallet = cryptoWalletCoerce (walletBase);
    wallet->wid = wid;

    return walletBase;
}

private_extern BRRippleWallet
cryptoWalletAsXRP (BRCryptoWallet walletBase) {
    BRCryptoWalletXRP wallet = cryptoWalletCoerce(walletBase);
    return wallet->wid;
}

static void
cryptoWalletReleaseXRP (BRCryptoWallet walletBase) {
    BRCryptoWalletXRP wallet = cryptoWalletCoerce (walletBase);
    rippleWalletFree (wallet->wid);
}

static BRCryptoAddress
cryptoWalletGetAddressXRP (BRCryptoWallet walletBase,
                           BRCryptoAddressScheme addressScheme) {
    assert (CRYPTO_ADDRESS_SCHEME_GEN_DEFAULT == addressScheme);
    BRCryptoWalletXRP wallet = cryptoWalletCoerce (walletBase);
    BRRippleAddress address = rippleWalletGetSourceAddress (wallet->wid);
    return cryptoAddressCreateAsXRP (address);
}

static bool
cryptoWalletHasAddressXRP (BRCryptoWallet walletBase,
                           BRCryptoAddress address) {
    BRCryptoWalletXRP wallet = cryptoWalletCoerce (walletBase);
    
    BRRippleWallet xrpWallet = wallet->wid;
    BRRippleAddress xrpAddress = cryptoAddressAsXRP (address);
    
    return rippleWalletHasAddress (xrpWallet, xrpAddress);
}

extern size_t
cryptoWalletGetTransferAttributeCountXRP (BRCryptoWallet walletBase,
                                          BRCryptoAddress target) {
    BRCryptoWalletXRP wallet = cryptoWalletCoerce (walletBase);
    BRRippleWallet xrpWallet = wallet->wid;
    BRRippleAddress xrpTarget = cryptoAddressAsXRP (target);
    
    size_t countRequired, countOptional;
    rippleWalletGetTransactionAttributeKeys (xrpWallet, xrpTarget, 1, &countRequired);
    rippleWalletGetTransactionAttributeKeys (xrpWallet, xrpTarget, 0, &countOptional);
    return countRequired + countOptional;
}

extern BRCryptoTransferAttribute
cryptoWalletGetTransferAttributeAtXRP (BRCryptoWallet walletBase,
                                       BRCryptoAddress target,
                                       size_t index) {
    BRCryptoWalletXRP wallet = cryptoWalletCoerce (walletBase);
    BRRippleWallet xrpWallet = wallet->wid;
    BRRippleAddress xrpTarget = cryptoAddressAsXRP (target);
    
    size_t countRequired, countOptional;
    const char **keysRequired = rippleWalletGetTransactionAttributeKeys (xrpWallet, xrpTarget, 1, &countRequired);
    const char **keysOptional = rippleWalletGetTransactionAttributeKeys (xrpWallet, xrpTarget, 0, &countOptional);

    assert (index < (countRequired + countOptional));

    BRCryptoBoolean isRequired = AS_CRYPTO_BOOLEAN (index < countRequired);
    const char **keys      = (isRequired ? keysRequired : keysOptional);
    size_t       keysIndex = (isRequired ? index : (index - countRequired));

    return cryptoTransferAttributeCreate(keys[keysIndex], NULL, isRequired);
}

extern BRCryptoTransferAttributeValidationError
cryptoWalletValidateTransferAttributeXRP (BRCryptoWallet walletBase,
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
    BRRippleWallet wid = walletXRP->wid;

    UInt256 value = cryptoAmountGetValue (amount);
    
    BRRippleAddress source  = rippleWalletGetSourceAddress (wid);
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
    
    BRCryptoTransfer transferBase = cryptoTransferCreateAsXRP (wallet->listenerTransfer,
                                                               unit,
                                                               unitForFee,
                                                               wid,
                                                               xrpTransfer);
    cryptoTransferSetAttributes (transferBase, attributes);
    
    return transferBase;
}

extern BRCryptoTransfer
cryptoWalletCreateTransferMultipleXRP (BRCryptoWallet walletBase,
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
cryptoWalletGetAddressesForRecoveryXRP (BRCryptoWallet walletBase) {
    BRSetOf(BRCryptoAddress) addresses = cryptoAddressSetCreate (1);

    BRCryptoWalletXRP wallet = cryptoWalletCoerce(walletBase);
    BRRippleWallet xrpWallet = wallet->wid;

    BRSetAdd (addresses, cryptoAddressCreateAsXRP (rippleWalletGetSourceAddress (xrpWallet)));

    return addresses;
}

static bool
cryptoWalletIsEqualXRP (BRCryptoWallet wb1, BRCryptoWallet wb2) {
    BRCryptoWalletXRP w1 = cryptoWalletCoerce(wb1);
    BRCryptoWalletXRP w2 = cryptoWalletCoerce(wb2);
    return w1->wid == w2->wid;
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
    cryptoWalletIsEqualXRP
};
