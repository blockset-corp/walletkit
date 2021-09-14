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

#include "support/BRInt.h"
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

#define AVALANCHE_PREFIX_STRING     ("\x1A""Avalanche Signed Message:\n")

extern uint8_t *
avalancheAccountCreateStandardMessage (BRAvalancheAccount account,
                                       uint8_t *bytes,
                                       size_t bytesCount,
                                       size_t *messageCount) {
    static const char  *avalanchePrefix      = AVALANCHE_PREFIX_STRING;
    static const size_t avalanchePrefixCount = strlen (AVALANCHE_PREFIX_STRING);

    assert (bytesCount < UINT32_MAX);
    assert (NULL != messageCount);

    size_t offset = 0;

    // https://docs.avax.network/build/references/cryptographic-primitives
    // The encoding is (prefix || len(bytes) || bytes) where `len(bytes)` is a four byte,
    // big-endian representation.
    *messageCount = avalanchePrefixCount + sizeof(uint32_t) + bytesCount;
    uint8_t *message = malloc (*messageCount);
    assert (NULL != message);

    memcpy (&message[offset], avalanchePrefix, avalanchePrefixCount);
    offset += avalanchePrefixCount;

    UInt32SetBE (&message[offset], (uint32_t) bytesCount);
    offset += sizeof(uint32_t);

    memcpy (&message[offset], bytes, bytesCount);

    return message;
#if 0
    char  *bytesPrefix;
    size_t bytesPrefixCount;
    int    bytesPrefixCountOrError = asprintf (&bytesPrefix, "%s%zu", avalanchePrefix, bytesCount);
    if (bytesPrefixCountOrError < 0) { *messageCount = 0; return NULL; }
    bytesPrefixCount = (size_t) bytesPrefixCountOrError;

    size_t   bytesAvalancheCount = bytesPrefixCount + bytesCount;
    uint8_t *bytesAvalanche = malloc (bytesAvalancheCount);

    memcpy (&bytesAvalanche[0],                bytesPrefix, bytesPrefixCount);
    memcpy (&bytesAvalanche[bytesPrefixCount], bytes,       bytesCount);
    free (bytesPrefix);

    *messageCount = bytesAvalancheCount;
    return bytesAvalanche;
#endif
}

extern BRAvalancheSignature
avalancheAccountSignData (BRAvalancheAccount account,
                         uint8_t *bytes,
                         size_t   bytesCount,
                         UInt512  seed) {
    BRAvalancheSignature signature = { 0 };

    // Derive the private key from `seed`
    BRKey keyPrivate = avalancheDerivePrivateKeyFromSeed (seed, /* index */ 0);

    /*
     * https://docs.avax.network/build/references/cryptographic-primitives#secp-256-k1-recoverable-signatures
     *
     * "Recoverable signatures are stored as the 65-byte [R || S || V] where V is 0 or 1 to allow
     * quick public key recoverability. S must be in the lower half of the possible range to
     * prevent signature malleability. Before signing a message, the message is hashed using sha256."
     */
    UInt256 md;
    BRSHA256 (md.u8, bytes, bytesCount);

    size_t  signatureBytesSize = BRKeyCompactSignEthereum (&keyPrivate, NULL, 0, md);   // RSV
    assert (65 == signatureBytesSize);
    assert (65 == sizeof(BRAvalancheSignature));

    BRKeyCompactSignEthereum (&keyPrivate, &signature, sizeof(BRAvalancheSignature), md);
    BRKeyClean(&keyPrivate);

    return signature;
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
