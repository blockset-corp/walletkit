//
//  WKCoder.c
//  WalletKitCore
//
//  Created by Michael Carrara on 9/23/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "WKCoder.h"
#include "support/util/BRHex.h"
#include "support/BRBase58.h"
#include "ripple/BRRippleBase.h"

struct WKCoderRecord {
    WKCoderType type;
    WKRef ref;
};

IMPLEMENT_WK_GIVE_TAKE (WKCoder, wkCoder);

extern WKCoder
wkCoderCreate(WKCoderType type) {
    WKCoder coder = NULL;

    switch (type) {
        case WK_CODER_HEX:
        case WK_CODER_BASE58:
        case WK_CODER_BASE58CHECK:
        case WK_CODER_BASE58RIPPLE: {
            coder = calloc (1, sizeof(struct WKCoderRecord));
            coder->type = type;
            coder->ref = WK_REF_ASSIGN(wkCoderRelease);
            break;
        }
        default: {
            assert (0);
            break;
        }
    }

    return coder;
}

static void
wkCoderRelease (WKCoder coder) {
    memset (coder, 0, sizeof(*coder));
    free (coder);
}

extern size_t
wkCoderEncodeLength (WKCoder coder,
                         const uint8_t *src,
                         size_t srcLen) {
    // - src CANNOT be NULL (see: BRBase58Encode), even with srcLen of 0
    if (NULL == src) {
        assert (0);
        return 0;
    }

    size_t length = 0;

    switch (coder->type) {
        case WK_CODER_HEX: {
            length = hexEncodeLength (srcLen);
            break;
        }
        case WK_CODER_BASE58: {
            length = BRBase58Encode (NULL, 0, src, srcLen);
            break;
        }
        case WK_CODER_BASE58CHECK: {
            length = BRBase58CheckEncode (NULL, 0, src, srcLen);
            break;
        }
        case WK_CODER_BASE58RIPPLE: {
            length = BRBase58EncodeEx (NULL, 0, src, srcLen, rippleAlphabet);
            break;
        }
        default: {
            assert (0);
            break;
        }
    }

    return length;
}

extern WKBoolean
wkCoderEncode (WKCoder coder,
                   char *dst,
                   size_t dstLen,
                   const uint8_t *src,
                   size_t srcLen) {
    // - src CANNOT be NULL (see: BRBase58Encode), even with srcLen of 0
    // - dst MUST be non-NULL and sufficiently sized
    if (NULL == src ||
        NULL == dst || dstLen < wkCoderEncodeLength (coder, src, srcLen)) {
        assert (0);
        return WK_FALSE;
    }

    WKBoolean result = WK_FALSE;

    switch (coder->type) {
        case WK_CODER_HEX: {
            hexEncode (dst, dstLen, src, srcLen);
            result = WK_TRUE;
            break;
        }
        case WK_CODER_BASE58: {
            result = AS_WK_BOOLEAN (BRBase58Encode (dst, dstLen, src, srcLen));
            break;
        }
        case WK_CODER_BASE58CHECK: {
            result = AS_WK_BOOLEAN (BRBase58CheckEncode (dst, dstLen, src, srcLen));
            break;
        }
        case WK_CODER_BASE58RIPPLE: {
            result = AS_WK_BOOLEAN (BRBase58EncodeEx (dst, dstLen, src, srcLen, rippleAlphabet));
            break;
        }
        default: {
            assert (0);
            break;
        }
    }

    return result;
}

extern size_t
wkCoderDecodeLength (WKCoder coder,
                         const char *src) {
    // - src CANNOT be NULL (see: BRBase58Decode), even with srcLen of 0
    if (NULL == src) {
        assert (0);
        return 0;
    }

    size_t length = 0;

    switch (coder->type) {
        case WK_CODER_HEX: {
            size_t strLen = strlen (src);
            length = (0 == strLen % 2) ? hexDecodeLength (strLen) : 0;
            break;
        }
        case WK_CODER_BASE58: {
            length = BRBase58Decode (NULL, 0, src);
            break;
        }
        case WK_CODER_BASE58CHECK: {
            length = BRBase58CheckDecode (NULL, 0, src);
            break;
        }
        case WK_CODER_BASE58RIPPLE: {
            length = BRBase58DecodeEx (NULL, 0, src, rippleAlphabet);
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
wkCoderDecode (WKCoder coder,
                   uint8_t *dst,
                   size_t dstLen,
                   const char *src) {
    // - src CANNOT be NULL (see: BRBase58Decode), even with srcLen of 0
    // - dst MUST be non-NULL and sufficiently sized
    if (NULL == src ||
        NULL == dst || dstLen < wkCoderDecodeLength (coder, src)) {
        assert (0);
        return WK_FALSE;
    }

    WKBoolean result = WK_FALSE;

    switch (coder->type) {
        case WK_CODER_HEX: {
            hexDecode (dst, dstLen, src, strlen (src));
            result = WK_TRUE;
            break;
        }
        case WK_CODER_BASE58: {
            result = AS_WK_BOOLEAN (BRBase58Decode (dst, dstLen, src));
            break;
        }
        case WK_CODER_BASE58CHECK: {
            result = AS_WK_BOOLEAN (BRBase58CheckDecode (dst, dstLen, src));
            break;
        }
        case WK_CODER_BASE58RIPPLE: {
            result = AS_WK_BOOLEAN (BRBase58DecodeEx (dst, dstLen, src, rippleAlphabet));
            break;
        }
        default: {
            // for an unsupported algorithm, assert
            assert (0);
            break;
        }
    }

    return result;
}
