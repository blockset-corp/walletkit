//
//  BR__Name__Address.h
//  WalletKitCore
//
//  Created by __USER__ on __DATE__.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BR__Name__Address_h
#define BR__Name__Address_h

#include "support/BRKey.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// prefix (3 bytes) + pkh (20 bytes)
#define __NAME___ADDRESS_BYTES (23)

// address prefixes
extern const uint8_t __SYMBOL__TZ1_PREFIX[3];
extern const uint8_t __SYMBOL__TZ2_PREFIX[3];
extern const uint8_t __SYMBOL__TZ3_PREFIX[3];
extern const uint8_t __SYMBOL__KT_PREFIX[3];


typedef struct BR__Name__AddressRecord *BR__Name__Address;

/**
 * Get the __name__ address string representation of the address.
 * Caller must free using the "free" function.
 *
 * @param address   - a BR__Name__Address
 *
 * @return pointer to allocated buffer holding the null terminated string
 */
extern char *
__name__AddressAsString (BR__Name__Address address);

/**
 * Create a __name__ address from a valid public key.
 * Caller must free using the "free" function.
 *
 * @param pubKey    - a __name__ public key
 * @param pubKeyLen - the size (bytes) of the public key
 *
 * @return address  - a BR__Name__Address object
 */
extern BR__Name__Address
__name__AddressCreateFromKey (const uint8_t * pubKey, size_t pubKeyLen);

/**
 * Create a __name__ address from a valid __name__ manager address string
 *
 * @param address   - __name__ address string in the "tz1..." format
 *
 * @return address  - a BR__Name__Address object
 */
extern BR__Name__Address
__name__AddressCreateFromString(const char * __name__AddressString, bool strict);

/**
 * Free the memory associated with a BR__Name__Address
 *
 * @param address   - a BR__Name__Address
 *
 * @return void
 */
extern void
__name__AddressFree (BR__Name__Address address);

/**
 * Check is this address is the
 *
 * @param address   - a BR__Name__Address
 *
 * @return 1 if this is the "Fee" address, 0 if not
 */
extern int
__name__AddressIsFeeAddress (BR__Name__Address address);

extern int
__name__AddressIsUnknownAddress (BR__Name__Address address);

/**
 * Copy a BR__Name__Address
 *
 * @param address   - a BR__Name__Address
 *
 * @return copy     - an exact copy of the specified address
 */
extern BR__Name__Address
__name__AddressClone (BR__Name__Address address);

/**
 * Get the size of the raw bytes for this address
 *
 * @param address   - a BR__Name__Address
 *
 * @return size of the raw bytes
 */
extern size_t
__name__AddressGetRawSize (BR__Name__Address address);

/**
 * Get the raw bytes for this address
 *
 * @param address    - a BR__Name__Address
 * @param buffer     - a buffer to hold the raw bytes
 * @param bufferSize - size of the buffer, obtained via __name__AddressGetRawSize
 *
 * @return void
 */
extern void
__name__AddressGetRawBytes (BR__Name__Address address, uint8_t *buffer, size_t bufferSize);

/**
 * Compare 2 __name__ addresses
 *
 * @param a1  first address
 * @param a2  second address
 *
 * @return 1 - if addresses are equal
 *         0 - if not equal
 */
extern int // 1 if equal
__name__AddressEqual (BR__Name__Address a1, BR__Name__Address a2);

extern size_t
__name__AddressHashValue (BR__Name__Address address);

extern bool
__name__AddressIsImplicit (BR__Name__Address address);

 
#ifdef __cplusplus
}
#endif

#endif /* BR__Name__Address_h */

