//
//  BRCryptoAccountP.h
//  BRCore
//
//  Created by Ed Gamble on 11/22/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoAccountP_h
#define BRCryptoAccountP_h

#include "BRCryptoAccount.h"

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

struct BRCryptoAccountRecord {
    BRMasterPubKey btc;
    BREthereumAccount eth;
    BRRippleAccount xrp;
    BRHederaAccount hbar;
    BRTezosAccount xtz;
    // ...

    char *uids;
    BRCryptoTimestamp timestamp;
    BRCryptoRef ref;
};

/**
 * Onetime install of BRCryptoAccount static state.  The static state includes the 'GEN Handlers'
 * which allow BRCryptoAccount to create the required GEN accounts.  In not-DEBUG environments the
 * static install happens w/i BRCryptoAccount; however, in a DEBUG environment, such as when unit
 * testing, some BRCryptoNetwork function are invoked prior to BRCryptoAccount install.  So, we
 * make this function `private_extern` and call it from BRCryptoNetwork.
 */
private_extern void
cryptoAccountInstall (void);

/**
 * Given a phrase (A BIP-39 PaperKey) dervie the corresponding 'seed'.  This is used
 * exclusive to sign transactions (BTC ones for sure).
 *
 * @param phrase A BIP-32 Paper Key
 *
 * @return A UInt512 seed
 */
private_extern UInt512
cryptoAccountDeriveSeed (const char *phrase);

// MARK: Account As {ETH,BTC,XRP,HBAR,XTZ}

static inline BRMasterPubKey
cryptoAccountAsBTC (BRCryptoAccount account) {
    return account->btc;
}

static inline BREthereumAccount
cryptoAccountAsETH (BRCryptoAccount account) {
    return account->eth;
}

static inline BRRippleAccount
cryptoAccountAsXRP (BRCryptoAccount account) {
    return account->xrp;
}

static inline BRHederaAccount
cryptoAccountAsHBAR (BRCryptoAccount account) {
    return account->hbar;
}

static inline BRTezosAccount
cryptoAccountAsXTZ (BRCryptoAccount account) {
    return account->xtz;
}

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoAccountP_h */
