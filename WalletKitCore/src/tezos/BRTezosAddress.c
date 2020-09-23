//
//  BRTezosAddress.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRTezosAddress.h"
#include "BRTezosBase.h"
#include "support/BRCrypto.h"
#include "support/BRBase58.h"

#include "blake2/blake2b.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <stdbool.h>


#define BLAKE20_BYTES 20

const uint8_t TZ1_PREFIX[3] = { 6, 161, 159 };
const uint8_t TZ2_PREFIX[3] = { 6, 161, 161 };
const uint8_t TZ3_PREFIX[3] = { 6, 161, 164 };

static uint8_t feeAddressBytes[TEZOS_ADDRESS_BYTES] = {
    0x42, 0x52, 0x44, //BRD
    0x5F, 0x5F, // __
    'f', 'e', 'e', // fee
    0x5F, 0x5F, // __
    0x42, 0x52, 0x44, // BRD
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // padding
};

static uint8_t unknownAddressBytes[TEZOS_ADDRESS_BYTES] = {
    0x42, 0x52, 0x44, //BRD
    0x5F, 0x5F, // __
    'u', 'n', 'k', 'n', 'o', 'w', 'n', // unknown
    0x5F, 0x5F, // __
    0x42, 0x52, 0x44, // BRD
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // padding
};

struct BRTezosAddressRecord {
    uint8_t bytes[TEZOS_ADDRESS_BYTES];
};

extern void
tezosAddressFree (BRTezosAddress address) {
    if (address) free(address);
}

BRTezosAddress
tezosAddressCreateFeeAddress() {
    BRTezosAddress address = calloc(1, sizeof(struct BRTezosAddressRecord));
    memcpy(address->bytes, feeAddressBytes, TEZOS_ADDRESS_BYTES);
    return address;
}

BRTezosAddress
tezosAddressCreateUnknownAddress() {
    BRTezosAddress address = calloc(1, sizeof(struct BRTezosAddressRecord));
    memcpy(address->bytes, unknownAddressBytes, TEZOS_ADDRESS_BYTES);
    return address;
}

extern int
tezosAddressIsFeeAddress (BRTezosAddress address) {
    assert(address);
    if (memcmp(address->bytes, feeAddressBytes, sizeof(feeAddressBytes)) == 0) {
        return 1;
    } else {
        return 0;
    }
}

extern int
tezosAddressIsUnknownAddress (BRTezosAddress address)
{
    assert(address);
    if (memcmp(address->bytes, unknownAddressBytes, sizeof(unknownAddressBytes)) == 0) {
        return 1;
    } else {
        return 0;
    }
}

extern char *
tezosAddressAsString (BRTezosAddress address) {
    assert(address);
    char * string = NULL;
    
    if (tezosAddressIsFeeAddress (address)) {
        string = strdup ("__fee__");
    } else if (tezosAddressIsUnknownAddress (address)) {
        string = strdup ("unknown");
    } else {
        // address string is Base58check(prefix + Blake2b(publicKey) (20 bytes))
        size_t addressLen = BRBase58CheckEncode(NULL, 0, address->bytes, TEZOS_ADDRESS_BYTES);
        string = calloc (1, addressLen);
        BRBase58CheckEncode(string, addressLen, address->bytes, TEZOS_ADDRESS_BYTES);
    }
    return string;
}

extern BRTezosAddress
tezosAddressCreateFromKey (const uint8_t * pubKey, size_t pubKeyLen) {
    BRTezosAddress address = calloc(1, sizeof(struct BRTezosAddressRecord));
    
    uint8_t pkh[BLAKE20_BYTES];
    blake2b(pkh, sizeof(pkh), NULL, 0, pubKey, pubKeyLen);

    memcpy(address->bytes, TZ1_PREFIX, sizeof(TZ1_PREFIX));
    memcpy(address->bytes + sizeof(TZ1_PREFIX), pkh, sizeof(pkh));
    return address;
}

static BRTezosAddress
tezosAddressCreateFromBytes (uint8_t * bytes, size_t length) {
    assert(bytes);
    assert(length == TEZOS_ADDRESS_BYTES);
    BRTezosAddress address = calloc(1, sizeof(struct BRTezosAddressRecord));
    memcpy(address->bytes, bytes, length);
    return address;
}

static BRTezosAddress
tezosAddressStringToAddress(const char *input) {
    uint8_t bytes[TEZOS_ADDRESS_BYTES];
    
    size_t length = BRBase58CheckDecode(bytes, sizeof(bytes), input);
    if (length != TEZOS_ADDRESS_BYTES) {
        return NULL;
    }
    
    if (0 == memcmp(bytes, TZ1_PREFIX, sizeof(TZ1_PREFIX))
        || 0 == memcmp(bytes, TZ2_PREFIX, sizeof(TZ2_PREFIX))
        || 0 == memcmp(bytes, TZ3_PREFIX, sizeof(TZ3_PREFIX))) {
        return tezosAddressCreateFromBytes(bytes, length);
    }
    
    return NULL;
}

extern BRTezosAddress
tezosAddressCreateFromString(const char * addressString, bool strict) {
    
    if (addressString == NULL || strlen(addressString) == 0) {
        return (strict
                ? NULL
                : tezosAddressCreateUnknownAddress ());
    } else if (strict) {
        return tezosAddressStringToAddress (addressString);
    } else if (strcmp(addressString, "unknown") == 0) {
        return tezosAddressCreateUnknownAddress ();
    } else if (strcmp(addressString, "__fee__") == 0) {
        return tezosAddressCreateFeeAddress ();
    } else {
        return tezosAddressStringToAddress (addressString);
    }
}

extern int // 1 if equal
tezosAddressEqual (BRTezosAddress a1, BRTezosAddress a2) {
    return 0 == memcmp (a1->bytes, a2->bytes, TEZOS_ADDRESS_BYTES);
}

extern size_t
tezosAddressHashValue (BRTezosAddress address) {
    return *((size_t*) address->bytes);
}

extern BRTezosAddress
tezosAddressClone (BRTezosAddress address) {
    if (address) {
        BRTezosAddress clone = calloc(1, sizeof(struct BRTezosAddressRecord));
        *clone = *address;
        return clone;
    }
    return NULL;
}

extern size_t
tezosAddressGetRawSize (BRTezosAddress address) {
    return TEZOS_ADDRESS_BYTES;
}

extern void tezosAddressGetRawBytes (BRTezosAddress address, uint8_t *buffer, size_t bufferSize) {
    assert(buffer);
    assert(bufferSize >= TEZOS_ADDRESS_BYTES);
    memcpy(buffer, address->bytes, TEZOS_ADDRESS_BYTES);
}

extern bool
tezosAddressIsImplicit (BRTezosAddress address) {
    return (0 == memcmp(address->bytes, TZ1_PREFIX, sizeof(TZ1_PREFIX))
            || 0 == memcmp(address->bytes, TZ2_PREFIX, sizeof(TZ1_PREFIX))
            || 0 == memcmp(address->bytes, TZ3_PREFIX, sizeof(TZ1_PREFIX)));
}
