//
//  BRTezosAccount.h
//  Core
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#ifndef BRTezosAccount_h
#define BRTezosAccount_h


#include "support/BRKey.h"
#include "support/BRInt.h"
#include "BRTezosBase.h"
#include "BRTezosAddress.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct BRTezosAccountRecord *BRTezosAccount;

/**
 * Create a Tezos account from a seed
 *
 * @param seed - UInt512 seed value
 *
 * @return account
 */
extern BRTezosAccount  /* caller must free - using "free" function */
tezosAccountCreateWithSeed (UInt512 seed);

/**
 * Create a Tezos account from a byte array (previously serialized
 *
 * @param bytes              array of bytes
 * @param bytesCount     size of byte array
 *
 * @return account
*/
extern BRTezosAccount  /* caller must free - using "free" function */
tezosAccountCreateWithSerialization (uint8_t *bytes, size_t bytesCount);

/**
 * Free all memory associated with this account
 *
 * @param account
 *
 * @return void
 */
extern void
tezosAccountFree (BRTezosAccount account);

/**
 * Get the public key for this Tezos account
 *
 * @param account
 *
 * @return public key
 */
extern BRKey tezosAccountGetPublicKey (BRTezosAccount account);

/**
 * Get the Tezos Address from the specified account.
 *
 * @param account
 *
 * @return address
 */
extern BRTezosAddress tezosAccountGetAddress (BRTezosAccount account);

/**
*
* Serialize `account`; return `bytes` and set `bytesCount`
*
* @param account
*
* @return address
*/
extern uint8_t * // Caller owns memory and must delete calling "free"
tezosAccountGetSerialization (BRTezosAccount account, size_t *bytesCount);

/**
 * Check if this account has the specified address
 *
 * @param account   tezos account
 * @param address   tezos address to check
 *
 * @return 1 if true, 0 if false
*/
extern int tezosAccountHasAddress (BRTezosAccount account, BRTezosAddress address);

#ifdef __cplusplus
}
#endif

#endif // BRTezosAccount_h
