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
            uint8_t * publicKey;
        } reveal;
    } u;
} BRTezosOperationData;

/**
 * Create a new Tezos transaction for sending a transfer
 *
 * @param source - account sending (or that sent) the amount
 * @param target - account receiving the amount
 * @param amount - amount that was (or will be) transferred
 *
 * @return transaction
 */
extern BRTezosTransaction /* caller owns memory and must call "tezosTransactionFree" function */
tezosTransactionCreateNew(BRTezosAddress source,
                          BRTezosAddress target,
                          BRTezosUnitMutez amount,
                          BRTezosFeeBasis feeBasis);

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
 * Sign a Tezos transaction
 *
 * @param transaction
 * @param public key      - of the source account
 * @param timeStamp       - used to create the transaction id - just use current
 *                          time
 * @param fee             - max number of tinybars the caller is willing to pay
 * @param seed            - seed for this account, used to create private key
 *
 * @return size           - number of bytes in the signed transaction
 */
extern size_t
tezosTransactionSignTransaction (BRTezosTransaction transaction,
                                 BRKey publicKey,
                                 UInt512 seed);

/**
 * Get serialiezd bytes for the specified transaction
 *
 * @param transaction
 * @param size         - pointer to variable to hold the size
 *
 * @return bytes       - pointer to serialized bytes
 */
extern uint8_t * /* caller owns and must free using normal "free" function */
tezosTransactionSerialize (BRTezosTransaction transaction, size_t *size);

extern BRTezosTransactionHash tezosTransactionGetHash(BRTezosTransaction transaction);

extern BRTezosUnitMutez tezosTransactionGetFee(BRTezosTransaction transaction);

extern BRTezosUnitMutez tezosTransactionGetAmount(BRTezosTransaction transaction);

extern BRTezosAddress tezosTransactionGetSource(BRTezosTransaction transaction);

extern BRTezosAddress tezosTransactionGetTarget(BRTezosTransaction transaction);

extern BRTezosOperationData tezosTransactionGetOperationData(BRTezosTransaction transaction);

extern bool tezosTransactionEqual (BRTezosTransaction t1, BRTezosTransaction t2);

#ifdef __cplusplus
}
#endif

#endif // BRTezosTransaction_h
