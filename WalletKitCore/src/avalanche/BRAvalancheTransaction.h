//
//  BRAvalancheTransaction.h
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
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
    BRArrayOf(uint32_t) addressIndices;
} BRAvalancheTransactionInput;

static inline int
avalancheTransactionInputCompare (const BRAvalancheTransactionInput *input1,
                                  const BRAvalancheTransactionInput *input2) {
    int hashCompre = avalancheHashCompre (&input1->transactionId, &input2->transactionId);
    if (0 == hashCompre) return 0;

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

typedef struct {
    /// A unique identifier, derived from `transaction{Identifier,Index}, used for BRSet
    BRAvalancheHash  identifier;

    BRAvalancheHash  transactionIdentifier;
    BRAvalancheIndex transactionIndex;
    BRAvalancheHash  assetIdentifier;
    // Locktime
    // Amount
    // Addresses
} BRAvalancheUTXO;

typedef enum {
    AVALANCHE_UTXO_SEARCH_MIN_FIRST,
    AVALANCHE_UTXO_SEARCH_MAX_FIRST,
    AVALANCHE_UTXO_SEARCH_RANDOM,
    AVALANCHE_UTXO_SERRCH_FIFO
} BRAvalancheUTXOSearchType;

// Modifies utxos, NULL if not found
extern OwnershipGiven BRArrayOf(BRAvalancheUTXO)
avalancheUTXOSearchForAmount (BRSetOf(BRAvalancheUTXO) utxos,
                              BRAvalancheUTXOSearchType type,
                              BRAvalancheAmount amount,
                              BRAvalancheAmount *fee);
// MARK: - Transaction

#define AVALANCHE_TRANSACTION_MEMO_SIZE     (265)

typedef struct BRAvalancheTransactionRecord {
    BRAvalancheTransactionPurpose purpose;
    BRAvalancheCodec codec;

    BRAvalancheAddress source;
    BRAvalancheAddress target;

    BRAvalancheAmount amount;
    BRAvalancheFeeBasis feeBasis;

    BRAvalancheHash hash;
    BRAvalancheSignature signature;

    BRAvalancheNetwork network;

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
avalancheTransactionCreate (BRAvalancheAddress source,
                           BRAvalancheAddress target,
                           BRAvalancheAmount amount,
                           BRAvalancheFeeBasis feeBasis);

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



#ifdef __cplusplus
}
#endif

#endif // BRAvalancheTransaction_h
