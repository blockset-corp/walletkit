//
//  BR__Name__Transaction.h
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BR__Name__Transaction_h
#define BR__Name__Transaction_h

#include "support/BRInt.h"
#include "support/BRKey.h"
#include "BR__Name__Account.h"
#include "BR__Name__Address.h"
#include "BR__Name__FeeBasis.h"
#include "BR__Name__Base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BR__Name__TransactionRecord *BR__Name__Transaction;
typedef struct BR__Name__SerializedOperationRecord *BR__Name__SerializedOperation;
typedef struct BR__Name__SignatureRecord *BR__Name__Signature;

typedef struct {
    BR__Name__OperationKind kind;
    union {
        struct {
            BR__Name__Address target;
            BR__Name__UnitMutez amount;
        } transaction;
        
        struct {
            BR__Name__Address target;
        } delegation;
        
        struct {
            uint8_t publicKey[__NAME___PUBLIC_KEY_SIZE];
        } reveal;
    } u;
} BR__Name__OperationData;

/**
 * Create a new __Name__ transaction for sending a transfer
 *
 * @param source - account sending (or that sent) the amount
 * @param target - account receiving the amount
 * @param amount - amount that was (or will be) transferred
 * @param feeBasis -
 * @param counter -
 *
 * @return transaction
 */
extern BR__Name__Transaction /* caller owns memory and must call "__name__TransactionFree" function */
__name__TransactionCreateTransaction (BR__Name__Address source,
                                   BR__Name__Address target,
                                   BR__Name__UnitMutez amount,
                                   BR__Name__FeeBasis feeBasis,
                                   int64_t counter);

/**
* Create a new __Name__ delegation operation
*
* @param source - account that is delegating its balance
* @param target - baker address or self
* @param amount - amount that was (or will be) transferred
* @param feeBasis -
* @param counter -
*
* @return transaction
*/
extern BR__Name__Transaction
__name__TransactionCreateDelegation (BR__Name__Address source,
                                  BR__Name__Address target,
                                  BR__Name__FeeBasis feeBasis,
                                  int64_t counter);

extern BR__Name__Transaction
__name__TransactionCreateReveal (BR__Name__Address source,
                              uint8_t * pubKey,
                              BR__Name__FeeBasis feeBasis,
                              int64_t counter);

/**
* Create a copy of a __Name__ transaction. Caller must free with __name__TrasactionFree.
*
* @param transaction - the transaction to clone.
*
* @return transaction copy
*/
extern BR__Name__Transaction
__name__TransactionClone (BR__Name__Transaction transaction);

/**
 * Free (destroy) a __Name__ transaction object
 *
 * @param transaction
 *
 * @return void
 */
extern void
__name__TransactionFree (BR__Name__Transaction transaction);

/**
* Serializes (forges) a __Name__ operation with an empty signature and stores the bytes in the transaction.
* If a reveal operation is required it will be prepended to the serialized operation list.
*
* @param transaction
* @param account         - the source account
* @param lastBlockHash   - hash of the network's most recent block, needed for serialization payload
*
 * @return size           - number of bytes in the serialization
 */
extern size_t
__name__TransactionSerializeForFeeEstimation (BR__Name__Transaction transaction,
                                           BR__Name__Account account,
                                           BR__Name__Hash lastBlockHash,
                                           bool needsReveal);

/**
 * Serializes (forges) and signs a __Name__ operation and stores the signed bytes in the transaction.
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
__name__TransactionSerializeAndSign (BR__Name__Transaction transaction,
                                  BR__Name__Account account,
                                  UInt512 seed,
                                  BR__Name__Hash lastBlockHash,
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
__name__TransactionGetSignedBytes (BR__Name__Transaction transaction, size_t *size);

extern BR__Name__Hash
__name__TransactionGetHash(BR__Name__Transaction transaction);

extern int64_t
__name__TransactionGetCounter(BR__Name__Transaction transaction);

extern BR__Name__UnitMutez
__name__TransactionGetFee(BR__Name__Transaction transaction);

extern BR__Name__FeeBasis
__name__TransactionGetFeeBasis(BR__Name__Transaction transaction);

extern void
__name__TransactionSetFeeBasis(BR__Name__Transaction transaction,
                            BR__Name__FeeBasis feeBasis);

extern BR__Name__UnitMutez
__name__TransactionGetAmount(BR__Name__Transaction transaction);

extern BR__Name__Address
__name__TransactionGetSource(BR__Name__Transaction transaction);

extern BR__Name__Address
__name__TransactionGetTarget(BR__Name__Transaction transaction);

extern BR__Name__OperationKind
__name__TransactionGetOperationKind(BR__Name__Transaction transaction);

extern BR__Name__OperationData
__name__TransactionGetOperationData(BR__Name__Transaction transaction);

extern void
__name__TransactionFreeOperationData(BR__Name__OperationData opData);

extern bool
__name__TransactionEqual (BR__Name__Transaction t1, BR__Name__Transaction t2);

#ifdef __cplusplus
}
#endif

#endif // BR__Name__Transaction_h
