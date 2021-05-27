//
//  BRCryptoAddress.c
//  BRCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoAddressP.h"
#include "BRCryptoNetworkP.h"

#include "BRCryptoHandlersP.h"

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoAddress, cryptoAddress);

private_extern BRCryptoAddress
cryptoAddressAllocAndInit (size_t sizeInBytes,
                           BRCryptoBlockChainType type,
                           size_t hashValue,
                           BRCryptoAddressCreateContext  createContext,
                           BRCryptoAddressCreateCallback createCallback) {
    assert (sizeInBytes >= sizeof (struct BRCryptoAddressRecord));

    BRCryptoAddress address = calloc (1, sizeInBytes);

    address->type = type;
    address->handlers = cryptoHandlersLookup(type)->address;
    address->ref = CRYPTO_REF_ASSIGN(cryptoAddressRelease);
    address->sizeInBytes = sizeInBytes;

    address->hashValue = hashValue;

    if (NULL != createCallback) createCallback (createContext, address);

    return address;
}

static void
cryptoAddressRelease (BRCryptoAddress address) {
    address->handlers->release (address);
    memset (address, 0, address->sizeInBytes);
    free (address);
}


private_extern BRCryptoBlockChainType
cryptoAddressGetType (BRCryptoAddress address) {
    return address->type;
}

private_extern size_t
cryptoAddressGetHashValue (BRCryptoAddress address) {
    return address->hashValue;;
}

private_extern BRSetOf(BRCryptoAddress)
cryptoAddressSetCreate (size_t count) {
    return BRSetNew ((size_t (*) (const void *)) cryptoAddressGetHashValue,
                     (int (*) (const void *, const void *))cryptoAddressIsEqual,
                     count);
}

private_extern void
cryptoAddressSetRelease (BRSetOf(BRCryptoAddress) addresses) {
    BRSetFreeAll (addresses,
                  (void (*) (void *)) cryptoAddressGive);
}

extern char *
cryptoAddressAsString (BRCryptoAddress address) {
    return address->handlers->asString (address);
}

extern BRCryptoBoolean
cryptoAddressIsIdentical (BRCryptoAddress a1,
                          BRCryptoAddress a2) {
    return AS_CRYPTO_BOOLEAN(cryptoAddressIsEqual(a1, a2));
}

private_extern int
cryptoAddressIsEqual (BRCryptoAddress a1,
                      BRCryptoAddress a2) {
    return (a1 == a2 ||
            (a1->type == a2->type &&
             a1->handlers->isEqual (a1, a2)));
}
