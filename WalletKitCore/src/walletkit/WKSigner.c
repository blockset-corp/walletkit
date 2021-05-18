//
//  WKSigner.c
//  WalletKitCore
//
//  Created by Michael Carrara on 9/23/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <assert.h>
#include <stdlib.h>

#include "WKSigner.h"
#include "WKKeyP.h"
#include "support/BRCrypto.h"

struct WKSignerRecord {
    WKSignerType type;
    WKRef ref;
};

IMPLEMENT_WK_GIVE_TAKE (WKSigner, wkSigner);

extern WKSigner
wkSignerCreate(WKSignerType type) {
    WKSigner signer = NULL;

    switch (type) {
        case WK_SIGNER_BASIC_DER:
        case WK_SIGNER_BASIC_JOSE:
        case WK_SIGNER_COMPACT: {
            signer = calloc (1, sizeof(struct WKSignerRecord));
            signer->type = type;
            signer->ref = WK_REF_ASSIGN(wkSignerRelease);
            break;
        }
        default: {
            // for an unsupported algorithm, assert
            assert (0);
            break;
        }
    }

    return signer;
}

static void
wkSignerRelease (WKSigner signer) {
    memset (signer, 0, sizeof(*signer));
    free (signer);
}

extern size_t
wkSignerSignLength (WKSigner signer,
                        WKKey key,
                        const uint8_t *src,
                        size_t srcLen) {
    // - key CANNOT be NULL
    // - src CANNOT be NULL and must be 32 bytes long (i.e. a UINT256)
    if (NULL == key ||
        NULL == src || 32 != srcLen) {
        assert (0);
        return 0;
    }

    size_t length = 0;

    switch (signer->type) {
        case WK_SIGNER_BASIC_DER: {
            length = BRKeySign (wkKeyGetCore (key), NULL, 0, UInt256Get (src));
            break;
        }
        case WK_SIGNER_BASIC_JOSE: {
            length = BRKeySignJOSE (wkKeyGetCore (key), NULL, 0, UInt256Get (src));
            break;
        }
        case WK_SIGNER_COMPACT: {
            length = BRKeyCompactSign (wkKeyGetCore (key), NULL, 0, UInt256Get (src));
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
wkSignerSign (WKSigner signer,
                  WKKey key,
                  uint8_t *dst,
                  size_t dstLen,
                  const uint8_t *src,
                  size_t srcLen) {
    // - key CANNOT be NULL
    // - src CANNOT be NULL and must be 32 bytes long (i.e. a UINT256)
    // - dst MUST be non-NULL and sufficiently sized
    if (NULL == key ||
        NULL == src || 32 != srcLen ||
        NULL == dst || dstLen < wkSignerSignLength (signer, key, src, srcLen)) {
        assert (0);
        return WK_FALSE;
    }

    WKBoolean result = WK_FALSE;

    switch (signer->type) {
        case WK_SIGNER_BASIC_DER: {
            result = AS_WK_BOOLEAN (BRKeySign (wkKeyGetCore (key), dst, dstLen, UInt256Get (src)));
            break;
        }
        case WK_SIGNER_BASIC_JOSE: {
            result = AS_WK_BOOLEAN (BRKeySignJOSE (wkKeyGetCore (key), dst, dstLen, UInt256Get (src)));
            break;
        }
        case WK_SIGNER_COMPACT: {
            result = AS_WK_BOOLEAN (BRKeyCompactSign (wkKeyGetCore (key), dst, dstLen, UInt256Get (src)));
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

extern WKKey
wkSignerRecover (WKSigner signer,
                     const uint8_t *digest,
                     size_t digestLen,
                     const uint8_t *signature,
                     size_t signatureLen) {
    // - digest CANNOT be NULL and must be 32 bytes long (i.e. a UINT256)
    // - signature CANNOT be NULL (size checked independently in recovery methods)
    if (NULL == digest || 32 != digestLen || NULL == signature) {
        assert (0);
        return NULL;
    }

    WKKey key = NULL;

    switch (signer->type) {
        case WK_SIGNER_BASIC_DER:
        case WK_SIGNER_BASIC_JOSE: {
            // not supported, but not necessarily worth an assert
            break;
        }
        case WK_SIGNER_COMPACT: {
            BRKey k;
            if (1 == BRKeyRecoverPubKey (&k, UInt256Get (digest), signature, signatureLen) ) {
                key = wkKeyCreateFromKey (&k);
            }
            BRKeyClean (&k);
            break;
        }
        default: {
            // for an unsupported algorithm, assert
            assert (0);
            break;
        }
    }

    return key;
}
