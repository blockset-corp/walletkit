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
#include "support/BRBase58.h"

#include "blake2/blake2b.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <stdbool.h>

//
#define AVALANCHE_ADDRESS_BYTES (1)


static uint8_t feeAddressBytes     [AVALANCHE_ADDRESS_BYTES] = { 0 };
static uint8_t unknownAddressBytes [AVALANCHE_ADDRESS_BYTES] = { 0 };


struct BRAvalancheAddressRecord {
    uint8_t bytes[AVALANCHE_ADDRESS_BYTES];
};

extern void
avalancheAddressFree (BRAvalancheAddress address) {
    if (address) free(address);
}

BRAvalancheAddress
avalancheAddressCreateFeeAddress() {
    BRAvalancheAddress address = calloc(1, sizeof(struct BRAvalancheAddressRecord));
    memcpy(address->bytes, feeAddressBytes, AVALANCHE_ADDRESS_BYTES);
    return address;
}

BRAvalancheAddress
avalancheAddressCreateUnknownAddress() {
    BRAvalancheAddress address = calloc(1, sizeof(struct BRAvalancheAddressRecord));
    memcpy(address->bytes, unknownAddressBytes, AVALANCHE_ADDRESS_BYTES);
    return address;
}

extern bool
avalancheAddressIsFeeAddress (BRAvalancheAddress address) {
    assert(address);
    return 0 == memcmp(address->bytes, feeAddressBytes, sizeof(feeAddressBytes));
}

extern bool
avalancheAddressIsUnknownAddress (BRAvalancheAddress address)
{
    assert(address);
    return  0 == memcmp(address->bytes, unknownAddressBytes, sizeof(unknownAddressBytes));
}

extern char *
avalancheAddressAsString (BRAvalancheAddress address) {
    assert(address);
    char * string = NULL;
    
    if (avalancheAddressIsFeeAddress (address)) {
        string = strdup ("__fee__");
    } else if (avalancheAddressIsUnknownAddress (address)) {
        string = strdup ("unknown");
    } else {
        ASSERT_UNIMPLEMENTED;
#if 0
        // address string is Base58check(prefix + Blake2b(publicKey) (20 bytes))
        size_t addressLen = BRBase58CheckEncode(NULL, 0, address->bytes, AVALANCHE_ADDRESS_BYTES);
        string = calloc (1, addressLen);
        BRBase58CheckEncode(string, addressLen, address->bytes, AVALANCHE_ADDRESS_BYTES);
#endif
    }
    return string;
}

extern BRAvalancheAddress
avalancheAddressCreateFromKey (const uint8_t * pubKey, size_t pubKeyLen) {
    BRAvalancheAddress address = calloc(1, sizeof(struct BRAvalancheAddressRecord));

    ASSERT_UNIMPLEMENTED;
#if 0
    uint8_t pkh[BLAKE20_BYTES];
    blake2b(pkh, sizeof(pkh), NULL, 0, pubKey, pubKeyLen);

    memcpy(address->bytes, AVAXTZ1_PREFIX, sizeof(AVAXTZ1_PREFIX));
    memcpy(address->bytes + sizeof(AVAXTZ1_PREFIX), pkh, sizeof(pkh));
#endif
    return address;
}

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

static BRAvalancheAddress
avalancheAddressStringToAddress(const char *input) {

    ASSERT_UNIMPLEMENTED;

#if 0
    uint8_t bytes[AVALANCHE_ADDRESS_BYTES];

    size_t length = BRBase58CheckDecode(bytes, sizeof(bytes), input);
    if (length != AVALANCHE_ADDRESS_BYTES) {
        return NULL;
    }
    
    if (0 == memcmp(bytes, AVAXTZ1_PREFIX, sizeof(AVAXTZ1_PREFIX))
        || 0 == memcmp(bytes, AVAXTZ2_PREFIX, sizeof(AVAXTZ2_PREFIX))
        || 0 == memcmp(bytes, AVAXTZ3_PREFIX, sizeof(AVAXTZ3_PREFIX))
        || 0 == memcmp(bytes, AVAXKT_PREFIX, sizeof(AVAXKT_PREFIX))) {
        return avalancheAddressCreateFromBytes(bytes, length);
    }
#endif
    return NULL;
}

extern BRAvalancheAddress
avalancheAddressCreateFromString(const char * addressString, bool strict) {
    
    if (addressString == NULL || strlen(addressString) == 0) {
        return (strict
                ? NULL
                : avalancheAddressCreateUnknownAddress ());
    } else if (strict) {
        return avalancheAddressStringToAddress (addressString);
    } else if (strcmp(addressString, "unknown") == 0) {
        return avalancheAddressCreateUnknownAddress ();
    } else if (strcmp(addressString, "__fee__") == 0) {
        return avalancheAddressCreateFeeAddress ();
    } else {
        return avalancheAddressStringToAddress (addressString);
    }
}

extern bool
avalancheAddressEqual (BRAvalancheAddress a1, BRAvalancheAddress a2) {
    return 0 == memcmp (a1->bytes, a2->bytes, AVALANCHE_ADDRESS_BYTES);
}

extern size_t
avalancheAddressHashValue (BRAvalancheAddress address) {
    return *((size_t*) address->bytes);
}

extern BRAvalancheAddress
avalancheAddressClone (BRAvalancheAddress address) {
    if (address) {
        BRAvalancheAddress clone = calloc(1, sizeof(struct BRAvalancheAddressRecord));
        *clone = *address;
        return clone;
    }
    return NULL;
}

extern size_t
avalancheAddressGetRawSize (BRAvalancheAddress address) {
    return AVALANCHE_ADDRESS_BYTES;
}

extern void avalancheAddressGetRawBytes (BRAvalancheAddress address, uint8_t *buffer, size_t bufferSize) {
    assert(buffer);
    assert(bufferSize >= AVALANCHE_ADDRESS_BYTES);
    memcpy(buffer, address->bytes, AVALANCHE_ADDRESS_BYTES);
}
