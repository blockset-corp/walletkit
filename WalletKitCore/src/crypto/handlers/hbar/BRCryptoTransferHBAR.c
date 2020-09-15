//
//  BRCryptoTransferHBAR.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoHBAR.h"
#include "crypto/BRCryptoAmountP.h"
#include "crypto/BRCryptoHashP.h"
#include "hedera/BRHederaTransaction.h"
#include "ethereum/util/BRUtilMath.h"

static BRCryptoTransferDirection
transferGetDirectionFromHBAR (BRHederaTransaction transaction,
                              BRHederaAccount account);

extern BRCryptoTransferHBAR
cryptoTransferCoerceHBAR (BRCryptoTransfer transfer) {
    assert (CRYPTO_NETWORK_TYPE_HBAR == transfer->type);
    return (BRCryptoTransferHBAR) transfer;
}

typedef struct {
    BRHederaTransaction hbarTransaction;
} BRCryptoTransferCreateContextHBAR;

static void
cryptoTransferCreateCallbackHBAR (BRCryptoTransferCreateContext context,
                                    BRCryptoTransfer transfer) {
    BRCryptoTransferCreateContextHBAR *contextHBAR = (BRCryptoTransferCreateContextHBAR*) context;
    BRCryptoTransferHBAR transferHBAR = cryptoTransferCoerceHBAR (transfer);

    transferHBAR->hbarTransaction = contextHBAR->hbarTransaction;
}

extern BRCryptoTransfer
cryptoTransferCreateAsHBAR (BRCryptoTransferListener listener,
                            BRCryptoUnit unit,
                            BRCryptoUnit unitForFee,
                            OwnershipKept BRHederaAccount hbarAccount,
                            OwnershipGiven BRHederaTransaction hbarTransaction) {
    
    BRCryptoTransferDirection direction = transferGetDirectionFromHBAR (hbarTransaction, hbarAccount);
    
    BRCryptoAmount amount = cryptoAmountCreateAsHBAR (unit,
                                                      CRYPTO_FALSE,
                                                      hederaTransactionGetAmount (hbarTransaction));
    
    BRHederaFeeBasis hbarFeeBasis = { hederaTransactionGetFee (hbarTransaction), 1 };
    BRCryptoFeeBasis feeBasisEstimated = cryptoFeeBasisCreateAsHBAR (unitForFee, hbarFeeBasis);
    
    BRCryptoAddress sourceAddress = cryptoAddressCreateAsHBAR (hederaTransactionGetSource (hbarTransaction));
    BRCryptoAddress targetAddress = cryptoAddressCreateAsHBAR (hederaTransactionGetTarget (hbarTransaction));

    BRCryptoTransferCreateContextHBAR contextHBAR = {
        hbarTransaction
    };

    BRCryptoTransfer transfer = cryptoTransferAllocAndInit (sizeof (struct BRCryptoTransferHBARRecord),
                                                            CRYPTO_NETWORK_TYPE_HBAR,
                                                            listener,
                                                            unit,
                                                            unitForFee,
                                                            feeBasisEstimated,
                                                            amount,
                                                            direction,
                                                            sourceAddress,
                                                            targetAddress,
                                                            &contextHBAR,
                                                            cryptoTransferCreateCallbackHBAR);

    cryptoFeeBasisGive (feeBasisEstimated);
    cryptoAddressGive (sourceAddress);
    cryptoAddressGive (targetAddress);
    
    return transfer;
}

static void
cryptoTransferReleaseHBAR (BRCryptoTransfer transfer) {
    BRCryptoTransferHBAR transferHBAR = cryptoTransferCoerceHBAR(transfer);
    hederaTransactionFree (transferHBAR->hbarTransaction);
}

static BRCryptoHash
cryptoTransferGetHashHBAR (BRCryptoTransfer transfer) {
    BRCryptoTransferHBAR transferHBAR = cryptoTransferCoerceHBAR(transfer);
    BRHederaTransactionHash hash = hederaTransactionGetHash (transferHBAR->hbarTransaction);
    return cryptoHashCreateInternal (hederaHashSetValue (&hash), sizeof (hash.bytes), hash.bytes);
}

static uint8_t *
cryptoTransferSerializeHBAR (BRCryptoTransfer transfer,
                             BRCryptoNetwork network,
                             BRCryptoBoolean  requireSignature,
                             size_t *serializationCount) {
    assert (CRYPTO_TRUE == requireSignature);
    BRCryptoTransferHBAR transferHBAR = cryptoTransferCoerceHBAR (transfer);
    return hederaTransactionSerialize (transferHBAR->hbarTransaction, serializationCount);
}

static int
cryptoTransferIsEqualHBAR (BRCryptoTransfer tb1, BRCryptoTransfer tb2) {
    return (tb1 == tb2 ||
            cryptoHashEqual (cryptoTransferGetHashHBAR(tb1),
                             cryptoTransferGetHashHBAR(tb2)));
}

static BRCryptoTransferDirection
transferGetDirectionFromHBAR (BRHederaTransaction transaction,
                              BRHederaAccount account) {
    BRHederaAddress source = hederaTransactionGetSource (transaction);
    BRHederaAddress target = hederaTransactionGetTarget (transaction);
    
    int isSource = hederaAccountHasAddress (account, source);
    int isTarget = hederaAccountHasAddress (account, target);
    
    return (isSource && isTarget
            ? CRYPTO_TRANSFER_RECOVERED
            : (isSource
               ? CRYPTO_TRANSFER_SENT
               : CRYPTO_TRANSFER_RECEIVED));
}

BRCryptoTransferHandlers cryptoTransferHandlersHBAR = {
    cryptoTransferReleaseHBAR,
    cryptoTransferGetHashHBAR,
    cryptoTransferSerializeHBAR,
    cryptoTransferIsEqualHBAR
};
