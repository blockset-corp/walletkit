//
//  WKCoder.h
//  WalletKitCore
//
//  Created by Michael Carrara on 9/23/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKCoder_h
#define WKCoder_h

#include "WKBase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WK_CODER_HEX,
    WK_CODER_BASE58,
    WK_CODER_BASE58CHECK,
    WK_CODER_BASE58RIPPLE
} WKCoderType;

typedef struct WKCoderRecord *WKCoder;

extern WKCoder
wkCoderCreate(WKCoderType type);

extern size_t
wkCoderEncodeLength (WKCoder coder,
                     const uint8_t *src,
                     size_t srcLen);

extern WKBoolean
wkCoderEncode (WKCoder coder,
               char *dst,
               size_t dstLen,
               const uint8_t *src,
               size_t srcLen);

extern size_t
wkCoderDecodeLength (WKCoder coder,
                     const char *src);

extern WKBoolean
wkCoderDecode (WKCoder coder,
               uint8_t *dst,
               size_t dstLen,
               const char *src);

DECLARE_WK_GIVE_TAKE (WKCoder, wkCoder);

#ifdef __cplusplus
}
#endif

#endif /* WKCoder_h */
