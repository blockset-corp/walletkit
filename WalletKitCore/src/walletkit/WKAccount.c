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
#include "WKAccountP.h"
#include "WKNetworkP.h"

#include "litecoin/BRLitecoinParams.h"
#include "dogecoin/BRDogecoinParams.h"

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
#define ACCOUNT_SERIALIZE_DEFAULT_VERSION  5

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
wkAccountCreateInternal (BRMasterPubKey btc,
                         BRMasterPubKey ltc,
                         BRMasterPubKey doge,
                         BREthereumAccount eth,
                         BRRippleAccount xrp,
                         BRHederaAccount hbar,
                         BRTezosAccount xtz,
                         BRStellarAccount xlm,
                         WKTimestamp timestamp,
                         const char *uids) {
    WKAccount account = malloc (sizeof (struct WKAccountRecord));

    account->btc = btc;
    account->ltc = ltc;
    account->doge = doge;
    account->eth = eth;
    account->xrp = xrp;
    account->hbar = hbar;
    account->xtz = xtz;
    account->xlm = xlm;
    account->uids = strdup (uids);
    account->timestamp = timestamp;
    account->ref = WK_REF_ASSIGN(wkAccountRelease);

    return account;
}

static WKAccount
wkAccountCreateFromSeedInternal (UInt512 seed,
                                 WKTimestamp timestamp,
                                 const char *uids) {
    return wkAccountCreateInternal (BRBIP32MasterPubKey (seed.u8, sizeof (seed.u8)),
                                    BRBIP32MasterPubKeyPath(seed.u8, sizeof(seed.u8), LTC_BIP32_DEPTH, LTC_BIP32_CHILD),
                                    BRBIP32MasterPubKeyPath(seed.u8, sizeof(seed.u8), DOGE_BIP32_DEPTH, DOGE_BIP32_CHILD),
                                    ethAccountCreateWithBIP32Seed(seed),
                                    rippleAccountCreateWithSeed (seed),
                                    hederaAccountCreateWithSeed(seed),
                                    tezosAccountCreateWithSeed(seed),
                                    stellarAccountCreateWithSeed(seed),
                                    timestamp,
                                    uids);
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
wkAccountCreateFromSerialization (const uint8_t *bytes, size_t bytesCount, const char *uids) {
    wkAccountInstall();

    uint8_t *bytesPtr = (uint8_t *) bytes;
    uint8_t *bytesEnd = bytesPtr + bytesCount;

#define BYTES_PTR_INCR_AND_CHECK(size) do {\
bytesPtr += (size);\
if (bytesPtr > bytesEnd) return NULL; /* overkill */ \
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

    // BTC
    size_t mpkSize = UInt32GetBE(bytesPtr);
    BYTES_PTR_INCR_AND_CHECK (szSize);

    // There is a slight chance that this fails IF AND ONLY IF the serialized format
    // of a MasterPublicKey either changes or is key dependent.  That is, we parse the MPK
    // from `bytes` but if THIS PARSE needs more than the original parse we might run
    // off the end of the provided `bytes`.  Must be REALLY UNLIKELY.
    //
    // TODO: Add `bytesCount` to BRBIP32ParseMasterPubKey()
    BRMasterPubKey btc = BRBIP32ParseMasterPubKey ((const char *) bytesPtr);
    if (mpkSize != BRBIP32SerializeMasterPubKey (NULL, btc)) return NULL;
    BYTES_PTR_INCR_AND_CHECK (mpkSize);

    // LTC
    mpkSize = UInt32GetBE(bytesPtr);
    BYTES_PTR_INCR_AND_CHECK (szSize);

    BRMasterPubKey ltc = BRBIP32ParseMasterPubKey ((const char *) bytesPtr);
    if (mpkSize != BRBIP32SerializeMasterPubKey (NULL, ltc)) return NULL;
    BYTES_PTR_INCR_AND_CHECK (mpkSize);

    // DOGE
    mpkSize = UInt32GetBE(bytesPtr);
    BYTES_PTR_INCR_AND_CHECK (szSize);

    BRMasterPubKey doge = BRBIP32ParseMasterPubKey ((const char *) bytesPtr);
    if (mpkSize != BRBIP32SerializeMasterPubKey (NULL, doge)) return NULL;
    BYTES_PTR_INCR_AND_CHECK (mpkSize);

    // ETH
    size_t ethSize = UInt32GetBE (bytesPtr);
    BYTES_PTR_INCR_AND_CHECK (szSize);
    assert (65 == ethSize);

    BRKey ethPublicKey;
    BRKeySetPubKey(&ethPublicKey, bytesPtr, 65);
    BYTES_PTR_INCR_AND_CHECK (65);
    BREthereumAccount eth = ethAccountCreateWithPublicKey(ethPublicKey);

    // XRP
    size_t xrpSize = UInt32GetBE(bytesPtr);
    BYTES_PTR_INCR_AND_CHECK (szSize);

    BRRippleAccount xrp = rippleAccountCreateWithSerialization(bytesPtr, xrpSize);
    assert (NULL != xrp);
    BYTES_PTR_INCR_AND_CHECK (xrpSize); // Move the pointer to then end of the XRP account

    // HBAR
    size_t hbarSize = UInt32GetBE(bytesPtr);
    BYTES_PTR_INCR_AND_CHECK (szSize);

    BRHederaAccount hbar = hederaAccountCreateWithSerialization(bytesPtr, hbarSize);
    assert (NULL != hbar);
    BYTES_PTR_INCR_AND_CHECK (hbarSize); // Move the pointer to the end of the Hedera account
    
    // XTZ
    size_t xtzSize = UInt32GetBE(bytesPtr);
    BYTES_PTR_INCR_AND_CHECK (szSize);

    BRTezosAccount xtz = tezosAccountCreateWithSerialization (bytesPtr, xtzSize);
    assert (NULL != xtz);
    BYTES_PTR_INCR_AND_CHECK (xtzSize); // Move the pointer to then end of the Tezos account

    // XLM
    size_t xlmSize = UInt32GetBE(bytesPtr);
    BYTES_PTR_INCR_AND_CHECK (szSize);

    BRStellarAccount xlm = stellarAccountCreateWithSerialization (bytesPtr, xlmSize);
    assert (NULL != xlm);
    BYTES_PTR_INCR_AND_CHECK (xlmSize); // Move the pointer to then end of the Stellar account

    return wkAccountCreateInternal (btc, ltc, doge, eth, xrp, hbar, xtz, xlm, AS_WK_TIMESTAMP (timestamp), uids);
#undef BYTES_PTR_INCR_AND_CHECK
}

static void
wkAccountRelease (WKAccount account) {
    // MPK {BTC, LTC, DOGE) - nothing
    ethAccountRelease(account->eth);
    rippleAccountFree(account->xrp);
    hederaAccountFree(account->hbar);
    tezosAccountFree(account->xtz);
    stellarAccountFree(account->xlm);

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

    size_t chkSize = sizeof (uint16_t); // checksum
    size_t szSize  = sizeof (uint32_t); // size
    size_t verSize = sizeof (uint16_t); // version
    size_t tsSize  = sizeof (uint64_t); // timestamp - written as uint64_t

    // Version
    uint16_t version = ACCOUNT_SERIALIZE_DEFAULT_VERSION;

    // BTC/BCH
    size_t btcSize = BRBIP32SerializeMasterPubKey (NULL, account->btc);

    // LTC
    size_t ltcSize = BRBIP32SerializeMasterPubKey (NULL, account->ltc);

    // DOGE
    size_t dogeSize = BRBIP32SerializeMasterPubKey (NULL, account->doge);

    // ETH
    BRKey ethPublicKey = ethAccountGetPrimaryAddressPublicKey (account->eth);
    ethPublicKey.compressed = 0;
    size_t ethSize = BRKeyPubKey (&ethPublicKey, NULL, 0);

    // XRP
    size_t   xrpSize = 0;
    uint8_t *xrpBytes = rippleAccountGetSerialization (account->xrp, &xrpSize);

    // HBAR
    size_t   hbarSize = 0;
    uint8_t *hbarBytes = hederaAccountGetSerialization (account->hbar, &hbarSize);

    // XTZ
    size_t   xtzSize = 0;
    uint8_t *xtzBytes = tezosAccountGetSerialization (account->xtz, &xtzSize);

    // XLM
    size_t   xlmSize = 0;
    uint8_t *xlmBytes = stellarAccountGetSerialization (account->xlm, &xtzSize);

    // Overall size - summing all factors.
    *bytesCount = (chkSize + szSize + verSize + tsSize
                   + (szSize + btcSize)
                   + (szSize + ltcSize)
                   + (szSize + dogeSize)
                   + (szSize + ethSize)
                   + (szSize + xrpSize)
                   + (szSize + hbarSize)
                   + (szSize + xtzSize)
                   + (szSize + xlmSize));
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

    // BTC
    UInt32SetBE (bytesPtr, (uint32_t) btcSize);
    bytesPtr += szSize;

    BRBIP32SerializeMasterPubKey ((char *) bytesPtr, account->btc);
    bytesPtr += btcSize;

    // LTC
    UInt32SetBE (bytesPtr, (uint32_t) ltcSize);
    bytesPtr += szSize;

    BRBIP32SerializeMasterPubKey ((char *) bytesPtr, account->ltc);
    bytesPtr += ltcSize;

    // DOGE
    UInt32SetBE (bytesPtr, (uint32_t) dogeSize);
    bytesPtr += szSize;

    BRBIP32SerializeMasterPubKey ((char *) bytesPtr, account->doge);
    bytesPtr += dogeSize;

    // ETH
    UInt32SetBE (bytesPtr, (uint32_t) ethSize);
    bytesPtr += szSize;

    BRKeyPubKey (&ethPublicKey, bytesPtr, ethSize);
    bytesPtr += ethSize;

    // XRP
    UInt32SetBE (bytesPtr, (uint32_t) xrpSize);
    bytesPtr += szSize;

    memcpy (bytesPtr, xrpBytes, xrpSize);
    bytesPtr += xrpSize;

    // HBAR
    UInt32SetBE (bytesPtr, (uint32_t) hbarSize);
    bytesPtr += szSize;

    memcpy (bytesPtr, hbarBytes, hbarSize);
    bytesPtr += hbarSize;
    
    // XTZ
    UInt32SetBE (bytesPtr, (uint32_t) xtzSize);
    bytesPtr += szSize;

    memcpy (bytesPtr, xtzBytes, xtzSize);
    bytesPtr += xtzSize;

    // XLM
    UInt32SetBE (bytesPtr, (uint32_t) xlmSize);
    bytesPtr += szSize;

    memcpy (bytesPtr, xlmBytes, xlmSize);
    bytesPtr += xlmSize;

    // Avoid static analysis warning
    (void) bytesPtr;

    // checksum
    uint16_t checksum = checksumFletcher16 (&bytes[chkSize], (*bytesCount - chkSize));
    UInt16SetBE (bytes, checksum);

    free (xrpBytes);
    free (hbarBytes);
    free (xtzBytes);
    free (xlmBytes);

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
    size_t mpkBytesCount = BRBIP32SerializeMasterPubKey (NULL, account->btc);
    uint8_t mpkBytes[mpkBytesCount];
    BRBIP32SerializeMasterPubKey ((char *) mpkBytes, account->btc);

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
    size_t   mpkSize  = BRBIP32SerializeMasterPubKey (NULL, account->btc);
    uint8_t *mpkBytes = malloc (mpkSize);
    BRBIP32SerializeMasterPubKey ((char*) mpkBytes, account->btc);

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

