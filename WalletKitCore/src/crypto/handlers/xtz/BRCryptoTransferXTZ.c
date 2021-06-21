//
//  BRCryptoTransferXTZ.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoXTZ.h"
#include "crypto/BRCryptoAmountP.h"
#include "crypto/BRCryptoHashP.h"
#include "tezos/BRTezosTransfer.h"
#include "tezos/BRTezosFeeBasis.h"
#include "ethereum/util/BRUtilMath.h"

static BRCryptoTransferDirection
transferGetDirectionFromXTZ (BRTezosTransfer transfer,
                             BRTezosAccount account);

extern BRCryptoTransferXTZ
cryptoTransferCoerceXTZ (BRCryptoTransfer transfer) {
    assert (CRYPTO_NETWORK_TYPE_XTZ == transfer->type);
    return (BRCryptoTransferXTZ) transfer;
}

typedef struct {
    BRTezosTransfer xtzTransfer;
} BRCryptoTransferCreateContextXTZ;

static void
cryptoTransferCreateCallbackXTZ (BRCryptoTransferCreateContext context,
                                    BRCryptoTransfer transfer) {
    BRCryptoTransferCreateContextXTZ *contextXTZ = (BRCryptoTransferCreateContextXTZ*) context;
    BRCryptoTransferXTZ transferXTZ = cryptoTransferCoerceXTZ (transfer);

    transferXTZ->xtzTransfer = contextXTZ->xtzTransfer;
}

extern BRCryptoTransfer
cryptoTransferCreateAsXTZ (BRCryptoTransferListener listener,
                           const char *uids,
                           BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BRCryptoTransferState state,
                           OwnershipKept BRTezosAccount xtzAccount,
                           OwnershipGiven BRTezosTransfer xtzTransfer) {
    
    BRCryptoTransferDirection direction = transferGetDirectionFromXTZ (xtzTransfer, xtzAccount);
    
    BRCryptoAmount amount = cryptoAmountCreateAsXTZ (unit,
                                                     CRYPTO_FALSE,
                                                     tezosTransferGetAmount (xtzTransfer));
    
    BRTezosFeeBasis xtzFeeBasis = tezosFeeBasisCreateActual (CRYPTO_TRANSFER_RECEIVED == direction ? 0 : tezosTransferGetFee(xtzTransfer));
    BRCryptoFeeBasis feeBasis = cryptoFeeBasisCreateAsXTZ (unitForFee,
                                                           xtzFeeBasis);
    
    BRCryptoAddress sourceAddress = cryptoAddressCreateAsXTZ (tezosTransferGetSource (xtzTransfer));
    BRCryptoAddress targetAddress = cryptoAddressCreateAsXTZ (tezosTransferGetTarget (xtzTransfer));

    BRCryptoTransferCreateContextXTZ contextXTZ = {
        xtzTransfer
    };

    BRCryptoTransfer transfer = cryptoTransferAllocAndInit (sizeof (struct BRCryptoTransferXTZRecord),
                                                            CRYPTO_NETWORK_TYPE_XTZ,
                                                            listener,
                                                            uids,
                                                            unit,
                                                            unitForFee,
                                                            feeBasis,
                                                            amount,
                                                            direction,
                                                            sourceAddress,
                                                            targetAddress,
                                                            state,
                                                            &contextXTZ,
                                                            cryptoTransferCreateCallbackXTZ);
    
    cryptoFeeBasisGive (feeBasis);
    cryptoAddressGive (sourceAddress);
    cryptoAddressGive (targetAddress);

    return transfer;
}

static void
cryptoTransferReleaseXTZ (BRCryptoTransfer transfer) {
    BRCryptoTransferXTZ transferXTZ = cryptoTransferCoerceXTZ(transfer);
    tezosTransferFree (transferXTZ->xtzTransfer);
}

static BRCryptoHash
cryptoTransferGetHashXTZ (BRCryptoTransfer transfer) {
    BRCryptoTransferXTZ transferXTZ = cryptoTransferCoerceXTZ(transfer);
    BRTezosHash hash = tezosTransferGetTransactionId (transferXTZ->xtzTransfer);
    return cryptoHashCreateAsXTZ (hash);
}

static uint8_t *
cryptoTransferSerializeXTZ (BRCryptoTransfer transfer,
                            BRCryptoNetwork network,
                            BRCryptoBoolean  requireSignature,
                            size_t *serializationCount) {
    BRCryptoTransferXTZ transferXTZ = cryptoTransferCoerceXTZ (transfer);

    uint8_t *serialization = NULL;
    *serializationCount = 0;
    BRTezosTransaction transaction = tezosTransferGetTransaction (transferXTZ->xtzTransfer);
    if (transaction) {
        serialization = tezosTransactionGetSignedBytes (transaction, serializationCount);
    }
    
    return serialization;
}

static int
cryptoTransferIsEqualXTZ (BRCryptoTransfer tb1, BRCryptoTransfer tb2) {
    if (tb1 == tb2) return 1;
    
    BRCryptoTransferXTZ tz1 = cryptoTransferCoerceXTZ (tb1);
    BRCryptoTransferXTZ tz2 = cryptoTransferCoerceXTZ (tb2);
    
    return tezosTransferIsEqual (tz1->xtzTransfer, tz2->xtzTransfer);
}

static BRCryptoTransferDirection
transferGetDirectionFromXTZ (BRTezosTransfer transfer,
                             BRTezosAccount account) {
    BRTezosAddress source = tezosTransferGetSource (transfer);
    BRTezosAddress target = tezosTransferGetTarget (transfer);
    
    int isSource = tezosAccountHasAddress (account, source);
    int isTarget = tezosAccountHasAddress (account, target);
    
    tezosAddressFree (target);
    tezosAddressFree (source);
    
    return (isSource && isTarget
            ? CRYPTO_TRANSFER_RECOVERED
            : (isSource
               ? CRYPTO_TRANSFER_SENT
               : CRYPTO_TRANSFER_RECEIVED));
}

BRCryptoTransferHandlers cryptoTransferHandlersXTZ = {
    cryptoTransferReleaseXTZ,
    cryptoTransferGetHashXTZ,
    NULL, // setHash
    NULL, // updateIdentifier
    cryptoTransferSerializeXTZ,
    NULL, // getBytesForFeeEstimate
    cryptoTransferIsEqualXTZ
};
