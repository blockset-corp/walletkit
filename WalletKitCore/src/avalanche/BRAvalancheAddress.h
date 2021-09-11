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

#include "BRAvalancheBase.h"

#ifdef __cplusplus
extern "C" {
#endif

// Avalanche defines two address formats: X and C
#define AVALANCHE_ADDRESS_BYTES_X   (20)
#define AVALANCHE_ADDRESS_BYTES_C   (20)

// An Avalance X address is based on RMD16 (SHA256 (pubkey)
typedef struct { uint8_t bytes [AVALANCHE_ADDRESS_BYTES_X]; } BRAvalancheAddressX;

// An Avalanche C address is identical to an Ethereum address
typedef struct { uint8_t bytes [AVALANCHE_ADDRESS_BYTES_C]; } BRAvalancheAddressC;

///
/// A BRAvalanceAddress represents an avalance address as an array of bytes, typically 20 bytes.
/// The string representation depends on BRAvalanceNetwork.  Any given address, of 20 bytes
/// typically, applies to any network.  The address itself does not maintain a reference to the
/// network; that reference must be kept elsewhere
///
typedef struct {
    BRAvalancheChainType type;
    union {
        BRAvalancheAddressX x;
        BRAvalancheAddressC c;
    } u;
} BRAvalancheAddress;

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
avalancheAddressCreateFromKey (const uint8_t * pubKey, size_t pubKeyLen, BRAvalancheChainType type);

/**
 * Create a avalanche address from a valid avalanche manager address string
 *
 * @param address   - avalanche address string in the "tz1..." format
 *
 * @return address  - a BRAvalancheAddress object
 */
extern BRAvalancheAddress
avalancheAddressCreateFromString(const char * avalancheAddressString, bool strict, BRAvalancheChainType type);


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

extern bool
avalancheAddressIsEmptyAddress (BRAvalancheAddress);

#if 0/**
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
#endif

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

