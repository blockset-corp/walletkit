//
//  BR__Name__Transaction.h
//  WalletKitCore
//
//  Created by __USER__ on __DATE__.
//  Copyright © __YEAR__ Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BR__Name__Transaction_h
#define BR__Name__Transaction_h

#include "support/BRInt.h"
#include "support/BRKey.h"
#include "BR__Name__Base.h"
#include "BR__Name__Account.h"
#include "BR__Name__Address.h"
#include "BR__Name__FeeBasis.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Signature

#define __NAME___SIGNATURE_BYTES (64)

typedef struct {
    uint8_t bytes[__NAME___SIGNATURE_BYTES];
} BR__Name__Signature;

static inline bool
__name__SignatureIsEmpty (const BR__Name__Signature *signature) {
    BR__Name__Signature empty = { 0 };
    return 0 == memcmp (signature->bytes, empty.bytes, __NAME___SIGNATURE_BYTES);
}

// MARK: - Attributes

extern const char **
__name__TransactionGetAttributeKeys (bool asRequired, size_t *count);


// MARK - Transaction

typedef struct BR__Name__TransactionRecord {
    BR__Name__Address source;
    BR__Name__Address target;

    BR__Name__Amount amount;
    BR__Name__FeeBasis feeBasis;

    BR__Name__Hash hash;
    BR__Name__Signature signature;

    // Serialization
    uint8_t *serialization;
    size_t   serializationCount;
} *BR__Name__Transaction;

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
__name__TransactionCreate (BR__Name__Address source,
                           BR__Name__Address target,
                           BR__Name__Amount amount,
                           BR__Name__FeeBasis feeBasis);

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

extern OwnershipGiven uint8_t *
__name__TransactionSerializeForFeeEstimation (BR__Name__Transaction transaction,
                                              BR__Name__Account account,
                                              size_t *count);

extern OwnershipGiven uint8_t *
__name__TransactionSerializeForSubmission (BR__Name__Transaction transaction,
                                           BR__Name__Account account,
                                           UInt512 seed,
                                           size_t *count);

extern  OwnershipGiven uint8_t *
__name__TransactionGetSerialization (BR__Name__Transaction transaction,
                                     size_t *count);

extern BR__Name__Hash
__name__TransactionGetHash(BR__Name__Transaction transaction);

extern BR__Name__FeeBasis
__name__TransactionGetFeeBasis(BR__Name__Transaction transaction);

extern void
__name__TransactionSetFeeBasis(BR__Name__Transaction transaction,
                            BR__Name__FeeBasis feeBasis);

extern BR__Name__Amount
__name__TransactionGetAmount(BR__Name__Transaction transaction);

extern BR__Name__Address
__name__TransactionGetSource(BR__Name__Transaction transaction);

extern BR__Name__Address
__name__TransactionGetTarget(BR__Name__Transaction transaction);

extern BR__Name__Signature
__name__TransactionGetSignature (BR__Name__Transaction transaction);

extern bool
__name__TransactionEqual (BR__Name__Transaction t1, BR__Name__Transaction t2);

#ifdef __cplusplus
}
#endif

#endif // BR__Name__Transaction_h
