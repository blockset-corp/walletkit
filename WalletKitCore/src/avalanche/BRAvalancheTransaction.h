//
//  BRAvalancheTransaction.h
//  WalletKitCore
//
//  Created by Ed Gamble on 2021-09-15.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRAvalancheTransaction_h
#define BRAvalancheTransaction_h

#include "support/BRInt.h"
#include "support/BRKey.h"
#include "support/BRArray.h"
#include "support/BRSet.h"
#include "BRAvalancheBase.h"
#include "BRAvalancheAccount.h"
#include "BRAvalancheAddress.h"
#include "BRAvalancheNetwork.h"
#include "BRAvalancheFeeBasis.h"

#include "support/BRData.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Base Types

typedef uint64_t BRAvalancheLocktime;
typedef uint32_t BRAvalancheThreshold;
typedef uint32_t BRAvalancheGroupId;
typedef uint32_t BRAvalancheIndex;
typedef uint16_t BRAvalancheCodec;

typedef enum {
    AVALANCHE_TRANSACTION_OUTPUT_TRANSFER,           // SECP256k1 Transfer
    AVALANCHE_TRANSACTION_OUTPUT_MINT,              // SECP256k1 Mint
    AVALANCHE_TRANSACTION_OUTPUT_NFT
} BRAvalancheTransactionOutputType;

static inline uint32_t
avalancheTransactionOutputTypeGetEncoding (BRAvalancheTransactionOutputType type) {
    static const uint32_t encodings[3] = {
        7,
        11,
        UINT32_MAX
    };
    assert (type < 3);
    return encodings[type];
}

typedef enum {
    AVALANCHE_TRANSACTION_INPUT_TRANSFER
} BRAvalancheTransactionInputType;

static inline uint32_t
avalancheTransactionInputTypeGetEncoding (BRAvalancheTransactionInputType type) {
    static const uint32_t encodings[3] = {
        5
    };
    assert (type < 1);
    return encodings[type];
}

typedef enum {
    AVALANCHE_TRANSATION_PURPOSE_BASE,
    AVALANCHE_TRANSATION_PURPOSE_CREDENTIAL,
} BRAvalancheTransactionPurpose;                         // tx_type

static inline uint32_t
avalancheTransactionPurposeGetEncoding (BRAvalancheTransactionPurpose purpose) {
    static const uint32_t encodings[3] = {
        0x0000,
        0x0009
    };
    assert (purpose < 2);
    return encodings[purpose];
}

// MARK: - Transaction Input

typedef struct {
    BRAvalancheTransactionInputType type;
    BRAvalancheHash  transactionId;
    BRAvalancheIndex transactionIndex;
    BRAvalancheHash  asset;
    union {
        struct {
            BRAvalancheAmount amount;
        } transfer;
    } u;
    BRArrayOf(size_t) addressIndices;
} BRAvalancheTransactionInput;

extern void
avalancheTransactionInputRelease (BRAvalancheTransactionInput input);

static inline int
avalancheTransactionInputCompare (const BRAvalancheTransactionInput *input1,
                                  const BRAvalancheTransactionInput *input2) {
    int hashCompre = avalancheHashCompare (&input1->transactionId, &input2->transactionId);
    if (0 != hashCompre) return hashCompre;

    return (input1->transactionIndex > input2->transactionIndex
            ? +1
            : (input1->transactionIndex < input2->transactionIndex
               ? -1
               :  0));
}

// MARK: - Transaction Output

typedef struct {
    BRAvalancheTransactionOutputType type;
    BRAvalancheHash assetHash;
    BRAvalancheLocktime  locktime;
    BRAvalancheThreshold threshold;
    union {
        struct {
            BRAvalancheAmount amount;
        } transfer;

        struct {
            int ignore;
        } mint;

        struct {
            BRAvalancheGroupId groupId;
            BRData payload;
        } nft;
    } u;
    BRArrayOf(BRAvalancheAddressX) addresses;

} BRAvalancheTransactionOutput;

// MARK: - Transaction UTXO

typedef struct BRAvalancheUTXORecord {
    /// A unique identifier, derived from `transaction{Identifier,Index}, used for BRSet
    BRAvalancheHash  identifier;

    BRAvalancheHash  transactionIdentifier;
    BRAvalancheIndex transactionIndex;
    BRAvalancheHash  assetIdentifier;
    // Locktime
    BRAvalancheAmount amount;
    BRArrayOf(BRAvalancheAddress) addresses;
} *BRAvalancheUTXO;

extern BRAvalancheUTXO
avalancheUTXOCreate (BRAvalancheHash   transactionIdentifier,
                     BRAvalancheIndex  transactionIndex,
                     BRAvalancheHash   assetIdentifier,
                     BRAvalancheAmount amount,
                     OwnershipKept BRArrayOf(BRAvalancheAddress) addresses);

extern void
avalancheUTXORelease (BRAvalancheUTXO utxo);

extern BRAvalancheUTXO
avalancheUTXOClone (BRAvalancheUTXO utxo);

static inline bool
avalancheUTXOHasAsset (const BRAvalancheUTXO utxo,
                       const BRAvalancheHash asset) {
    return avalancheHashIsEqual (&utxo->assetIdentifier, &asset);
}

extern bool
avalancheUTXOHasAddress (const BRAvalancheUTXO utxo,
                         BRAvalancheAddress address,
                         size_t *addressIndex);

extern bool
avalancheUTXOSValidate (BRArrayOf(BRAvalancheUTXO) utxos,
                        BRAvalancheAddress source,
                        BRAvalancheHash    asset,
                        BRAvalancheAmount  amountWithFee);

extern BRAvalancheAmount
avalancheUTXOAmountTotal (BRArrayOf(BRAvalancheUTXO) utxos);

// BRSet Support
inline static size_t
avalancheUTXOSetValue (const BRAvalancheUTXO utxo) {
    return avalancheHashSetValue (&utxo->identifier);
}

// BRSet Support
inline static int
avalancheUTXOSetEqual (const BRAvalancheUTXO utxo1,
                       const BRAvalancheUTXO utxo2) {
    return avalancheHashSetEqual (&utxo1->identifier, &utxo2->identifier);
}

static inline BRSetOf (BRAvalancheUTXO)
avalancheUTXOSetCreate (size_t capacity) {
    return BRSetNew ((size_t (*)(const void *)) avalancheUTXOSetValue,
                     (int (*)(const void *, const void *))avalancheUTXOSetEqual,
                     capacity);
}

typedef enum {
    AVALANCHE_UTXO_SEARCH_MIN_FIRST,
    AVALANCHE_UTXO_SEARCH_MAX_FIRST,
    AVALANCHE_UTXO_SEARCH_RANDOM,
 //   AVALANCHE_UTXO_SERRCH_FIFO            // No data/blockNo/index for FIFO
} BRAvalancheUTXOSearchType;

/**
 * Find an array of utxos from the set of utxos that add up to greater than `amount`.  The `amount`
 * should include any fee.
 *
 * @param utxos  - The set of utxos available
 * @param type   - Indicates how to search the available utxos
 * @param source - The address that must be in the utxo
 * @param asset  - The asset that must be in the utxo
 * @param amountWithFee - The amount that must be exceeded
 * @param amountUTXOs   - The total amount in the returned utxos
 * @param updateUTXOx   - If true, remove returned utxos from the set of utxos
 *
 * @return An array of UTXOs with an amount adding up to at least `amount`.  If `amount` cannot be
 * reached, then the returned array as a count of 0.
 */
extern OwnershipGiven BRArrayOf(BRAvalancheUTXO)
avalancheUTXOSearchForAmount (BRSetOf(BRAvalancheUTXO) utxos,
                              BRAvalancheUTXOSearchType type,
                              BRAvalancheAddress source,
                              BRAvalancheHash    asset,
                              BRAvalancheAmount  amountWithFee,
                              BRAvalancheAmount *amountUTXOs,
                              bool updateUTXOs);

// MARK: - Transaction

#define AVALANCHE_TRANSACTION_MEMO_SIZE     (265)

typedef struct BRAvalancheTransactionRecord {
    BRAvalancheTransactionPurpose purpose;
    BRAvalancheCodec codec;

    BRAvalancheNetwork network;

    BRAvalancheAddress source;
    BRAvalancheAddress target;

    BRAvalancheAmount amount;
    BRAvalancheFeeBasis feeBasis;

    BRAvalancheHash hash;
    BRAvalancheSignature signature;

    // Serialization
    BRData serialization;

    BRArrayOf(BRAvalancheTransactionInput)  inputs;
    BRArrayOf(BRAvalancheTransactionOutput) outputs;

    char memo [AVALANCHE_TRANSACTION_MEMO_SIZE + 1];
} *BRAvalancheTransaction;

/**
 * Create a new Avalanche transaction for sending a transfer
 *
 * @param source - account sending (or that sent) the amount
 * @param target - account receiving the amount
 * @param amount - amount that was (or will be) transferred
 * @param feeBasis -
 * @param counter -
 *
 * @return transaction
 */
extern BRAvalancheTransaction /* caller owns memory and must call "avalancheTransactionFree" function */
avalancheTransactionCreate (BRAvalancheAddress  source,
                            BRAvalancheAddress  target,
                            BRAvalancheAddress  change, // if needed
                            BRAvalancheHash     asset,
                            BRAvalancheAmount   amount,
                            BRAvalancheFeeBasis feeBasis,
                            const char *memo,
                            OwnershipGiven BRArrayOf (BRAvalancheUTXO) utxos,
                            BRAvalancheNetwork  network);

/**
* Create a copy of a Avalanche transaction. Caller must free with avalancheTrasactionFree.
*
* @param transaction - the transaction to clone.
*
* @return transaction copy
*/
extern BRAvalancheTransaction
avalancheTransactionClone (BRAvalancheTransaction transaction);

/**
 * Free (destroy) a Avalanche transaction object
 *
 * @param transaction
 *
 * @return void
 */
extern void
avalancheTransactionFree (BRAvalancheTransaction transaction);

extern OwnershipGiven uint8_t *
avalancheTransactionSerializeForFeeEstimation (BRAvalancheTransaction transaction,
                                              BRAvalancheAccount account,
                                              size_t *count);

extern OwnershipGiven uint8_t *
avalancheTransactionSerializeForSubmission (BRAvalancheTransaction transaction,
                                           BRAvalancheAccount account,
                                           UInt512 seed,
                                           size_t *count);

extern  OwnershipGiven uint8_t *
avalancheTransactionGetSerialization (BRAvalancheTransaction transaction,
                                     size_t *count);

extern BRAvalancheHash
avalancheTransactionGetHash(BRAvalancheTransaction transaction);

extern BRAvalancheFeeBasis
avalancheTransactionGetFeeBasis(BRAvalancheTransaction transaction);

extern void
avalancheTransactionSetFeeBasis(BRAvalancheTransaction transaction,
                            BRAvalancheFeeBasis feeBasis);

extern BRAvalancheAmount
avalancheTransactionGetAmount(BRAvalancheTransaction transaction);

extern BRAvalancheAddress
avalancheTransactionGetSource(BRAvalancheTransaction transaction);

extern BRAvalancheAddress
avalancheTransactionGetTarget(BRAvalancheTransaction transaction);

extern BRAvalancheSignature
avalancheTransactionGetSignature (BRAvalancheTransaction transaction);

extern bool
avalancheTransactionEqual (BRAvalancheTransaction t1, BRAvalancheTransaction t2);

// MARK: - Attributes

extern const char **
avalancheTransactionGetAttributeKeys (bool asRequired, size_t *count);

// Private-ish

extern OwnershipGiven BRData
avalancheTransactionOutputEncode (BRAvalancheTransactionOutput output);

extern OwnershipGiven BRData
avalancheTransactionInputEncode (BRAvalancheTransactionInput input);

extern OwnershipGiven BRData
avalancheTransactionEncode (BRAvalancheTransaction transaction);

extern OwnershipGiven BRData
avalancheSignatureArrayEncode (BRArrayOf(BRAvalancheSignature) signatures);

#ifdef __cplusplus
}
#endif

#endif // BRAvalancheTransaction_h
