//
//  BRCryptoTransferXRP.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoXRP.h"
#include "crypto/BRCryptoAmountP.h"
#include "crypto/BRCryptoHashP.h"
#include "ripple/BRRippleTransaction.h"
#include "ethereum/util/BRUtilMath.h"

static BRCryptoTransferDirection
transferGetDirectionFromXRP (BRRippleTransaction transaction,
                             BRRippleAccount account);

extern BRCryptoTransferXRP
cryptoTransferCoerceXRP (BRCryptoTransfer transfer) {
    assert (CRYPTO_NETWORK_TYPE_XRP == transfer->type);
    return (BRCryptoTransferXRP) transfer;
}

typedef struct {
    BRRippleTransaction xrpTransaction;
} BRCryptoTransferCreateContextXRP;

extern BRRippleTransaction
cryptoTransferAsXRP (BRCryptoTransfer transfer) {
    BRCryptoTransferXRP transferXRP = cryptoTransferCoerceXRP (transfer);
    return transferXRP->xrpTransaction;
}

static void
cryptoTransferCreateCallbackXRP (BRCryptoTransferCreateContext context,
                                    BRCryptoTransfer transfer) {
    BRCryptoTransferCreateContextXRP *contextXRP = (BRCryptoTransferCreateContextXRP*) context;
    BRCryptoTransferXRP transferXRP = cryptoTransferCoerceXRP (transfer);

    transferXRP->xrpTransaction = contextXRP->xrpTransaction;
}

extern BRCryptoTransfer
cryptoTransferCreateAsXRP (BRCryptoTransferListener listener,
                           const char *uids,
                           BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BRCryptoTransferState state,
                           OwnershipKept BRRippleAccount xrpAccount,
                           OwnershipGiven BRRippleTransaction xrpTransfer) {
    
    BRCryptoTransferDirection direction = transferGetDirectionFromXRP (xrpTransfer, xrpAccount);
    
    BRCryptoAmount amount = cryptoAmountCreateAsXRP (unit,
                                                     CRYPTO_FALSE,
                                                     rippleTransactionGetAmount(xrpTransfer));

    BRCryptoFeeBasis feeBasisEstimated = cryptoFeeBasisCreateAsXRP (unitForFee, (CRYPTO_TRANSFER_RECEIVED == direction ? 0 : rippleTransactionGetFee(xrpTransfer)));
    
    BRCryptoAddress sourceAddress = cryptoAddressCreateAsXRP (rippleTransactionGetSource(xrpTransfer));
    BRCryptoAddress targetAddress = cryptoAddressCreateAsXRP (rippleTransactionGetTarget(xrpTransfer));

    BRCryptoTransferCreateContextXRP contextXRP = {
        xrpTransfer
    };

    BRCryptoTransfer transfer = cryptoTransferAllocAndInit (sizeof (struct BRCryptoTransferXRPRecord),
                                                            CRYPTO_NETWORK_TYPE_XRP,
                                                            listener,
                                                            uids,
                                                            unit,
                                                            unitForFee,
                                                            feeBasisEstimated,
                                                            amount,
                                                            direction,
                                                            sourceAddress,
                                                            targetAddress,
                                                            state,
                                                            &contextXRP,
                                                            cryptoTransferCreateCallbackXRP);

    cryptoFeeBasisGive (feeBasisEstimated);
    cryptoAddressGive (sourceAddress);
    cryptoAddressGive (targetAddress);

    return transfer;
}

static void
cryptoTransferReleaseXRP (BRCryptoTransfer transfer) {
    BRCryptoTransferXRP transferXRP = cryptoTransferCoerceXRP(transfer);
    rippleTransactionFree (transferXRP->xrpTransaction);
}

static BRCryptoHash
cryptoTransferGetHashXRP (BRCryptoTransfer transfer) {
    BRCryptoTransferXRP transferXRP = cryptoTransferCoerceXRP(transfer);
    BRRippleTransactionHash hash = rippleTransactionGetHash(transferXRP->xrpTransaction);
    return cryptoHashCreateAsXRP (hash);
}

static uint8_t *
cryptoTransferSerializeXRP (BRCryptoTransfer transfer,
                            BRCryptoNetwork network,
                            BRCryptoBoolean  requireSignature,
                            size_t *serializationCount) {
    assert (CRYPTO_TRUE == requireSignature);
    BRCryptoTransferXRP transferXRP = cryptoTransferCoerceXRP (transfer);

    uint8_t *serialization = NULL;
    *serializationCount = 0;
    BRRippleTransaction transaction = transferXRP->xrpTransaction;
    if (transaction) {
        serialization = rippleTransactionSerialize (transaction, serializationCount);
    }
    
    return serialization;
}

static int
cryptoTransferIsEqualXRP (BRCryptoTransfer tb1, BRCryptoTransfer tb2) {
    if (tb1 == tb2) return 1;

    BRCryptoHash h1 = cryptoTransferGetHashXRP (tb1);
    BRCryptoHash h2 = cryptoTransferGetHashXRP (tb2);

    int result = (CRYPTO_TRUE == cryptoHashEqual (h1, h2));

    cryptoHashGive (h2);
    cryptoHashGive (h1);

    return result;
}

static BRCryptoTransferDirection
transferGetDirectionFromXRP (BRRippleTransaction transaction,
                             BRRippleAccount account) {
    BRRippleAddress address = rippleAccountGetAddress(account);

    int isSource = rippleTransactionHasSource(transaction, address);
    int isTarget = rippleTransactionHasTarget(transaction, address);

    rippleAddressFree(address);

    return (isSource && isTarget
            ? CRYPTO_TRANSFER_RECOVERED
            : (isSource
               ? CRYPTO_TRANSFER_SENT
               : CRYPTO_TRANSFER_RECEIVED));
}

BRCryptoTransferHandlers cryptoTransferHandlersXRP = {
    cryptoTransferReleaseXRP,
    cryptoTransferGetHashXRP,
    NULL, // setHash
    NULL,
    cryptoTransferSerializeXRP,
    NULL, // getBytesForFeeEstimate
    cryptoTransferIsEqualXRP
};
