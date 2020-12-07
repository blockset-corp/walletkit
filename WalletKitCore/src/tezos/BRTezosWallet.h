//
//  BRTezosWallet.h
//  Core
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRTezosWallet_h
#define BRTezosWallet_h

#include "BRTezosAccount.h"
#include "BRTezosFeeBasis.h"
#include "BRTezosTransfer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BRTezosWalletRecord *BRTezosWallet;

/**
 * Create a Tezos wallet. Caller must free using tezosWalletFree.
 *
 * @param account   - a BRTezosAccount
 *
 * @return wallet   - a valid BRTezosWallet object
 */
extern BRTezosWallet
tezosWalletCreate (BRTezosAccount account);

/**
 * Free all memory associated with this wallet
 *
 * @param wallet    - a valid BRTezosWallet
 *
 */
extern void
tezosWalletFree (BRTezosWallet wallet);

/**
 * Returns the Tezos wallet's "manager" address suitable for receiving Tezos. Caller must free.
 *
 * @param wallet the wallet for target address
 *
 * @return address  tezos manager address associated with this account
 */
extern BRTezosAddress
tezosWalletGetSourceAddress (BRTezosWallet wallet);

/**
 * Return an address suitable for receiving Tezos. Caller must free.
 *
 * @param wallet the wallet for target address
 *
 * @return address  tezos receiving address associated with this account
 */
extern BRTezosAddress
tezosWalletGetTargetAddress (BRTezosWallet wallet);

/**
* Returns true if the address belongs to the wallet.
*
* @param wallet
* @param address
*
* @return 1 if address belongs to the account associated with the wallet, 0 otherwise.
*/
extern int
tezosWalletHasAddress (BRTezosWallet wallet, BRTezosAddress address);

/**
 * Return the tezos balance for this wallet
 *
 * @param wallet the wallet for this account
 *
 * @return balance - balance of the wallet's account in mutez units
 */
extern BRTezosUnitMutez
tezosWalletGetBalance (BRTezosWallet wallet);

/**
 * Set the balance for this wallet - in mutez units
 *
 * @param wallet    - the wallet for target address
 * @param balance   - mutez balance amount for this account
 *
 * @return address  tezos receiving address associated with this account
 */
extern void
tezosWalletSetBalance (BRTezosWallet wallet, BRTezosUnitMutez balance);

/**
 * Return the balance limit for this wallet, either asMaximum or asMinimum
 *
 * @param wallet - the wallet
 * @param asMaximum - if true, return the wallet maximum limit; otherwise minimum limit
 * @param hasLimit  - must be non-NULL; assigns if wallet as the specified limit
 *
 * @return balance limit - in mutez units
 */
extern BRTezosUnitMutez
tezosWalletGetBalanceLimit (BRTezosWallet wallet,
                            int asMaximum,
                            int *hasLimit);
/**
 * Set the fee basis default amount
 *
 * @param wallet     the specified wallet
 * @param feeBasis   the default fee basis to be used for all transactions
 *
 * @return void
 */
extern void
tezosWalletSetDefaultFeeBasis (BRTezosWallet wallet, BRTezosFeeBasis feeBasis);

/**
 * Get the default fee basis that is stored with this wallet
 *
 * @param wallet the specified tezos wallet
 *
 * @return feeBasis  the default base fee that has been set for this wallet
 */
extern BRTezosFeeBasis
tezosWalletGetDefaultFeeBasis (BRTezosWallet wallet);

extern int64_t
tezosWalletGetCounter (BRTezosWallet wallet);

extern bool
tezosWalletNeedsReveal (BRTezosWallet wallet);

// Wallet transfer list functions
extern int tezosWalletHasTransfer (BRTezosWallet wallet,
                                   OwnershipKept BRTezosTransfer transfer);

extern void tezosWalletAddTransfer (BRTezosWallet wallet,
                                    OwnershipKept BRTezosTransfer transfer);

extern void tezosWalletRemTransfer (BRTezosWallet wallet,
                                    OwnershipKept BRTezosTransfer transfer);

extern void tezosWalletUpdateTransfer (BRTezosWallet wallet,
                                       OwnershipKept BRTezosTransfer transfer);

#ifdef __cplusplus
}
#endif

#endif // BRTezosWallet_h
