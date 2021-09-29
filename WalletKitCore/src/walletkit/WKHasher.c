//
//  WKHasher.c
//  WalletKitCore
//
//  Created by Michael Carrara on 9/23/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <assert.h>
#include <stdlib.h>

#include "WKHasher.h"
#include "support/BRCrypto.h"

struct WKHasherRecord {
    WKHasherType type;
    WKRef ref;
};

IMPLEMENT_WK_GIVE_TAKE (WKHasher, wkHasher);

extern WKHasher
wkHasherCreate(WKHasherType type) {
    WKHasher hasher = NULL;

    switch (type) {
        case WK_HASHER_SHA1:
        case WK_HASHER_SHA224:
        case WK_HASHER_SHA256:
        case WK_HASHER_SHA256_2:
        case WK_HASHER_SHA384:
        case WK_HASHER_SHA512:
        case WK_HASHER_SHA3:
        case WK_HASHER_RMD160:
        case WK_HASHER_HASH160:
        case WK_HASHER_KECCAK256:
        case WK_HASHER_MD5: {
            hasher = calloc (1, sizeof(struct WKHasherRecord));
            hasher->type = type;
            hasher->ref = WK_REF_ASSIGN(wkHasherRelease);
            break;
        }
        default: {
            // for an unsupported algorithm, assert
            assert (0);
            break;
        }
    }

    return hasher;
}

static void
wkHasherRelease (WKHasher hasher) {
    memset (hasher, 0, sizeof(*hasher));
    free (hasher);
}

extern size_t
wkHasherLength (WKHasher hasher) {
    size_t length = 0;

    switch (hasher->type) {
        case WK_HASHER_SHA1: {
            length = 20;
            break;
        }
        case WK_HASHER_SHA224: {
            length = 28;
            break;
        }
        case WK_HASHER_SHA256: {
            length = 32;
            break;
        }
        case WK_HASHER_SHA256_2: {
            length = 32;
            break;
        }
        case WK_HASHER_SHA384: {
            length = 48;
            break;
        }
        case WK_HASHER_SHA512: {
            length = 64;
            break;
        }
        case WK_HASHER_SHA3: {
            length = 32;
            break;
        }
        case WK_HASHER_RMD160: {
            length = 20;
            break;
        }
        case WK_HASHER_HASH160: {
            length = 20;
            break;
        }
        case WK_HASHER_KECCAK256: {
            length = 32;
            break;
        }
        case WK_HASHER_MD5: {
            length = 16;
            break;
        }
        default: {
            // for an unsupported algorithm, assert
            assert (0);
            break;
        }
    }

    return length;
}

extern WKBoolean
wkHasherHash (WKHasher hasher,
                  uint8_t *dst,
                  size_t dstLen,
                  const uint8_t *src,
                  size_t srcLen) {
    // - src CAN be NULL, if srcLen is 0
    // - dst MUST be non-NULL and sufficiently sized
    if ((NULL == src && 0 != srcLen) ||
        NULL == dst || dstLen < wkHasherLength (hasher)) {
        assert (0);
        return WK_FALSE;
    }

    // the hash routines don't return anything so assume success
    // and only treat the unhandled case as failure
    WKBoolean result = WK_TRUE;

    switch (hasher->type) {
        case WK_HASHER_SHA1: {
            BRSHA1 (dst, src, srcLen);
            break;
        }
        case WK_HASHER_SHA224: {
            BRSHA224 (dst, src, srcLen);
            break;
        }
        case WK_HASHER_SHA256: {
            BRSHA256 (dst, src, srcLen);
            break;
        }
        case WK_HASHER_SHA256_2: {
            BRSHA256_2 (dst, src, srcLen);
            break;
        }
        case WK_HASHER_SHA384: {
            BRSHA384 (dst, src, srcLen);
            break;
        }
        case WK_HASHER_SHA512: {
            BRSHA512 (dst, src, srcLen);
            break;
        }
        case WK_HASHER_SHA3: {
            BRSHA3_256 (dst, src, srcLen);
            break;
        }
        case WK_HASHER_RMD160: {
            BRRMD160 (dst, src, srcLen);
            break;
        }
        case WK_HASHER_HASH160: {
            BRHash160 (dst, src, srcLen);
            break;
        }
        case WK_HASHER_KECCAK256: {
            BRKeccak256 (dst, src, srcLen);
            break;
        }
        case WK_HASHER_MD5: {
            BRMD5 (dst, src, srcLen);
            break;
        }
        default: {
            // for an unsupported algorithm, assert
            assert (0);
            result = WK_FALSE;
            break;
        }
    }

    return result;
}
