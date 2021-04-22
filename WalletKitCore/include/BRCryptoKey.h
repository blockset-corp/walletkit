//
//  BRCryptoKey.h
//  WalletKitCore
//
//  Created by Ed Gamble on 7/30/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoKey_h
#define BRCryptoKey_h

#include "BRCryptoBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: - Crypto Secret

/**
 * A Secret is 256 bits of data.
 */
typedef struct {
    uint8_t data[256/8];
} BRCryptoSecret;

/**
 * Zero-out the secret
 */
static inline void cryptoSecretClear (BRCryptoSecret *secret) {
    memset (secret->data, 0, sizeof (secret->data));
}

/// MARK: Crypto Key

/**
 * @brief A Key represents a public/private key pair.
 */
typedef struct BRCryptoKeyRecord *BRCryptoKey;

/**
 * Check if the `privateKey` string represents as valid BIP38 string (private key enscripted
 * with a passphrase).
 */
extern BRCryptoBoolean
cryptoKeyIsProtectedPrivate (const char *privateKey);

/**
 * Create a key from a secret
 */
extern BRCryptoKey
cryptoKeyCreateFromSecret (BRCryptoSecret secret);

/**
 * Create a key from a BIP39 phrase using works
 */
extern BRCryptoKey
cryptoKeyCreateFromPhraseWithWords (const char *phrase, const char *words[]);

/**
 * Create a key by decoding the privateKey with passphrase
 */
extern BRCryptoKey
cryptoKeyCreateFromStringProtectedPrivate (const char *privateKey, const char * passphrase);

/**
 * Create a key from a WIF encoded private string.
 */
extern BRCryptoKey
cryptoKeyCreateFromStringPrivate (const char *string);

/**
 * Create a key fro a WIF encoded public string
 */
extern BRCryptoKey
cryptoKeyCreateFromStringPublic (const char *string);

extern BRCryptoKey
cryptoKeyCreateForPigeon (BRCryptoKey key, uint8_t *nonce, size_t nonceCount);

extern BRCryptoKey
cryptoKeyCreateForBIP32ApiAuth (const char *phrase, const char *words[]);

extern BRCryptoKey
cryptoKeyCreateForBIP32BitID (const char *phrase, int index, const char *uri,  const char *words[]);

#if 0
/**
 * Serialize a public key into `data`.  If `data` is NULL, then the return value is the size
 * in bytes required by `data`.  If `data` is not NULL and `dataCound` is less than the required
 * size then 0 is returned an `data` is unmodified.  Otherwise data contains a serialization.
 */
extern size_t
cryptoKeySerializePublic (BRCryptoKey key, /* ... */ uint8_t *data, size_t dataCount);

/**
 * <See above>
 */
extern size_t
cryptoKeySerializePrivate(BRCryptoKey key, /* ... */ uint8_t *data, size_t dataCount);
#endif

/**
 * Check if key has a secret; that is, is a private key.
 */
extern int
cryptoKeyHasSecret (BRCryptoKey key);

/**
 * Encode the private key to a string.
 */
extern char *
cryptoKeyEncodePrivate (BRCryptoKey key);

/**
 * Encode the public key into a string.
 */
extern char *
cryptoKeyEncodePublic (BRCryptoKey key);

/**
 * Get the key's secret
 */
extern BRCryptoSecret
cryptoKeyGetSecret (BRCryptoKey key);

/**
 * Check if two keys secret matches.
 */
extern int
cryptoKeySecretMatch (BRCryptoKey key1, BRCryptoKey key2);

/**
 * Check if teh two keys public keys match
 */
extern int
cryptoKeyPublicMatch (BRCryptoKey key1, BRCryptoKey key2);

/**
 * Derived the DER encoded public key from key's private key
 */
extern void
cryptoKeyProvidePublicKey (BRCryptoKey key, int useCompressed, int compressed);

DECLARE_CRYPTO_GIVE_TAKE (BRCryptoKey, cryptoKey);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoKey_h */
