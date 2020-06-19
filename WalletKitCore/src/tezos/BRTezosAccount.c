//
//  BRTezosAccount.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRTezosAccount.h"
#include "support/BRBIP32Sequence.h"
#include "ed25519/ed25519.h"

#include "blake2/blake2b.h"

#include <stdlib.h>
#include <assert.h>

#define TEZOS_PUBLIC_KEY_SIZE 32

struct BRTezosAccountRecord {
    BRTezosAddress address;
    uint8_t publicKey[TEZOS_PUBLIC_KEY_SIZE];
};

// MARK: - Crypto

// ed25519 child key derivation
static void _CKDpriv(UInt256 *k, UInt256 *c, uint32_t i)
{
    uint8_t buf[sizeof(BRECPoint) + sizeof(i)];
    UInt512 I;
    
    if (i & BIP32_HARD) {
        buf[0] = 0;
        UInt256Set(&buf[1], *k);
    } else {
        //ed25519 only supports hardened key paths
        return;
    }
    
    UInt32SetBE(&buf[sizeof(BRECPoint)], i);
    
    BRHMAC(&I, BRSHA512, sizeof(UInt512), c, sizeof(*c), buf, sizeof(buf)); // I = HMAC-SHA512(c, k|P(k) || i)
    
    // Split I into two 32-byte sequences, IL and IR
    *k = *(UInt256 *)&I; // child key = IL
    *c = *(UInt256 *)&I.u8[sizeof(UInt256)]; // chain code = IR
    
    var_clean(&I);
    mem_clean(buf, sizeof(buf));
}

// https://github.com/satoshilabs/slips/blob/master/slip-0010.md
#define ED25519_SEED_KEY "ed25519 seed"
void
ed25519vPrivKeyPath(BRKey *key, const void *seed, size_t seedLen, int depth, va_list vlist)
{
    UInt512 I;
    UInt256 secret, chainCode;
    
    assert(key != NULL);
    assert(seed != NULL || seedLen == 0);
    assert(depth >= 0);
    
    if (key && (seed || seedLen == 0)) {
        BRHMAC(&I, BRSHA512, sizeof(UInt512), ED25519_SEED_KEY, strlen
               (ED25519_SEED_KEY), seed, seedLen);
        secret = *(UInt256 *)&I;
        chainCode = *(UInt256 *)&I.u8[sizeof(UInt256)];
        var_clean(&I);
        
        for (int i = 0; i < depth; i++) {
            _CKDpriv(&secret, &chainCode, va_arg(vlist, uint32_t));
        }
        
        BRKeySetSecret(key, &secret, 1);
        var_clean(&secret, &chainCode);
    }
}
#undef ED25519_SEED_KEY

void
ed25519PrivKeyPath(BRKey * key, const void * seed, size_t seedLen, int depth , ... ){
    va_list ap;

    va_start(ap, depth);
    ed25519vPrivKeyPath(key, seed, seedLen, depth, ap);
    va_end(ap);
}

static BRKey
deriveTezosPrivateKeyFromSeed (UInt512 seed, uint32_t index) {
    BRKey privateKey;
    
    ed25519PrivKeyPath (&privateKey, &seed, sizeof(UInt512), 4,
                        44 | BIP32_HARD,   // purpose  : BIP-44
                        1729 | BIP32_HARD, // coin_type: Tezos
                        0 | BIP32_HARD,    // account
                        index | BIP32_HARD);
    
    return privateKey;
}

void
tezosKeyGetPublicKey (BRKey key, uint8_t * publicKey) {
    unsigned char privateKey[64] = {0};
    ed25519_create_keypair(publicKey, privateKey, key.secret.u8);
    memset(privateKey, 0x00, 64);
}

extern BRKey
tezosAccountGetPublicKey (BRTezosAccount account) {
    assert(account);
    BRKey key;
    memset(&key, 0x00, sizeof(BRKey));
    memcpy(key.pubKey, account->publicKey, TEZOS_PUBLIC_KEY_SIZE);
    return key;
}

extern uint8_t *
tezosAccountGetPublicKeyBytes (BRTezosAccount account, size_t *bytesCount) {
    uint8_t *bytes = malloc (TEZOS_PUBLIC_KEY_SIZE);
    memcpy (bytes, account->publicKey, TEZOS_PUBLIC_KEY_SIZE);
    *bytesCount = TEZOS_PUBLIC_KEY_SIZE;
    return bytes;
}

// MARK: -

extern BRTezosAccount
tezosAccountCreateWithSeed (UInt512 seed) {
    BRTezosAccount account = calloc(1, sizeof(struct BRTezosAccountRecord));
    
    // Private key
    BRKey privateKey = deriveTezosPrivateKeyFromSeed(seed, 0);

    tezosKeyGetPublicKey(privateKey, account->publicKey);

    //TODO:TEZOS set account->address

    return account;
}

extern BRTezosAccount
tezosAccountCreateWithSerialization (uint8_t *bytes, size_t bytesCount)
{
    // TODO:TEZOS
    return NULL;
}
}
