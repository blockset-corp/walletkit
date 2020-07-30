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
#include "ripple/BRRippleTransfer.h"
#include "ripple/BRRipplePrivateStructs.h"
#include "ethereum/util/BRUtilMath.h"

static BRCryptoTransferDirection
transferGetDirectionFromXRP (BRRippleTransfer transfer,
                             BRRippleWallet wallet);

extern BRCryptoTransferXRP
cryptoTransferCoerceXRP (BRCryptoTransfer transfer) {
    assert (CRYPTO_NETWORK_TYPE_XRP == transfer->type);
    return (BRCryptoTransferXRP) transfer;
}

extern BRCryptoTransfer
cryptoTransferCreateAsXRP (BRCryptoTransferListener listener,
                           BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           OwnershipKept BRRippleWallet wallet,
                           OwnershipGiven BRRippleTransfer xrpTransfer) {
    
    BRCryptoTransferDirection direction = transferGetDirectionFromXRP (xrpTransfer, wallet);
    
    BRCryptoAmount amount = cryptoAmountCreateAsXRP (unit,
                                                     CRYPTO_FALSE,
                                                     xrpTransfer->amount);
    
    BRCryptoAmount feeAmount = cryptoAmountCreateAsXRP (unitForFee,
                                                        CRYPTO_FALSE,
                                                        xrpTransfer->fee);
    BRCryptoFeeBasis feeBasisEstimated = cryptoFeeBasisCreate (feeAmount, 1.0);
    
    BRCryptoAddress sourceAddress = cryptoAddressCreateAsXRP (xrpTransfer->sourceAddress);
    BRCryptoAddress targetAddress = cryptoAddressCreateAsXRP (xrpTransfer->targetAddress);
    
    BRCryptoTransfer transfer = cryptoTransferAllocAndInit (sizeof (struct BRCryptoTransferXRPRecord),
                                                            CRYPTO_NETWORK_TYPE_XRP,
                                                            listener,
                                                            unit,
                                                            unitForFee,
                                                            feeBasisEstimated,
                                                            amount,
                                                            direction,
                                                            sourceAddress,
                                                            targetAddress);
    BRCryptoTransferXRP transferXRP = cryptoTransferCoerceXRP (transfer);
    
    transferXRP->xrpTransfer = xrpTransfer;
    
    cryptoFeeBasisGive (feeBasisEstimated);
    cryptoAddressGive (sourceAddress);
    cryptoAddressGive (targetAddress);

    return transfer;
}

static void
cryptoTransferReleaseXRP (BRCryptoTransfer transfer) {
    BRCryptoTransferXRP transferXRP = cryptoTransferCoerceXRP(transfer);
    rippleTransferFree (transferXRP->xrpTransfer);
}

static BRCryptoHash
cryptoTransferGetHashXRP (BRCryptoTransfer transfer) {
    BRCryptoTransferXRP transferXRP = cryptoTransferCoerceXRP(transfer);
    BRRippleTransactionHash hash = rippleTransferGetTransactionId (transferXRP->xrpTransfer);
    return cryptoHashCreateInternal (CRYPTO_NETWORK_TYPE_XRP, sizeof (hash.bytes), hash.bytes);
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
    BRRippleTransaction transaction = rippleTransferGetTransaction (transferXRP->xrpTransfer);
    if (transaction) {
        serialization = rippleTransactionSerialize (transaction, serializationCount);
    }
    
    return serialization;
}

static int
cryptoTransferIsEqualXRP (BRCryptoTransfer tb1, BRCryptoTransfer tb2) {
    return (tb1 == tb2 ||
            cryptoHashEqual (cryptoTransferGetHashXRP(tb1),
                             cryptoTransferGetHashXRP(tb2)));
}

static BRCryptoTransferDirection
transferGetDirectionFromXRP (BRRippleTransfer transfer,
                             BRRippleWallet wallet) {
    BRRippleAddress source = rippleTransferGetSource (transfer);
    BRRippleAddress target = rippleTransferGetTarget (transfer);
    
    int isSource = rippleWalletHasAddress (wallet, source);
    int isTarget = rippleWalletHasAddress (wallet, target);
    
    return (isSource && isTarget
            ? CRYPTO_TRANSFER_RECOVERED
            : (isSource
               ? CRYPTO_TRANSFER_SENT
               : CRYPTO_TRANSFER_RECEIVED));
}

BRCryptoTransferHandlers cryptoTransferHandlersXRP = {
    cryptoTransferReleaseXRP,
    cryptoTransferGetHashXRP,
    cryptoTransferSerializeXRP,
    cryptoTransferIsEqualXRP
};
