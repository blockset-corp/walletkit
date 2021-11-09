//
//  WKAccount.h
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKAccount_h
#define WKAccount_h

#include "WKBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief An Account holds the public key information associated with a User.
 *
 * @discussion An Account is created
 * from a `paperKey`.  (A `paperKey` being 12 words from the BIP-39 word list appropriate for
 * the User's locale.)
 *
 * An Account can be serialized, which never ever includes private key information, and then
 * recreated from that serialization.
 *
 * An Account has a 'creation timestamp' which can be used to limit the identification of
 * transfers as younger than the timestamp.
 */
typedef struct WKAccountRecord *WKAccount;

/**
 * Generate a BIP-39 PaperKey from a BIP-39 word list
 *
 * @note Asserts if words is itself an invalide BIP-39 word list
 *
 * @param wordList The BIP-39 word list.
 *
 * @return A BIP-39 PaperKey
 */
extern char *
wkAccountGeneratePaperKey (const char *wordList[]);

/**
 * Validate a candidate BIP-39 PaperKey with a BIP-39 word list
 *
 * @note Asserts if words is itself an invalide BIP-39 word list
 *
 * @param phrase the candiate paper key
 * @param words the BIP-39 word list
 *
 * @return true if valid; false otherwise
 */
extern WKBoolean
wkAccountValidatePaperKey (const char *phrase, const char *words[]);

/**
 * Validate the number of words in the word list.
 *
 * @param wordsCount number of words
 *
 * @return WK_TRUE if valid; false otherwise.
 */
extern WKBoolean
wkAccountValidateWordsList (size_t wordsCount);

/**
 * Create an Account from a paperKey.  There is absolutely no check on the paperKey as a valid
 * BIP-39 paperKey for a specified word-list.  Therefore Users should call
 * `wkAccountValidatePaperKey()` prior to any attempt to create an account.
 *
 * @param paperKey the paper key
 * @param timestamp the paper key's creation timestamp
 * @param uids a uids
 * @param isMainnet Indicates the network is a main network
 *
 * @return The Account, or NULL.  In practice NULL is never returned.
 */
extern WKAccount
wkAccountCreate (const char     *paperKey, 
                 WKTimestamp    timestamp, 
                 const char     *uids,
                 WKBoolean      isMainnet   );

/**
 * Recreate an Account from a serialization
 *
 * @param bytes serialized bytes
 * @param bytesCount serialized bytes count
 *
 * @return The Account, or NULL.  If the serialization is invalid then the account *must be
 * recreated* from the `phrase` (aka 'Paper Key').  A serialization will be invald when the
 * serialization format changes which will *always occur* when a new blockchain is added.  For
 * example, when XRP is added the XRP public key must be serialized; the old serialization w/o
 * the XRP public key will be invalid and the `phrase` is *required* in order to produce the
 * XRP public key.
 */
extern WKAccount
wkAccountCreateFromSerialization (const uint8_t *bytes, size_t bytesCount, const char *uids);

/*
 * Serialize an account.
 *
 * @param bytesCount the serialized bytes count; must not be NULL
 *
 * @return An array of bytes holding the account serialization that is suitable for use
 * by wkAccountCreateFromSerialization.  The returned array is owned by the caller and
 * must be freed.
 */
extern uint8_t *
wkAccountSerialize (WKAccount account, size_t *bytesCount);

/*
 * Validate that the serialization (represented with `bytes` and `bytesCount`) is consistent
 * with `account`.  This does not serialize account and then compare with bytes; it simply
 * checks that the serialization is for account.
 *
 * @param account the account
 * @param bytes the serialization bytes
 * @param bytesCount the count of serialization bytes
 */
extern WKBoolean
wkAccountValidateSerialization (WKAccount account,
                                const uint8_t *bytes,
                                size_t bytesCount);

/**
 * The the account's timestamp
 */
extern WKTimestamp
wkAccountGetTimestamp (WKAccount account);

/**
 * Get a unique file-system-identier for the account.  This is derived from the account's
 * public data only.
 */
extern char *
wkAccountGetFileSystemIdentifier (WKAccount account);

/**
 * Get the account's unique-identifier.
 *
 * @discussion This is used by the `SystemClient` when subscribing to client events.
 */
extern const char *
wkAccountGetUids (WKAccount account);

DECLARE_WK_GIVE_TAKE (WKAccount, wkAccount);

#ifdef __cplusplus
}
#endif

#endif /* WKAccount_h */
