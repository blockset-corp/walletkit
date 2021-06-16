//
//  WKAccount.c
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "support/BROSCompat.h"
#include "WKHandlersP.h"
#include "WKAccountP.h"
#include <string.h>

#include "litecoin/BRLitecoinParams.h"
#include "dogecoin/BRDogecoinParams.h"
#include "support/BRBIP32Sequence.h"
#include "support/BRBIP39Mnemonic.h"

static pthread_once_t  _accounts_once = PTHREAD_ONCE_INIT;

static void _accounts_init (void) {
//    genHandlersInstall (genericRippleHandlers);
//    genHandlersInstall (genericHederaHandlers);
    // ...
}

private_extern void
wkAccountInstall (void) {
    pthread_once (&_accounts_once, _accounts_init);
}

static uint16_t
checksumFletcher16 (const uint8_t *data, size_t count);

// Version 1: BTC (w/ BCH), ETH
// Version 2: BTC (w/ BCH), ETH, XRP
// Version 3: V2 + HBAR
// Version 4: V3 + XTZ
// Version 5: V4 + LTC, DOGE, XLM
// Version 6: V5 + serialization of BCH and BSV with their own keys
#define ACCOUNT_SERIALIZE_DEFAULT_VERSION  6

IMPLEMENT_WK_GIVE_TAKE (WKAccount, wkAccount);

static UInt512
wkAccountDeriveSeedInternal (const char *phrase) {
    UInt512 seed;
    BRBIP39DeriveKey (seed.u8, phrase, NULL);
    return seed;
}

extern UInt512
wkAccountDeriveSeed (const char *phrase) {
    return wkAccountDeriveSeedInternal(phrase);
}

extern char *
wkAccountGeneratePaperKey (const char *words[]) {
    UInt128 entropy;
    arc4random_buf_brd (entropy.u8, sizeof(entropy));

    size_t phraseLen = BRBIP39Encode (NULL, 0, words, entropy.u8, sizeof(entropy));
    char  *phrase    = calloc (phraseLen, 1);

    // xor to avoid needing an additional variable to perform assert
    phraseLen ^= BRBIP39Encode (phrase, phraseLen, words, entropy.u8, sizeof(entropy));
    assert (0 == phraseLen);

    return phrase;
}

extern WKBoolean
wkAccountValidatePaperKey (const char *phrase, const char *words[]) {
    return AS_WK_BOOLEAN (BRBIP39PhraseIsValid (words, phrase));
}

extern WKBoolean
wkAccountValidateWordsList (size_t wordsCount) {
    return AS_WK_BOOLEAN (wordsCount == BIP39_WORDLIST_COUNT);
}

static WKAccount
wkAccountCreateInternal (WKTimestamp timestamp,
                         const char *uids) {
    WKAccount account = malloc (sizeof (struct WKAccountRecord));

    account->uids = strdup (uids);
    account->timestamp = timestamp;
    account->ref = WK_REF_ASSIGN(wkAccountRelease);

    return account;
}

static WKAccount
wkAccountCreateFromSeedInternal (UInt512 seed,
                                 WKTimestamp timestamp,
                                 const char *uids) {

    const WKHandlers        *netHandlers;
    const WKAccountHandlers *acctHandlers;
    WKAccount               acct;

    acct = wkAccountCreateInternal(timestamp, uids);
    assert (acct != NULL);

    for (WKNetworkType netNo = WK_NETWORK_TYPE_BTC;
         netNo < NUMBER_OF_NETWORK_TYPES;
         netNo++                            ) {

        netHandlers = wkHandlersLookup(netNo);
        acctHandlers = netHandlers->account;
        acct->networkAccounts[netNo] = acctHandlers->createFromSeed(seed);
    }

    return acct;
}

extern WKAccount
wkAccountCreate (const char *phrase,
                 WKTimestamp timestamp,
                 const char *uids) {
    wkAccountInstall();

    return wkAccountCreateFromSeedInternal (wkAccountDeriveSeedInternal(phrase), timestamp, uids);
}

/**
 * Deserialize into an Account.  The serialization format is:
 *  <checksum16><size32><version>
 *      <BTC size><BTC master public key>
 *      <ETH size><ETH public key>
 *      <XRP size><XRP public key>
 *
 * @param bytes the serialized bytes
 * @param bytesCount the number of serialized bytes
 *
 * @return An Account, or NULL.
 */
extern WKAccount
wkAccountCreateFromSerialization (
    const uint8_t   *bytes,
    size_t          bytesCount,
    const char      *uids       ) {

    const WKHandlers        *netHandlers;
    const WKAccountHandlers *acctHandlers;
    WKAccount               acct = NULL;

    wkAccountInstall();

    uint8_t *bytesPtr = (uint8_t *) bytes;
    uint8_t *bytesEnd = bytesPtr + bytesCount;

#define BYTES_PTR_INCR_AND_CHECK(size)      \
    do {                                    \
        bytesPtr += (size);                 \
        if (bytesPtr > bytesEnd) {          \
            free (acct);                    \
            return NULL; /* overkill */     \
        }                                   \
    } while (0)

    size_t chkSize = sizeof (uint16_t); // checksum
    size_t szSize  = sizeof (uint32_t); // size
    size_t verSize = sizeof (uint16_t); // version
    size_t tsSize  = sizeof (uint64_t); // timestamp - read as uint64_t

    // Demand at least <checksum16><size32> in `bytes`
    if (bytesCount < (chkSize + szSize)) return NULL;

    // Checksum
    uint16_t checksum = UInt16GetBE(bytesPtr);
    bytesPtr += chkSize;

    // Confirm checksum, otherwise done
    if (checksum != checksumFletcher16 (&bytes[chkSize], (bytesCount - chkSize))) return NULL;

    // Size
    uint32_t size = UInt32GetBE(bytesPtr);
    bytesPtr += szSize;

    if (size != bytesCount) return NULL;

    // Version
    uint16_t version = UInt16GetBE (bytesPtr);
    BYTES_PTR_INCR_AND_CHECK(verSize);

    // Require the current verion, otherwise done.  Will force account create using
    // `wkAccountCreate()` and a re-serialization
    if (ACCOUNT_SERIALIZE_DEFAULT_VERSION != version) return NULL;

    // Timestamp - read as uint64_t
    uint64_t timestamp = UInt64GetBE (bytesPtr);
    BYTES_PTR_INCR_AND_CHECK (tsSize);

<<<<<<< HEAD
    acct = wkAccountCreateInternal(AS_WK_TIMESTAMP (timestamp),
                                   uids);
    assert (acct != NULL);

    // Deserialize per network
    for (WKNetworkType netNo = WK_NETWORK_TYPE_BTC;
         netNo < NUMBER_OF_NETWORK_TYPES;
         netNo++                            ) {

        netHandlers = wkHandlersLookup(netNo);

        // Get network account len and check available buffer
        size_t mpkSize = UInt32GetBE(bytesPtr);
        BYTES_PTR_INCR_AND_CHECK (szSize);

        // Recreate the network account from available bytes of indicated
        // mpkSize
        acctHandlers = netHandlers->account;
        acct->networkAccounts[netNo] = acctHandlers->createFromBytes(bytesPtr,
                                                                     mpkSize    );

        BYTES_PTR_INCR_AND_CHECK (mpkSize);
    }

    return acct;
}
#undef BYTES_PTR_INCR_AND_CHECK

static void
wkAccountRelease (WKAccount account) {

    const WKHandlers *netHandlers;

    // Release per network
    for (WKNetworkType netNo = WK_NETWORK_TYPE_BTC;
         netNo < NUMBER_OF_NETWORK_TYPES;
         netNo++                            ) {

        netHandlers = wkHandlersLookup(netNo);
        netHandlers->account->release(account->networkAccounts[netNo]);
    }

    free (account->uids);
    memset (account, 0, sizeof(*account));
    free (account);
}


/**
 * Serialize the account as per ACCOUNT_SERIALIZE_DEFAULT_VERSION.  The serialization format is:
 *  <checksum16><size32><version>
 *      <BTC size><BTC master public key>
 *      <ETH size><ETH public key>
 *      <XRP size><XRP public key>
 *
 * @param account The account
 * @param bytesCount A non-NULL size_t pointer filled with the bytes count
 *
 * @return The serialization as uint8_t*
 */
extern uint8_t *
wkAccountSerialize (WKAccount account, size_t *bytesCount) {
    assert (NULL != bytesCount);

    const WKHandlers  *netHandlers;
    size_t            acctSerSize;

    size_t chkSize = sizeof (uint16_t); // checksum
    size_t szSize  = sizeof (uint32_t); // size
    size_t verSize = sizeof (uint16_t); // version
    size_t tsSize  = sizeof (uint64_t); // timestamp - written as uint64_t

    // Version
    uint16_t version = ACCOUNT_SERIALIZE_DEFAULT_VERSION;

    // Overall size - summing all factors.
    *bytesCount = (chkSize + szSize + verSize + tsSize);
    for (WKNetworkType netNo = WK_NETWORK_TYPE_BTC;
         netNo < NUMBER_OF_NETWORK_TYPES;
         netNo++                            ) {

        netHandlers = wkHandlersLookup(netNo);
        size_t serSize = netHandlers->account->serialize(NULL, account);

        if (serSize != 0) {
            // Space for account serialization length & serialization itself
            *bytesCount += szSize;
            *bytesCount += serSize;
        }
    }

    uint8_t *bytes = calloc (1, *bytesCount);
    uint8_t *bytesPtr = bytes;

    // Encode

    // Skip the checksum; will comeback to it
    bytesPtr += chkSize;

    // Size
    UInt32SetBE (bytesPtr, (uint32_t) *bytesCount);
    bytesPtr += szSize;

    // Version
    UInt16SetBE (bytesPtr, version);
    bytesPtr += verSize;

    // timestamp - written as uint64_t
    UInt64SetBE (bytesPtr, account->timestamp);
    bytesPtr += tsSize;

    for (WKNetworkType netNo = WK_NETWORK_TYPE_BTC;
         netNo < NUMBER_OF_NETWORK_TYPES;
         netNo++                            ) {

        netHandlers = wkHandlersLookup(netNo);

        // Skip size field until its known
        bytesPtr += szSize;

        // Write account specific serialization into ser buffer
        acctSerSize = netHandlers->account->serialize(bytesPtr, account);

        // Backpatch account serial size & prep for next account
        UInt32SetBE((bytesPtr - szSize), (uint32_t) acctSerSize);
        bytesPtr += acctSerSize;
    }

    // Avoid static analysis warning
    (void) bytesPtr;

    // checksum
    uint16_t checksum = checksumFletcher16 (&bytes[chkSize], (*bytesCount - chkSize));
    UInt16SetBE (bytes, checksum);

    return bytes;
}

extern WKBoolean
wkAccountValidateSerialization (WKAccount account,
                                const uint8_t *bytes,
                                size_t bytesCount) {

    uint8_t *bytesPtr = (uint8_t *) bytes;
    uint8_t *bytesEnd = bytesPtr + bytesCount;

    size_t chkSize = sizeof (uint16_t); // checksum
    size_t szSize  = sizeof (uint32_t); // size
    size_t verSize = sizeof (uint16_t); // version
    size_t tsSize  = sizeof (uint64_t); // timestamp - as uint64_t

    // Skip directly to the BTC MPK
    bytesPtr += (chkSize + szSize + verSize + tsSize);
    if (bytesPtr + szSize > bytesEnd) return WK_FALSE;

    // BTC
    size_t mpkSize = UInt32GetBE(bytesPtr);
    bytesPtr += szSize;
    // Not enough bytes
    if (bytesPtr + mpkSize > bytesEnd) return WK_FALSE;

    // We'll check thsee bytes
    uint8_t *mpkBytesToCheck = bytesPtr;

    // Generate a serialization from account->btc
    BRMasterPubKey* pubKey = (BRMasterPubKey*) wkAccountAs ( account,
                                                             WK_NETWORK_TYPE_BTC);
    size_t mpkBytesCount = BRBIP32SerializeMasterPubKey (NULL,
                                                         *pubKey);
    uint8_t mpkBytes[mpkBytesCount];
    BRBIP32SerializeMasterPubKey ((char *) mpkBytes,
                                  *pubKey);

    if (mpkSize != mpkBytesCount) return WK_FALSE;

    return AS_WK_BOOLEAN (0 == memcmp (mpkBytesToCheck, mpkBytes, mpkBytesCount));
}

extern WKTimestamp
wkAccountGetTimestamp (WKAccount account) {
    return account->timestamp;
}

extern char *
wkAccountGetFileSystemIdentifier (WKAccount account) {
    // Seriailize the master public key
    BRMasterPubKey* pubKey = (BRMasterPubKey*) wkAccountAs ( account,
                                                             WK_NETWORK_TYPE_BTC);
    size_t   mpkSize  = BRBIP32SerializeMasterPubKey (NULL,
                                                      *pubKey);
    uint8_t *mpkBytes = malloc (mpkSize);
    BRBIP32SerializeMasterPubKey ((char*) mpkBytes,
                                  *pubKey);

    // Double SHA the serialization
    UInt256 hash;
    BRSHA256_2(&hash, mpkBytes, mpkSize);
    free (mpkBytes);

    // Take the first 32 characters.
    return strndup(u256hex(hash), 32);
}

extern const char *
wkAccountGetUids (WKAccount account) {
    return account->uids;
}

// https://en.wikipedia.org/wiki/Fletcher%27s_checksum
static uint16_t
checksumFletcher16(const uint8_t *data, size_t count )
{
    uint16_t sum1 = 0;
    uint16_t sum2 = 0;
    int index;

    for( index = 0; index < count; ++index )
    {
        sum1 = (sum1 + data[index]) % 255;
        sum2 = (sum2 + sum1) % 255;
    }
    return (sum2 << 8) | sum1;
}
