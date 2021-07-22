//
//  BRCryptoHash.c
//  Core
//
//  Created by Ed Gamble on 5/15/19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRCryptoHashP.h"
#include "BRCryptoBaseP.h"
#include "BRCryptoNetworkP.h"

#include "support/util/BRHex.h"

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoHash, cryptoHash)

extern BRCryptoHash
cryptoHashCreateInternal (uint32_t setValue,
                          size_t   bytesCount,
                          uint8_t *bytes,
                          BRCryptoBlockChainType type) {
    assert (bytesCount <= CRYPTO_HASH_BYTES);
    BRCryptoHash hash = calloc (1, sizeof (struct BRCryptoHashRecord));

    hash->setValue   = setValue;
    hash->bytesCount = bytesCount;
    memcpy (hash->bytes, bytes, bytesCount);
    
    hash->type = type;
    hash->ref  = CRYPTO_REF_ASSIGN (cryptoHashRelease);

    return hash;
}

static void
cryptoHashRelease (BRCryptoHash hash) {
    memset (hash, 0, sizeof(*hash));
    free (hash);
}

extern BRCryptoBoolean
cryptoHashEqual (BRCryptoHash h1, BRCryptoHash h2) {
    return AS_CRYPTO_BOOLEAN (h1 == h2 ||
                              (NULL != h1     && NULL != h2     &&
                               h1->setValue   == h2->setValue   &&
                               h1->bytesCount == h2->bytesCount &&
                               0 == memcmp (h1->bytes, h2->bytes, h1->bytesCount)));
}

extern OwnershipGiven char *
cryptoHashEncodeString (BRCryptoHash hash) {
    return cryptoNetworkEncodeHash (hash);
}

static char *
_cryptoHashAddPrefix (char *hash, int freeHash) {
    char *result = malloc (2 + strlen (hash) + 1);
    strcpy (result, "0x");
    strcat (result, hash);
    if (freeHash) free (hash);
    return result;
}

private_extern OwnershipGiven char *
cryptoHashStringAsHex (BRCryptoHash hash, bool includePrefix) {
    size_t stringLength = 2 * hash->bytesCount + 1;
    char string [stringLength];

    hexEncode (string, stringLength, hash->bytes, hash->bytesCount);
    return (includePrefix ? _cryptoHashAddPrefix (string, 0) : strdup (string));
}

extern int
cryptoHashGetHashValue (BRCryptoHash hash) {
    return (int) hash->setValue;
}
