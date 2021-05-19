//
//  WKHash.c
//  WalletKitCore
//
//  Created by Ed Gamble on 5/15/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "WKHashP.h"
#include "WKBaseP.h"
#include "WKNetworkP.h"

#include "support/util/BRHex.h"

IMPLEMENT_WK_GIVE_TAKE (WKHash, wkHash)

extern WKHash
wkHashCreateInternal (uint32_t setValue,
                      size_t   bytesCount,
                      uint8_t *bytes,
                      WKNetworkType type) {
    assert (bytesCount <= WK_HASH_BYTES);
    WKHash hash = calloc (1, sizeof (struct WKHashRecord));

    hash->setValue   = setValue;
    hash->bytesCount = bytesCount;
    memcpy (hash->bytes, bytes, bytesCount);
    
    hash->type = type;
    hash->ref  = WK_REF_ASSIGN (wkHashRelease);

    return hash;
}

static void
wkHashRelease (WKHash hash) {
    assert (NULL != hash);
    memset (hash, 0, sizeof(*hash));
    free (hash);
}

extern WKBoolean
wkHashEqual (WKHash h1, WKHash h2) {
    return AS_WK_BOOLEAN (h1 == h2 ||
                          (NULL != h1     && NULL != h2     &&
                           h1->setValue   == h2->setValue   &&
                           h1->bytesCount == h2->bytesCount &&
                           0 == memcmp (h1->bytes, h2->bytes, h1->bytesCount)));
}

extern OwnershipGiven char *
wkHashEncodeString (WKHash hash) {
    return (NULL == hash ? NULL : wkNetworkEncodeHash (hash));
}

static char *
_wkHashAddPrefix (char *hash, int freeHash) {
    char *result = malloc (2 + strlen (hash) + 1);
    strcpy (result, "0x");
    strcat (result, hash);
    if (freeHash) free (hash);
    return result;
}

private_extern OwnershipGiven char *
wkHashStringAsHex (WKHash hash, bool includePrefix) {
    if (NULL == hash) return NULL;
    
    size_t stringLength = 2 * hash->bytesCount + 1;
    char string [stringLength];

    hexEncode (string, stringLength, hash->bytes, hash->bytesCount);
    return (includePrefix ? _wkHashAddPrefix (string, 0) : strdup (string));
}

extern int
wkHashGetHashValue (WKHash hash) {
    assert (NULL != hash);
    return (int) hash->setValue;
}
