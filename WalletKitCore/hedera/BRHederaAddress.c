//
//  BRHederaAddress.c
//  Core
//
//  Created by Carl Cherry on Oct. 21, 2019.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRHederaAddress.h"
#include "BRHederaBase.h"
#include "support/BRCrypto.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <stdbool.h>

// An 'uninitialized address' has {shard, realm, account} as {0, 0, 0}
struct BRHederaAddressRecord {
    BRHederaAddressComponentType shard;
    BRHederaAddressComponentType realm;
    BRHederaAddressComponentType account;
};

extern void hederaAddressFree (BRHederaAddress address)
{
    if (address) free(address);
}

BRHederaAddress hederaAddressCreateFeeAddress()
{
    BRHederaAddress address = calloc(1, sizeof(struct BRHederaAddressRecord));
    address->shard  = -1;
    address->realm  = -1;
    address->account = -1;
    return address;
}

BRHederaAddress hederaAddressCreateUnknownAddress()
{
    BRHederaAddress address = calloc(1, sizeof(struct BRHederaAddressRecord));
    address->shard  = -1;
    address->realm  = -1;
    address->account = -2;
    return address;
}

extern char * hederaAddressAsString (BRHederaAddress address)
{
    assert(address);
    char * string = NULL;

    // Check for our special case __fee__ address
    // See the note above with respect to the feeAddressBytes
    if (address->account == -1) {
        string = strdup ("__fee__");
    }
    else if (address->account == -2) {
        string = strdup ("unknown");
    }
    else {
        // Hedera addresses are shown as a.b.c
        char buffer[1024];
        memset(buffer, 0x00, sizeof(buffer));
        size_t stringSize = (size_t) sprintf(buffer, "%" PRIi64 ".%" PRIi64  ".%" PRIi64, address->shard, address->realm, address->account);
        assert(stringSize > 0);
        string = strdup (buffer);
    }
    return string;
}

static bool hederaStringIsValid (const char * input)
{
    //const char * largestInt64Number = "9223372036854775807";
    int64_t shard = -1;
    int64_t realm = -1;
    int64_t account = -1;
    sscanf(input, "%" PRIi64 ".%" PRIi64  ".%" PRIi64, &shard, &realm, &account);
    if (shard >= 0 && realm >= 0 && account >= 0) {
        return true;
    }
    return false;
}

BRHederaAddress hederaAddressStringToAddress(const char* input)
{
    if (!hederaStringIsValid(input)) {
        return NULL;
    }

    // Hedera address are shard.realm.account
    BRHederaAddress address = (BRHederaAddress) calloc(1, sizeof(struct BRHederaAddressRecord));
    sscanf(input, "%" PRIi64 ".%" PRIi64  ".%" PRIi64,
           &address->shard, &address->realm, &address->account);
    return address;
}

extern BRHederaAddress
hederaAddressCreateFromString(const char * hederaAddressString, bool strict)
{
    // Handle an 'invalid' string argument.
    if (hederaAddressString == NULL || strlen(hederaAddressString) == 0) {
        return (strict
                ? NULL
                : hederaAddressCreateUnknownAddress ());
    }

    //  If strict, only accept 'r...' addresses
    else if (strict) {
        return hederaAddressStringToAddress (hederaAddressString);
    }

    // Handle an 'unknown' address
    else if (strcmp(hederaAddressString, "unknown") == 0) {
        return hederaAddressCreateUnknownAddress ();
    }

    // Handle a '__fee__' address
    else if (strcmp(hederaAddressString, "__fee__") == 0) {
        return hederaAddressCreateFeeAddress ();
    }

    // Handle an 'r...' address (in a non-strict mode).
    else {
        // Work backwards from this ripple address (string) to what is
        // known as the acount ID (20 bytes)
        return hederaAddressStringToAddress (hederaAddressString);
    }
}

extern BRHederaAddress
hederaAddressCreate(int64_t shard, int64_t realm, int64_t account_num)
{
    BRHederaAddress address = (BRHederaAddress) calloc(1, sizeof(struct BRHederaAddressRecord));
    address->shard = shard;
    address->realm = realm;
    address->account = account_num;
    return address;
}

extern int // 1 if equal
hederaAddressEqual (BRHederaAddress a1, BRHederaAddress a2) {
    return (a1->shard == a2->shard &&
            a1->realm == a2->realm &&
            a1->account == a2->account);
}

extern int
hederaAddressIsFeeAddress (BRHederaAddress address)
{
    assert(address);
    if (address->shard == -1 && address->realm == -1 && address->account == -1) {
        return 1;
    } else {
        return 0;
    }
}

extern int
hederaAddressIsUninitializedAddress (BRHederaAddress address) {
    return 0 == address->shard && 0 == address->realm && 0 == address->account;
}

extern BRHederaAddress hederaAddressClone (BRHederaAddress address)
{
    if (address) {
        BRHederaAddress clone = calloc(1, sizeof(struct BRHederaAddressRecord));
        *clone = *address;
        return clone;
    }
    return NULL;
}

BRHederaAddressComponentType hederaAddressGetShard (BRHederaAddress address)
{
    assert (address);
    return address->shard;
}

BRHederaAddressComponentType hederaAddressGetRealm (BRHederaAddress address)
{
    assert (address);
    return address->realm;
}

BRHederaAddressComponentType hederaAddressGetAccount (BRHederaAddress address)
{
    assert (address);
    return address->account;
}

void hederaAddressSerialize(BRHederaAddress address, uint8_t * buffer, size_t sizeOfBuffer)
{
    assert(sizeOfBuffer == HEDERA_ADDRESS_SERIALIZED_SIZE);

    // The Hedera account IDs are made up of 3 int64_t numbers
    // Get the account id values convert to network order
    BRHederaAddressComponentType shard   = (BRHederaAddressComponentType) htonll(hederaAddressGetShard(address));
    BRHederaAddressComponentType realm   = (BRHederaAddressComponentType) htonll(hederaAddressGetRealm(address));
    BRHederaAddressComponentType account = (BRHederaAddressComponentType) htonll(hederaAddressGetAccount(address));

    // Copy the values to the buffer
    size_t componentSize = sizeof (BRHederaAddressComponentType);

    memcpy(buffer, &shard, componentSize);
    memcpy(buffer + componentSize, &realm, componentSize);
    memcpy(buffer + (2 * componentSize), &account, componentSize);
}

