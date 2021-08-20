//
//  BRAvalancheAccount.c
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include <stdlib.h>
#include <assert.h>

#include "support/BRBIP32Sequence.h"
#include "ed25519/ed25519.h"

#include "BRAvalancheAccount.h"
#include "BRAvalancheAddress.h"

// #include "blake2/blake2b.h"

struct BRAvalancheAccountRecord {
    BRAvalancheAddress addresses [NUMBER_OF_AVALANCHE_CHAIN_TYPES];
};

// MARK: Forward Declarations

static BRKey
avalancheDerivePrivateKeyFromSeed (UInt512 seed, uint32_t index);

static size_t
avalancheKeyGetPublicKey (BRKey key, uint8_t * publicKey, size_t publicKeyLen, bool compresses);

#if 0
static void
avalancheKeyGetPrivateKey (BRKey key, uint8_t * privateKey);
#endif

// MARK: - Init/Free

extern BRAvalancheAccount
avalancheAccountCreateWithSeed (UInt512 seed) {
    BRAvalancheAccount account = calloc(1, sizeof(struct BRAvalancheAccountRecord));
    
    // Private key
    BRKey privateKey = avalancheDerivePrivateKeyFromSeed(seed, 0);

    // The X privateKey is compresses; the C privateKey is uncompressed
    BRKey privateKeyX = privateKey; privateKeyX.compressed = 1; BRKeyPubKey(&privateKeyX, NULL, 0);
    BRKey privateKeyC = privateKey; privateKeyC.compressed = 0; BRKeyPubKey(&privateKeyC, NULL, 0);

    BRKeyClean (&privateKey);
    
    // Determine the X and C addresses
    account->addresses[AVALANCHE_CHAIN_TYPE_X] = avalancheAddressCreateFromKey (privateKeyX.pubKey, 33, AVALANCHE_CHAIN_TYPE_X);
    account->addresses[AVALANCHE_CHAIN_TYPE_C] = avalancheAddressCreateFromKey (privateKeyC.pubKey, 65, AVALANCHE_CHAIN_TYPE_C);

    BRKeyClean(&privateKeyX);
    BRKeyClean(&privateKeyC);

    return account;

#if 0
    privateKey.compressed=0;
    BRKeyPubKey(&privateKey, &(privateKey.pubKey) , 65);
    //we need uncompressed pubkey to generate address
    account->caddress = ethAddressCreateKey(&privateKey);
    //zero out the pubkey so we can reuse
    memset(&privateKey.pubKey, 0,65);

    //Avalanche requires compressed pub key
    privateKey.compressed=1;
    BRKeyPubKey(&privateKey, &privateKey.pubKey, 33);
    account->xaddress = avalancheAddressCreateFromKey(&privateKey.pubKey, 33);

    //memcpy(&(account->caddress), ethAddress.bytes, sizeof(ethAddress.bytes));
    //cleanup the privateKey
    BRKeyClean(&privateKey);
    return account;
#endif
}

extern BRAvalancheAccount
avalancheAccountCreateWithSerialization (uint8_t *bytes, size_t bytesCount)
{
    BRAvalancheAccount account = calloc(1, sizeof(struct BRAvalancheAccountRecord));

#if 0
    assert (bytesCount == AVALANCHE_PUBLIC_KEY_SIZE);

    memcpy(account->publicKey, bytes, AVALANCHE_PUBLIC_KEY_SIZE);
    account->addresses[AVALANCHE_CHAIN_TYPE_X] = avalancheAddressCreateFromKey(account->publicKey, AVALANCHE_PUBLIC_KEY_SIZE, AVALANCHE_CHAIN_TYPE_X);
    account->addresses[AVALANCHE_CHAIN_TYPE_C] = avalancheAddressCreateFromKey(account->publicKey, AVALANCHE_PUBLIC_KEY_SIZE, AVALANCHE_CHAIN_TYPE_C);
#endif
    return account;
}

extern void
avalancheAccountFree (BRAvalancheAccount account)
{
    assert (account);
    free (account);
}

// MARK: - Signing

extern OwnershipGiven uint8_t *
avalancheAccountSignData (BRAvalancheAccount account,
                         uint8_t *bytes,
                         size_t   bytesCount,
                         UInt512 seed,
                         size_t  *count) {

#if 0
    BRKey keyPublic  = avalancheAccountGetPublicKey (account);
#endif
    BRKey keyPrivate = avalancheDerivePrivateKeyFromSeed (seed, /* index */ 0);


#if 0
    uint8_t privateKeyBytes[64];
    avalancheKeyGetPrivateKey(privateKey, privateKeyBytes);
    
    uint8_t watermark[] = { 0x03 };
    size_t watermarkSize = sizeof(watermark);
    
    WKData watermarkedData = wkDataNew(data.size + watermarkSize);
    
    memcpy(watermarkedData.bytes, watermark, watermarkSize);
    memcpy(&watermarkedData.bytes[watermarkSize], data.bytes, data.size);
    
    uint8_t hash[32];
    blake2b(hash, sizeof(hash), NULL, 0, watermarkedData.bytes, watermarkedData.size);
    
    WKData signature = wkDataNew(64);
    ed25519_sign(signature.bytes, hash, sizeof(hash), publicKey.pubKey, privateKeyBytes);
    
    mem_clean(privateKeyBytes, 64);
    wkDataFree(watermarkedData);
    
    return signature;
#endif

    ASSERT_UNIMPLEMENTED; (void) keyPrivate;

    *count = 0;
    return NULL;
}

// MARK: - Accessors

#if 0
extern BRKey
avalancheAccountGetPublicKey (BRAvalancheAccount account) {
    assert(account);
    BRKey key;
    memset(&key, 0x00, sizeof(BRKey));
    memcpy(key.pubKey, account->publicKey, AVALANCHE_PUBLIC_KEY_SIZE);
    return key;
}
#endif

extern BRAvalancheAddress
avalancheAccountGetAddress (BRAvalancheAccount account,
                            BRAvalancheChainType type)
{
    assert(account);
    return account->addresses[type];
}

extern uint8_t *
avalancheAccountGetSerialization (BRAvalancheAccount account, size_t *bytesCount) {
    assert (NULL != bytesCount);
    assert (NULL != account);

    ASSERT_UNIMPLEMENTED;

#if 0
    // If just the public key
    *bytesCount = AVALANCHE_PUBLIC_KEY_SIZE;
    uint8_t *bytes = calloc (1, *bytesCount);
    
    // Copy the public key
    memcpy(bytes, account->publicKey, AVALANCHE_PUBLIC_KEY_SIZE);
    
    return bytes;
#endif
    *bytesCount = 0;
    return NULL;
}

extern int
avalancheAccountHasAddress (BRAvalancheAccount account,
                            BRAvalancheAddress address) {
    assert(account);
    return avalancheAddressEqual (account->addresses[address.type], address);
}

extern BRAvalancheAmount
avalancheAccountGetBalanceLimit (BRAvalancheAccount account,
                             int asMaximum,
                             int *hasLimit) {
    assert (NULL != hasLimit);
    *hasLimit = 0;
    return 0;
}

// MARK: - Keys


//44'/9000'/0'/0/0
#define AVAX_BIP32_CHILD ((const uint32_t []){ 44 | BIP32_HARD, 9000 | BIP32_HARD, 0 | BIP32_HARD, 0, 0 })
#define AVAX_BIP32_DEPTH 5

#define AVAX_PUBKEY_LENGTH 33

static BRKey
avalancheDerivePrivateKeyFromSeed (UInt512 seed, uint32_t index) {
    BRKey privateKey;
    BRBIP32PrivKeyPath(&privateKey, &seed, sizeof(UInt512), AVAX_BIP32_DEPTH, AVAX_BIP32_CHILD);
    return privateKey;
}

static size_t
avalancheKeyGetPublicKey (BRKey key, uint8_t * publicKey, size_t publicKeyLen, bool compressed) {
    key.compressed = compressed;
    return BRKeyPubKey (&key, publicKey, publicKeyLen);
}

#if 0
static size_t
avalancheKeyGetPrivateKey (BRKey key, uint8_t * privateKey, size_t privateKeyLen) {
    unsigned char publicKey[32] = {0};
    ed25519_create_keypair(publicKey, privateKey, key.secret.u8);
}
#endif
