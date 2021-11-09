//
//  WKAccountP.h
//  WalletKitCore
//
//  Created by Ed Gamble on 11/22/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKAccountP_h
#define WKAccountP_h

#include "WKAccount.h"
#include "support/BRInt.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: - Account Handlers

/** Generic identifier for stored network accounts.
 */
typedef void* WKAccountDetails;

/** Create a network specific account provided the specified
 *  seed material.
 *
 *  @param seed The seed from BIP39 phrase
 *
 *  @return The new network specific account
 */
typedef WKAccountDetails
(*WKAccountCreateFromSeedHandler)(WKBoolean isMainnet,
                                  UInt512   seed);

/** Create a network specific account from a series of bytes
 *
 * @param bytes Pointer to start of account bytes
 * @param len The number of bytes describing this
 *            network account
 */
typedef WKAccountDetails
(*WKAccountCreateFromBytesHandler)(uint8_t* bytes,
                                   size_t   len);

/**
 * Release the account any associated resources
 *
 * @param account The account to be released
 */
typedef void
(*WKAccountReleaseHandler)(WKAccountDetails accountDetails);

/**
 * Serialize the account provided into the specified
 * serialization buffer. The input serialization buffer
 * may be NULL in which case no serialization occurs but
 * the number of bytes buffer required to serialize this
 * account is returned
 *
 * @param accountSerBuf The output serialization buffer or NULL
 * @param account The account object to be serialized
 *
 * @return The size bytes serialized or required to be serialized
 */
typedef size_t
(*WKAccountSerializeHandler)(uint8_t        *accountSerBuf,
                             WKAccount      account         );

/**
 * The account handlers interface for plug-in support
 * of account creation, serialization and resource deallocation
 */
typedef struct {
    WKAccountCreateFromSeedHandler      createFromSeed;
    WKAccountCreateFromBytesHandler     createFromBytes;
    WKAccountReleaseHandler             release;
    WKAccountSerializeHandler           serialize;
} WKAccountHandlers;

struct WKAccountRecord {

    WKAccountDetails networkAccounts[NUMBER_OF_NETWORK_TYPES];

    char *uids;
    WKTimestamp timestamp;
    WKRef ref;
};

/**
 * Onetime install of WKAccount static state.  The static state includes the 'GEN Handlers'
 * which allow WKAccount to create the required GEN accounts.  In not-DEBUG environments the
 * static install happens w/i WKAccount; however, in a DEBUG environment, such as when unit
 * testing, some WKNetwork function are invoked prior to WKAccount install.  So, we
 * make this function `private_extern` and call it from WKNetwork.
 */
private_extern void
wkAccountInstall (void);

/**
 * Given a phrase (A BIP-39 PaperKey) dervie the corresponding 'seed'.  This is used
 * exclusive to sign transactions (BTC ones for sure).
 *
 * @param phrase A BIP-32 Paper Key
 *
 * @return A UInt512 seed
 */
private_extern UInt512
wkAccountDeriveSeed (const char *phrase);

// MARK: Account As {ETH,BTC,XRP,HBAR,XTZ,XLM etc}
static inline WKAccountDetails
wkAccountAs(
    WKAccount       account,
    WKNetworkType   type    ) {

    assert ( (type >= WK_NETWORK_TYPE_BTC) &&
             (type < NUMBER_OF_NETWORK_TYPES)   );

    return account->networkAccounts[type];
}

#ifdef __cplusplus
}
#endif

#endif /* WKAccountP_h */
