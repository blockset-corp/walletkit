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


#ifdef __cplusplus
}
#endif

#endif /* BRTezosAddress_h */

