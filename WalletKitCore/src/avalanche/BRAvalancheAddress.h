//
//  BRAvalancheAddress.h
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRAvalancheAddress_h
#define BRAvalancheAddress_h

#include <stdbool.h>

#include "support/BRKey.h"

#ifdef __cplusplus
extern "C" {
#endif

// Possibly a 'value' type instead of this 'reference' type
typedef struct BRAvalancheAddressRecord *BRAvalancheAddress;

/**
 * Get the avalanche address string representation of the address.
 * Caller must free using the "free" function.
 *
 * @param address   - a BRAvalancheAddress
 *
 * @return pointer to allocated buffer holding the null terminated string
 */
extern char *
avalancheAddressAsString (BRAvalancheAddress address);

/**
 * Create a avalanche address from a valid public key.
 * Caller must free using the "free" function.
 *
 * @param pubKey    - a avalanche public key
 * @param pubKeyLen - the size (bytes) of the public key
 *
 * @return address  - a BRAvalancheAddress object
 */
extern BRAvalancheAddress
avalancheAddressCreateFromKey (const uint8_t * pubKey, size_t pubKeyLen);

/**
 * Create a avalanche address from a valid avalanche manager address string
 *
 * @param address   - avalanche address string in the "tz1..." format
 *
 * @return address  - a BRAvalancheAddress object
 */
extern BRAvalancheAddress
avalancheAddressCreateFromString(const char * avalancheAddressString, bool strict);

/**
 * Free the memory associated with a BRAvalancheAddress
 *
 * @param address   - a BRAvalancheAddress
 *
 * @return void
 */
extern void
avalancheAddressFree (BRAvalancheAddress address);

/**
 * Check is this address is the
 *
 * @param address   - a BRAvalancheAddress
 *
 * @return `true` if this is the "Fee" address, `false` if not
 */
extern bool
avalancheAddressIsFeeAddress (BRAvalancheAddress address);

extern bool
avalancheAddressIsUnknownAddress (BRAvalancheAddress address);

/**
 * Copy a BRAvalancheAddress
 *
 * @param address   - a BRAvalancheAddress
 *
 * @return copy     - an exact copy of the specified address
 */
extern BRAvalancheAddress
avalancheAddressClone (BRAvalancheAddress address);

/**
 * Get the size of the raw bytes for this address
 *
 * @param address   - a BRAvalancheAddress
 *
 * @return size of the raw bytes
 */
extern size_t
avalancheAddressGetRawSize (BRAvalancheAddress address);

/**
 * Get the raw bytes for this address
 *
 * @param address    - a BRAvalancheAddress
 * @param buffer     - a buffer to hold the raw bytes
 * @param bufferSize - size of the buffer, obtained via avalancheAddressGetRawSize
 *
 * @return void
 */
extern void
avalancheAddressGetRawBytes (BRAvalancheAddress address, uint8_t *buffer, size_t bufferSize);

/**
 * Compare 2 avalanche addresses
 *
 * @param a1  first address
 * @param a2  second address
 *
 * @return `true`  - if addresses are equal
 *         `false` - if not equal
 */
extern bool
avalancheAddressEqual (BRAvalancheAddress a1, BRAvalancheAddress a2);

extern size_t
avalancheAddressHashValue (BRAvalancheAddress address);

#ifdef __cplusplus
}
#endif

#endif /* BRAvalancheAddress_h */

