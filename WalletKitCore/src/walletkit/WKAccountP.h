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

#include "support/BRBIP32Sequence.h"
#include "support/BRBIP39Mnemonic.h"
#include "support/BRKey.h"
#include "ethereum/blockchain/BREthereumAccount.h"
#include "ripple/BRRippleAccount.h"
#include "hedera/BRHederaAccount.h"
#include "tezos/BRTezosAccount.h"

#ifdef __cplusplus
extern "C" {
#endif

struct WKAccountRecord {
    BRMasterPubKey btc;
    BRMasterPubKey ltc;
    BRMasterPubKey doge;
    BREthereumAccount eth;
    BRRippleAccount xrp;
    BRHederaAccount hbar;
    BRTezosAccount xtz;
    // ...

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

// MARK: Account As {ETH,BTC,XRP,HBAR,XTZ}

static inline BRMasterPubKey
wkAccountAsBTC (WKAccount account) {
    return account->btc;
}

static inline BRMasterPubKey
wkAccountAsLTC (WKAccount account) {
    return account->ltc;
}

static inline BRMasterPubKey
wkAccountAsDOGE (WKAccount account) {
    return account->doge;
}

static inline BREthereumAccount
wkAccountAsETH (WKAccount account) {
    return account->eth;
}

static inline BRRippleAccount
wkAccountAsXRP (WKAccount account) {
    return account->xrp;
}

static inline BRHederaAccount
wkAccountAsHBAR (WKAccount account) {
    return account->hbar;
}

static inline BRTezosAccount
wkAccountAsXTZ (WKAccount account) {
    return account->xtz;
}

#ifdef __cplusplus
}
#endif

#endif /* WKAccountP_h */
