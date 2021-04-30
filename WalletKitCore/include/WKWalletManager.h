//
//  WKWalletManager.h
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKWalletManager_h
#define WKWalletManager_h

#include "WKBase.h"
#include "WKKey.h"
#include "WKNetwork.h"
#include "WKPeer.h"
#include "WKAccount.h"
#include "WKTransfer.h"
#include "WKWallet.h"
#include "WKSync.h"
#include "WKClient.h"
#include "WKWalletSweeper.h"
#include "WKListener.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: Wallet Manager

/**
 * Get the manager's network
 */
extern WKNetwork
wkWalletManagerGetNetwork (WKWalletManager cwm);

/**
 * Check if manager has `network`
 */
extern WKBoolean
wkWalletManagerHasNetwork (WKWalletManager cwm,
                               WKNetwork network);

/*
 * Get the manager's account
 */
extern WKAccount
wkWalletManagerGetAccount (WKWalletManager cwm);

/**
 * Check if manager has `account`
 */
extern WKBoolean
wkWalletManagerHasAccount (WKWalletManager cwm,
                               WKAccount account);

/**
 * Get the manager's sync mode
 */
extern WKSyncMode
wkWalletManagerGetMode (WKWalletManager cwm);

/**
 * Set the manager' sync mode
 */
extern void
wkWalletManagerSetMode (WKWalletManager cwm, WKSyncMode mode);

/**
 * Get the manager's state
 */
extern WKWalletManagerState
wkWalletManagerGetState (WKWalletManager cwm);

/**
 * Get the manager's address scheme
 */
extern WKAddressScheme
wkWalletManagerGetAddressScheme (WKWalletManager cwm);

/**
 * Set the manager's address scheme
 */
extern void
wkWalletManagerSetAddressScheme (WKWalletManager cwm,
                                     WKAddressScheme scheme);

/**
 * Get the manager's path
 */
extern const char *
wkWalletManagerGetPath (WKWalletManager cwm);

/**
 * Set the manager's reachablity.  Reachability refers to 'internet connectivity' and should be
 * set to WK_FALSE, for example, when a device is in 'airplane mode'.  Setting the
 * reachablity does not change the manager's connected/disonnected state but does inform
 * peer-to-peer modes to expect a loss of peers.
 */
extern void
wkWalletManagerSetNetworkReachable (WKWalletManager cwm,
                                        WKBoolean isNetworkReachable);

/**
 * Create a manager wallet for `currency`.  If the wallet already exists, it is returned.
 * Othereise a new wallet is created.
 */
extern WKWallet
wkWalletManagerCreateWallet (WKWalletManager cwm,
                                 WKCurrency currency);

/**
 * Get the manager's primary wallet.  This is the wallet for the network's native currency.
 * For example, for a 'Bitcoin Wallet Manager', the primary wallet will be a wallet holding
 * BTC.  The primary wallet is where transaction fees are credited.
 */
extern WKWallet
wkWalletManagerGetWallet (WKWalletManager cwm);

/**
 * Returns a newly allocated array of the managers's wallets.
 *
 * The caller is responsible for deallocating the returned array using
 * free().
 *
 * @param cwm the wallet manager
 * @param count the number of wallets returned
 *
 * @return An array of wallets w/ an incremented reference count (aka 'taken')
 *         or NULL if there are no wallters in the manager.
 */
extern WKWallet *
wkWalletManagerGetWallets (WKWalletManager cwm,
                               size_t *count);

/**
 Get the manager's wallet holding assets for `currency`.  Result may be NULL.
 */
extern WKWallet
wkWalletManagerGetWalletForCurrency (WKWalletManager cwm,
                                         WKCurrency currency);

/**
 * Check if manager has `wallet`
 */
extern WKBoolean
wkWalletManagerHasWallet (WKWalletManager cwm,
                              WKWallet wallet);

/**
 * Start the WalletManager; allows for handling of events.  This does not connect to an
 * associated P2P network or a QRY interface. The WalletManger is started once created.  There
 * is no harm calling this multiple times - it is a noop if already started.
 */
extern void
wkWalletManagerStart (WKWalletManager cwm);

/**
 * Stop the WalletManager; ends handling of events and stops any sync by disconnecting .  In
 * practice this should only be called when the WalletManger is to be disposed of.  There is
 * no harm calling this multiple times - it is a noop if already stopped.
 */
extern void
wkWalletManagerStop (WKWalletManager cwm);

/**
 * Connect the wallet manager; if `peer` is provided use it for the P2P network; otherwise use
 * dynamically discovered peers.  (This only applies for P2P sync modes).
 */
extern void
wkWalletManagerConnect (WKWalletManager cwm,
                            WKPeer peer);

/**
 * Disconnect the wallet manager.
 */
extern void
wkWalletManagerDisconnect (WKWalletManager cwm);

/**
 * Start a manager sync.
 */
extern void
wkWalletManagerSync (WKWalletManager cwm);

/**
 * Start a manager sync from `depth`
 */
extern void
wkWalletManagerSyncToDepth (WKWalletManager cwm,
                                WKSyncDepth depth);

/**
 * Sign the `transfer` in `wallet` with the `paperKey`.
 */
extern WKBoolean
wkWalletManagerSign (WKWalletManager cwm,
                         WKWallet wallet,
                         WKTransfer transfer,
                         const char *paperKey);

/**
 * Sign and then submit `transfer` in `wallet` with the `paperKey`
 */
extern void
wkWalletManagerSubmit (WKWalletManager cwm,
                           WKWallet wid,
                           WKTransfer tid,
                           const char *paperKey);

/**
 * Sign and then submit `transfer` in `wallet` with the private `key`
 */
extern void
wkWalletManagerSubmitForKey (WKWalletManager cwm,
                                 WKWallet wallet,
                                 WKTransfer transfer,
                                 WKKey key);

/**
 * Submit a `transfer` in `wallet` that is already signed
 *
 * TODO: Ensure that `transfer` is in the SIGNED state.
 */
extern void
wkWalletManagerSubmitSigned (WKWalletManager cwm,
                                 WKWallet wallet,
                                 WKTransfer transfer);

/**
 * Estimate the wallet's maximum or minimun transfer amount.
 *
 * @param manager the manager
 * @param wallet the wallet
 * @param asMaximum WK_TREU if the limit estimate is for the wallet maximum
 * @param target the target address
 * @param fee the network fee
 * @param needEstimate Filled with WK_TRUE if an estimate is needed.  Some transfers have
 * variable fees (such as different Ethereum ERC20 token transfers) and thus a maximum transfer
 * can't be determeined unless a fee is estimated.
 * @param isZeroIfInsuffientFunds filled with WK_TRUE if a zero indicates insufficient funds
 *
 * @return the maximium or minimum amount for a transfer from wallet.
 */
extern WKAmount
wkWalletManagerEstimateLimit (WKWalletManager manager,
                                  WKWallet  wallet,
                                  WKBoolean asMaximum,
                                  WKAddress target,
                                  WKNetworkFee fee,
                                  WKBoolean *needEstimate,
                                  WKBoolean *isZeroIfInsuffientFunds);

/**
 * Estimate the fee to transfer `amount` from `wallet` using the `feeBasis`.  Return an amount
 * represented in the wallet's fee currency.
 *
 * @param manager the manager
 * @param wallet the wallet
 * @param amount the amount to transfer
 * @param feeBasis the fee basis for the transfer
 *
 * @return the fee
 */
extern void
wkWalletManagerEstimateFeeBasis (WKWalletManager manager,
                                     WKWallet  wallet,
                                     WKCookie cookie,
                                     WKAddress target,
                                     WKAmount  amount,
                                     WKNetworkFee fee,
                                     size_t attributesCount,
                                     OwnershipKept WKTransferAttribute *attributes);

extern WKWalletSweeperStatus
wkWalletManagerWalletSweeperValidateSupported (WKWalletManager cwm,
                                                   WKWallet wallet,
                                                   WKKey key);

extern WKWalletSweeper
wkWalletManagerCreateWalletSweeper (WKWalletManager manager,
                                        WKWallet wallet,
                                        WKKey key);

extern void
wkWalletManagerEstimateFeeBasisForPaymentProtocolRequest (WKWalletManager manager,
                                                              WKWallet wallet,
                                                              WKCookie cookie,
                                                              WKPaymentProtocolRequest request,
                                                              WKNetworkFee fee);

/**
 * Delete the persistent state for a wallet manager on `network` with `path`.  This is a
 * last-ditched effort to recover from manager errors.  If the manager for `network` is in
 * a P2P sync mode, then the persistate state deleted may require many hours to recover - as if
 * from a full (block 0) P2P sync.  In a non-P2P mode, the impact to manager's state is small.
 */
extern void
wkWalletManagerWipe (WKNetwork network,
                         const char *path);

DECLARE_WK_GIVE_TAKE (WKWalletManager, wkWalletManager);

#ifdef __cplusplus
}
#endif

#endif /* WKWalletManager_h */
