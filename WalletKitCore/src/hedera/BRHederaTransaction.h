//
//  BRHederaTransaction.h
//  Core
//
//  Created by Carl Cherry on Oct. 16, 2019.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRHederaTransaction_h
#define BRHederaTransaction_h

#include "support/BRInt.h"
#include "support/BRKey.h"
#include "BRHederaAccount.h"
#include "BRHederaAddress.h"
#include "BRHederaFeeBasis.h"
#include "BRHederaBase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BRHederaTransactionRecord *BRHederaTransaction;

/**
 * Create a new Hedera transaction for sending a transfer
 *
 * @param source - account sending (or that sent) the amount
 * @param target - account receiving the amount
 * @param amount - amount that was (or will be) transferred
 *
 * @return transaction
 */
extern BRHederaTransaction /* caller owns memory and must call "hederaTransactionFree" function */
hederaTransactionCreateNew(BRHederaAddress source,
                           BRHederaAddress target,
                           BRHederaUnitTinyBar amount,
                           BRHederaFeeBasis feeBasis,
                           BRHederaTimeStamp *timeStamp);

/**
 * Create a Hedera transaction recovered from the blockset server
 *
 * @param source    - account that sent the amount
 * @param target    - account received the amount
 * @param amount    - amount that was transferred
 * @param txID      - transaction ID for this transfer
 * @param hash      - transaction hash (generated by hedera)
 * @param timestamp      - timestamp for transaction
 * @param blockHeight      - block containing transaction
 *
 * @return transaction
 */
extern BRHederaTransaction /* caller must free - hederaTransactionFree */
hederaTransactionCreate(BRHederaAddress source, BRHederaAddress target,
                        BRHederaUnitTinyBar amount, BRHederaUnitTinyBar fee, const char *txID,
                        BRHederaTransactionHash hash, uint64_t timestamp, uint64_t blockHeight,
                        int error);

extern BRHederaTransaction /* caller must free - hederaTrasactionFree */
hederaTransactionClone (BRHederaTransaction transaction);

/**
 * Free (destroy) a Hedera transaction object
 *
 * @param transaction
 *
 * @return void
 */
extern void hederaTransactionFree (BRHederaTransaction transaction);

/**
 * Sign a Hedera transaction
 *
 * @param transaction
 * @param public key      - of the source account
 * @param seed            - seed for this account, used to create private key
 * @param nodeAddress - use specific node (if not NULL) otherwise create a serialization for all nodes
 *
 * @return size           - number of bytes in the signed transaction
 */
extern size_t
hederaTransactionSignTransaction (BRHederaTransaction transaction,
                                  BRKey publicKey,
                                  UInt512 seed,
                                  BRHederaAddress nodeAddress);

/**
 * Get serialiezd bytes for the specified transaction
 *
 * @param transaction
 * @param size         - pointer to variable to hold the size
 *
 * @return bytes       - pointer to serialized bytes
 */
extern uint8_t * /* caller owns and must free using normal "free" function */
hederaTransactionSerialize (BRHederaTransaction transaction, size_t *size);

// Getters for the various transaction fields
extern BRHederaTransactionHash hederaTransactionGetHash(BRHederaTransaction transaction);
extern char * // Caller owns memory and must free calling "free"
hederaTransactionGetTransactionId(BRHederaTransaction transaction);
extern BRHederaFeeBasis hederaTransactionGetFeeBasis (BRHederaTransaction transaction);
extern BRHederaUnitTinyBar hederaTransactionGetFee(BRHederaTransaction transaction);
extern BRHederaUnitTinyBar hederaTransactionGetAmount(BRHederaTransaction transaction);
extern BRHederaAddress hederaTransactionGetSource(BRHederaTransaction transaction);
extern BRHederaAddress hederaTransactionGetTarget(BRHederaTransaction transaction);

extern int hederaTransactionHasError (BRHederaTransaction transaction);
extern BRHederaTimeStamp hederaTransactionGetTimestamp (BRHederaTransaction transaction);
extern uint64_t hederaTransactionGetBlockheight (BRHederaTransaction transaction);

// Memo
extern void hederaTransactionSetMemo(BRHederaTransaction transaction, const char* memo);
extern char * // Caller owns memory and must free calling "free"
hederaTransactionGetMemo(BRHederaTransaction transaction);

// Check equality
extern bool hederaTransactionEqual (BRHederaTransaction t1, BRHederaTransaction t2);

extern BRHederaTimeStamp hederaGenerateTimeStamp(void);
extern BRHederaTimeStamp hederaParseTimeStamp(const char* txID);

// Internal
extern int hederaTransactionHasSource (BRHederaTransaction tranaction, BRHederaAddress address);
extern int hederaTransactionHasTarget (BRHederaTransaction tranaction, BRHederaAddress address);

extern BRHederaUnitTinyBar hederaTransactionGetAmountDirected (BRHederaTransaction transfer,
                                                               BRHederaAddress address,
                                                               int *negative);

extern int hederaTransactionUpdateHash (BRHederaTransaction transaction, BRHederaTransactionHash hash);
extern bool hederaTransactionHashEqual (BRHederaTransaction t1, BRHederaTransaction t2);

#ifdef __cplusplus
}
#endif

#endif // BRHederaTransaction_h
