//
//  BRStellarTransaction.h
//  Core
//
//  Created by Carl Cherry on 5/21/2019.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "BRStellar.h"
#include "BRStellarBase.h"
#include "BRStellarPrivateStructs.h"
#include "BRStellarAccountUtils.h"
#include "BRStellarEncode.h"
#include "BRStellarAccount.h"
#include "support/BRCrypto.h"
#include "support/BRInt.h"
#include "support/BRArray.h"
#include "utils/b64.h"
#include "ed25519/ed25519.h"

struct BRStellarSerializedTransactionRecord {
    size_t   size;
    uint8_t  *buffer;
    uint8_t  txHash[32];
};

struct BRStellarTransactionRecord {
    // The address of the account "doing" the transaction
    BRStellarAddress from;
    BRStellarAddress to;
    BRStellarFeeBasis feeBasis;
    BRStellarFee fee;
    BRStellarAmount amount;
    BRStellarAsset asset;
    BRStellarSequence sequence;
    BRStellarTransactionHash hash;
    BRStellarTimeBounds *timeBounds;
    uint32_t numTimeBounds;
    BRStellarMemo *memo;
    uint32_t numSignatures;

    BRStellarSerializedTransaction signedBytes; // Set after signing

    BRStellarTransactionResult result;

    uint64_t timestamp;
    uint64_t blockHeight;
    int error;
};

void stellarSerializedTransactionRecordFree(BRStellarSerializedTransaction * signedBytes)
{
    // Free the signed bytes object
    assert(signedBytes);
    assert(*signedBytes);
    if ((*signedBytes)->buffer) {
        free((*signedBytes)->buffer);
    }
    free(*signedBytes);
}

extern BRStellarTransaction
stellarTransactionCreate(BRStellarAddress sourceAddress,
                         BRStellarAddress targetAddress,
                         BRStellarAmount amount, // For now assume XLM drops.
                         BRStellarFeeBasis feeBasis)
{
    // Called when the user is actually creating a fully populated transaction
    BRStellarTransaction transaction = calloc (1, sizeof (struct BRStellarTransactionRecord));
    assert(transaction);
    transaction->from = sourceAddress;
    transaction->to = targetAddress;
    transaction->amount = amount;
    transaction->feeBasis = feeBasis;
    transaction->fee = (BRStellarFee)stellarFeeBasisGetFee(&transaction->feeBasis);
    return transaction;
}

extern BRStellarTransaction /* caller must free - stellarTransferFree */
stellarTransactionCreateFull (BRStellarAddress sourceAddress,
                             BRStellarAddress targetAddress,
                             BRStellarAmount amount, // For now assume XRP drops.
                             BRStellarFeeBasis feeBasis,
                             BRStellarTransactionHash hash,
                             uint64_t timestamp,
                             uint64_t blockHeight,
                             int error) {

    BRStellarTransaction transaction = stellarTransactionCreate (sourceAddress, targetAddress, amount, feeBasis);

    transaction->hash        = hash;
    transaction->timestamp   = timestamp;
    transaction->blockHeight = blockHeight;
    transaction->error       = error;

    return transaction;
}

extern void stellarTransactionFree(BRStellarTransaction transaction)
{
    assert(transaction);
    if (transaction->signedBytes) {
        stellarSerializedTransactionRecordFree(&transaction->signedBytes);
    }
    stellarAddressFree(transaction->from);
    stellarAddressFree(transaction->to);

    free(transaction);
}

static void createTransactionHash(uint8_t *md32, uint8_t *tx, size_t txLength, const char* networkID)
{
    // What are we going to hash
    // sha256(networkID) + tx_type + tx
    // tx_type is basically a 4-byte packed int
    size_t size = 32 + 4 + txLength;
    uint8_t bytes_to_hash[size];
    uint8_t *pHash = bytes_to_hash;
    
    // Hash the networkID
    uint8_t networkHash[32];
    BRSHA256(networkHash, networkID, strlen(networkID));
    memcpy(pHash, networkHash, 32);
    pHash += 32;
    uint8_t tx_type[4] = {0, 0, 0, 2}; // Add the tx_type
    memcpy(pHash, tx_type, 4);
    pHash += 4;
    memcpy(pHash, tx, txLength); // Add the serialized transaction
    
    // Do a sha256 hash of the data
    BRSHA256(md32, bytes_to_hash, size);
}

// Map the network types to a string - get's hashed into the transaction
const char *stellarNetworks[] = {
    "Public Global Stellar Network ; September 2015",
    "Test SDF Network ; September 2015"
};

static BRStellarSignatureRecord stellarTransactionSign(uint8_t * tx_hash, size_t txHashLength,
                                                uint8_t *privateKey, uint8_t *publicKey)
{
    // Create a signature from the incoming bytes
    unsigned char signature[64];
    ed25519_sign(signature, tx_hash, txHashLength, publicKey, privateKey);
    
    // This is what they call a decorated signature - it includes
    // a 4-byte hint of what public key is used for signing
    BRStellarSignatureRecord sig;
    memcpy(sig.signature, &publicKey[28], 4); // Last 4 bytes of public key
    memcpy(&sig.signature[4], signature, 64);
    return sig;
}

extern size_t
stellarTransactionSerializeAndSign(BRStellarTransaction transaction, uint8_t *privateKey,
                                  uint8_t *publicKey, int64_t sequence, BRStellarNetworkType networkType)
{
    // If this transaction was previously signed - delete that info
    if (transaction->signedBytes) {
        stellarSerializedTransactionRecordFree(&transaction->signedBytes);
        transaction->signedBytes = 0;
    }
    
    // Add in the provided parameters
    transaction->sequence = sequence;

    // Serialize the bytes
    uint8_t * buffer = NULL;
    size_t length = stellarSerializeTransaction(transaction->from,
                                                transaction->to,
                                                transaction->fee,
                                                transaction->amount,
                                                sequence,
                                                transaction->timeBounds,
                                                transaction->numTimeBounds,
                                                transaction->memo,
                                                0, NULL, &buffer);

    // Create the transaction hash that needs to be signed
    uint8_t tx_hash[32];
    createTransactionHash(tx_hash, buffer, length, stellarNetworks[networkType]);

    // Sign the bytes and get signature
    BRStellarSignatureRecord sig = stellarTransactionSign(tx_hash, 32, privateKey, publicKey);

    // Serialize the bytes and sign
    free(buffer);
    length = stellarSerializeTransaction(transaction->from,
                                         transaction->to,
                                         transaction->fee,
                                         transaction->amount,
                                         sequence,
                                         transaction->timeBounds,
                                         transaction->numTimeBounds,
                                         transaction->memo,
                                         0, sig.signature, &buffer);

    if (length) {
        transaction->signedBytes = calloc(1, sizeof(struct BRStellarSerializedTransactionRecord));
        transaction->signedBytes->buffer = calloc(1, length);
        memcpy(transaction->signedBytes->buffer, buffer, length);
        transaction->signedBytes->size = length;
        memcpy(transaction->signedBytes->txHash, tx_hash, 32);
    }
    
    // Return the number of bytes written to the buffer
    return length;

}

extern BRStellarTransactionHash stellarTransactionGetHash(BRStellarTransaction transaction)
{
    assert(transaction);
    BRStellarTransactionHash hash;
    // The only time we get the hash is when we do the serialize and sign. That way
    // if someone changes the transaction we would need to generate a new hash
    if (transaction->signedBytes) {
        memcpy(hash.bytes, transaction->signedBytes->txHash, 32);
    } else {
        memset(hash.bytes, 0x00, 32);
    }
    return hash;
}

extern size_t stellarGetSerializedSize(BRStellarSerializedTransaction s)
{
    assert(s);
    return s->size;
}
extern uint8_t* stellarGetSerializedBytes(BRStellarSerializedTransaction s)
{
    assert(s);
    return (s->buffer);
}

extern int stellarTransactionHasSource (BRStellarTransaction transaction,
                                       BRStellarAddress source)
{
    // TODO - Carl
    return 0;
}

extern int stellarTransactionHasTarget (BRStellarTransaction transaction,
                                       BRStellarAddress target)
{
    // TODO - Carl
    return 0;
}

extern BRStellarAddress
stellarTransactionGetSource(BRStellarTransaction transaction)
{
    assert(transaction);
    return transaction->from;
}

extern BRStellarAddress
stellarTransactionGetTarget(BRStellarTransaction transaction)
{
    assert(transaction);
    return transaction->to;
}

extern BRStellarAmount
stellarTransactionGetAmount(BRStellarTransaction transaction)
{
    assert(transaction);
    return transaction->amount;
}
extern BRStellarFee
stellarTransactionGetFee(BRStellarTransaction transaction)
{
    assert(transaction);
    return transaction->fee;
}

extern uint8_t* stellarTransactionSerialize(BRStellarTransaction transaction, size_t * bufferSize)
{
    assert(transaction);
    assert(bufferSize);
    // If we have serialized and signed this transaction then copy the bytes to the caller
    if (transaction->signedBytes) {
        uint8_t * buffer = calloc(1, transaction->signedBytes->size);
        memcpy(buffer, transaction->signedBytes->buffer, transaction->signedBytes->size);
        *bufferSize = transaction->signedBytes->size;
        return buffer;
    } else {
        // Not yet seralialize and signed
        *bufferSize = 0;
        return NULL;
    }
}

extern void
stellarTransactionSetMemo(BRStellarTransaction transaction, BRStellarMemo * memo)
{
    assert(transaction);
    transaction->memo = memo;
}

extern int
stellarTransactionHasError(BRStellarTransaction transaction) {
    return transaction->error;
}

extern int
stellarTransactionIsInBlock (BRStellarTransaction transaction) {
    assert(transaction);
    return transaction->blockHeight == 0 ? 0 : 1;
}

extern uint64_t stellarTransactionGetBlockHeight (BRStellarTransaction transaction) {
    assert(transaction);
    return transaction->blockHeight;
}

extern BRStellarFeeBasis
stellarTransactionGetFeeBasis (BRStellarTransaction transaction)
{
    assert(transaction);
    return transaction->feeBasis;
}
