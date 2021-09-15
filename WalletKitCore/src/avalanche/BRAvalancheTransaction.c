//
//  BRAvalancheTransaction.c
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRAvalancheTransaction.h"
#include "support/BRInt.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>

#ifndef MIN
#define MIN(a,b)  ((a)<(b)?(a):(b))
#endif

// MARK: - Encoding

#define AVALANCHE_AMOUNT_ENCODING_SIZE       (sizeof (uint64_t))
#define AVALANCHE_LOCKTIME_ENCODING_SIZE     (sizeof (uint64_t))
#define AVALANCHE_THRESHOLD_ENCODING_SIZE    (sizeof (uint32_t))

#define AVALANCHE_TRANSACTION_OUTPUT_TYPE_ENCODING_SIZE         (sizeof (uint32_t))

static int
dataCompare (const BRData *d1, const BRData *d2) {
    return memcmp (d1->bytes, d2->bytes, MIN (d1->size, d2->size));
}

static OwnershipGiven BRData
avalancheBytesEncode (const uint8_t *bytes, size_t bytesCount) {
    return dataCreate ((uint8_t *) bytes, bytesCount);
}

static OwnershipGiven BRData
avalancheValueEncode (uint64_t value, size_t byteSize) {
    switch (byteSize) {
        case 1:
            return avalancheBytesEncode ((uint8_t *) &value, 1);

        case 2: {
            uint8_t bytes [2];
            UInt16SetBE (bytes, (uint16_t) value);
            return avalancheBytesEncode (bytes, 2);
        }

        case 4: {
            uint8_t bytes [4];
            UInt32SetBE (bytes, (uint32_t) value);
            return avalancheBytesEncode (bytes, 4);
        }

        case 8: {
            uint8_t bytes [8];
            UInt64SetBE (bytes, (uint64_t) value);
            return avalancheBytesEncode (bytes, 8);
        }

        default:
            assert (false);
            return dataCreateEmpty();
    }
}

static OwnershipGiven BRData
avalancheStringEncode (const char *string) {
    BRData encodings[2];

    encodings[0] = avalancheValueEncode (strlen(string), 4);
    encodings[1] = avalancheBytesEncode((const uint8_t*) string, strlen(string));

    BRData result = dataConcatTwo (encodings[0], encodings[1]);

    dataFree(encodings[0]);
    dataFree(encodings[1]);

    return result;
}

static OwnershipGiven BRData
avalancheHashEncode (BRAvalancheHash hash) {
    return avalancheBytesEncode (hash.bytes, sizeof(hash.bytes));
}

static OwnershipGiven BRData
avalancheSignatureEncode (BRAvalancheSignature signature) {
    BRData encodings[3];

    encodings[0] = avalancheValueEncode(avalancheTransactionPurposeGetEncoding (AVALANCHE_TRANSATION_PURPOSE_CREDENTIAL), 4);
    encodings[1] = avalancheValueEncode(1, 4);
    encodings[2] = avalancheSignatureGetBytes (&signature);

    BRData result = dataConcat(encodings, 3);

    dataFree(encodings[2]);
    dataFree(encodings[1]);
    dataFree(encodings[0]);

    return result;
}

static OwnershipGiven BRData
avalancheSignatureArrayEncode (BRArrayOf(BRAvalancheSignature) signatures) {
    BRArrayOf (BRData) encodings;
    array_new (encodings, 1 + array_count(signatures));

    array_add (encodings, avalancheValueEncode (array_count(signatures), 4));
    for (size_t index = 0; index < array_count(signatures); index++)
        array_add (encodings, avalancheSignatureEncode (signatures[index]));

    BRData result = dataConcat (encodings, array_count(encodings));

    array_free_all (encodings, dataFree);

    return result;
}

static OwnershipGiven BRData
avalancheAddressXEncode (BRAvalancheAddressX address) {
    return avalancheBytesEncode(address.bytes, sizeof(address.bytes));
}

static OwnershipGiven BRData
avalancheAddressXArrayEncode (BRArrayOf(BRAvalancheAddressX) addresses) {
    size_t addressesCount = array_count(addresses);

    BRArrayOf (BRData) addressEncodings;
    array_new (addressEncodings, 1 + addressesCount);

    // Prepend 4 bytes with the number-of-addresses
    array_add (addressEncodings,
               avalancheValueEncode ((uint64_t) addressesCount, 4));

    // Add each address' encoding
    for (size_t index = 0; index < addressesCount; index++)
        array_add (addressEncodings,
                   avalancheAddressXEncode (addresses[index]));

    // Concat everything together
    BRData result = dataConcat (addressEncodings, array_count(addressEncodings));

    array_free_all (addressEncodings, dataFree);

    return result;
}

static OwnershipGiven BRData
avalancheTransactionOutputEncode (BRAvalancheTransactionOutput output) {
    BRData assetEncoding     = avalancheHashEncode(output.assetHash);
    BRData addressesEncoding = avalancheAddressXArrayEncode(output.addresses);
    BRData typeEncodiing     = dataCreateEmpty();

    switch (output.type) {
        case AVALANCHE_TRANSACTION_OUTPUT_TRANSFER: {
            // Encode a variety of items
            size_t  varietyCount = (AVALANCHE_TRANSACTION_OUTPUT_TYPE_ENCODING_SIZE +
                                    AVALANCHE_AMOUNT_ENCODING_SIZE +
                                    AVALANCHE_LOCKTIME_ENCODING_SIZE +
                                    AVALANCHE_THRESHOLD_ENCODING_SIZE);
            uint8_t variety [varietyCount];

            size_t offset = 0;

            UInt32SetBE (&variety[offset],
                         avalancheTransactionOutputTypeGetEncoding(output.type));
            offset += AVALANCHE_TRANSACTION_OUTPUT_TYPE_ENCODING_SIZE;

            UInt64SetBE (&variety[offset], output.u.transfer.amount);
            offset += AVALANCHE_AMOUNT_ENCODING_SIZE;

            UInt64SetBE (&variety[offset], output.locktime);
            offset += AVALANCHE_LOCKTIME_ENCODING_SIZE;

            UInt64SetBE(&variety[offset], output.threshold);
            offset += AVALANCHE_THRESHOLD_ENCODING_SIZE;

            typeEncodiing = dataCreate (variety, varietyCount);
            break;
        }
        case AVALANCHE_TRANSACTION_OUTPUT_MINT:
            break;
        case AVALANCHE_TRANSACTION_OUTPUT_NFT:
            break;
    }

    BRData dataItems[3] = { assetEncoding, typeEncodiing, addressesEncoding};

    BRData result = dataConcat (dataItems, 3);

    dataFree(typeEncodiing);
    dataFree(addressesEncoding);
    dataFree(assetEncoding);

    return result;
}

static OwnershipGiven BRData
avalancheTransactionOutputArrayEncode (BRArrayOf(BRAvalancheTransactionOutput) outputs) {
    size_t outputsCount = array_count(outputs);

    BRArrayOf (BRData) encodings;
    array_new (encodings, 1 + outputsCount);

    array_add (encodings, avalancheValueEncode (outputsCount, 4));

    for (size_t index = 0; index < outputsCount; index++)
        array_add (encodings, avalancheTransactionOutputEncode (outputs[index]));

    // TODO: Sort encodings??

    BRData result = dataConcat (encodings, array_count(encodings));

    array_free_all (encodings, dataFree);

    return  result;
}

static OwnershipGiven BRData
avalancheTransactionInputEncode (BRAvalancheTransactionInput input) {
    BRArrayOf (BRData) encodings;
    array_new (encodings, 6 + array_count(input.addressIndices));

    array_add (encodings, avalancheHashEncode  (input.transactionId));
    array_add (encodings, avalancheValueEncode (input.transactionIndex, 4));
    array_add (encodings, avalancheHashEncode  (input.asset));
    array_add (encodings, avalancheValueEncode (avalancheTransactionInputTypeGetEncoding(input.type), 4));

    switch (input.type) {
        case AVALANCHE_TRANSACTION_INPUT_TRANSFER:
            array_add (encodings, avalancheValueEncode (input.u.transfer.amount, 8));
            break;

        default:
            assert(false);
            break;
    }

    array_add (encodings, avalancheValueEncode (array_count(input.addressIndices), 4));
    for (size_t index = 0; index < array_count(input.addressIndices); index++)
        array_add (encodings, avalancheValueEncode (input.addressIndices[index], 4));

    BRData result = dataConcat (encodings, array_count(encodings));

    array_free_all(encodings, dataFree);

    return result;
}

static OwnershipGiven BRData
avalancheTransactionInputArrayEncode (BRArrayOf(BRAvalancheTransactionInput) inputs) {
    size_t inputsCount = array_count(inputs);

    BRArrayOf (BRData) encodings;
    array_new (encodings, 1 + inputsCount);

    array_add (encodings, avalancheValueEncode (inputsCount, 4));

    for (size_t index = 0; index < inputsCount; index++)
        array_add (encodings, avalancheTransactionInputEncode (inputs[index]));

    // TODO: Sort encodings??

    BRData result = dataConcat (encodings, array_count(encodings));

    array_free_all (encodings, dataFree);

    return  result;
}

static OwnershipGiven BRData
avalancheTransactionEncode (BRAvalancheTransaction transaction) {
    BRArrayOf(BRData) encodings;
    array_new (encodings, 6);

    array_add (encodings, avalancheValueEncode (transaction->codec, 4));
    array_add (encodings, avalancheValueEncode (avalancheTransactionPurposeGetEncoding (transaction->purpose), 4));
    array_add (encodings, avalancheValueEncode (avalancheNetworkGetIdentifier (transaction->network), 4));
    array_add (encodings, avalancheHashEncode  (avalancheNetworkGetBlockchain (transaction->network)));
    array_add (encodings, avalancheTransactionOutputArrayEncode (transaction->outputs));
    array_add (encodings, avalancheTransactionInputArrayEncode  (transaction->inputs));
    array_add (encodings, avalancheStringEncode (transaction->memo));
}

// MARK: - Attributes

extern const char **
avalancheTransactionGetAttributeKeys (bool asRequired, size_t *count) {
    static size_t requiredCount = 0;
    static const char **requiredNames = NULL;

    static size_t optionalCount = 3;
    static const char **optionalNames = NULL;

    if (asRequired) { *count = requiredCount; return requiredNames; }
    else {            *count = optionalCount; return optionalNames; }
}


// MARK - Transaction

static BRAvalancheTransaction
avalancheTransactionCreateTransactionInternal (BRAvalancheAddress source,
                                              BRAvalancheAddress target,
                                              BRAvalancheAmount amount,
                                              BRAvalancheFeeBasis feeBasis,
                                              BRAvalancheHash hash,
                                              BRAvalancheSignature signature) {
    BRAvalancheTransaction transaction = calloc (1, sizeof(struct BRAvalancheTransactionRecord));

    transaction->source = source;
    transaction->target = target;

    transaction->amount   = amount;
    transaction->feeBasis = feeBasis;

    transaction->hash = AVALANCHE_HASH_EMPTY;
    transaction->signature = ((BRAvalancheSignature) { 0 });

    transaction->serialization = dataCreateEmpty();
    return transaction;
}

extern BRAvalancheTransaction
avalancheTransactionCreate (BRAvalancheAddress source,
                           BRAvalancheAddress target,
                           BRAvalancheAmount amount,
                           BRAvalancheFeeBasis feeBasis) {
    return avalancheTransactionCreateTransactionInternal (source,
                                                         target,
                                                         amount,
                                                         feeBasis,
                                                         ((BRAvalancheHash) { 0 }),
                                                         ((BRAvalancheSignature) { 0 }));
}

extern BRAvalancheTransaction
avalancheTransactionClone (BRAvalancheTransaction transaction) {
    assert(transaction);

    return avalancheTransactionCreateTransactionInternal (transaction->source,
                                                         transaction->target,
                                                         transaction->amount,
                                                         transaction->feeBasis,
                                                         transaction->hash,
                                                         transaction->signature);
}

extern void
avalancheTransactionFree (BRAvalancheTransaction transaction)
{
    assert (transaction);

    ASSERT_UNIMPLEMENTED;

    dataFree (transaction->serialization);

    memset (transaction, 0, sizeof (struct BRAvalancheTransactionRecord));
    free (transaction);
}


extern OwnershipGiven uint8_t *
avalancheTransactionSerializeForFeeEstimation (BRAvalancheTransaction transaction,
                                              BRAvalancheAccount account,
                                              size_t *count) {
    BRData serialization = avalancheTransactionEncode(transaction);

    *count = serialization.size;
    return serialization.bytes;
}

extern OwnershipGiven uint8_t *
avalancheTransactionSerializeForSubmission (BRAvalancheTransaction transaction,
                                           BRAvalancheAccount account,
                                           UInt512 seed,
                                           size_t *count) {
    ASSERT_UNIMPLEMENTED;

     // Fill in the serialization

    // Fill in the hash, if possible

   *count = 0;
    return NULL;
}

extern  OwnershipGiven uint8_t *
avalancheTransactionGetSerialization (BRAvalancheTransaction transaction,
                                     size_t *count) {
    *count = transaction->serialization.size;
    return dataClone(transaction->serialization).bytes;
}

extern BRAvalancheHash
avalancheTransactionGetHash(BRAvalancheTransaction transaction){
    assert(transaction);
    return transaction->hash;
}

extern BRAvalancheFeeBasis
avalancheTransactionGetFeeBasis(BRAvalancheTransaction transaction) {
    assert(transaction);
    return transaction->feeBasis;
}

extern void
avalancheTransactionSetFeeBasis(BRAvalancheTransaction transaction,
                                        BRAvalancheFeeBasis feeBasis) {
    assert(transaction);
    transaction->feeBasis = feeBasis;
}

extern BRAvalancheAmount
avalancheTransactionGetAmount(BRAvalancheTransaction transaction){
    assert(transaction);
    return transaction->amount;
}

extern BRAvalancheAddress
avalancheTransactionGetSource(BRAvalancheTransaction transaction){
    assert(transaction);
    return transaction->source;
}

extern BRAvalancheAddress
avalancheTransactionGetTarget(BRAvalancheTransaction transaction){
    assert(transaction);
    return transaction->target;
}

extern bool
avalancheTransactionEqual (BRAvalancheTransaction t1, BRAvalancheTransaction t2) {
    assert(t1);
    assert(t2);
    // Equal means the same transaction id, source, target
    ASSERT_UNIMPLEMENTED;
    return false;
}

