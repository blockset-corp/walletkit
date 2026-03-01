//
//  WKSigner.h
//  WalletKitCore
//
//  Created by Michael Carrara on 9/23/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKSigner_h
#define WKSigner_h

#include "WKBase.h"
#include "WKKey.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * An enumeration of the possible types of cryptographic signing functions
 */
typedef enum {
    WK_SIGNER_BASIC_DER,
    WK_SIGNER_BASIC_JOSE,
    WK_SIGNER_COMPACT
} WKSignerType;

/**
 * @brief A signer represents a cryptographic signing function.
 */
typedef struct WKSignerRecord *WKSigner;

/**
 * Create a signer from a signer type.
 */
extern WKSigner
wkSignerCreate(WKSignerType type);

/*
 * Get the signer's length in bytes.  This is the number of bytes produced when the signer is
 * applied.
 */
extern size_t
wkSignerSignLength (WKSigner signer,
                    WKKey key,
                    const uint8_t *src,
                    size_t srcLen);

/**
 * Fill `dst` with the signature as the result of applying `signer` to `digest` where
 * `digestLen` must be 32 (bytes).  Returns WK_TRUE if successful.  Typically the
 * application would be unsuccessful if the `dstLen` is not at least as large as the
 * `wkSignerLength()` result.
 */
extern WKBoolean
wkSignerSign (WKSigner signer,
              WKKey key,
              uint8_t *dst,
              size_t dstLen,
              const uint8_t *digest,
              size_t digestLen);

/**
 * Recover a public Key from `signature` and `digest` by applying the `signer`.  The `digestLen`
 * must be 32 (bytes).
 */
extern WKKey
wkSignerRecover (WKSigner signer,
                 const uint8_t *digest,
                 size_t digestLen,
                 const uint8_t *signature,
                 size_t signatureLen);

DECLARE_WK_GIVE_TAKE (WKSigner, wkSigner);

#ifdef __cplusplus
}
#endif

#endif /* WKSigner_h */
