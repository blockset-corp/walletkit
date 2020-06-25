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

uint8_t tezosFeeAddressBytes[TEZOS_ADDRESS_BYTES] = {
    0x42, 0x52, 0x44, //BRD
    0x5F, 0x5F, // __
    'f', 'e', 'e', // fee
    0x5F, 0x5F, // __
    0x42, 0x52, 0x44, // BRD
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // padding
};

struct BRTezosAddressRecord {
    uint8_t bytes[TEZOS_ADDRESS_BYTES];
};

extern void
tezosAddressFree (BRTezosAddress address) {
    if (address) free(address);
}

BRTezosAddress
tezosAddressCreateFeeAddress()
{
    BRTezosAddress address = calloc(1, sizeof(struct BRTezosAddressRecord));
    memcpy(address->bytes, tezosFeeAddressBytes, TEZOS_ADDRESS_BYTES);
    return address;
}

extern int
tezosAddressIsFeeAddress (BRTezosAddress address)
{
    assert(address);
    if (memcmp(address->bytes, tezosFeeAddressBytes, sizeof(tezosFeeAddressBytes)) == 0) {
        return 1;
    } else {
        return 0;
    }
}

// address string is Base58check(prefix + Blake2b(publicKey) (20 bytes))
extern char *
tezosAddressAsString (BRTezosAddress address)
{
    assert(address);
    
    size_t addressLen = BRBase58CheckEncode(NULL, 0, address->bytes, TEZOS_ADDRESS_BYTES);
    
    char *string = calloc (1, addressLen);
    BRBase58CheckEncode(string, addressLen, address->bytes, TEZOS_ADDRESS_BYTES);
    
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
    
    if (0 == memcmp(bytes, TZ1_PREFIX, sizeof(TZ1_PREFIX))) {
        return tezosAddressCreateFromBytes(bytes, length);
    }
    
    return NULL;
}

extern BRTezosAddress
tezosAddressCreateFromString(const char * addressString, bool strict) {
    
    if (addressString == NULL || strlen(addressString) == 0) {
        return NULL;
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

extern BRTezosAddress
tezosAddressClone (BRTezosAddress address)
{
    if (address) {
        BRTezosAddress clone = calloc(1, sizeof(struct BRTezosAddressRecord));
        *clone = *address;
        return clone;
    }
    return NULL;
}
