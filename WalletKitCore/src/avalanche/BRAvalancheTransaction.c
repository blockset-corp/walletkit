//
//  BRAvalancheTransaction.c
//  WalletKitCore
//
//  Created by Ed Gamble on 2021-09-15.
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

static int // (d1 < d2 ? +1 : (d1 > d2 ? -1 : 0))
dataCompare (const BRData *d1, const BRData *d2) {
    return memcmp (d1->bytes, d2->bytes, MIN (d1->size, d2->size));
}

static int
dataCompareHelper (const void * i1, const void * i2){
    const BRData *v1 = (BRData *) i1;
    const BRData *v2 = (BRData *) i2;
    return dataCompare (v1, v2);
}

static int
avalancheTransactionInputCompareHelper (const void * i1, const void * i2) {
    const BRAvalancheTransactionInput * v1 = (const BRAvalancheTransactionInput *) i1;
    const BRAvalancheTransactionInput * v2 = (const BRAvalancheTransactionInput *) i2;
    return avalancheTransactionInputCompare (v1, v2);
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

extern OwnershipGiven BRData
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

extern OwnershipGiven BRData
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

            UInt32SetBE(&variety[offset], output.threshold);
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

    // The encodings are sorted based on a straight `memcmp` of the BRData bytes
    mergesort_brd (&encodings[1], outputsCount, sizeof (BRData), dataCompareHelper);

    BRData result = dataConcat (encodings, array_count(encodings));

    array_free_all (encodings, dataFree);

    return  result;
}

extern OwnershipGiven BRData
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

    // Sort the inputs directly
    mergesort (inputs, inputsCount, sizeof(BRAvalancheTransactionInput), avalancheTransactionInputCompareHelper);

    for (size_t index = 0; index < inputsCount; index++)
        array_add (encodings, avalancheTransactionInputEncode (inputs[index]));

    // TODO: Sort encodings??

    BRData result = dataConcat (encodings, array_count(encodings));

    array_free_all (encodings, dataFree);

    return  result;
}

extern OwnershipGiven BRData
avalancheTransactionEncode (BRAvalancheTransaction transaction) {
    BRArrayOf(BRData) encodings;
    array_new (encodings, 6);

    array_add (encodings, avalancheValueEncode (transaction->codec, 2));
    array_add (encodings, avalancheValueEncode (avalancheTransactionPurposeGetEncoding (transaction->purpose), 4));
    array_add (encodings, avalancheValueEncode (avalancheNetworkGetIdentifier (transaction->network), 4));
    array_add (encodings, avalancheHashEncode  (avalancheNetworkGetBlockchain (transaction->network)));
    array_add (encodings, avalancheTransactionOutputArrayEncode (transaction->outputs));
    array_add (encodings, avalancheTransactionInputArrayEncode  (transaction->inputs));
    array_add (encodings, avalancheStringEncode (transaction->memo));

    BRData result = dataConcat (encodings, array_count (encodings));

    array_free_all (encodings, dataFree);

    return result;
}

// MARK: - Transaction Input

extern void
avalancheTransactionInputRelease (BRAvalancheTransactionInput input) {
    array_free (input.addressIndices);
}

// MARK: - Transaction Output

extern void
avalancheTransactionOutputRelease (BRAvalancheTransactionOutput output) {
    array_free (output.addresses);
}

// MARK: - Transaction UTXO

extern BRAvalancheUTXO
avalancheUTXOCreate (BRAvalancheHash   transactionIdentifier,
                     BRAvalancheIndex  transactionIndex,
                     BRAvalancheHash   assetIdentifier,
                     BRAvalancheAmount amount,
                     OwnershipKept BRArrayOf(BRAvalancheAddress) addresses) {
    BRAvalancheUTXO utxo = malloc (sizeof (struct BRAvalancheUTXORecord));

    *utxo = (struct BRAvalancheUTXORecord) {
        AVALANCHE_HASH_EMPTY,
        transactionIdentifier,
        transactionIndex,
        assetIdentifier,
        amount,
        NULL
    };

    //
    // Build a unique identifier as transaction{Identifier,Index}
    //
    size_t  bytesCount = sizeof (BRAvalancheHash) + sizeof (BRAvalancheIndex);
    uint8_t bytes [bytesCount];

    size_t offset = 0;
    memcpy (&bytes[offset], &transactionIdentifier, sizeof (BRAvalancheHash));
    offset += sizeof (BRAvalancheHash);

    memcpy (&bytes[offset], &transactionIndex, sizeof (BRAvalancheIndex));

    utxo->identifier = avalancheHashCreate (bytes, bytesCount);

    //
    // Copy addresses array
    //
    array_new (utxo->addresses, array_count (addresses));
    array_add_array (utxo->addresses, addresses, array_count(addresses));

    return utxo;
}

extern void
avalancheUTXORelease (BRAvalancheUTXO utxo) {
    if (NULL == utxo) return;

    if (NULL != utxo->addresses) array_free (utxo->addresses);
    memset (utxo, 0, sizeof (struct BRAvalancheUTXORecord));
    free (utxo);
}

extern BRAvalancheUTXO
avalancheUTXOClone (BRAvalancheUTXO utxo) {
    return avalancheUTXOCreate (utxo->transactionIdentifier,
                                utxo->transactionIndex,
                                utxo->assetIdentifier,
                                utxo->amount,
                                utxo->addresses);
}

extern bool
avalancheUTXOHasAddress (const BRAvalancheUTXO utxo,
                         BRAvalancheAddress address,
                         size_t *addressIndex) {
    for (size_t index = 0; index < array_count(utxo->addresses); index++)
        if (avalancheAddressEqual (utxo->addresses[index], address)) {
            if (NULL != addressIndex) *addressIndex = index;
            return true;
        }
    return false;
}

/**
 * Check of all of `utxos` has `source` and `asset` and if their total amount meets or exceeds
 * `amountWithFee`.
 */
extern bool
avalancheUTXOSValidate (BRArrayOf(BRAvalancheUTXO) utxos,
                        BRAvalancheAddress source,
                        BRAvalancheHash    asset,
                        BRAvalancheAmount  amountWithFee) {
    BRAvalancheAmount amount = 0;

    for (size_t index = 0; index < array_count(utxos); index++) {
        const BRAvalancheUTXO utxo = utxos[index];

        if (!avalancheUTXOHasAsset   (utxo, asset) ||
            !avalancheUTXOHasAddress (utxo, source, NULL))
            return false;

        amount += utxo->amount;
    }

    return amount >= amountWithFee;
}

extern BRAvalancheAmount
avalancheUTXOAmountTotal (BRArrayOf(BRAvalancheUTXO) utxos) {
    BRAvalancheAmount amount = 0;
    for (size_t index = 0; index < array_count(utxos); index++)
        amount += utxos[index]->amount;
    return amount;
}

static int
avalancheUTXOSortByAmountIncreasing (const void *v1, const void *v2) {
    const BRAvalancheUTXO utxo1 = *(const BRAvalancheUTXO *) v1;
    const BRAvalancheUTXO utxo2 = *(const BRAvalancheUTXO *) v2;

    return (utxo1->amount < utxo2->amount
            ? -1
            : (utxo1->amount > utxo2->amount
               ? +1
               :  0));
}

static int
avalanceUTXOSortByAmountDecreasing (const void *v1, const void *v2) {
    return -avalancheUTXOSortByAmountIncreasing (v1, v2);  // negate
}

static int
avalancheUTXOSortByDateIncreasing (const void *v1, const void *v2) {
    const BRAvalancheUTXO utxo1 = *(const BRAvalancheUTXO *) v1;
    const BRAvalancheUTXO utxo2 = *(const BRAvalancheUTXO *) v2;

    (void) utxo1; (void) utxo2;

    return 0;
}

static SortCompareRoutine
avalancheUTXOSearchGetCompareRoutine (BRAvalancheUTXOSearchType type) {
    static SortCompareRoutine routines[] = {
        avalancheUTXOSortByAmountIncreasing,
        avalanceUTXOSortByAmountDecreasing,
        NULL,
        avalancheUTXOSortByDateIncreasing
    };
    return routines[type];
}

extern OwnershipGiven BRArrayOf(BRAvalancheUTXO)
avalancheUTXOSearchForAmount (BRSetOf(BRAvalancheUTXO) utxos,
                              BRAvalancheUTXOSearchType type,
                              BRAvalancheAddress source,
                              BRAvalancheHash    asset,
                              BRAvalancheAmount  amountWithFee,
                              BRAvalancheAmount *amountUTXOs,
                              bool updateUTXOs) {
    assert (NULL != amountUTXOs);
    *amountUTXOs = 0;

    size_t utxosCount = BRSetCount(utxos);

    BRArrayOf (BRAvalancheUTXO) utxosOrdered;
    array_new (utxosOrdered, utxosCount);
    array_set_count(utxosOrdered, utxosCount);

    // Everything in `utxosOrdered` is a reference to UTXOS in `utxos`.  (Memory is shared)
    BRSetAll(utxos, (void**) utxosOrdered, utxosCount);

    // Optionally sort the utxos
    SortCompareRoutine sorter = avalancheUTXOSearchGetCompareRoutine (type);
    if (NULL != sorter)
        mergesort_brd (utxosOrdered, utxosCount, sizeof (BRAvalancheUTXO), sorter);

    BRArrayOf(BRAvalancheUTXO) result;
    array_new (result, 1);

    // Find the UTXOS
    for (size_t index = 0; index < utxosCount; index++) {
        BRAvalancheUTXO utxo = utxosOrdered[index];

        if (avalancheUTXOHasAsset   (utxo, asset) &&
            avalancheUTXOHasAddress (utxo, source, NULL)) {

            // We'll add `utxo` to `result` but if we are removing it from `utxos` then we can
            // simply take ownership of it; otherwise we need to copy it.
            if (updateUTXOs) BRSetRemove (utxos, utxo);
            else utxo = avalancheUTXOClone (utxo);

            array_add (result, utxo);
            *amountUTXOs += utxo->amount;

            if (*amountUTXOs >= amountWithFee) break /* from for-loop */;
        }
    }

    // Check if `amount` was met
    if (*amountUTXOs < amountWithFee) {
        // Return the utxos in `result`
        for (size_t index = 0; index < array_count(result); index++) {
            if (updateUTXOs) BRSetAdd (utxos, result[index]);
            else avalancheUTXORelease(result[index]);
        }
        array_clear(result);
        *amountUTXOs = 0;
    }

    return result;
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


// MARK: - Transaction

static BRAvalancheTransaction
avalancheTransactionCreateTransactionInternal (BRAvalancheTransactionPurpose purpose,
                                               BRAvalancheCodec codec,
                                               BRAvalancheNetwork network,
                                               BRAvalancheAddress source,
                                              BRAvalancheAddress target,
                                              BRAvalancheAmount amount,
                                              BRAvalancheFeeBasis feeBasis,
                                              BRAvalancheHash hash,
                                              BRAvalancheSignature signature,
                                               const char *memo) {
    BRAvalancheTransaction transaction = calloc (1, sizeof(struct BRAvalancheTransactionRecord));

    transaction->purpose  = purpose;
    transaction->codec    = codec;
    transaction->network  = network;
    transaction->source   = source;
    transaction->target   = target;
    transaction->amount   = amount;
    transaction->feeBasis = feeBasis;
    transaction->hash      = hash;
    transaction->signature = signature;

    transaction->serialization = dataCreateEmpty();

    strncpy (transaction->memo, memo, AVALANCHE_TRANSACTION_MEMO_SIZE);
    transaction->memo[AVALANCHE_TRANSACTION_MEMO_SIZE] = '\0';

    array_new (transaction->inputs,  2);
    array_new (transaction->outputs, 2);

    return transaction;
}

extern BRAvalancheTransaction
avalancheTransactionCreate (BRAvalancheAddress  source,
                            BRAvalancheAddress  target,
                            BRAvalancheAddress  change, // if needed
                            BRAvalancheHash     asset,
                            BRAvalancheAmount   amount,
                            BRAvalancheFeeBasis feeBasis,
                            const char *memo,
                            OwnershipGiven BRArrayOf (BRAvalancheUTXO) utxos,
                            BRAvalancheNetwork  network) {
    // The total amount sent
    BRAvalancheAmount amountTotal = amount + avalancheFeeBasisGetFee(&feeBasis);

    // The UTXOS have been pre-selected to validate.
    if (!avalancheUTXOSValidate (utxos, source, asset, amountTotal)) {
        assert (false);
        return NULL;
    }

    // TODO: check address types

    BRAvalancheTransaction transaction =
    avalancheTransactionCreateTransactionInternal (AVALANCHE_TRANSATION_PURPOSE_BASE,
                                                   0,
                                                   network,
                                                   source,
                                                   target,
                                                   amount,
                                                   feeBasis,
                                                   AVALANCHE_HASH_EMPTY,
                                                   avalancheSignatureCreateEmpty(),
                                                   memo);

    // A running total of UTXO amounts
    BRAvalancheAmount amountTotalUTXOs = 0;

    //
    // Process UTXOs into inputs.
    //
    for (size_t index = 0; index < array_count(utxos); index++) {
        // This UTXO is validated
        const BRAvalancheUTXO utxo = utxos[index];

        size_t addressIndex;
        avalancheUTXOHasAddress (utxo, source, &addressIndex);

        BRArrayOf(size_t) addressIndices;
        array_new (addressIndices, 1);
        array_add (addressIndices, addressIndex);

        BRAvalancheTransactionInput input = (BRAvalancheTransactionInput) {
            AVALANCHE_TRANSACTION_INPUT_TRANSFER,
            utxos[index]->transactionIdentifier,
            utxos[index]->transactionIndex,
            asset,
            { .transfer = { utxos[index]->amount }},
            addressIndices
        };
        array_add (transaction->inputs, input);

        amountTotalUTXOs += utxos[index]->amount;

        // If we've accumulated enough for `amount + fee`, then skip out.
        if (amountTotalUTXOs >= amountTotal)
            break;
    }

    // The amountTotalUTXOs is already validated; belt-and-suspenders
    assert (amountTotalUTXOs >= amountTotal);

    // The change amount is anything left over after amountTotal
    BRAvalancheAmount amountChange = amountTotalUTXOs - amountTotal;

    //
    // Create `output1`
    //

    BRArrayOf (BRAvalancheAddressX) targetAddresses;
    array_new (targetAddresses, 1);
    array_add (targetAddresses, target.u.x);

    BRAvalancheTransactionOutput output1 = (BRAvalancheTransactionOutput) {
        AVALANCHE_TRANSACTION_OUTPUT_TRANSFER,
        asset,
        0, // locktime
        1, // threshold
        { .transfer = { amount }},
        targetAddresses
    };
    array_add (transaction->outputs, output1);

    //
    // Create `output2` if needed.
    //

    if (amountChange > 0) {

        BRArrayOf (BRAvalancheAddressX) changeAddresses;
        array_new (changeAddresses, 1);
        array_add (changeAddresses, change.u.x);

        BRAvalancheTransactionOutput output2 = (BRAvalancheTransactionOutput) {
            AVALANCHE_TRANSACTION_OUTPUT_TRANSFER,
            asset,
            0, // locktime
            1, // threshold
            { .transfer = { amountChange }},
            changeAddresses
        };
        array_add (transaction->outputs, output2);
    }

    return transaction;
}

#if 0
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
#endif

extern void
avalancheTransactionFree (BRAvalancheTransaction transaction)
{
    assert (transaction);

    array_free_all (transaction->inputs,  avalancheTransactionInputRelease);
    array_free_all (transaction->outputs, avalancheTransactionOutputRelease);

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

