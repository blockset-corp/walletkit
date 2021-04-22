//
//  BRCryptoSigner.h
//  WalletKitCore
//
//  Created by Michael Carrara on 9/23/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoSigner_h
#define BRCryptoSigner_h

#include "BRCryptoBase.h"
#include "BRCryptoKey.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * An enumeration of the possible types of cryptographic signing functions
 */
typedef enum {
    CRYPTO_SIGNER_BASIC_DER,
    CRYPTO_SIGNER_BASIC_JOSE,
    CRYPTO_SIGNER_COMPACT
} BRCryptoSignerType;

/**
 * @brief A signer represents a cryptographic signing function.
 */
typedef struct BRCryptoSignerRecord *BRCryptoSigner;

/**
 * Create a signer from a signer type.
 */
extern BRCryptoSigner
cryptoSignerCreate(BRCryptoSignerType type);

/*
 * Get the signer's length in bytes.  This is the number of bytes produced when the signer is
 * applied.
 */
extern size_t
cryptoSignerSignLength (BRCryptoSigner signer,
                        BRCryptoKey key,
                        const uint8_t *src,
                        size_t srcLen);

/**
 * Fill `dst` with the signature as the result of applying `signer` to `digest` where
 * `digestLen` must be 32 (bytes).  Returns CRYPTO_TRUE if successful.  Typically the
 * application would be unsuccessfull if the `dstLen` is not at least as large as the
 * `cryptoSignerLength()` result.
 */
extern BRCryptoBoolean
cryptoSignerSign (BRCryptoSigner signer,
                  BRCryptoKey key,
                  uint8_t *dst,
                  size_t dstLen,
                  const uint8_t *digest,
                  size_t digestLen);

/**
 * Recover a public Key from `signature` and `digest` by applying the `signer`.  The `digestLen`
 * must be 32 (bytes).
 */
extern BRCryptoKey
cryptoSignerRecover (BRCryptoSigner signer,
                     const uint8_t *digest,
                     size_t digestLen,
                     const uint8_t *signature,
                     size_t signatureLen);

DECLARE_CRYPTO_GIVE_TAKE (BRCryptoSigner, cryptoSigner);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoSigner_h */
