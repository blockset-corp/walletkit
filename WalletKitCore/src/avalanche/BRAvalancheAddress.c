//
//  BRAvalancheAddress.c
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRAvalancheAddress.h"
#include "BRAvalancheBase.h"

#include "support/BRCrypto.h"
#include "support/util/BRHex.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <stdbool.h>

//
#define AVALANCHE_ADDRESS_BYTES     (AVALANCHE_ADDRESS_BYTES_X > AVALANCHE_ADDRESS_BYTES_C ? AVALANCHE_ADDRESS_BYTES_X : AVALANCHE_ADDRESS_BYTES_C)


static bool
avalancheAddressHasBytes (const BRAvalancheAddress *address, const uint8_t *bytes) {
    switch (address->type) {
        case AVALANCHE_CHAIN_TYPE_X:
            return 0 == memcmp (address->u.x.bytes, bytes, AVALANCHE_ADDRESS_BYTES_X);

        case AVALANCHE_CHAIN_TYPE_C:
            return 0 == memcmp (address->u.c.bytes, bytes, AVALANCHE_ADDRESS_BYTES_C);

        case AVALANCHE_CHAIN_TYPE_P:
            return false;
    }
}

static BRAvalancheAddress
avalancheAddressCreateWithBytes (BRAvalancheChainType type, const uint8_t *bytes) {
    BRAvalancheAddress address = { type };

    switch (type) {
        case AVALANCHE_CHAIN_TYPE_X:
            memcpy (address.u.x.bytes, bytes, AVALANCHE_ADDRESS_BYTES_X);
            break;

        case AVALANCHE_CHAIN_TYPE_C:
            memcpy (address.u.c.bytes, bytes, AVALANCHE_ADDRESS_BYTES_C);
            break;

        case AVALANCHE_CHAIN_TYPE_P:
            assert (false);
            break;
    }

    return address;
}

// MARK: Fee Address

static const uint8_t feeAddressBytes [AVALANCHE_ADDRESS_BYTES] = {
    '_', '_', 'f', 'e',   'e', '_', '_', '\0', 0, 0,
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0
};

extern BRAvalancheAddress
avalancheAddressCreateFeeAddress(BRAvalancheChainType type) {
    return avalancheAddressCreateWithBytes(type, feeAddressBytes);
}

extern bool
avalancheAddressIsFeeAddress (BRAvalancheAddress address) {
    return avalancheAddressHasBytes (&address, feeAddressBytes);
}

// MARK: - Unknown Address

static const uint8_t unknownAddressBytes [AVALANCHE_ADDRESS_BYTES] = {
    'u', 'n', 'k', 'n',   'o', 'w', 'n', '\0', 0, 0,
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0
};

extern BRAvalancheAddress
avalancheAddressCreateUnknownAddress(BRAvalancheChainType type) {
    return avalancheAddressCreateWithBytes (type, unknownAddressBytes);
}

extern bool
avalancheAddressIsUnknownAddress (BRAvalancheAddress address) {
    return avalancheAddressHasBytes (&address, unknownAddressBytes);
}

// MARK: - Empty Address

static const uint8_t emptyAddressBytes [AVALANCHE_ADDRESS_BYTES] = {
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0,
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0
};

extern bool
avalancheAddressIsEmptyAddress (BRAvalancheAddress address) {
    return avalancheAddressHasBytes (&address, emptyAddressBytes);
 }

extern BRAvalancheAddress
avalancheAddressCreateEmptyAddress(BRAvalancheChainType type) {
    return avalancheAddressCreateWithBytes(type, emptyAddressBytes);
}

// MARK: - Address As String

extern char *
avalancheAddressAsStringX (BRAvalancheAddress address, const char *hrp) {
    assert (AVALANCHE_CHAIN_TYPE_X == address.type);

    const size_t hrpLen = strlen (hrp);

    UInt256 pkh;
    size_t pkhLen = sizeof(UInt256);

    char *result = malloc (44);
    size_t resultLen = 44;

    avax_base32_encode (pkh.u8, &pkhLen, address.u.x.bytes, sizeof(address.u.x.bytes));
    avax_bech32_encode (result, &resultLen, hrp, hrpLen, pkh.u8, pkhLen);

    return result;
}

extern char *
avalancheAddressAsStringC (BRAvalancheAddress address) {
    assert (AVALANCHE_CHAIN_TYPE_C == address.type);
    return hexEncodeCreate (NULL, address.u.c.bytes, AVALANCHE_ADDRESS_BYTES_C);
}

// MARK: - Create From Key

static BRAvalancheAddress
avalancheAddressCreateFromKeyX (const uint8_t * pubKey, size_t pubKeyLen) {
    assert (33 == pubKeyLen);

    BRAvalancheAddress address = { AVALANCHE_CHAIN_TYPE_X };

    // Take the SHA256 of the `pubKey`
    UInt256 pubKeySha256;
    BRSHA256 (pubKeySha256.u8, pubKey, pubKeyLen);

    // The X address is the 20 bytes of the RMD160
    assert (20 == AVALANCHE_ADDRESS_BYTES_X);
    BRRMD160 (address.u.x.bytes, pubKeySha256.u8, sizeof (UInt256));

    return address;
}

static BRAvalancheAddress
avalancheAddressCreateFromKeyC (const uint8_t * pubKey, size_t pubKeyLen) {
    assert (65 == pubKeyLen);
    assert (32 >= AVALANCHE_ADDRESS_BYTES_C);

    BRAvalancheAddress address = { AVALANCHE_CHAIN_TYPE_C };

    UInt256 hash;
    BRKeccak256(hash.u8, &pubKey[1], pubKeyLen - 1);

    memcpy (address.u.c.bytes, &hash.u8[32 - AVALANCHE_ADDRESS_BYTES_C], AVALANCHE_ADDRESS_BYTES_C);

    return address;
}

extern BRAvalancheAddress
avalancheAddressCreateFromKey (const uint8_t * pubKey, size_t pubKeyLen, BRAvalancheChainType type) {
    switch (type) {
        case AVALANCHE_CHAIN_TYPE_X:
            return avalancheAddressCreateFromKeyX (pubKey, pubKeyLen);

        case AVALANCHE_CHAIN_TYPE_C:
            return avalancheAddressCreateFromKeyC (pubKey, pubKeyLen);

        case AVALANCHE_CHAIN_TYPE_P:
            assert (false);
            return ((BRAvalancheAddress) { (BRAvalancheChainType) -1 });
    }
}

// MARK: - Create From Bytes

#if 0
static BRAvalancheAddress
avalancheAddressCreateFromBytes (uint8_t * bytes, size_t length) {
    assert(bytes);
    assert(length == AVALANCHE_ADDRESS_BYTES);

    ASSERT_UNIMPLEMENTED;
    BRAvalancheAddress address = calloc(1, sizeof(struct BRAvalancheAddressRecord));
    memcpy(address->bytes, bytes, length);
    return address;
}
#endif

// MARK: - String to Address

extern BRAvalancheAddress
avalancheAddressStringToAddressX (const char *input,
                                  const char *prefix) {
    BRAvalancheAddress address = { AVALANCHE_CHAIN_TYPE_X };

    size_t addressBytesCount = AVALANCHE_ADDRESS_BYTES_X;

    bool success = avax_addr_bech32_decode(address.u.x.bytes, &addressBytesCount, prefix, input);

    return (success ? address : avalancheAddressCreateEmptyAddress (AVALANCHE_ADDRESS_BYTES_X));
}

extern BRAvalancheAddress
avalancheAddressStringToAddressC (const char *input) {
    assert (40 == strlen(input));

    BRAvalancheAddress address = { AVALANCHE_CHAIN_TYPE_C };
    hexDecode (address.u.c.bytes, AVALANCHE_ADDRESS_BYTES_C, input, strlen(input));
    return address;
}

extern bool
avalancheAddressEqual (BRAvalancheAddress a1, BRAvalancheAddress a2) {
    return (a1.type == a2.type &&
            (AVALANCHE_CHAIN_TYPE_X == a1.type
             ? 0 == memcmp (a1.u.x.bytes, a2.u.x.bytes, AVALANCHE_ADDRESS_BYTES_X)
             : 0 == memcmp (a1.u.c.bytes, a2.u.c.bytes, AVALANCHE_ADDRESS_BYTES_C)));
}

extern size_t
avalancheAddressHashValue (BRAvalancheAddress address) {
    return (AVALANCHE_CHAIN_TYPE_X == address.type
            ? *((size_t*) address.u.x.bytes)
            : *((size_t*) address.u.c.bytes));
}

