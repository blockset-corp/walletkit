//
//  BR__Name__Account.h
//  WalletKitCore
//
//  Created by __USER__ on __DATE__.
//  Copyright © __YEAR__ Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#ifndef BR__Name__Account_h
#define BR__Name__Account_h


#include "support/BRKey.h"
#include "support/BRInt.h"
#include "BR__Name__Base.h"
#include "BR__Name__Address.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct BR__Name__AccountRecord *BR__Name__Account;

/**
 * Create a __Name__ account from a seed
 *
 * @param seed - UInt512 seed value
 *
 * @return account
 */
extern BR__Name__Account  /* caller must free - using "free" function */
__name__AccountCreateWithSeed (UInt512 seed);

/**
 * Create a __Name__ account from a byte array (previously serialized
 *
 * @param bytes              array of bytes
 * @param bytesCount     size of byte array
 *
 * @return account
*/
extern BR__Name__Account  /* caller must free - using "free" function */
__name__AccountCreateWithSerialization (uint8_t *bytes, size_t bytesCount);

/**
 * Free all memory associated with this account
 *
 * @param account
 *
 * @return void
 */
extern void
__name__AccountFree (BR__Name__Account account);

/**
 * Signs a message using the account private key.
 *
 * @param account
 * @param data - the message to sign
 * @param seed - account seed
 *
 * @return signature
*/
extern WKData
__name__AccountSignData (BR__Name__Account account,
                      WKData data,
                      UInt512 seed);

/**
 * Get the public key for this __Name__ account
 *
 * @param account
 *
 * @return public key
 */
extern BRKey __name__AccountGetPublicKey (BR__Name__Account account);

/**
 * Get the __Name__ Address from the specified account.
 *
 * @param account
 *
 * @return address
 */
extern BR__Name__Address __name__AccountGetAddress (BR__Name__Account account);

/**
*
* Serialize `account`; return `bytes` and set `bytesCount`
*
* @param account
*
* @return address
*/
extern uint8_t * // Caller owns memory and must delete calling "free"
__name__AccountGetSerialization (BR__Name__Account account, size_t *bytesCount);

/**
 * Check if this account has the specified address
 *
 * @param account   __name__ account
 * @param address   __name__ address to check
 *
 * @return 1 if true, 0 if false
*/
extern int __name__AccountHasAddress (BR__Name__Account account, BR__Name__Address address);

/**
 * Return the balance limit, either asMaximum or asMinimum
 *
 * @param account   __name__ account
 * @param asMaximum - if true, return the wallet maximum limit; otherwise minimum limit
 * @param hasLimit  - must be non-NULL; assigns if wallet as the specified limit
 *
 * @return balance limit - in mutez units
 */
extern BR__Name__UnitMutez
__name__AccountGetBalanceLimit (BR__Name__Account account,
                            int asMaximum,
                            int *hasLimit);

#ifdef __cplusplus
}
#endif

#endif // BR__Name__Account_h
