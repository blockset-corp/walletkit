//
//  BRStellarAccount.c
//  WalletKitCore
//
//  Created by Carl Cherry on 5/21/2019.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "support/BRCrypto.h"
#include "support/BRKey.h"
#include "support/BRBIP32Sequence.h"
#include "support/BRBIP39WordsEn.h"
#include "BRStellar.h"
#include "BRStellarBase.h"
#include "BRStellarAccount.h"
#include "utils/base32.h"
#include "ed25519/ed25519.h"
#include "utils/crc16.h"
#include "BRStellarAccountUtils.h"

#define STELLAR_ACCOUNT_MINIMUM   (1000)

struct BRStellarAccountRecord {
    BRStellarAddress address;
    BRStellarAccountID accountID;

    // The public key - needed when sending 
    BRKey publicKey;

    int64_t blockNumberAtCreation;
    int64_t sequence;
    BRStellarNetworkType networkType;
};

extern void stellarAccountSetNetworkType(BRStellarAccount account, BRStellarNetworkType networkType)
{
    // The network type is required when we sign the transaction. The string included in the
    // data to hash must match the network we connect to
    account->networkType = networkType;
}

static BRStellarAccount createStellarAccountObject(BRKey * key)
{
    // Create an initialize a BRStellarAccountRecord object
    BRStellarAccount account = (BRStellarAccount) calloc (1, sizeof (struct BRStellarAccountRecord));

    // Generate the public key from the secret
    unsigned char privateKey[64] = {0};
    unsigned char publicKey[STELLAR_ADDRESS_BYTES] = {0};
    ed25519_create_keypair(publicKey, privateKey, key->secret.u8);
    var_clean(&privateKey); // never leave the private key in memory
    var_clean(&key);
    memcpy(&account->publicKey.pubKey[0], &publicKey[0], STELLAR_ADDRESS_BYTES);
    account->networkType = STELLAR_NETWORK_PUBLIC;
    account->accountID.accountType = PUBLIC_KEY_TYPE_ED25519;
    memcpy(account->accountID.accountID, publicKey, STELLAR_ADDRESS_BYTES);
    // The address is the public key
    account->address = stellarAddressCreate(&account->publicKey);
    return account;
}

// Create an account from the paper key
extern BRStellarAccount stellarAccountCreate (const char *paperKey)
{
    BRKey key = createStellarKeyFromPaperKey(paperKey);
    return createStellarAccountObject(&key);
}

// Create an account object with the seed
extern BRStellarAccount stellarAccountCreateWithSeed(UInt512 seed)
{
    BRKey key = createStellarKeyFromSeed(seed);
    return createStellarAccountObject(&key);
}

// Create an account object using the key
extern BRStellarAccount stellarAccountCreateWithKey(BRKey key)
{
    // NOTE: since this is a public function that passes in a copy
    // of the key/secret it is up to the caller to wipe the secret from memory
    return createStellarAccountObject(&key);
}

extern BRStellarAccount
stellarAccountCreateWithSerialization (uint8_t *bytes, size_t bytesCount)
{
    // We only support the ed25519 keyp pair
    if (!bytes || bytesCount != STELLAR_ADDRESS_BYTES) {
        return NULL;
    }
    BRStellarAccount account = (BRStellarAccount) calloc (1, sizeof (struct BRStellarAccountRecord));
    memcpy(&account->publicKey.pubKey[0], bytes, STELLAR_ADDRESS_BYTES);
    account->networkType = STELLAR_NETWORK_PUBLIC;
    account->accountID.accountType = PUBLIC_KEY_TYPE_ED25519;
    memcpy(account->accountID.accountID, bytes, STELLAR_ADDRESS_BYTES);
    // The address is the public key
    account->address = stellarAddressCreate(&account->publicKey);

    return account;
}

extern uint8_t * // Caller owns memory and must delete calling "free"
stellarAccountGetSerialization (BRStellarAccount account, size_t *bytesCount)
{
    assert (NULL != bytesCount);
    assert (NULL != account);

    *bytesCount = STELLAR_ADDRESS_BYTES;
    uint8_t *bytes = calloc (1, STELLAR_ADDRESS_BYTES);
    memcpy(bytes, account->accountID.accountID, STELLAR_ADDRESS_BYTES);
    return bytes;
}

extern BRStellarAddress stellarAccountGetAddress(BRStellarAccount account)
{
    assert(account);
    // The account object should already have a public key - so generate the
    // stellar address from the public key.
    return stellarAddressClone(account->address);
}

extern BRStellarAccountID stellarAccountGetAccountID(BRStellarAccount account)
{
    assert(account);
    return account->accountID;
}

extern BRKey stellarAccountGetPublicKey(BRStellarAccount account)
{
    // The accounts BRKey object should NEVER have the secret but zero it out just in case
    account->publicKey.secret = UINT256_ZERO;
    return account->publicKey;
}

extern void stellarAccountFree(BRStellarAccount account)
{
    assert(account);
    assert(account->address);
    stellarAddressFree(account->address);
    free(account);
}

extern BRStellarAddress stellarAccountGetPrimaryAddress (BRStellarAccount account)
{
    // Currently we only have the primary address - so just return it
    return stellarAccountGetAddress(account);
}

// Private function implemented in BRStellarTransaction.c
extern size_t
stellarTransactionSerializeAndSign(BRStellarTransaction transaction, uint8_t *privateKey,
                                   uint8_t *publicKey, int64_t sequence, BRStellarNetworkType networkType);

extern size_t
stellarAccountSignTransaction(BRStellarAccount account, BRStellarTransaction transaction, UInt512 seed)
{
    assert(account);
    assert(transaction);

    BRKey key = deriveStellarKeyFromSeed(seed, 0);
    unsigned char privateKey[64] = {0};
    unsigned char publicKey[32] = {0};
    ed25519_create_keypair(publicKey, privateKey, key.secret.u8);

    // Update the sequence number for this request
    int64_t sequence = (account->blockNumberAtCreation << 32) + account->sequence + 1;

    // Send it off to the transaction code to serialize and sign since we don't know
    // the internal details of a transaction
    size_t tx_size = stellarTransactionSerializeAndSign(transaction, privateKey,
                                                        publicKey, sequence,
                                                        account->networkType);

    if (tx_size > 0) {
        account->sequence++;
    }

    return tx_size;
}

extern void stellarAccountSetSequence(BRStellarAccount account, int64_t sequence)
{
    assert(account);
    // The sequence is very important as it must be 1 greater than the previous
    // transaction sequence.
    account->sequence = sequence;
}

extern void stellarAccountSetBlockNumberAtCreation(BRStellarAccount account, uint64_t blockNumber)
{
    assert(account);
    // The sequence is very important as it must be 1 greater than the previous
    // transaction sequence.
    // NOTE 1 - the sequence is created like this: blockNumberAtCreation << 32, also note that
    // the sequence is an int64_t and block number is also int64_t so at some point in the future
    // the block number might get very large and break things, but for now the block number must be
    // small enough to not overrun
    // So we will cast to int64_t for now
    account->blockNumberAtCreation = (int64_t)blockNumber;
}

extern BRStellarAccountID stellerAccountCreateStellarAccountID(const char * stellarAddress)
{
    BRStellarAccountID accountID = createStellarAccountIDFromStellarAddress(stellarAddress);
    return accountID;
}

extern BRStellarAmount
stellarAccountGetBalanceLimit (BRStellarAccount account,
                            int asMaximum,
                            int *hasLimit)
{
    assert (NULL != hasLimit);

    *hasLimit = !asMaximum;
    return asMaximum ? 0 : STELLAR_ACCOUNT_MINIMUM;
}

extern BRStellarFeeBasis
stellarAccountGetDefaultFeeBasis (BRStellarAccount account) {
    return (BRStellarFeeBasis) {
        100.0, 1
    };
}

extern int
stellarAccountHasAddress (BRStellarAccount account,
                         BRStellarAddress address) {
    return stellarAddressEqual (account->address, address);
}

