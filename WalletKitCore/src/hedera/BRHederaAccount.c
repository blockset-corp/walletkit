//
//  BRHederaAccount.c
//  Core
//
//  Created by Carl Cherry on Oct. 15, 2019.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRHederaAccount.h"
#include "BRHederaCrypto.h"

#include <stdlib.h>
#include <assert.h>
#include "support/BRArray.h"

#define HEDERA_EXCHANGE_RATE                     0.03  // 1 Hbar == 0.03 USD (3 US Cents)
#define HEDERA_WALLET_CREATE_FEE_TINY_BAR       (0.01     * HEDERA_HBAR_SCALE_FACTOR / HEDERA_EXCHANGE_RATE)  // 1.0    US Cents
#define HEDERA_WALLET_QUARTERY_FEE_HBAR         (0.000002 * HEDERA_HBAR_SCALE_FACTOR / HEDERA_EXCHANGE_RATE)  // 0.0002 US Cents
#define HEDERA_WALLET_MINIMUM_BALANCE_TINY_BAR  (HEDERA_HBAR_SCALE_FACTOR / 10) // 0.1 HBAR

// Our ed25519 implementation is simple and only support the raw
// 64-byte private key and 32-byte public key. We have no need to
// convert them to any other format internally.
#define HEDERA_PRIVATE_KEY_SIZE 64
#define HEDERA_PUBLIC_KEY_SIZE 32

struct BRHederaAccountRecord {
    BRHederaAddress address;
    uint8_t publicKey[HEDERA_PUBLIC_KEY_SIZE];
    BRArrayOf(BRHederaAddress) nodes;
    BRHederaFeeBasis feeBasis;

};

static BRHederaAccount
hederaAccountCreate (BRHederaAddress address) {
    BRHederaAccount account = calloc(1, sizeof(struct BRHederaAccountRecord));

    account->address = address;

    // TODO - do we just hard code Hedera nodes here - we probably can for now
    // but perhaps in the future there will be additional shards/realms
    /*
     {"0.0.3":"104.196.1.78:50211", "0.0.4": "35.245.250.134:50211",
     "0.0.5":"34.68.209.35:50211", "0.0.6": "34.82.173.33:50211",
     "0.0.7":"35.200.105.230:50211", "0.0.8": "35.203.87.206:50211",
     "0.0.9":"35.189.221.159:50211", "0.0.10": "35.234.104.86:50211",
     "0.0.11":"34.90.238.202:50211", "0.0.12": "35.228.11.53:50211",
     "0.0.13":"35.234.132.107:50211", "0.0.14": "34.94.67.202:50211",
     "0.0.15":"35.236.2.27:50211"}
     */
    array_new (account->nodes, HEDERA_NODE_COUNT);
    // int64_t hedera_node_start = HEDERA_NODE_START;
    for (int i = HEDERA_NODE_START; i < (HEDERA_NODE_COUNT + HEDERA_NODE_START); i++) {
        array_add (account->nodes, hederaAddressCreate(0, 0, i));
    }

    // Add a default fee basis
    account->feeBasis = (BRHederaFeeBasis) { 500000, 1 };

    return account;
}

extern BRHederaAccount hederaAccountCreateWithSeed (UInt512 seed)
{
    BRHederaAccount account = hederaAccountCreate(hederaAddressCreate (0, 0, 0));

    // Generate the secret from the seed
    BRKey privateKey = hederaKeyCreate(seed);

    // From the secret get the public key
    hederaKeyGetPublicKey(privateKey, account->publicKey);

    return account;
}

extern BRHederaAccount
hederaAccountCreateWithSerialization (uint8_t *bytes, size_t bytesCount)
{
    assert(bytesCount == HEDERA_PUBLIC_KEY_SIZE + HEDERA_ADDRESS_SERIALIZED_SIZE);

    // The first 24 bytes will be the account address
    uint8_t addressBytes[HEDERA_ADDRESS_SERIALIZED_SIZE];
    memcpy(addressBytes, bytes, HEDERA_ADDRESS_SERIALIZED_SIZE);
    int64_t shard, realm, accountNum;
    memcpy(&shard, addressBytes, 8);
    memcpy(&realm, addressBytes + 8, 8);
    memcpy(&accountNum, addressBytes + 16, 8);

    BRHederaAccount account = hederaAccountCreate(hederaAddressCreate ((BRHederaAddressComponentType) ntohll(shard),
                                                                       (BRHederaAddressComponentType) ntohll(realm),
                                                                       (BRHederaAddressComponentType) ntohll(accountNum)));

    // Now copy the public key
    memcpy(account->publicKey, bytes + HEDERA_ADDRESS_SERIALIZED_SIZE, HEDERA_PUBLIC_KEY_SIZE);

    return account;
}

extern void hederaAccountFree (BRHederaAccount account)
{
    assert(account && account->address);
    hederaAddressFree (account->address);

    for (int i = 0; i < array_count(account->nodes); i++) {
        hederaAddressFree(account->nodes[i]);
    }
    array_free(account->nodes);

    free(account);
}

extern void hederaAccountSetAddress (BRHederaAccount account, BRHederaAddress address)
{
    assert(account);
    assert(address);
    hederaAddressFree(account->address);
    account->address = hederaAddressClone (address);
}

extern BRKey hederaAccountGetPublicKey (BRHederaAccount account)
{
    assert(account);
    // TODO - we need to extend the BRKey to support other key types or ???
    BRKey key;
    memset(&key, 0x00, sizeof(BRKey));
    memcpy(key.pubKey, account->publicKey, HEDERA_PUBLIC_KEY_SIZE);
    return key;
}

extern uint8_t *
hederaAccountGetPublicKeyBytes (BRHederaAccount account, size_t *bytesCount) {
    uint8_t *bytes = malloc (HEDERA_PUBLIC_KEY_SIZE);
    memcpy (bytes, account->publicKey, HEDERA_PUBLIC_KEY_SIZE);
    *bytesCount = HEDERA_PUBLIC_KEY_SIZE;
    return bytes;
}

extern BRHederaAddress hederaAccountGetAddress (BRHederaAccount account)
{
    assert(account);
    assert(account->address);
    return hederaAddressClone (account->address);
}

extern BRHederaAddress hederaAccountGetPrimaryAddress (BRHederaAccount account)
{
    return hederaAccountGetAddress(account);
}

extern int hederaAccountHasPrimaryAddress (BRHederaAccount account) {
    return !hederaAddressIsUninitializedAddress (account->address);
}

extern uint8_t *hederaAccountGetSerialization (BRHederaAccount account, size_t *bytesCount) {
    assert (NULL != bytesCount);
    assert (NULL != account);
    assert (NULL != account->address);

    // Get the sizes of what we are storing and allocate storage
    uint8_t addressBuffer[HEDERA_ADDRESS_SERIALIZED_SIZE] = {0};
    hederaAddressSerialize (account->address, addressBuffer, HEDERA_ADDRESS_SERIALIZED_SIZE);

    *bytesCount = HEDERA_ADDRESS_SERIALIZED_SIZE + HEDERA_PUBLIC_KEY_SIZE;
    uint8_t *bytes = calloc (1, *bytesCount);

    // Copy the serialized address
    memcpy(bytes, addressBuffer, HEDERA_ADDRESS_SERIALIZED_SIZE);

    // Copy the public key
    memcpy(bytes + HEDERA_ADDRESS_SERIALIZED_SIZE, account->publicKey, HEDERA_PUBLIC_KEY_SIZE);

    return bytes;
}

extern int
hederaAccountHasAddress (BRHederaAccount account,
                         BRHederaAddress address) {
    assert(account);
    assert(address);
    assert(account->address);
    return hederaAddressEqual (account->address, address);
}


extern BRHederaUnitTinyBar
hederaAccountGetBalanceLimit (BRHederaAccount account,
                              int asMaximum,
                              int *hasLimit) {
    assert (NULL != hasLimit);
    
    *hasLimit = !asMaximum;
    return (asMaximum ? 0 : HEDERA_WALLET_MINIMUM_BALANCE_TINY_BAR);
}

extern BRHederaFeeBasis
hederaAccountGetDefaultFeeBasis (BRHederaAccount account) {
    return account->feeBasis;
}

extern BRHederaAddress
hederaAccountGetNodeAddress(BRHederaAccount account)
{
    static unsigned index = 0;
    assert(account);
    if (index > HEDERA_NODE_COUNT - 1) {
        index = 0;
    }

    return hederaAddressClone(account->nodes[index++]);
}

