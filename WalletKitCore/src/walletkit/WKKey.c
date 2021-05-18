//
//  WKKey.c
//  WalletKitCore
//
//  Created by Ed Gamble on 7/30/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <assert.h>
#include <stdlib.h>

#include "WKKey.h"
#include "support/BRKey.h"
#include "support/BRBIP39Mnemonic.h"
#include "support/BRBIP32Sequence.h"
#include "support/BRKeyECIES.h"
#include "support/util/BRUtil.h"
#include "support/BRBIP38Key.h"

// We create an arbitary BRAddressParams - disconnected from any know BITCOIN, etc address params.
// We do this until we know how the Key is used.  Right now, the only accessors are to get the
// serialized form of the Key.  Is somebody expecting a particular encoding?  Is somebody expectring
// to decode the serialization and then pass it to some other 'string-y' interface?  Until we know
// what those other interfaces require, we don't know appropriate address parameters.
//
// Our serialization/deserialization works because we use the same address params.  If the above
// questions are answered, the various wkKeyCreate*() functions will need to specify their
// address params AND (AND AND) the serialization will need to WRITE OUT the specific params.
//
// Resulting Base58 Prefix:
//  6: Uncompressed
//  T: Compressed
//
#define WK_PREFIX_OFFET    0x30
#define WK_ADDRESS_PARAMS  ((BRAddressParams) { \
    WK_PREFIX_OFFET + BITCOIN_PUBKEY_PREFIX,  \
    WK_PREFIX_OFFET + BITCOIN_SCRIPT_PREFIX,  \
    WK_PREFIX_OFFET + BITCOIN_PRIVKEY_PREFIX, \
    "cry" \
})

struct WKKeyRecord {
    BRKey core;
    BRAddressParams coreAddressParams;
    WKRef ref;
};

IMPLEMENT_WK_GIVE_TAKE (WKKey, wkKey);

static WKKey
wkKeyCreateInternal (BRKey core, BRAddressParams params) {
    WKKey key = calloc (1, sizeof (struct WKKeyRecord));

    key->core = core;
    key->coreAddressParams = params;
    key->ref = WK_REF_ASSIGN(wkKeyRelease);

    // clean up the key from the stack
    BRKeyClean (&core);

    return key;
}

static void
wkKeyRelease (WKKey key) {
    BRKeyClean (&key->core);
    free (key);
}

extern WKBoolean
wkKeyIsProtectedPrivate (const char *privateKey) {
    return AS_WK_BOOLEAN (BRBIP38KeyIsValid (privateKey));
}

private_extern WKKey
wkKeyCreateFromKey (BRKey *key) {
    return wkKeyCreateInternal (*key, WK_ADDRESS_PARAMS);
}

extern WKKey
wkKeyCreateFromSecret (WKSecret secret) {
    BRKey core;
    BRKeySetSecret (&core, (UInt256*) secret.data, 1);
    WKKey result = wkKeyCreateInternal (core, WK_ADDRESS_PARAMS);

    BRKeyClean(&core);
    memset (secret.data, 0, sizeof(secret.data));

    return result;
}

extern WKKey
wkKeyCreateFromPhraseWithWords (const char *phrase, const char *words[]) {
    if (!BRBIP39PhraseIsValid (words, phrase)) return NULL;

    UInt512 seed;
    BRBIP39DeriveKey (seed.u8, phrase, NULL);
    UInt256 *secret = (UInt256*) &seed;

    BRKey core;
    BRKeySetSecret(&core, secret, 1);
    WKKey result = wkKeyCreateInternal(core, WK_ADDRESS_PARAMS);

    BRKeyClean(&core);
    seed = UINT512_ZERO;

    return result;
}

static BRAddressParams
wkKeyFindAddressParams (const char *string) {
    if (BRPrivKeyIsValid (BITCOIN_ADDRESS_PARAMS, string)) return BITCOIN_ADDRESS_PARAMS;
    if (BRPrivKeyIsValid (BITCOIN_TEST_ADDRESS_PARAMS, string)) return BITCOIN_TEST_ADDRESS_PARAMS;
    if (BRPrivKeyIsValid (WK_ADDRESS_PARAMS, string)) return WK_ADDRESS_PARAMS;
    return EMPTY_ADDRESS_PARAMS;
}

extern WKKey
wkKeyCreateFromStringProtectedPrivate (const char *privateKey, const char * passphrase) {
    if (!BRBIP38KeyIsValid (privateKey)) return NULL;

    BRKey core;
    WKKey result = (1 == BRBIP38KeySetKey(&core, privateKey, passphrase, BITCOIN_ADDRESS_PARAMS)
                          ? wkKeyCreateInternal (core, BITCOIN_ADDRESS_PARAMS)
                          : (1 == BRBIP38KeySetKey(&core, privateKey, passphrase, BITCOIN_TEST_ADDRESS_PARAMS)
                             ? wkKeyCreateInternal (core, BITCOIN_TEST_ADDRESS_PARAMS)
                             : (1 == BRBIP38KeySetKey(&core, privateKey, passphrase, WK_ADDRESS_PARAMS)
                                ? wkKeyCreateInternal (core, WK_ADDRESS_PARAMS)
                                : NULL)));

    BRKeyClean (&core);

    return result;
}

extern WKKey
wkKeyCreateFromStringPrivate (const char *string) {
    BRAddressParams params = wkKeyFindAddressParams(string);

    BRKey core;
    WKKey result = (1 == BRKeySetPrivKey (&core, params, string)
                          ? wkKeyCreateInternal (core, params)
                          : NULL);

    BRKeyClean (&core);

    return result;
}

extern WKKey
wkKeyCreateFromStringPublic (const char *string) {
    size_t  targetLen = strlen (string) / 2;
    uint8_t target [targetLen];
    hexDecode(target, targetLen, string, strlen (string));

    BRKey core;
    WKKey result = (1 == BRKeySetPubKey (&core, target, targetLen)
                          ? wkKeyCreateInternal (core, WK_ADDRESS_PARAMS)
                          : NULL);

    // key doesn't need to be cleaned; just a silly public key
    return result;
}

extern WKKey
wkKeyCreateForPigeon (WKKey privateKey, uint8_t *nonce, size_t nonceCount) {
    BRKey pairingKey;

    BRKeyPigeonPairingKey (&privateKey->core, &pairingKey, nonce, nonceCount);
    WKKey result = wkKeyCreateInternal (pairingKey, WK_ADDRESS_PARAMS);

    BRKeyClean (&pairingKey);
    return result;
}

extern WKKey
wkKeyCreateForBIP32ApiAuth (const char *phrase, const char *words[]) {
    if (!BRBIP39PhraseIsValid (words, phrase)) return NULL;

    UInt512 seed;
    BRBIP39DeriveKey (seed.u8, phrase, NULL);

    BRKey core;
    BRBIP32APIAuthKey (&core, &seed, sizeof (UInt512));


    //        BRBIP39DeriveKey(&seed, phrase, nil)
    //        BRBIP32APIAuthKey(&key, &seed, MemoryLayout<UInt512>.size)
    //        seed = UInt512() // clear seed
    //        let pkLen = BRKeyPrivKey(&key, nil, 0)
    //        var pkData = CFDataCreateMutable(secureAllocator, pkLen) as Data
    //        pkData.count = pkLen
    //        guard pkData.withUnsafeMutableBytes({ BRKeyPrivKey(&key, $0.baseAddress?.assumingMemoryBound(to: Int8.self), pkLen) }) == pkLen else { return nil }
    //        key.clean()

    WKKey result = wkKeyCreateInternal(core, WK_ADDRESS_PARAMS);

    BRKeyClean (&core);
    seed = UINT512_ZERO;

    return result;
}

extern WKKey
wkKeyCreateForBIP32BitID (const char *phrase, int index, const char *uri,  const char *words[]) {
    if (!BRBIP39PhraseIsValid (words, phrase)) return NULL;

    UInt512 seed;
    BRBIP39DeriveKey (seed.u8, phrase, NULL);

    BRKey core;
    BRBIP32BitIDKey (&core, &seed, sizeof(UInt512), (uint32_t) index, uri);

    WKKey result = wkKeyCreateInternal(core, WK_ADDRESS_PARAMS);

    BRKeyClean (&core);
    seed = UINT512_ZERO;

    return result;
}

#if defined (NEVER_DEFINED) // keep as example
typedef enum {
    SERIALIZE_PUBLIC_IDENTIFIER = 1,
    SERIALIZE_PRIVATE_IDENTIFIER = 2
} WKKeySerializeIdentifier;


extern WKKey
wkKeyCreateFromSerializationPublic (uint8_t *data, size_t dataCount) {
    if (dataCount < 1) return NULL;
    if (SERIALIZE_PUBLIC_IDENTIFIER != (WKKeySerializeIdentifier) data[0]) return NULL;

    BRKey core;

    return (1 == BRKeySetPubKey (&core, &data[1], dataCount - 1)
            ? wkKeyCreateInternal (core, WK_ADDRESS_PARAMS)
            : NULL);
}

extern size_t
wkKeySerializePublic (WKKey key, /* ... */ uint8_t *data, size_t dataCount) {
    size_t publicKeySize = BRKeyPubKey (&key->core, NULL, 0);
    size_t serializeSize = 1 + publicKeySize;

    if (NULL == data) return serializeSize;
    if (dataCount < serializeSize) return 0;

    data[0] = SERIALIZE_PUBLIC_IDENTIFIER;
    BRKeyPubKey (&key->core, &data[1], dataCount);
    return dataCount;
}

extern WKKey
wkKeyCreateFromSerializationPrivate (uint8_t *data, size_t dataCount) {
    if (dataCount < 1) return NULL;
    if (SERIALIZE_PRIVATE_IDENTIFIER != (WKKeySerializeIdentifier) data[0]) return NULL;

    // Get the AddressParms
    BRAddressParams addressParams = WK_ADDRESS_PARAMS;
    BRKey core;

    // Make a string
    char privKey[dataCount];
    memcpy (privKey, &data[1], dataCount - 1);
    privKey[dataCount - 1] = 0;

    if (!BRPrivKeyIsValid (addressParams, privKey)) return NULL;

    return (1 == BRKeySetPrivKey (&core, addressParams, privKey)
            ? wkKeyCreateInternal (core, addressParams)
            : NULL);
}

extern size_t
wkKeySerializePrivate (WKKey key, /* ... */ uint8_t *data, size_t dataCount) {
    // If we encode the BRAddressParams, we'll need to increate the required dataCount
    size_t privateKeySize = BRKeyPrivKey (&key->core, NULL, 0, key->coreAddressParams);
    size_t serializeSize  = 1 + privateKeySize;

    if (NULL == data) return serializeSize;
    if (dataCount < serializeSize) return 0;

    data[0] = SERIALIZE_PRIVATE_IDENTIFIER;

    // Make a string
    char privKey[dataCount]; // includes 1 for '\0'
    memcpy (privKey, &data[1], dataCount - 1);
    privKey[dataCount - 1] = 0;

    return (0 != BRKeyPrivKey (&key->core, privKey, dataCount - 1, key->coreAddressParams)
            ? dataCount
            : 0);
}
#endif

extern int
wkKeyHasSecret (WKKey key) {
    UInt256 zero = UINT256_ZERO;
    return 0 != memcmp (key->core.secret.u8, zero.u8, sizeof (zero.u8));
}

extern char *
wkKeyEncodePrivate (WKKey key) {
    size_t encodedLength = BRKeyPrivKey (&key->core, NULL, 0, key->coreAddressParams);
    char  *encoded = malloc (encodedLength + 1);
    BRKeyPrivKey (&key->core, encoded, encodedLength, key->coreAddressParams);
    encoded[encodedLength] = '\0';
    return encoded;
}

extern char *
wkKeyEncodePublic (WKKey key) {
    size_t encodedLength = BRKeyPubKey (&key->core, NULL, 0);
    uint8_t encoded[encodedLength];

    BRKeyPubKey(&key->core, encoded, encodedLength);
    return hexEncodeCreate (NULL, encoded, encodedLength);
}

extern WKSecret
wkKeyGetSecret (WKKey key) {
    WKSecret secret;
    memcpy (secret.data, key->core.secret.u8, sizeof (secret.data));
    return secret;
}

extern int
wkKeySecretMatch (WKKey key1, WKKey key2) {
    return 0 == memcmp (key1->core.secret.u8, key2->core.secret.u8, sizeof (key1->core.secret));
}

extern int
wkKeyPublicMatch (WKKey key1, WKKey key2) {
    return 1 == BRKeyPubKeyMatch (&key1->core, &key2->core);
}

//extern size_t
//wkKeySign (WKKey key, void *sig, size_t sigLen, UInt256 md) {
//    return BRKeySign (&key->core, sig, sigLen, md);
//}

extern BRKey *
wkKeyGetCore (WKKey key) {
    return &key->core;
}

extern void
wkKeyProvidePublicKey (WKKey key, int useCompressed, int compressed) {
    if (useCompressed) BRKeySetCompressed (&key->core, compressed);
    BRKeyPubKey (&key->core, NULL, 0);
}
