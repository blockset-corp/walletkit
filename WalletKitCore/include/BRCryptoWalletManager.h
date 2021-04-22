//
//  BRCryptoWalletManager.h
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoWalletManager_h
#define BRCryptoWalletManager_h

#include "BRCryptoBase.h"
#include "BRCryptoKey.h"
#include "BRCryptoNetwork.h"
#include "BRCryptoPeer.h"
#include "BRCryptoAccount.h"
#include "BRCryptoTransfer.h"
#include "BRCryptoWallet.h"
#include "BRCryptoSync.h"
#include "BRCryptoClient.h"
#include "BRCryptoWalletSweeper.h"
#include "BRCryptoListener.h"

#ifdef __cplusplus
extern "C" {
#endif

    /// MARK: Wallet Manager

    /**
     * Get the manager's network
     */
    extern BRCryptoNetwork
    cryptoWalletManagerGetNetwork (BRCryptoWalletManager cwm);

    /**
     * Check if manager has `network`
     */
    extern BRCryptoBoolean
    cryptoWalletManagerHasNetwork (BRCryptoWalletManager cwm,
                                   BRCryptoNetwork network);

    /*
     * Get the manager's account
     */
    extern BRCryptoAccount
    cryptoWalletManagerGetAccount (BRCryptoWalletManager cwm);

    /**
     * Check if manager has `account`
     */
    extern BRCryptoBoolean
    cryptoWalletManagerHasAccount (BRCryptoWalletManager cwm,
                                   BRCryptoAccount account);

    /**
     * Get the manager's sync mode
     */
    extern BRCryptoSyncMode
    cryptoWalletManagerGetMode (BRCryptoWalletManager cwm);

    /**
     * Set the manager' sync mode
     */
    extern void
    cryptoWalletManagerSetMode (BRCryptoWalletManager cwm, BRCryptoSyncMode mode);

    /**
     * Get the manager's state
     */
    extern BRCryptoWalletManagerState
    cryptoWalletManagerGetState (BRCryptoWalletManager cwm);

    /**
     * Get the manager's address scheme
     */
    extern BRCryptoAddressScheme
    cryptoWalletManagerGetAddressScheme (BRCryptoWalletManager cwm);

    /**
     * Set the manager's address scheme
     */
    extern void
    cryptoWalletManagerSetAddressScheme (BRCryptoWalletManager cwm,
                                         BRCryptoAddressScheme scheme);

    /**
     * Get the manager's path
     */
    extern const char *
    cryptoWalletManagerGetPath (BRCryptoWalletManager cwm);

    /**
     * Set the manager's reachablity.  Reachability refers to 'internet connectivity' and should be
     * set to CRYPTO_FALSE, for example, when a device is in 'airplane mode'.  Setting the
     * reachablity does not change the manager's connected/disonnected state but does inform
     * peer-to-peer modes to expect a loss of peers.
     */
    extern void
    cryptoWalletManagerSetNetworkReachable (BRCryptoWalletManager cwm,
                                            BRCryptoBoolean isNetworkReachable);

    /**
     * Create a manager wallet for `currency`.  If the wallet already exists, it is returned.
     * Othereise a new wallet is created.
     */
    extern BRCryptoWallet
    cryptoWalletManagerCreateWallet (BRCryptoWalletManager cwm,
                                     BRCryptoCurrency currency);

    /**
     * Get the manager's primary wallet.  This is the wallet for the network's native currency.
     * For example, for a 'Bitcoin Wallet Manager', the primary wallet will be a wallet holding
     * BTC.  The primary wallet is where transaction fees are credited.
     */
    extern BRCryptoWallet
    cryptoWalletManagerGetWallet (BRCryptoWalletManager cwm);

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
    extern BRCryptoWallet *
    cryptoWalletManagerGetWallets (BRCryptoWalletManager cwm,
                                   size_t *count);

    /**
     Get the manager's wallet holding assets for `currency`.  Result may be NULL.
     */
    extern BRCryptoWallet
    cryptoWalletManagerGetWalletForCurrency (BRCryptoWalletManager cwm,
                                             BRCryptoCurrency currency);

    /**
     * Check if manager has `wallet`
     */
    extern BRCryptoBoolean
    cryptoWalletManagerHasWallet (BRCryptoWalletManager cwm,
                                  BRCryptoWallet wallet);

    /**
     * Start the WalletManager; allows for handling of events.  This does not connect to an
     * associated P2P network or a QRY interface. The WalletManger is started once created.  There
     * is no harm calling this multiple times - it is a noop if already started.
     */
    extern void
    cryptoWalletManagerStart (BRCryptoWalletManager cwm);

    /**
     * Stop the WalletManager; ends handling of events and stops any sync by disconnecting .  In
     * practice this should only be called when the WalletManger is to be disposed of.  There is
     * no harm calling this multiple times - it is a noop if already stopped.
     */
    extern void
    cryptoWalletManagerStop (BRCryptoWalletManager cwm);

    /**
     * Connect the wallet manager; if `peer` is provided use it for the P2P network; otherwise use
     * dynamically discovered peers.  (This only applies for P2P sync modes).
     */
    extern void
    cryptoWalletManagerConnect (BRCryptoWalletManager cwm,
                                BRCryptoPeer peer);

    /**
     * Disconnect the wallet manager.
     */
    extern void
    cryptoWalletManagerDisconnect (BRCryptoWalletManager cwm);

    /**
     * Start a manager sync.
     */
    extern void
    cryptoWalletManagerSync (BRCryptoWalletManager cwm);

    /**
     * Start a manager sync from `depth`
     */
    extern void
    cryptoWalletManagerSyncToDepth (BRCryptoWalletManager cwm,
                                    BRCryptoSyncDepth depth);

    /**
     * Sign the `transfer` in `wallet` with the `paperKey`.
     */
    extern BRCryptoBoolean
    cryptoWalletManagerSign (BRCryptoWalletManager cwm,
                             BRCryptoWallet wallet,
                             BRCryptoTransfer transfer,
                             const char *paperKey);

    /**
     * Sign and then submit `transfer` in `wallet` with the `paperKey`
     */
    extern void
    cryptoWalletManagerSubmit (BRCryptoWalletManager cwm,
                               BRCryptoWallet wid,
                               BRCryptoTransfer tid,
                               const char *paperKey);

    /**
     * Sign and then submit `transfer` in `wallet` with the private `key`
     */
    extern void
    cryptoWalletManagerSubmitForKey (BRCryptoWalletManager cwm,
                                     BRCryptoWallet wallet,
                                     BRCryptoTransfer transfer,
                                     BRCryptoKey key);

    /**
     * Submit a `transfer` in `wallet` that is already signed
     *
     * TODO: Ensure that `transfer` is in the SIGNED state.
     */
    extern void
    cryptoWalletManagerSubmitSigned (BRCryptoWalletManager cwm,
                                     BRCryptoWallet wallet,
                                     BRCryptoTransfer transfer);

    /**
     * Estimate the wallet's maximum or minimun transfer amount.
     *
     * @param manager the manager
     * @param wallet the wallet
     * @param asMaximum CRYPTO_TREU if the limit estimate is for the wallet maximum
     * @param target the target address
     * @param fee the network fee
     * @param needEstimate Filled with CRYPTO_TRUE if an estimate is needed.  Some transfers have
     * variable fees (such as different Ethereum ERC20 token transfers) and thus a maximum transfer
     * can't be determeined unless a fee is estimated.
     * @param isZeroIfInsuffientFunds filled with CRYPTO_TRUE if a zero indicates insufficient funds
     *
     * @return the maximium or minimum amount for a transfer from wallet.
     */
    extern BRCryptoAmount
    cryptoWalletManagerEstimateLimit (BRCryptoWalletManager manager,
                                      BRCryptoWallet  wallet,
                                      BRCryptoBoolean asMaximum,
                                      BRCryptoAddress target,
                                      BRCryptoNetworkFee fee,
                                      BRCryptoBoolean *needEstimate,
                                      BRCryptoBoolean *isZeroIfInsuffientFunds);

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
    cryptoWalletManagerEstimateFeeBasis (BRCryptoWalletManager manager,
                                         BRCryptoWallet  wallet,
                                         BRCryptoCookie cookie,
                                         BRCryptoAddress target,
                                         BRCryptoAmount  amount,
                                         BRCryptoNetworkFee fee,
                                         size_t attributesCount,
                                         OwnershipKept BRCryptoTransferAttribute *attributes);

    extern BRCryptoWalletSweeperStatus
    cryptoWalletManagerWalletSweeperValidateSupported (BRCryptoWalletManager cwm,
                                                       BRCryptoWallet wallet,
                                                       BRCryptoKey key);

    extern BRCryptoWalletSweeper
    cryptoWalletManagerCreateWalletSweeper (BRCryptoWalletManager manager,
                                            BRCryptoWallet wallet,
                                            BRCryptoKey key);

    extern void
    cryptoWalletManagerEstimateFeeBasisForPaymentProtocolRequest (BRCryptoWalletManager manager,
                                                                  BRCryptoWallet wallet,
                                                                  BRCryptoCookie cookie,
                                                                  BRCryptoPaymentProtocolRequest request,
                                                                  BRCryptoNetworkFee fee);

    /**
     * Delete the persistent state for a wallet manager on `network` with `path`.  This is a
     * last-ditched effort to recover from manager errors.  If the manager for `network` is in
     * a P2P sync mode, then the persistate state deleted may require many hours to recover - as if
     * from a full (block 0) P2P sync.  In a non-P2P mode, the impact to manager's state is small.
     */
    extern void
    cryptoWalletManagerWipe (BRCryptoNetwork network,
                             const char *path);

    DECLARE_CRYPTO_GIVE_TAKE (BRCryptoWalletManager, cryptoWalletManager);


#ifdef __cplusplus
}
#endif

#endif /* BRCryptoWalletManager_h */
