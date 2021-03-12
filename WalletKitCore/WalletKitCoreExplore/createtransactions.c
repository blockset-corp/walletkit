//
#include "createtransactions.h"
#include "ripple/BRRipple.h"
#include "support/BRBase58.h"
#include "support/BRBIP32Sequence.h"
#include "support/BRBIP39WordsEn.h"
#include "support/BRKey.h"
#include "ripple/BRRipple.h"

#include "tezos/BRTezosTransaction.h"
#include "tezos/BRTezosTransfer.h"
#include "tezos/BRTezosAccount.h"
#include "tezos/BRTezosEncoder.h"

static void
assembleTransaction (const char * source_paper_key,
                     BRRippleAccount sourceAccount,
                     BRRippleAddress targetAddress,
                     BRRippleUnitDrops amount,
                     uint32_t sequence,
                     uint32_t destinationTag) {
    BRRippleTransaction transaction;

    BRRippleAddress sourceAddress = rippleAccountGetAddress(sourceAccount);
    BRRippleFeeBasis feeBasis;
    feeBasis.pricePerCostFactor = 5;
    feeBasis.costFactor = 1;
    transaction = rippleTransactionCreate(sourceAddress, targetAddress, amount, feeBasis);
    rippleAddressFree(sourceAddress);

    if (destinationTag > 0) {
        rippleTransactionSetDestinationTag(transaction, destinationTag);
    }

    // Serialize and sign
    rippleAccountSetSequence(sourceAccount, sequence);
    UInt512 seed = UINT512_ZERO;
    BRBIP39DeriveKey(seed.u8, source_paper_key, NULL);
    rippleAccountSignTransaction(sourceAccount, transaction, seed);

    // Sign the transaction
    size_t signedBytesSize = 0;
    uint8_t * signedBytes = rippleTransactionSerialize(transaction, &signedBytesSize);
    for (int i = 0; i < signedBytesSize; i++) {
        if (i == 0) printf("Signed bytes: \n");
        printf("%02X", signedBytes[i]);
    }
    printf("\n");

    // Print out the hash (transactionID)
    BRRippleTransactionHash hash = rippleTransactionGetHash(transaction);
    for (int i = 0; i < 32; i++) {
        if (i == 0) printf("HASH: \n");
        printf("%02X", hash.bytes[i]);
    }
    printf("\n");

    rippleTransactionFree(transaction);
    free(signedBytes);
}

void createRippleTransaction() {
    const char * sourcePaperKey = "patient doctor olympic frog force glimpse endless antenna online dragon bargain someone";
    const char * targetPaperKey = "choose color rich dose toss winter dutch cannon over air cash market"; // rwjruMZqtebGhobxYuFVoNg6KmVMbEUws3
    BRRippleAccount sourceAccount = rippleAccountCreate(sourcePaperKey);
    BRRippleAccount targetAccount = rippleAccountCreate(targetPaperKey);
    BRRippleAddress targetAddress = rippleAccountGetAddress(targetAccount);

    BRRippleAddress sourceAddress = rippleAccountGetAddress(sourceAccount);
    char * cstrSourceAddress = rippleAddressAsString(sourceAddress);
    char * cstrTargetAddress = rippleAddressAsString(targetAddress);
    printf("source is %s, target is %s\n", cstrSourceAddress, cstrTargetAddress);
    free(cstrSourceAddress);
    free(cstrTargetAddress);

    // Now create a transaction and sign
    assembleTransaction(sourcePaperKey, sourceAccount, targetAddress, 500000, 60110803, 0);
    rippleAccountFree(sourceAccount);
    rippleAddressFree(targetAddress);
}


void createTezosTransaction() {
    const char * paperKey = "patient doctor olympic frog force glimpse endless antenna online dragon bargain someone";
    const char * branch = "BKoGoxTy8vr7WrP3KzcUbH1jZ9a9em67rx4a5hhC5DdoHgp3t4s";
    const char * fromAddress = "tz1SeV3tueHQMTfquZSU7y98otvQTw6GDKaY";
    const char * toAddress = "tz1es8RjqHUD483BN9APWtvCzgjTFVGeMh3y";
    int64_t amount = 1000;
    int64_t counter = 9886579;
    // Use the following 2 fields to get some sort of fee
    BRTezosUnitMutez pricePerByte = 100; // Not the true fee
    double sizeInBytes = 1;
    int64_t gasLimit = 10200;
    int64_t storageLimit = 0;
    BRTezosFeeBasis feeBasis = tezosFeeBasisCreateEstimate(pricePerByte, sizeInBytes, gasLimit, storageLimit, counter);

    BRTezosAddress sourceAddress = tezosAddressCreateFromString(fromAddress, true);
    BRTezosAddress targetAddress = tezosAddressCreateFromString(toAddress, true);
    UInt512 seed = UINT512_ZERO;
    BRBIP39DeriveKey(seed.u8, paperKey, NULL); // no passphrase
    BRTezosAccount account = tezosAccountCreateWithSeed(seed);

    BRTezosHash lastBlockHash;
    BRBase58CheckDecode(lastBlockHash.bytes, sizeof(lastBlockHash.bytes), branch);

    BRTezosTransfer transfer = tezosTransferCreateNew(sourceAddress, targetAddress, amount, feeBasis, counter, /*delegation*/0);
    BRTezosTransaction tx = tezosTransferGetTransaction(transfer);

    size_t signedSize = tezosTransactionSerializeAndSign(tx, account, seed, lastBlockHash, 0);
    assert(signedSize > 0);

    size_t signedSize2 = 0;
    uint8_t *signedBytes = tezosTransactionGetSignedBytes(tx, &signedSize2);
    assert(signedSize == signedSize2);

    for (int i = 0; i < signedSize2; i++) {
        if (i == 0) printf("Signed bytes (Tezos): \n");
        printf("%02X", signedBytes[i]);
    }
    printf("\n");

    tezosAddressFree(targetAddress);
    tezosAddressFree(sourceAddress);
    tezosTransferFree(transfer);
    tezosAccountFree(account);
}

void createTransactions() {
    createRippleTransaction();
    createTezosTransaction();
}


