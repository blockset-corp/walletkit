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
#include "BRAvalancheBase.h"
#include "BRAvalancheAccount.h"
#include "BRAvalancheAddress.h"
#include "BRAvalancheFeeBasis.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Signature

#define AVALANCHE_SIGNATURE_BYTES (64)

typedef struct {
    uint8_t bytes[AVALANCHE_SIGNATURE_BYTES];
} BRAvalancheSignature;

static inline bool
avalancheSignatureIsEmpty (const BRAvalancheSignature *signature) {
    BRAvalancheSignature empty = { 0 };
    return 0 == memcmp (signature->bytes, empty.bytes, AVALANCHE_SIGNATURE_BYTES);
}

// MARK: - Attributes

extern const char **
avalancheTransactionGetAttributeKeys (bool asRequired, size_t *count);


// MARK - Transaction

typedef struct BRAvalancheTransactionRecord {
    BRAvalancheAddress source;
    BRAvalancheAddress target;

    BRAvalancheAmount amount;
    BRAvalancheFeeBasis feeBasis;

    BRAvalancheHash hash;
    BRAvalancheSignature signature;

    // Serialization
    uint8_t *serialization;
    size_t   serializationCount;
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

#ifdef __cplusplus
}
#endif

#endif // BRAvalancheTransaction_h
