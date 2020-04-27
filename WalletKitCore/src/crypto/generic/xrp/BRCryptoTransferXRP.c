#include "BRCryptoXRP.h"
#include "generic/BRGeneric.h"

// A XRP Transfer
struct BRCryptoTransferXRPRecord {
    struct BRCryptoTransferRecord base;
    BRGenericTransfer gen;
};

extern BRCryptoTransfer
cryptoTransferCreateAsGEN (BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           OwnershipGiven BRGenericTransfer tid) {
    BRCryptoTransfer transfer = cryptoTransferCreateInternal (BLOCK_CHAIN_TYPE_GEN, unit, unitForFee);
    transfer->u.gen = tid;

    BRGenericFeeBasis gwmFeeBasis = genTransferGetFeeBasis (tid); // Will give ownership
    transfer->feeBasisEstimated = cryptoFeeBasisCreateAsGEN (transfer->unitForFee, gwmFeeBasis);

    transfer->sourceAddress = cryptoAddressCreateAsGEN (genTransferGetSourceAddress (tid));
    transfer->targetAddress = cryptoAddressCreateAsGEN (genTransferGetTargetAddress (tid));

    return transfer;
}

static void
cryptoTransferReleaseBTC (BRCryptoTransfer transfer) {
   genTransferRelease(transfer->u.gen);
}


static BRCryptoAmount
cryptoTransferGetAmountAsSignXRP (BRCryptoTransfer transfer, BRCryptoBoolean isNegative) {
    BRGenericTransfer tid = transfer->u.gen;

    return cryptoAmountCreate (transfer->unit,
                               isNegative,
                               genTransferGetAmount (tid));
}

static BRCryptoTransferDirection
cryptoTransferGetDirectionXRP (BRCryptoTransfer transfer) {
            switch (genTransferGetDirection (transfer->u.gen)) {
                case GENERIC_TRANSFER_SENT:      return CRYPTO_TRANSFER_SENT;
                case GENERIC_TRANSFER_RECEIVED:  return CRYPTO_TRANSFER_RECEIVED;
                case GENERIC_TRANSFER_RECOVERED: return CRYPTO_TRANSFER_RECOVERED;
            }
}

extern BRCryptoHash
cryptoTransferGetHashXRP (BRCryptoTransfer transfer) {
    BRGenericTransfer tid = transfer->u.gen;

    BRGenericHash hash = genTransferGetHash (tid);
    return (genericHashIsEmpty (hash)
            ? NULL
            : cryptoHashCreateAsGEN (hash));
}

private_extern BRGenericTransfer
cryptoTransferAsGEN (BRCryptoTransfer transfer) {
    assert (BLOCK_CHAIN_TYPE_GEN == transfer->type);
    return transfer->u.gen;
}

private_extern BRCryptoBoolean
cryptoTransferHasGEN (BRCryptoTransfer transfer,
                      BRGenericTransfer gen) {
    return AS_CRYPTO_BOOLEAN (BLOCK_CHAIN_TYPE_GEN == transfer->type &&
                              genTransferEqual (gen, transfer->u.gen));
}

static int
cryptoTransferEqualAsGEN (BRCryptoTransfer t1, BRCryptoTransfer t2) {
    return genTransferEqual (t1->u.gen, t2->u.gen);
}

