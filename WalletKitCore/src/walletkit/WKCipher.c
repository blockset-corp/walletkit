//
//  WKCipher.c
//  WalletKitCore
//
//  Created by Michael Carrara on 9/23/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <assert.h>
#include <stdlib.h>

#include "WKCipher.h"
#include "WKKeyP.h"

#include "support/BRBase.h"
#include "support/BRCrypto.h"
#include "support/BRKeyECIES.h"

struct WKCipherRecord {
    WKCipherType type;

    union {
        struct {
            uint8_t key[32];
            size_t keyLen;
        } aesecb;

        struct {
            WKKey key;
            uint8_t nonce[12];
            uint8_t *ad;
            size_t adLen;
        } chacha20;

        struct {
            WKKey privKey;
            WKKey pubKey;
            uint8_t nonce[12];
        } pigeon;
    } u;

    WKRef ref;
};

IMPLEMENT_WK_GIVE_TAKE (WKCipher, wkCipher);

static WKCipher
wkCipherCreateInternal(WKCipherType type) {
    WKCipher cipher = calloc (1, sizeof(struct WKCipherRecord));
    cipher->type = type;
    cipher->ref = WK_REF_ASSIGN(wkCipherRelease);
    return cipher;
}

extern WKCipher
wkCipherCreateForAESECB(const uint8_t *key,
                            size_t keyLen) {
    // argument check; early exit
    if (NULL == key || (keyLen != 16 && keyLen != 24 && keyLen != 32)) {
        assert (0);
        return NULL;
    }

    WKCipher cipher  = wkCipherCreateInternal (WK_CIPHER_AESECB);
    // aesecb.key initialized to zero via calloc
    memcpy (cipher->u.aesecb.key, key, keyLen);
    cipher->u.aesecb.keyLen = keyLen;

    return cipher;
}

extern WKCipher
wkCipherCreateForChacha20Poly1305(WKKey key,
                                      const uint8_t *nonce, size_t nonceLen,
                                      const uint8_t *ad, size_t adLen) {
    // argument check; early exit
    if (NULL == key || NULL == nonce || nonceLen != 12 ||
        (NULL == ad && 0 != adLen)) {
        assert (0);
        return NULL;
    }

    WKCipher cipher  = wkCipherCreateInternal (WK_CIPHER_CHACHA20_POLY1305);
    cipher->u.chacha20.key = wkKeyTake (key);
    memcpy (cipher->u.chacha20.nonce, nonce, nonceLen);
    if (NULL != ad && 0 != adLen) {
        cipher->u.chacha20.ad = malloc (adLen);
        cipher->u.chacha20.adLen = adLen;
        memcpy (cipher->u.chacha20.ad, ad, adLen);
    }

    return cipher;
}

extern WKCipher
wkCipherCreateForPigeon(WKKey privKey,
                            WKKey pubKey,
                            const uint8_t *nonce, size_t nonceLen) {
    // argument check; early exit
    if (NULL == pubKey || NULL == privKey || NULL == nonce || nonceLen != 12) {
        assert (0);
        return NULL;
    }

    WKCipher cipher  = wkCipherCreateInternal (WK_CIPHER_PIGEON);
    cipher->u.pigeon.privKey = wkKeyTake (privKey);
    cipher->u.pigeon.pubKey = wkKeyTake (pubKey);
    memcpy (cipher->u.pigeon.nonce, nonce, nonceLen);

    return cipher;
}

static void
wkCipherRelease (WKCipher cipher) {
    switch (cipher->type) {
        case WK_CIPHER_AESECB: {
            break;
        }
        case WK_CIPHER_CHACHA20_POLY1305: {
            wkKeyGive (cipher->u.chacha20.key);
            if (NULL != cipher->u.chacha20.ad) {
                free (cipher->u.chacha20.ad);
            }
            break;
        }
        case WK_CIPHER_PIGEON: {
            wkKeyGive (cipher->u.pigeon.privKey);
            wkKeyGive (cipher->u.pigeon.pubKey);
            break;
        }
        default: {
            break;
        }
    }

    memset (cipher, 0, sizeof(*cipher));
    free (cipher);
}

extern size_t
wkCipherEncryptLength (WKCipher cipher,
                           const uint8_t *src,
                           size_t srcLen) {
    // - src CAN be NULL, if srcLen is 0
    if (NULL == src && 0 != srcLen) {
        assert (0);
        return 0;
    }

    size_t length = 0;
    switch (cipher->type) {
        case WK_CIPHER_AESECB: {
            length = (0 == srcLen % 16) ? srcLen : 0;
            break;
        }
        case WK_CIPHER_CHACHA20_POLY1305: {
            WKSecret secret = wkKeyGetSecret (cipher->u.chacha20.key);
            length = BRChacha20Poly1305AEADEncrypt (NULL,
                                                    0,
                                                    secret.data,
                                                    cipher->u.chacha20.nonce,
                                                    src,
                                                    srcLen,
                                                    cipher->u.chacha20.ad,
                                                    cipher->u.chacha20.adLen);
            wkSecretClear(&secret);
            break;
        }
        case WK_CIPHER_PIGEON: {
            length = BRKeyPigeonEncrypt (NULL,
                                         NULL,
                                         0,
                                         NULL,
                                         cipher->u.pigeon.nonce,
                                         src,
                                         srcLen);
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
wkCipherEncrypt (WKCipher cipher,
                     uint8_t *dst,
                     size_t dstLen,
                     const uint8_t *src,
                     size_t srcLen) {
    // - src CAN be NULL, if srcLen is 0
    // - dst MUST be non-NULL and sufficiently sized
    if ((NULL == src && 0 != srcLen) ||
        NULL == dst || dstLen < wkCipherEncryptLength (cipher, src, srcLen)) {
        assert (0);
        return WK_FALSE;
    }

    WKBoolean result = WK_FALSE;

    switch (cipher->type) {
        case WK_CIPHER_AESECB: {
            if (srcLen == dstLen && (0 == srcLen % 16)) {
                memcpy (dst, src, dstLen);
                for (size_t index = 0; index < dstLen; index += 16) {
                    BRAESECBEncrypt (&dst[index], cipher->u.aesecb.key, cipher->u.aesecb.keyLen);
                }
                result = WK_TRUE;
            }
            break;
        }
        case WK_CIPHER_CHACHA20_POLY1305: {
            WKSecret secret = wkKeyGetSecret (cipher->u.chacha20.key);
            result = AS_WK_BOOLEAN (BRChacha20Poly1305AEADEncrypt (dst,
                                                                       dstLen,
                                                                       secret.data,
                                                                       cipher->u.chacha20.nonce,
                                                                       src,
                                                                       srcLen,
                                                                       cipher->u.chacha20.ad,
                                                                       cipher->u.chacha20.adLen));
            wkSecretClear(&secret);
            break;
        }
        case WK_CIPHER_PIGEON: {
            result = AS_WK_BOOLEAN (BRKeyPigeonEncrypt (wkKeyGetCore (cipher->u.pigeon.privKey),
                                                            dst,
                                                            dstLen,
                                                            wkKeyGetCore (cipher->u.pigeon.pubKey),
                                                            cipher->u.pigeon.nonce,
                                                            src,
                                                            srcLen));
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

extern size_t
wkCipherDecryptLength (WKCipher cipher,
                           const uint8_t *src,
                           size_t srcLen) {
    // - src CAN be NULL, if srcLen is 0
    if (NULL == src && 0 != srcLen) {
        assert (0);
        return 0;
    }

    size_t length = 0;

    switch (cipher->type) {
        case WK_CIPHER_AESECB: {
            length = (0 == srcLen % 16) ? srcLen : 0;
            break;
        }
        case WK_CIPHER_CHACHA20_POLY1305: {
            WKSecret secret = wkKeyGetSecret (cipher->u.chacha20.key);
            length = BRChacha20Poly1305AEADDecrypt (NULL,
                                                    0,
                                                    secret.data,
                                                    cipher->u.chacha20.nonce,
                                                    src,
                                                    srcLen,
                                                    cipher->u.chacha20.ad,
                                                    cipher->u.chacha20.adLen);
            wkSecretClear(&secret);
            break;
        }
        case WK_CIPHER_PIGEON: {
            length = BRKeyPigeonDecrypt (NULL,
                                         NULL,
                                         0,
                                         NULL,
                                         cipher->u.pigeon.nonce,
                                         src,
                                         srcLen);
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
wkCipherDecrypt (WKCipher cipher,
                     uint8_t *dst,
                     size_t dstLen,
                     const uint8_t *src,
                     size_t srcLen) {
    // - src CAN be NULL, if srcLen is 0
    // - dst MUST be non-NULL and sufficiently sized
    if ((NULL == src && 0 != srcLen) ||
        NULL == dst || dstLen < wkCipherDecryptLength (cipher, src, srcLen)) {
        assert (0);
        return WK_FALSE;
    }

    WKBoolean result = WK_FALSE;

    switch (cipher->type) {
        case WK_CIPHER_AESECB: {
            if (srcLen == dstLen && (0 == srcLen % 16)) {
                memcpy (dst, src, dstLen);
                for (size_t index = 0; index < dstLen; index += 16) {
                    BRAESECBDecrypt (&dst[index], cipher->u.aesecb.key, cipher->u.aesecb.keyLen);
                }
                result = WK_TRUE;
            }
            break;
        }
        case WK_CIPHER_CHACHA20_POLY1305: {
            WKSecret secret = wkKeyGetSecret (cipher->u.chacha20.key);
            result = AS_WK_BOOLEAN (BRChacha20Poly1305AEADDecrypt (dst,
                                                                       dstLen,
                                                                       secret.data,
                                                                       cipher->u.chacha20.nonce,
                                                                       src,
                                                                       srcLen,
                                                                       cipher->u.chacha20.ad,
                                                                       cipher->u.chacha20.adLen));
            wkSecretClear(&secret);
            break;
        }
        case WK_CIPHER_PIGEON: {
            result = AS_WK_BOOLEAN (BRKeyPigeonDecrypt (wkKeyGetCore (cipher->u.pigeon.privKey),
                                                            dst,
                                                            dstLen,
                                                            wkKeyGetCore (cipher->u.pigeon.pubKey),
                                                            cipher->u.pigeon.nonce,
                                                            src,
                                                            srcLen));
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

static size_t
wkCipherDecryptForMigrateLength (WKCipher cipher,
                                     const uint8_t *src,
                                     size_t srcLen) {
    // - src CAN be NULL, if srcLen is 0
    if (NULL == src && 0 != srcLen) {
        assert (0);
        return 0;
    }

    size_t length = 0;

    switch (cipher->type) {
        case WK_CIPHER_CHACHA20_POLY1305: {
            BRKey *coreKey  = wkKeyGetCore (cipher->u.chacha20.key);

            uint8_t pubKeyBytes[65];
            size_t pubKeyLen = BRKeyPubKey (coreKey, pubKeyBytes, sizeof(pubKeyBytes));
            if (0 == pubKeyLen || pubKeyLen > sizeof(pubKeyBytes)) break;

            UInt256 secret;
            BRSHA256 (&secret, &pubKeyBytes[1], pubKeyLen - 1);
            length = BRChacha20Poly1305AEADDecrypt (NULL,
                                                    0,
                                                    &secret,
                                                    cipher->u.chacha20.nonce,
                                                    src,
                                                    srcLen,
                                                    cipher->u.chacha20.ad,
                                                    cipher->u.chacha20.adLen);
            secret = UINT256_ZERO; (void) &secret;
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

static WKBoolean
wkCipherDecryptForMigrate (WKCipher cipher,
                               uint8_t *dst,
                               size_t dstLen,
                               const uint8_t *src,
                               size_t srcLen) {
    // - src CAN be NULL, if srcLen is 0
    // - dst MUST be non-NULL and sufficiently sized
    if ((NULL == src && 0 != srcLen) ||
        NULL == dst || dstLen < wkCipherDecryptLength (cipher, src, srcLen)) {
        assert (0);
        return WK_FALSE;
    }

    WKBoolean result = WK_FALSE;

    switch (cipher->type) {
        case WK_CIPHER_CHACHA20_POLY1305: {
            BRKey *coreKey  = wkKeyGetCore (cipher->u.chacha20.key);

            uint8_t pubKeyBytes[65];
            size_t pubKeyLen = BRKeyPubKey (coreKey, pubKeyBytes, sizeof(pubKeyBytes));
            if (0 == pubKeyLen || pubKeyLen > sizeof(pubKeyBytes)) break;

            UInt256 secret;
            BRSHA256 (&secret, &pubKeyBytes[1], pubKeyLen - 1);
            result = AS_WK_BOOLEAN (BRChacha20Poly1305AEADDecrypt (dst,
                                                                       dstLen,
                                                                       &secret,
                                                                       cipher->u.chacha20.nonce,
                                                                       src,
                                                                       srcLen,
                                                                       cipher->u.chacha20.ad,
                                                                       cipher->u.chacha20.adLen));
            secret = UINT256_ZERO; (void) &secret;
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

extern WKBoolean
wkCipherMigrateBRCoreKeyCiphertext (WKCipher cipher,
                                        uint8_t *migratedCiphertext,
                                        size_t migratedCiphertextLen,
                                        const uint8_t *originalCiphertext,
                                        size_t originalCiphertextLen) {
    // calculate the length of the plaintext using the modified decryption routine
    size_t plaintextLen = wkCipherDecryptForMigrateLength (cipher,
                                                               originalCiphertext,
                                                               originalCiphertextLen);
    if (0 == plaintextLen) {
        return WK_FALSE;
    }

    // allocate the plaintext buffer
    uint8_t *plaintext = (uint8_t *) malloc (plaintextLen);
    if (NULL == plaintext) {
        return WK_FALSE;
    }

    // decrypt the original ciphertext using the modified decryption routine
    WKBoolean decryptResult = wkCipherDecryptForMigrate (cipher,
                                                                   plaintext,
                                                                   plaintextLen,
                                                                   originalCiphertext,
                                                                   originalCiphertextLen);
    if (WK_TRUE != decryptResult) {
        memset (plaintext, 0, plaintextLen);
        free (plaintext);
        return WK_FALSE;
    }

    // calculate the length of the migrated ciphertext using the current encryption routine
    if (migratedCiphertextLen < wkCipherEncryptLength (cipher,
                                                           plaintext,
                                                           plaintextLen)) {
        memset (plaintext, 0, plaintextLen);
        free (plaintext);
        return WK_FALSE;
    }

    // encrypt the plaintext using the current encryption routine
    WKBoolean encryptResult = wkCipherEncrypt (cipher,
                                                         migratedCiphertext,
                                                         migratedCiphertextLen,
                                                         plaintext,
                                                         plaintextLen);

    // release the cipher and plaintext memory
    memset (plaintext, 0, plaintextLen);
    free (plaintext);

    return encryptResult;
}
