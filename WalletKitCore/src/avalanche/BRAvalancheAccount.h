//
//  BRAvalancheAccount.h
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#ifndef BRAvalancheAccount_h
#define BRAvalancheAccount_h

#include "support/BRKey.h"
#include "support/BRInt.h"
#include "support/BRData.h"

#include "BRAvalancheBase.h"
#include "BRAvalancheAddress.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Account

/**
 * BRAvalancheAccount
 */
typedef struct BRAvalancheAccountRecord *BRAvalancheAccount;

/**
 * Create a Avalanche account from a seed
 *
 * @param seed - UInt512 seed value
 *
 * @return account
 */
extern BRAvalancheAccount  /* caller must free - using "free" function */
avalancheAccountCreateWithSeed (UInt512 seed);

/**
 * Create a Avalanche account from a byte array (previously serialized
 *
 * @param bytes              array of bytes
 * @param bytesCount     size of byte array
 *
 * @return account
*/
extern BRAvalancheAccount  /* caller must free - using "free" function */
avalancheAccountCreateWithSerialization (uint8_t *bytes,
                                        size_t bytesCount);

/**
 * Free all memory associated with this account
 *
 * @param account
 *
 * @return void
 */
extern void
avalancheAccountFree (BRAvalancheAccount account);

/**
 * Get the Avalanche Address from the specified account.
 *
 * @param account
 *
 * @return address
 */
extern BRAvalancheAddress
avalancheAccountGetAddress (BRAvalancheAccount account,
                            BRAvalancheChainType type);

/**
*
* Serialize `account`; return `bytes` and set `bytesCount`
*
* @param account
*
* @return address
*/
extern uint8_t * // Caller owns memory and must delete calling "free"
avalancheAccountGetSerialization (BRAvalancheAccount account,
                                 size_t *bytesCount);

/**
 * Check if this account has the specified address
 *
 * @param account   avalanche account
 * @param address   avalanche address to check
 *
 * @return 1 if true, 0 if false
*/
extern int
avalancheAccountHasAddress (BRAvalancheAccount account,
                            BRAvalancheAddress address);

/**
 * Return the balance limit, either asMaximum or asMinimum
 *
 * @param account   avalanche account
 * @param asMaximum - if true, return the wallet maximum limit; otherwise minimum limit
 * @param hasLimit  - must be non-NULL; assigns if wallet as the specified limit
 *
 * @return balance limit - in mutez units
 */
extern BRAvalancheAmount
avalancheAccountGetBalanceLimit (BRAvalancheAccount account,
                            int asMaximum,
                            int *hasLimit);

// MARK: - Signature

typedef struct {
    uint8_t r[32];
    uint8_t s[32];
    uint8_t v;
} BRAvalancheSignature; // RSV

static inline bool
avalancheSignatureIsEmpty (const BRAvalancheSignature *signature) {
    BRAvalancheSignature empty = { 0 };
    return 0 == memcmp (signature, &empty, sizeof(BRAvalancheSignature));
}

static inline OwnershipGiven BRData
avalancheSignatureGetBytes (const BRAvalancheSignature *signature) {
    return dataCreate ((uint8_t *) signature, sizeof(BRAvalancheSignature));
}

static inline BRAvalancheSignature
avalancheSignatureCreateEmpty (void) {
    return (BRAvalancheSignature) { 0 };
}

/**
 * Signs a message using the account private key.
 *
 * @param account
 * @param bytes - the bytes to sign
 * @param bytesCount - the number of byes
 * @param seed - account seed
 *
 * @return result BRAvalancheSignature
 */
extern BRAvalancheSignature
avalancheAccountSignData (BRAvalancheAccount account,
                          uint8_t *bytes,
                          size_t   bytesCount,
                          UInt512 seed);

/**
 * Create a standard avalanche message for signing as:
 *     ("\x1A""Avalanche Signed Message:\n" || bytesCount || bytes).
 *
 * @param account
 * @param bytes - the bytes
 * @param bytesCount - the number of bytes
 * @param messageCount - pointer filled with the message size
 *
 * @return a byte array of a standard avalanche message or NULL if an error occurred.
 *
 */
extern uint8_t *
avalancheAccountCreateStandardMessage (BRAvalancheAccount account,
                                       uint8_t *bytes,
                                       size_t bytesCount,
                                       size_t *messageCount);

#ifdef __cplusplus
}
#endif

#endif // BRAvalancheAccount_h
