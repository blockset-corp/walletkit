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
    
    BRCryptoWallet wallet = cryptoWalletAllocAndInit (sizeof (struct BRCryptoWalletHBARRecord),
                                                      CRYPTO_NETWORK_TYPE_HBAR,
                                                      listener,
                                                      unit,
                                                      unitForFee,
                                                      hasMinBalance ? cryptoAmountCreateAsHBAR(unit, CRYPTO_FALSE, minBalance) : NULL,
                                                      hasMaxBalance ? cryptoAmountCreateAsHBAR(unit, CRYPTO_FALSE, maxBalance) : NULL);
    
    BRCryptoWalletHBAR walletHBAR = cryptoWalletCoerce (wallet);
    walletHBAR->wid = wid;
    
    return wallet;
}

private_extern BRHederaWallet
cryptoWalletAsHBAR (BRCryptoWallet wallet) {
    BRCryptoWalletHBAR walletHBAR = cryptoWalletCoerce(wallet);
    return walletHBAR->wid;
}

static void
cryptoWalletReleaseHBAR (BRCryptoWallet wallet) {
    BRCryptoWalletHBAR walletHBAR = cryptoWalletCoerce (wallet);
    hederaWalletFree (walletHBAR->wid);
}

static BRCryptoAddress
cryptoWalletGetAddressHBAR (BRCryptoWallet wallet,
                            BRCryptoAddressScheme addressScheme) {
    assert (CRYPTO_ADDRESS_SCHEME_GEN_DEFAULT == addressScheme);
    BRCryptoWalletHBAR walletHBAR = cryptoWalletCoerce (wallet);
    BRHederaAddress address = hederaWalletGetSourceAddress (walletHBAR->wid);
    return cryptoAddressCreateAsHBAR (address);
}

static bool
cryptoWalletHasAddressHBAR (BRCryptoWallet wallet,
                            BRCryptoAddress address) {
    BRCryptoWalletHBAR walletHBAR = cryptoWalletCoerce (wallet);
    
    BRHederaWallet hbarWallet = walletHBAR->wid;
    BRHederaAddress hbarAddress = cryptoAddressAsHBAR (address);
    
    return hederaWalletHasAddress (hbarWallet, hbarAddress);
}

extern size_t
cryptoWalletGetTransferAttributeCountHBAR (BRCryptoWallet wallet,
                                           BRCryptoAddress target) {
    BRCryptoWalletHBAR walletHBAR = cryptoWalletCoerce (wallet);
    BRHederaWallet hbarWallet = walletHBAR->wid;
    BRHederaAddress hbarTarget = cryptoAddressAsHBAR (target);
    
    size_t countRequired, countOptional;
    hederaWalletGetTransactionAttributeKeys (hbarWallet, hbarTarget, 1, &countRequired);
    hederaWalletGetTransactionAttributeKeys (hbarWallet, hbarTarget, 0, &countOptional);
    return countRequired + countOptional;
}

extern BRCryptoTransferAttribute
cryptoWalletGetTransferAttributeAtHBAR (BRCryptoWallet wallet,
                                        BRCryptoAddress target,
                                        size_t index) {
    BRCryptoWalletHBAR walletHBAR = cryptoWalletCoerce (wallet);
    BRHederaWallet hbarWallet = walletHBAR->wid;
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
cryptoWalletValidateTransferAttributeHBAR (BRCryptoWallet wallet,
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
    
    BRCryptoTransfer transfer = cryptoTransferCreateAsHBAR (wallet->listenerTransfer,
                                                                unit,
                                                                unitForFee,
                                                                wid,
                                                                tid);
    cryptoTransferSetAttributes (transfer, attributes);
    
    return transfer;
}

extern BRCryptoTransfer
cryptoWalletCreateTransferMultipleHBAR (BRCryptoWallet wallet,
                                        size_t outputsCount,
                                        BRCryptoTransferOutput *outputs,
                                        BRCryptoFeeBasis estimatedFeeBasis,
                                        BRCryptoCurrency currency,
                                        BRCryptoUnit unit,
                                        BRCryptoUnit unitForFee) {
    return NULL;
}

static OwnershipGiven BRSetOf(BRCryptoAddress)
cryptoWalletGetAddressesForRecoveryHBAR (BRCryptoWallet wallet) {
    BRSetOf(BRCryptoAddress) addresses = cryptoAddressSetCreate (1);
    
    BRCryptoWalletHBAR walletHBAR = cryptoWalletCoerce(wallet);
    BRHederaWallet hbarWallet = walletHBAR->wid;
    
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
