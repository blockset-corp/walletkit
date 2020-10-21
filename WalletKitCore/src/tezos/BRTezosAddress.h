//
//  BRTezosAddress.h
//  Core
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRTezosAddress_h
#define BRTezosAddress_h

#include "support/BRKey.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// prefix (3 bytes) + pkh (20 bytes)
#define TEZOS_ADDRESS_BYTES (23)

// address prefixes
extern const uint8_t TZ1_PREFIX[3];
extern const uint8_t TZ2_PREFIX[3];
extern const uint8_t TZ3_PREFIX[3];


typedef struct BRTezosAddressRecord *BRTezosAddress;

/**
 * Get the tezos address string representation of the address.
 * Caller must free using the "free" function.
 *
 * @param address   - a BRTezosAddress
 *
 * @return pointer to allocated buffer holding the null terminated string
 */
extern char *
tezosAddressAsString (BRTezosAddress address);

/**
 * Create a tezos address from a valid public key.
 * Caller must free using the "free" function.
 *
 * @param pubKey    - a tezos public key
 * @param pubKeyLen - the size (bytes) of the public key
 *
 * @return address  - a BRTezosAddress object
 */
extern BRTezosAddress
tezosAddressCreateFromKey (const uint8_t * pubKey, size_t pubKeyLen);

/**
 * Create a tezos address from a valid tezos manager address string
 *
 * @param address   - tezos address string in the "tz1..." format
 *
 * @return address  - a BRTezosAddress object
 */
extern BRTezosAddress
tezosAddressCreateFromString(const char * tezosAddressString, bool strict);

/**
 * Free the memory associated with a BRTezosAddress
 *
 * @param address   - a BRTezosAddress
 *
 * @return void
 */
extern void
tezosAddressFree (BRTezosAddress address);

/**
 * Check is this address is the
 *
 * @param address   - a BRTezosAddress
 *
 * @return 1 if this is the "Fee" address, 0 if not
 */
extern int
tezosAddressIsFeeAddress (BRTezosAddress address);

extern int
tezosAddressIsUnknownAddress (BRTezosAddress address);

/**
 * Copy a BRTezosAddress
 *
 * @param address   - a BRTezosAddress
 *
 * @return copy     - an exact copy of the specified address
 */
extern BRTezosAddress
tezosAddressClone (BRTezosAddress address);

/**
 * Get the size of the raw bytes for this address
 *
 * @param address   - a BRTezosAddress
 *
 * @return size of the raw bytes
 */
extern size_t
tezosAddressGetRawSize (BRTezosAddress address);

/**
 * Get the raw bytes for this address
 *
 * @param address    - a BRTezosAddress
 * @param buffer     - a buffer to hold the raw bytes
 * @param bufferSize - size of the buffer, obtained via tezosAddressGetRawSize
 *
 * @return void
 */
extern void
tezosAddressGetRawBytes (BRTezosAddress address, uint8_t *buffer, size_t bufferSize);

/**
 * Compare 2 tezos addresses
 *
 * @param a1  first address
 * @param a2  second address
 *
 * @return 1 - if addresses are equal
 *         0 - if not equal
 */
extern int // 1 if equal
tezosAddressEqual (BRTezosAddress a1, BRTezosAddress a2);

extern size_t
tezosAddressHashValue (BRTezosAddress address);

extern bool
tezosAddressIsImplicit (BRTezosAddress address);

 
#ifdef __cplusplus
}
#endif

#endif /* BRTezosAddress_h */

