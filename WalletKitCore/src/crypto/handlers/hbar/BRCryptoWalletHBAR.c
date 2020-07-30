//
//  BRCryptoWalletHBAR.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoHBAR.h"
#include "BRCryptoBase.h"
#include "crypto/BRCryptoWalletP.h"
#include "crypto/BRCryptoAmountP.h"
#include "crypto/BRCryptoFeeBasisP.h"
#include "hedera/BRHederaWallet.h"
#include "hedera/BRHederaTransaction.h"
#include "support/BRSet.h"
#include "ethereum/util/BRUtilMath.h"
#include <stdio.h>


static BRCryptoWalletHBAR
cryptoWalletCoerce (BRCryptoWallet wallet) {
    assert (CRYPTO_NETWORK_TYPE_HBAR == wallet->type);
    return (BRCryptoWalletHBAR) wallet;
}

private_extern BRCryptoWallet
cryptoWalletCreateAsHBAR (BRCryptoWalletListener listener,
                          BRCryptoUnit unit,
                          BRCryptoUnit unitForFee,
                          BRHederaWallet wid) {
    int hasMinBalance;
    int hasMaxBalance;
    BRHederaUnitTinyBar minBalance = hederaWalletGetBalanceLimit (wid, 0, &hasMinBalance);
    BRHederaUnitTinyBar maxBalance = hederaWalletGetBalanceLimit (wid, 1, &hasMaxBalance);
    
    BRCryptoWallet walletBase = cryptoWalletAllocAndInit (sizeof (struct BRCryptoWalletHBARRecord),
                                                          CRYPTO_NETWORK_TYPE_HBAR,
                                                          listener,
                                                          unit,
                                                          unitForFee,
                                                          hasMinBalance ? cryptoAmountCreateAsHBAR(unit, CRYPTO_FALSE, minBalance) : NULL,
                                                          hasMaxBalance ? cryptoAmountCreateAsHBAR(unit, CRYPTO_FALSE, maxBalance) : NULL);
    
    BRCryptoWalletHBAR wallet = cryptoWalletCoerce (walletBase);
    wallet->wid = wid;
    
    return walletBase;
}

private_extern BRHederaWallet
cryptoWalletAsHBAR (BRCryptoWallet walletBase) {
    BRCryptoWalletHBAR wallet = cryptoWalletCoerce(walletBase);
    return wallet->wid;
}

static void
cryptoWalletReleaseHBAR (BRCryptoWallet walletBase) {
    BRCryptoWalletHBAR wallet = cryptoWalletCoerce (walletBase);
    hederaWalletFree (wallet->wid);
}

static BRCryptoAddress
cryptoWalletGetAddressHBAR (BRCryptoWallet walletBase,
                            BRCryptoAddressScheme addressScheme) {
    assert (CRYPTO_ADDRESS_SCHEME_GEN_DEFAULT == addressScheme);
    BRCryptoWalletHBAR wallet = cryptoWalletCoerce (walletBase);
    BRHederaAddress address = hederaWalletGetSourceAddress (wallet->wid);
    return cryptoAddressCreateAsHBAR (address);
}

static bool
cryptoWalletHasAddressHBAR (BRCryptoWallet walletBase,
                            BRCryptoAddress address) {
    BRCryptoWalletHBAR wallet = cryptoWalletCoerce (walletBase);
    
    BRHederaWallet hbarWallet = wallet->wid;
    BRHederaAddress hbarAddress = cryptoAddressAsHBAR (address);
    
    return hederaWalletHasAddress (hbarWallet, hbarAddress);
}

extern size_t
cryptoWalletGetTransferAttributeCountHBAR (BRCryptoWallet walletBase,
                                           BRCryptoAddress target) {
    BRCryptoWalletHBAR wallet = cryptoWalletCoerce (walletBase);
    BRHederaWallet hbarWallet = wallet->wid;
    BRHederaAddress hbarTarget = cryptoAddressAsHBAR (target);
    
    size_t countRequired, countOptional;
    hederaWalletGetTransactionAttributeKeys (hbarWallet, hbarTarget, 1, &countRequired);
    hederaWalletGetTransactionAttributeKeys (hbarWallet, hbarTarget, 0, &countOptional);
    return countRequired + countOptional;
}

extern BRCryptoTransferAttribute
cryptoWalletGetTransferAttributeAtHBAR (BRCryptoWallet walletBase,
                                        BRCryptoAddress target,
                                        size_t index) {
    BRCryptoWalletHBAR wallet = cryptoWalletCoerce (walletBase);
    BRHederaWallet hbarWallet = wallet->wid;
    BRHederaAddress hbarTarget = cryptoAddressAsHBAR (target);
    
    size_t countRequired, countOptional;
    const char **keysRequired = hederaWalletGetTransactionAttributeKeys (hbarWallet, hbarTarget, 1, &countRequired);
    const char **keysOptional = hederaWalletGetTransactionAttributeKeys (hbarWallet, hbarTarget, 0, &countOptional);
    
    assert (index < (countRequired + countOptional));
    
    BRCryptoBoolean isRequired = AS_CRYPTO_BOOLEAN (index < countRequired);
    const char **keys      = (isRequired ? keysRequired : keysOptional);
    size_t       keysIndex = (isRequired ? index : (index - countRequired));
    
    return cryptoTransferAttributeCreate(keys[keysIndex], NULL, isRequired);
}

extern BRCryptoTransferAttributeValidationError
cryptoWalletValidateTransferAttributeHBAR (BRCryptoWallet walletBase,
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
    
    if (hederaCompareAttribute (key, TRANSFER_ATTRIBUTE_MEMO_TAG)) {
        // There is no constraint on the form of the 'memo' field.
        *validates = CRYPTO_TRUE;
    }
    else {
        error = CRYPTO_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_RELATIONSHIP_INCONSISTENCY;
        *validates = CRYPTO_FALSE;
    }
    
    return error;
}

extern BRCryptoTransfer
cryptoWalletCreateTransferHBAR (BRCryptoWallet  wallet,
                                BRCryptoAddress target,
                                BRCryptoAmount  amount,
                                BRCryptoFeeBasis estimatedFeeBasis,
                                size_t attributesCount,
                                OwnershipKept BRCryptoTransferAttribute *attributes,
                                BRCryptoCurrency currency,
                                BRCryptoUnit unit,
                                BRCryptoUnit unitForFee) {
    BRCryptoWalletHBAR walletHBAR = cryptoWalletCoerce (wallet);
    BRHederaWallet wid = walletHBAR->wid;
    
    BRHederaAddress source = hederaWalletGetSourceAddress (wid);
    UInt256 value = cryptoAmountGetValue (amount);
    BRHederaUnitTinyBar thbar = (BRHederaUnitTinyBar) value.u64[0];
    BRHederaAddress nodeAddress = hederaWalletGetNodeAddress (wid);
    BRHederaFeeBasis feeBasis;
    feeBasis.costFactor = (uint32_t) estimatedFeeBasis->costFactor;
    int overflow = 0;
    feeBasis.pricePerCostFactor = (BRHederaUnitTinyBar) uint64Coerce(cryptoAmountGetValue(estimatedFeeBasis->pricePerCostFactor), &overflow);
    assert(overflow == 0);
    
    BRHederaTransaction tid = hederaTransactionCreateNew (source,
                                                          cryptoAddressAsHBAR (target),
                                                          thbar,
                                                          feeBasis,
                                                          nodeAddress,
                                                          NULL);
    if (NULL == tid) {
        return NULL;
    }
    
    for (size_t index = 0; index < attributesCount; index++) {
        BRCryptoTransferAttribute attribute = attributes[index];
        if (NULL != cryptoTransferAttributeGetValue(attribute)) {
            if (hederaCompareAttribute (cryptoTransferAttributeGetKey(attribute), TRANSFER_ATTRIBUTE_MEMO_TAG)) {
                hederaTransactionSetMemo (tid, cryptoTransferAttributeGetValue(attribute));
            }
            else {
                // TODO: Impossible if validated?
            }
        }
    }
    
    hederaAddressFree (source);
    hederaAddressFree (nodeAddress);
    
    BRCryptoTransfer transferBase = cryptoTransferCreateAsHBAR (wallet->listenerTransfer,
                                                                unit,
                                                                unitForFee,
                                                                wid,
                                                                tid);
    cryptoTransferSetAttributes (transferBase, attributes);
    
    return transferBase;
}

extern BRCryptoTransfer
cryptoWalletCreateTransferMultipleHBAR (BRCryptoWallet walletBase,
                                        size_t outputsCount,
                                        BRCryptoTransferOutput *outputs,
                                        BRCryptoFeeBasis estimatedFeeBasis,
                                        BRCryptoCurrency currency,
                                        BRCryptoUnit unit,
                                        BRCryptoUnit unitForFee) {
    return NULL;
}

static OwnershipGiven BRSetOf(BRCryptoAddress)
cryptoWalletGetAddressesForRecoveryHBAR (BRCryptoWallet walletBase) {
    BRSetOf(BRCryptoAddress) addresses = cryptoAddressSetCreate (1);
    
    BRCryptoWalletHBAR wallet = cryptoWalletCoerce(walletBase);
    BRHederaWallet hbarWallet = wallet->wid;
    
    BRSetAdd (addresses, cryptoAddressCreateAsHBAR (hederaWalletGetSourceAddress (hbarWallet)));
    
    return addresses;
}

static bool
cryptoWalletIsEqualHBAR (BRCryptoWallet wb1, BRCryptoWallet wb2) {
    BRCryptoWalletHBAR w1 = cryptoWalletCoerce(wb1);
    BRCryptoWalletHBAR w2 = cryptoWalletCoerce(wb2);
    return w1->wid == w2->wid;
}

BRCryptoWalletHandlers cryptoWalletHandlersHBAR = {
    cryptoWalletReleaseHBAR,
    cryptoWalletGetAddressHBAR,
    cryptoWalletHasAddressHBAR,
    cryptoWalletGetTransferAttributeCountHBAR,
    cryptoWalletGetTransferAttributeAtHBAR,
    cryptoWalletValidateTransferAttributeHBAR,
    cryptoWalletCreateTransferHBAR,
    cryptoWalletCreateTransferMultipleHBAR,
    cryptoWalletGetAddressesForRecoveryHBAR,
    cryptoWalletIsEqualHBAR
};
