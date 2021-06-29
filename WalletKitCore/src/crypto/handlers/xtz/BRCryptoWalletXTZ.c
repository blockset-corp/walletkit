//
//  BRCryptoWalletXTZ.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoXTZ.h"
#include "BRCryptoBase.h"
#include "crypto/BRCryptoWalletP.h"
#include "crypto/BRCryptoAmountP.h"
#include "crypto/BRCryptoFeeBasisP.h"
#include "tezos/BRTezos.h"
#include "support/BRSet.h"
#include "ethereum/util/BRUtilMath.h"

#include <stdio.h>
#include <errno.h>

static BRCryptoWalletXTZ
cryptoWalletCoerce (BRCryptoWallet wallet) {
    assert (CRYPTO_NETWORK_TYPE_XTZ == wallet->type);
    return (BRCryptoWalletXTZ) wallet;
}

typedef struct {
    BRTezosAccount xtzAccount;
} BRCryptoWalletCreateContextXTZ;

static void
cryptoWalletCreateCallbackXTZ (BRCryptoWalletCreateContext context,
                               BRCryptoWallet wallet) {
    BRCryptoWalletCreateContextXTZ *contextXTZ = (BRCryptoWalletCreateContextXTZ*) context;
    BRCryptoWalletXTZ walletXTZ = cryptoWalletCoerce (wallet);

    walletXTZ->xtzAccount = contextXTZ->xtzAccount;
}

private_extern BRCryptoWallet
cryptoWalletCreateAsXTZ (BRCryptoWalletListener listener,
                         BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BRTezosAccount xtzAccount) {
    int hasMinBalance;
    int hasMaxBalance;
    BRTezosUnitMutez minBalanceXTZ = tezosAccountGetBalanceLimit (xtzAccount, 0, &hasMinBalance);
    BRTezosUnitMutez maxBalanceXTZ = tezosAccountGetBalanceLimit (xtzAccount, 1, &hasMaxBalance);

    BRCryptoAmount minBalance = hasMinBalance ? cryptoAmountCreateAsXTZ(unit, CRYPTO_FALSE, minBalanceXTZ) : NULL;
    BRCryptoAmount maxBalance = hasMaxBalance ? cryptoAmountCreateAsXTZ(unit, CRYPTO_FALSE, maxBalanceXTZ) : NULL;

    BRTezosFeeBasis feeBasisXTZ = tezosDefaultFeeBasis (TEZOS_DEFAULT_MUTEZ_PER_BYTE);
    BRCryptoFeeBasis feeBasis   = cryptoFeeBasisCreateAsXTZ (unitForFee, feeBasisXTZ);

    BRCryptoWalletCreateContextXTZ contextXTZ = {
        xtzAccount
    };

    BRCryptoWallet wallet = cryptoWalletAllocAndInit (sizeof (struct BRCryptoWalletXTZRecord),
                                                      CRYPTO_NETWORK_TYPE_XTZ,
                                                      listener,
                                                      unit,
                                                      unitForFee,
                                                      minBalance,
                                                      maxBalance,
                                                      feeBasis,
                                                      &contextXTZ,
                                                      cryptoWalletCreateCallbackXTZ);

    cryptoFeeBasisGive(feeBasis);
    cryptoAmountGive (maxBalance);
    cryptoAmountGive (minBalance);

    return wallet;
}

static void
cryptoWalletReleaseXTZ (BRCryptoWallet wallet) {
    BRCryptoWalletXTZ walletXTZ = cryptoWalletCoerce (wallet);
    tezosAccountFree(walletXTZ->xtzAccount);
}

static BRCryptoAddress
cryptoWalletGetAddressXTZ (BRCryptoWallet wallet,
                           BRCryptoAddressScheme addressScheme) {
    assert (CRYPTO_ADDRESS_SCHEME_NATIVE == addressScheme);
    BRCryptoWalletXTZ walletXTZ = cryptoWalletCoerce (wallet);
    return cryptoAddressCreateAsXTZ (tezosAccountGetAddress(walletXTZ->xtzAccount));
}

static bool
cryptoWalletHasAddressXTZ (BRCryptoWallet wallet,
                           BRCryptoAddress address) {
    BRCryptoWalletXTZ walletXTZ = cryptoWalletCoerce (wallet);
    BRTezosAddress xtzAddress = cryptoAddressAsXTZ (address);
    
    return tezosAccountHasAddress (walletXTZ->xtzAccount, xtzAddress);
}

private_extern bool
cryptoWalletNeedsRevealXTZ (BRCryptoWallet wallet) {
    assert(wallet);
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        // reveal is needed before the first outgoing transfer
        BRCryptoTransferDirection direction = cryptoTransferGetDirection (wallet->transfers[index]);
        if (CRYPTO_TRANSFER_SENT == direction) return false;
    }
    return true;
}

private_extern BRCryptoTransfer
cryptoWalletGetTransferByHashOrUIDSAndTargetXTZ (BRCryptoWallet wallet,
                                           BRCryptoHash hashToMatch,
                                           const char *uids,
                                           BRCryptoAddress targetToMatch) {
    BRCryptoTransfer transfer = NULL;

    // Do the 'special match' based on hash and/or uids
    transfer = cryptoWalletGetTransferByHashOrUIDS (wallet, hashToMatch, uids);

    // Confirmed with the address
    if (NULL != transfer && !cryptoAddressIsEqual (transfer->targetAddress, targetToMatch)) {
        cryptoTransferGive(transfer);
        transfer = NULL;
    }

    return transfer;
}

extern size_t
cryptoWalletGetTransferAttributeCountXTZ (BRCryptoWallet wallet,
                                          BRCryptoAddress target) {
    size_t countRequired, countOptional;
    tezosGetTransactionAttributeKeys (1, &countRequired);
    tezosGetTransactionAttributeKeys (0, &countOptional);
    return countRequired + countOptional;
}

extern BRCryptoTransferAttribute
cryptoWalletGetTransferAttributeAtXTZ (BRCryptoWallet wallet,
                                       BRCryptoAddress target,
                                       size_t index) {
    size_t countRequired, countOptional;
    const char **keysRequired = tezosGetTransactionAttributeKeys (1, &countRequired);
    const char **keysOptional = tezosGetTransactionAttributeKeys (0, &countOptional);

    assert (index < (countRequired + countOptional));

    BRCryptoBoolean isRequired = AS_CRYPTO_BOOLEAN (index < countRequired);
    const char **keys      = (isRequired ? keysRequired : keysOptional);
    size_t       keysIndex = (isRequired ? index : (index - countRequired));

    return cryptoTransferAttributeCreate(keys[keysIndex], NULL, isRequired);
}

static int // 1 if equal, 0 if not.
tezosCompareFieldOption (const char *t1, const char *t2) {
    return 0 == strcasecmp (t1, t2);
}

extern BRCryptoTransferAttributeValidationError
cryptoWalletValidateTransferAttributeXTZ (BRCryptoWallet wallet,
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

    if (tezosCompareFieldOption (key, FIELD_OPTION_DELEGATION_OP)) {
        // expect 0 or 1
        char *end = NULL;
        errno = 0;

        uintmax_t flag = strtoumax (val, &end, 10);
        if (ERANGE != errno && EINVAL != errno && '\0' == end[0] && flag >= 0 && flag <= 1) {
            *validates = CRYPTO_TRUE;
        } else {
            *validates = CRYPTO_FALSE;
            error = CRYPTO_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_MISMATCHED_TYPE;
        }
    }
    else if (tezosCompareFieldOption (key, FIELD_OPTION_DELEGATE)) {
        // expect string
        *validates = CRYPTO_TRUE;
    }
    else if (tezosCompareFieldOption (key, FIELD_OPTION_OPERATION_TYPE)) {
        // expect string
        *validates = CRYPTO_TRUE;
    }

    else {
        error = CRYPTO_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_RELATIONSHIP_INCONSISTENCY;
        *validates = CRYPTO_FALSE;
    }
    
    return error;
}

// create for send
extern BRCryptoTransfer
cryptoWalletCreateTransferXTZ (BRCryptoWallet  wallet,
                               BRCryptoAddress target,
                               BRCryptoAmount  amount,
                               BRCryptoFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept BRCryptoTransferAttribute *attributes,
                               BRCryptoCurrency currency,
                               BRCryptoUnit unit,
                               BRCryptoUnit unitForFee) {
    BRCryptoWalletXTZ walletXTZ = cryptoWalletCoerce (wallet);
    
    BRTezosAddress source  = tezosAccountGetAddress (walletXTZ->xtzAccount);
    BRTezosAddress xtzTarget  = cryptoAddressAsXTZ (target);
    BRTezosUnitMutez mutez = tezosMutezCreate (amount);
    BRTezosFeeBasis feeBasis = cryptoFeeBasisCoerceXTZ (estimatedFeeBasis)->xtzFeeBasis;
    int64_t counter = (FEE_BASIS_ESTIMATE == feeBasis.type) ? feeBasis.u.estimate.counter : 0;
    
    bool delegationOp = false;
    
    for (size_t index = 0; index < attributesCount; index++) {
        BRCryptoTransferAttribute attribute = attributes[index];
        if (NULL != cryptoTransferAttributeGetValue(attribute)) {
            if (tezosCompareFieldOption (cryptoTransferAttributeGetKey(attribute), FIELD_OPTION_DELEGATION_OP)) {
                int op;
                sscanf (cryptoTransferAttributeGetValue(attribute), "%d", &op);
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

    BRCryptoTransferState state    = cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_CREATED);
    BRCryptoTransfer      transfer = cryptoTransferCreateAsXTZ (wallet->listenerTransfer,
                                                                NULL,
                                                                unit,
                                                                unitForFee,
                                                                state,
                                                                walletXTZ->xtzAccount,
                                                                xtzTransfer);
    
    cryptoTransferSetAttributes (transfer, attributesCount, attributes);
    cryptoTransferStateGive (state);
    
    return transfer;
}

extern BRCryptoTransfer
cryptoWalletCreateTransferMultipleXTZ (BRCryptoWallet wallet,
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
cryptoWalletGetAddressesForRecoveryXTZ (BRCryptoWallet wallet) {
    BRSetOf(BRCryptoAddress) addresses = cryptoAddressSetCreate (1);

    BRCryptoWalletXTZ walletXTZ = cryptoWalletCoerce(wallet);

    BRSetAdd (addresses, cryptoAddressCreateAsXTZ (tezosAccountGetAddress (walletXTZ->xtzAccount)));

    return addresses;
}

static bool
cryptoWalletIsEqualXTZ (BRCryptoWallet wb1, BRCryptoWallet wb2) {
    if (wb1 == wb2) return true;

    BRCryptoWalletXTZ w1 = cryptoWalletCoerce(wb1);
    BRCryptoWalletXTZ w2 = cryptoWalletCoerce(wb2);
    return w1->xtzAccount == w2->xtzAccount;
}

BRCryptoWalletHandlers cryptoWalletHandlersXTZ = {
    cryptoWalletReleaseXTZ,
    cryptoWalletGetAddressXTZ,
    cryptoWalletHasAddressXTZ,
    cryptoWalletGetTransferAttributeCountXTZ,
    cryptoWalletGetTransferAttributeAtXTZ,
    cryptoWalletValidateTransferAttributeXTZ,
    cryptoWalletCreateTransferXTZ,
    cryptoWalletCreateTransferMultipleXTZ,
    cryptoWalletGetAddressesForRecoveryXTZ,
    NULL,//BRCryptoWalletAnnounceTransfer
    cryptoWalletIsEqualXTZ
};
