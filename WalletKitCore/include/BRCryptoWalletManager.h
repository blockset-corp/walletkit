//
//  BRCryptoWalletManager.h
//  BRCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
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

    /// Can return NULL
    extern BRCryptoWalletManager
    cryptoWalletManagerCreate (BRCryptoWalletManagerListener listener,
                               BRCryptoClient client,
                               BRCryptoAccount account,
                               BRCryptoNetwork network,
                               BRCryptoSyncMode mode,
                               BRCryptoAddressScheme scheme,
                               const char *path);

    extern BRCryptoNetwork
    cryptoWalletManagerGetNetwork (BRCryptoWalletManager cwm);

    extern BRCryptoBoolean
    cryptoWalletManagerHasNetwork (BRCryptoWalletManager cwm,
                                   BRCryptoNetwork network);

    extern BRCryptoAccount
    cryptoWalletManagerGetAccount (BRCryptoWalletManager cwm);

    extern BRCryptoBoolean
    cryptoWalletManagerHasAccount (BRCryptoWalletManager cwm,
                                   BRCryptoAccount account);

    extern BRCryptoSyncMode
    cryptoWalletManagerGetMode (BRCryptoWalletManager cwm);

    extern void
    cryptoWalletManagerSetMode (BRCryptoWalletManager cwm, BRCryptoSyncMode mode);

    extern BRCryptoWalletManagerState
    cryptoWalletManagerGetState (BRCryptoWalletManager cwm);

    extern BRCryptoAddressScheme
    cryptoWalletManagerGetAddressScheme (BRCryptoWalletManager cwm);

    extern void
    cryptoWalletManagerSetAddressScheme (BRCryptoWalletManager cwm,
                                         BRCryptoAddressScheme scheme);

    extern const char *
    cryptoWalletManagerGetPath (BRCryptoWalletManager cwm);

    extern void
    cryptoWalletManagerSetNetworkReachable (BRCryptoWalletManager cwm,
                                            BRCryptoBoolean isNetworkReachable);


    extern BRCryptoWallet
    cryptoWalletManagerCreateWallet (BRCryptoWalletManager cwm,
                                     BRCryptoCurrency currency);

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

    extern BRCryptoWallet
    cryptoWalletManagerGetWalletForCurrency (BRCryptoWalletManager cwm,
                                             BRCryptoCurrency currency);

    extern BRCryptoBoolean
    cryptoWalletManagerHasWallet (BRCryptoWalletManager cwm,
                                  BRCryptoWallet wallet);

    extern void
    cryptoWalletManagerAddWallet (BRCryptoWalletManager cwm,
                                  BRCryptoWallet wallet);

    extern void
    cryptoWalletManagerRemWallet (BRCryptoWalletManager cwm,
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

    extern void
    cryptoWalletManagerConnect (BRCryptoWalletManager cwm,
                                BRCryptoPeer peer);

    extern void
    cryptoWalletManagerDisconnect (BRCryptoWalletManager cwm);

    extern void
    cryptoWalletManagerSync (BRCryptoWalletManager cwm);

    extern void
    cryptoWalletManagerSyncToDepth (BRCryptoWalletManager cwm,
                                    BRCryptoSyncDepth depth);

    extern BRCryptoBoolean
    cryptoWalletManagerSign (BRCryptoWalletManager cwm,
                             BRCryptoWallet wallet,
                             BRCryptoTransfer transfer,
                             const char *paperKey);

    extern void
    cryptoWalletManagerSubmit (BRCryptoWalletManager cwm,
                               BRCryptoWallet wid,
                               BRCryptoTransfer tid,
                               const char *paperKey);

    extern void
    cryptoWalletManagerSubmitForKey (BRCryptoWalletManager cwm,
                                     BRCryptoWallet wallet,
                                     BRCryptoTransfer transfer,
                                     BRCryptoKey key);

    extern void
    cryptoWalletManagerSubmitSigned (BRCryptoWalletManager cwm,
                                     BRCryptoWallet wallet,
                                     BRCryptoTransfer transfer);

    /**
     * Estimate the wallet's maximum or minimun transfer amount.
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

    extern void
    cryptoWalletManagerWipe (BRCryptoNetwork network,
                             const char *path);

    DECLARE_CRYPTO_GIVE_TAKE (BRCryptoWalletManager, cryptoWalletManager);


#ifdef __cplusplus
}
#endif

#endif /* BRCryptoWalletManager_h */
