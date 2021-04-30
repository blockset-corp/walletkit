//
//  WKCipher.h
//  WalletKitCore
//
//  Created by Michael Carrara on 9/23/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKCipher_h
#define WKCipher_h

#include "WKBase.h"
#include "WKKey.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WK_CIPHER_AESECB,
    WK_CIPHER_CHACHA20_POLY1305,
    WK_CIPHER_PIGEON
} WKCipherType;

typedef struct WKCipherRecord *WKCipher;

extern WKCipher
wkCipherCreateForAESECB(const uint8_t *key,
                            size_t keyLen);

extern WKCipher
wkCipherCreateForChacha20Poly1305(WKKey key,
                                      const uint8_t *nonce, size_t nonceLen,
                                      const uint8_t *authenticatedData, size_t authenticatedDataLen);

extern WKCipher
wkCipherCreateForPigeon(WKKey privKey,
                            WKKey pubKey,
                            const uint8_t *nonce, size_t nonceLen);

extern size_t
wkCipherEncryptLength (WKCipher cipher,
                           const uint8_t *plaintext,
                           size_t plaintextLen);

extern WKBoolean
wkCipherEncrypt (WKCipher cipher,
                     uint8_t *ciphertext,
                     size_t ciphertextLen,
                     const uint8_t *plaintext,
                     size_t plaintextLen);

extern size_t
wkCipherDecryptLength (WKCipher cipher,
                           const uint8_t *ciphertext,
                           size_t ciphertextLen);

extern WKBoolean
wkCipherDecrypt (WKCipher cipher,
                     uint8_t *plaintext,
                     size_t plaintextLen,
                     const uint8_t *ciphertext,
                     size_t ciphertextLen);

extern WKBoolean
wkCipherMigrateBRCoreKeyCiphertext (WKCipher cipher,
                                        uint8_t *migratedCiphertext,
                                        size_t migratedCiphertextLen,
                                        const uint8_t *originalCiphertext,
                                        size_t originalCiphertextLen);

DECLARE_WK_GIVE_TAKE (WKCipher, wkCipher);

#ifdef __cplusplus
}
#endif

#endif /* WKCipher_h */
