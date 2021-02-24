//
//  BRTezosTransaction.h
//  Core
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
#include "BRTezosAccount.h"
#include "BRTezosAddress.h"
#include "BRTezosFeeBasis.h"
#include "BRTezosBase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BRTezosTransactionRecord *BRTezosTransaction;
typedef struct BRTezosSerializedOperationRecord *BRTezosSerializedOperation;
typedef struct BRTezosSignatureRecord *BRTezosSignature;

typedef struct {
    BRTezosOperationKind kind;
    union {
        struct {
            BRTezosAddress target;
            BRTezosUnitMutez amount;
        } transaction;
        
        struct {
            BRTezosAddress target;
        } delegation;
        
        struct {
            uint8_t publicKey[TEZOS_PUBLIC_KEY_SIZE];
        } reveal;
    } u;
} BRTezosOperationData;

/**
 * Create a new Tezos transaction for sending a transfer
 *
 * @param source - account sending (or that sent) the amount
 * @param target - account receiving the amount
 * @param amount - amount that was (or will be) transferred
 * @param feeBasis -
 * @param counter -
 *
 * @return transaction
 */
extern BRTezosTransaction /* caller owns memory and must call "tezosTransactionFree" function */
tezosTransactionCreateTransaction (BRTezosAddress source,
                                   BRTezosAddress target,
                                   BRTezosUnitMutez amount,
                                   BRTezosFeeBasis feeBasis,
                                   int64_t counter);

/**
* Create a new Tezos delegation operation
*
* @param source - account that is delegating its balance
* @param target - baker address or self
* @param amount - amount that was (or will be) transferred
* @param feeBasis -
* @param counter -
*
* @return transaction
*/
extern BRTezosTransaction
tezosTransactionCreateDelegation (BRTezosAddress source,
                                  BRTezosAddress target,
                                  BRTezosFeeBasis feeBasis,
                                  int64_t counter);

extern BRTezosTransaction
tezosTransactionCreateReveal (BRTezosAddress source,
                              uint8_t * pubKey,
                              BRTezosFeeBasis feeBasis,
                              int64_t counter);

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

/**
* Serializes (forges) a Tezos operation with an empty signature and stores the bytes in the transaction.
* If a reveal operation is required it will be prepended to the serialized operation list.
*
* @param transaction
* @param account         - the source account
* @param lastBlockHash   - hash of the network's most recent block, needed for serialization payload
*
 * @return size           - number of bytes in the serialization
 */
extern size_t
tezosTransactionSerializeForFeeEstimation (BRTezosTransaction transaction,
                                           BRTezosAccount account,
                                           BRTezosHash lastBlockHash,
                                           bool needsReveal);

/**
 * Serializes (forges) and signs a Tezos operation and stores the signed bytes in the transaction.
 * If a reveal operation is required it will be prepended to the serialized operation list.
 *
 * @param transaction
 * @param account         - the source account
 * @param seed            - seed for this account, used to create private key
 * @param lastBlockHash   - hash of the network's most recent block, needed for serialization payload
 *
 * @return size           - number of bytes in the signed transaction
 */
extern size_t
tezosTransactionSerializeAndSign (BRTezosTransaction transaction,
                                  BRTezosAccount account,
                                  UInt512 seed,
                                  BRTezosHash lastBlockHash,
                                  bool needsReveal);

/**
 * Get serialiezd bytes for the specified transaction
 *
 * @param transaction
 * @param size         - pointer to variable to hold the size
 *
 * @return bytes       - pointer to serialized bytes
 */
extern uint8_t * /* caller owns and must free using normal "free" function */
tezosTransactionGetSignedBytes (BRTezosTransaction transaction, size_t *size);

extern BRTezosHash
tezosTransactionGetHash(BRTezosTransaction transaction);

extern int64_t
tezosTransactionGetCounter(BRTezosTransaction transaction);

extern BRTezosUnitMutez
tezosTransactionGetFee(BRTezosTransaction transaction);

extern BRTezosFeeBasis
tezosTransactionGetFeeBasis(BRTezosTransaction transaction);

extern void
tezosTransactionSetFeeBasis(BRTezosTransaction transaction,
                            BRTezosFeeBasis feeBasis);

extern BRTezosUnitMutez
tezosTransactionGetAmount(BRTezosTransaction transaction);

extern BRTezosAddress
tezosTransactionGetSource(BRTezosTransaction transaction);

extern BRTezosAddress
tezosTransactionGetTarget(BRTezosTransaction transaction);

extern BRTezosOperationKind
tezosTransactionGetOperationKind(BRTezosTransaction transaction);

extern BRTezosOperationData
tezosTransactionGetOperationData(BRTezosTransaction transaction);

extern void
tezosTransactionFreeOperationData(BRTezosOperationData opData);

extern bool
tezosTransactionEqual (BRTezosTransaction t1, BRTezosTransaction t2);

#ifdef __cplusplus
}
#endif

#endif // BRTezosTransaction_h
