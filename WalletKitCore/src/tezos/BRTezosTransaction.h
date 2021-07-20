//
//  BRTezosTransaction.h
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRTezosTransaction_h
#define BRTezosTransaction_h

#include "support/BRInt.h"
#include "support/BRKey.h"

#include "BRTezosBase.h"
#include "BRTezosAccount.h"
#include "BRTezosAddress.h"
#include "BRTezosFeeBasis.h"
#include "BRTezosOperation.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BRTezosTransactionRecord *BRTezosTransaction;
typedef struct BRTezosSignatureRecord *BRTezosSignature;

/// Create a new Tezos Transaction with a single Operations
///
/// @param operation - the operation
///
/// @return transaction
extern BRTezosTransaction
tezosTransactionCreate (BRTezosOperation operation);

/// Create a new Tezos Transaction with a primaryOperation and a revealOperation
///
/// @param operation The primary Operation
/// @param revel The reveal Operation
///
/// @return transaction
///
extern BRTezosTransaction
tezosTransactionCreateWithReveal (BRTezosOperation operation,
                                  BRTezosOperation revel);

/**
* Create a copy of a Tezos transaction. Caller must free with tezosTrasactionFree.
*
* @param transaction - the transaction to clone.
*
* @return transaction copy
*/
extern BRTezosTransaction
tezosTransactionClone (BRTezosTransaction transaction);

/**
 * Free (destroy) a Tezos transaction object
 *
 * @param transaction
 *
 * @return void
 */
extern void
tezosTransactionFree (BRTezosTransaction transaction);

extern OwnershipKept BRTezosOperation
tezosTransactionGetPrimaryOperation (BRTezosTransaction transaction);

extern OwnershipKept BRTezosOperation
tezosTransactionGetRevealOperation (BRTezosTransaction transaction);

extern bool
tezosTransactionHasReveal (BRTezosTransaction transaction);

/**
* Serializes (forges) a Tezos operation with an empty signature and stores the bytes in the transaction.
* If a reveal operation is required it will be prepended to the serialized operation list.  The
* result is in transaction->signedBytes (even though not signed).
*
* @param transaction
* @param account         - the source account
* @param lastBlockHash   - hash of the network's most recent block, needed for serialization payload
*
 */
extern void
tezosTransactionSerializeForFeeEstimation (BRTezosTransaction transaction,
                                           BRTezosAccount account,
                                           BRTezosHash lastBlockHash);

/**
 * Serializes (forges) and signs a Tezos operation and stores the signed bytes in the transaction.
 * If a reveal operation is required it will be prepended to the serialized operation list.  The
 * result is in transaction->signedBytes.
 *
 * @param transaction
 * @param account         - the source account
 * @param seed            - seed for this account, used to create private key
 * @param lastBlockHash   - hash of the network's most recent block, needed for serialization payload
 *
 */
extern void
tezosTransactionSerializeAndSign (BRTezosTransaction transaction,
                                  BRTezosAccount account,
                                  UInt512 seed,
                                  BRTezosHash lastBlockHash);

/**
 * Get serialiezd bytes for the specified transaction
 *
 * @param transaction
 * @param size         - pointer to variable to hold the size
 *
 * @return bytes       - pointer to serialized bytes
 */

// TODO: Clarify Ownership - Up Through Swift/Java
extern  uint8_t * /* caller owns and must free using normal "free" function */
tezosTransactionGetSignedBytes (BRTezosTransaction transaction, size_t *size);

extern size_t
tezosTransactionGetSignedBytesCount (BRTezosTransaction transaction);

extern BRTezosHash
tezosTransactionGetHash(BRTezosTransaction transaction);

extern bool
tezosTransactionRequiresReveal (BRTezosTransaction transaction);

extern bool
tezosTransactionEqual (BRTezosTransaction t1, BRTezosTransaction t2);

#ifdef __cplusplus
}
#endif

#endif // BRTezosTransaction_h
