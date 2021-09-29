//
//  WKKey.h
//  WalletKitCore
//
//  Created by Ed Gamble on 7/30/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKKey_h
#define WKKey_h

#include "WKBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: - Crypto Secret

/**
 * A Secret is 256 bits of data.
 */
typedef struct {
    uint8_t data[256/8];
} WKSecret;

/**
 * Zero-out the secret
 */
static inline void wkSecretClear (WKSecret *secret) {
    memset (secret->data, 0, sizeof (secret->data));
}

/// MARK: Crypto Key

/**
 * @brief A Key represents a public/private key pair.
 */
typedef struct WKKeyRecord *WKKey;

/**
 * Check if the `privateKey` string represents as valid BIP38 string (private key enscripted
 * with a passphrase).
 */
extern WKBoolean
wkKeyIsProtectedPrivate (const char *privateKey);

/**
 * Create a key from a secret
 */
extern WKKey
wkKeyCreateFromSecret (WKSecret secret);

/**
 * Create a key from a BIP39 phrase using works
 */
extern WKKey
wkKeyCreateFromPhraseWithWords (const char *phrase, const char *words[]);

/**
 * Create a key by decoding the privateKey with passphrase
 */
extern WKKey
wkKeyCreateFromStringProtectedPrivate (const char *privateKey, const char * passphrase);

/**
 * Create a key from a WIF encoded private string.
 */
extern WKKey
wkKeyCreateFromStringPrivate (const char *string);

/**
 * Create a key fro a WIF encoded public string
 */
extern WKKey
wkKeyCreateFromStringPublic (const char *string);

extern WKKey
wkKeyCreateForPigeon (WKKey key, uint8_t *nonce, size_t nonceCount);

extern WKKey
wkKeyCreateForBIP32ApiAuth (const char *phrase, const char *words[]);

extern WKKey
wkKeyCreateForBIP32BitID (const char *phrase, int index, const char *uri,  const char *words[]);

#if 0
/**
 * Serialize a public key into `data`.  If `data` is NULL, then the return value is the size
 * in bytes required by `data`.  If `data` is not NULL and `dataCound` is less than the required
 * size then 0 is returned an `data` is unmodified.  Otherwise data contains a serialization.
 */
extern size_t
wkKeySerializePublic (WKKey key, /* ... */ uint8_t *data, size_t dataCount);

/**
 * <See above>
 */
extern size_t
wkKeySerializePrivate(WKKey key, /* ... */ uint8_t *data, size_t dataCount);
#endif

/**
 * Check if key has a secret; that is, is a private key.
 */
extern int
wkKeyHasSecret (WKKey key);

/**
 * Encode the private key to a string.
 */
extern char *
wkKeyEncodePrivate (WKKey key);

/**
 * Encode the public key into a string.
 */
extern char *
wkKeyEncodePublic (WKKey key);

/**
 * Get the key's secret
 */
extern WKSecret
wkKeyGetSecret (WKKey key);

/**
 * Check if two keys secret matches.
 */
extern int
wkKeySecretMatch (WKKey key1, WKKey key2);

/**
 * Check if teh two keys public keys match
 */
extern int
wkKeyPublicMatch (WKKey key1, WKKey key2);

/**
 * Derived the DER encoded public key from key's private key
 */
extern void
wkKeyProvidePublicKey (WKKey key, int useCompressed, int compressed);

DECLARE_WK_GIVE_TAKE (WKKey, wkKey);

#ifdef __cplusplus
}
#endif

#endif /* WKKey_h */
