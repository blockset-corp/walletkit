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
cryptoTransferCreateAsXRP (BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BRRippleWallet wallet,
                           OwnershipGiven BRRippleTransfer xrpTransfer) {
    BRCryptoTransfer transferBase = cryptoTransferAllocAndInit (sizeof (struct BRCryptoTransferXRPRecord),
                                                                CRYPTO_NETWORK_TYPE_XRP,
                                                                unit,
                                                                unitForFee);
    BRCryptoTransferXRP transfer = cryptoTransferCoerceXRP (transferBase);
    
    transfer->xrpTransfer = xrpTransfer;
    
    transferBase->direction = transferGetDirectionFromXRP (xrpTransfer, wallet);
    transferBase->amount = cryptoAmountCreateAsXRP (transferBase->unit,
                                                    CRYPTO_FALSE,
                                                    xrpTransfer->amount);
    
    BRCryptoAmount feeAmount = cryptoAmountCreateAsXRP (transferBase->unitForFee,
                                                        CRYPTO_FALSE,
                                                        xrpTransfer->fee);
    transferBase->feeBasisEstimated = cryptoFeeBasisCreate (feeAmount, 1.0);
    
    transferBase->sourceAddress = cryptoAddressCreateAsXRP (xrpTransfer->sourceAddress);
    transferBase->targetAddress = cryptoAddressCreateAsXRP (xrpTransfer->targetAddress);

    return (BRCryptoTransfer) transfer;
}

static void
cryptoTransferReleaseXRP (BRCryptoTransfer transferBase) {
    BRCryptoTransferXRP transfer = cryptoTransferCoerceXRP(transferBase);
    rippleTransferFree (transfer->xrpTransfer);
}

extern BRCryptoHash
cryptoTransferGetHashXRP (BRCryptoTransfer transferBase) {
    BRCryptoTransferXRP transfer = cryptoTransferCoerceXRP(transferBase);
    BRRippleTransactionHash hash = rippleTransferGetTransactionId (transfer->xrpTransfer);
    return cryptoHashCreateInternal (CRYPTO_NETWORK_TYPE_XRP, sizeof (hash.bytes), hash.bytes);
}

extern uint8_t *
cryptoTransferSerializeForSubmissionXRP (BRCryptoTransfer transferBase,
                                         size_t *serializationCount) {
    BRCryptoTransferXRP transfer = cryptoTransferCoerceXRP (transferBase);

    uint8_t *serialization = NULL;
    *serializationCount = 0;
    BRRippleTransaction transaction = rippleTransferGetTransaction (transfer->xrpTransfer);
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
    cryptoTransferSerializeForSubmissionXRP,
    cryptoTransferIsEqualXRP
};
