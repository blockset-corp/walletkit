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

#include "BRAvalancheAccount.h"
#include "BRAvalancheAddress.h"

struct BRAvalancheAccountRecord {
    BRAvalancheAddress addresses [NUMBER_OF_AVALANCHE_CHAIN_TYPES];
};

// MARK: - Create / Free

static BRKey
avalancheDerivePrivateKeyFromSeed (UInt512 seed, uint32_t index) {
    BRKey privateKey;

    BRBIP32PrivKeyPath(&privateKey, &seed, sizeof(UInt512), 5, ((const uint32_t []) {
        // 44'/9000'/0'/0/index
        44   | BIP32_HARD,
        9000 | BIP32_HARD,
        0    | BIP32_HARD,
        0,
        index }));

    return privateKey;
}

extern BRAvalancheAccount
avalancheAccountCreateWithSeed (UInt512 seed) {
    BRAvalancheAccount account = calloc(1, sizeof(struct BRAvalancheAccountRecord));
    
    // Private key
    BRKey privateKey = avalancheDerivePrivateKeyFromSeed (seed, 0);

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
}

extern BRAvalancheAccount
avalancheAccountCreateWithSerialization (uint8_t *bytes, size_t bytesCount)
{
    BRAvalancheAccount account = calloc(1, sizeof(struct BRAvalancheAccountRecord));

    BRAvalancheAddressX addressX;
    BRAvalancheAddressC addressC;

    size_t bytesCountX = AVALANCHE_ADDRESS_BYTES_X;
    size_t bytesCountC = AVALANCHE_ADDRESS_BYTES_C;
    assert (bytesCount == bytesCountX + bytesCountC);

    memcpy (addressX.bytes, &bytes[          0], bytesCountX);
    memcpy (addressC.bytes, &bytes[bytesCountX], bytesCountC);

    account->addresses[AVALANCHE_CHAIN_TYPE_X] = ((BRAvalancheAddress) {
        AVALANCHE_CHAIN_TYPE_X,
        { .x = addressX }
    });

    account->addresses[AVALANCHE_CHAIN_TYPE_C] = ((BRAvalancheAddress) {
        AVALANCHE_CHAIN_TYPE_C,
        { .c = addressC }
    });

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
                         UInt512  seed,
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

    size_t bytesCountX = AVALANCHE_ADDRESS_BYTES_X;
    size_t bytesCountC = AVALANCHE_ADDRESS_BYTES_C;

    *bytesCount = bytesCountX + bytesCountC;
    uint8_t *bytes = malloc (*bytesCount);

    // Copy the X and C addresses, back-to-back.
    memcpy (&bytes[          0], account->addresses[AVALANCHE_CHAIN_TYPE_X].u.x.bytes, bytesCountX);
    memcpy (&bytes[bytesCountX], account->addresses[AVALANCHE_CHAIN_TYPE_C].u.c.bytes, bytesCountC);

    return bytes;
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
