//
//  WKWalletManagerP.h
//  WK
//
//  Created by Michael Carrara on 6/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKWalletManagerP_h
#define WKWalletManagerP_h

#include <pthread.h>
#include "support/BRArray.h"

#include "WKBase.h"
#include "WKNetwork.h"
#include "WKAccount.h"
#include "WKWallet.h"
#include "WKWalletManager.h"

#include "WKClientP.h"
#include "WKWalletP.h"

#include "support/BRFileService.h"
#include "support/event/BREvent.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - WalletManager Handlers

typedef WKWalletManager
(*WKWalletManagerCreateHandler) (WKWalletManagerListener listener,
                                       WKClient client,
                                       WKAccount account,
                                       WKNetwork network,
                                       WKSyncMode mode,
                                       WKAddressScheme scheme,
                                       const char *path);

// The manager's lock is held
typedef void
(*WKWalletManagerReleaseHandler) (WKWalletManager manager);

typedef BRFileService
(*WKWalletManagerCreateFileServiceHandler) (WKWalletManager manager,
                                                  const char *basePath,
                                                  const char *currency,
                                                  const char *network,
                                                  BRFileServiceContext context,
                                                  BRFileServiceErrorHandler handler);

typedef const BREventType **
(*WKWalletManagerGetEventTypesHandler) (WKWalletManager manager,
                                              size_t *eventTypesCount);

typedef WKBoolean
(*WKWalletManagerSignTransactionWithSeedHandler) (WKWalletManager manager,
                                                        WKWallet wallet,
                                                        WKTransfer transfer,
                                                        UInt512 seed);

typedef WKBoolean
(*WKWalletManagerSignTransactionWithKeyHandler) (WKWalletManager manager,
                                                       WKWallet wallet,
                                                       WKTransfer transfer,
                                                       WKKey key);

typedef WKAmount
(*WKWalletManagerEstimateLimitHandler) (WKWalletManager cwm,
                                              WKWallet  wallet,
                                              WKBoolean asMaximum,
                                              WKAddress target,
                                              WKNetworkFee fee,
                                              WKBoolean *needEstimate,
                                              WKBoolean *isZeroIfInsuffientFunds,
                                              WKUnit unit);


typedef WKFeeBasis // If NULL, don't generate WalletEvent; expect QRY callback invoked.
(*WKWalletManagerEstimateFeeBasisHandler) (WKWalletManager cwm,
                                                 WKWallet  wallet,
                                                 WKCookie cookie,
                                                 WKAddress target,
                                                 WKAmount amount,
                                                 WKNetworkFee fee,
                                                 size_t attributesCount,
                                                 OwnershipKept WKTransferAttribute *attributes);

typedef WKClientP2PManager
(*WKWalletManagerCreateP2PManagerHandler) (WKWalletManager cwm);

typedef WKWallet
(*WKWalletManagerCreateWalletHandler) (WKWalletManager cwm,
                                             WKCurrency currency,
                                             Nullable OwnershipKept BRArrayOf(WKClientTransactionBundle) transactions,
                                             Nullable OwnershipKept BRArrayOf(WKClientTransferBundle) transfers);

typedef void
(*WKWalletManagerSaveTransactionBundleHandler) (WKWalletManager cwm,
                                                      OwnershipKept WKClientTransactionBundle bundle);

typedef void
(*WKWalletManagerSaveTransferBundleHandler) (WKWalletManager cwm,
                                                   OwnershipKept WKClientTransferBundle bundle);

typedef void
(*WKWalletManagerRecoverTransfersFromTransactionBundleHandler) (WKWalletManager cwm,
                                                                      OwnershipKept WKClientTransactionBundle bundle);

typedef void
(*WKWalletManagerRecoverTransferFromTransferBundleHandler) (WKWalletManager cwm,
                                                                  OwnershipKept WKClientTransferBundle bundle);

typedef WKFeeBasis
(*WKWalletManagerRecoverFeeBasisFromFeeEstimateHandler) (WKWalletManager cwm,
                                                               WKNetworkFee networkFee,
                                                               WKFeeBasis initialFeeBasis,
                                                               double costUnits,
                                                               size_t attributesCount,
                                                               OwnershipKept const char **attributeKeys,
                                                               OwnershipKept const char **attributeVals);

typedef WKWalletSweeperStatus
(*WKWalletManagerWalletSweeperValidateSupportedHandler) (WKWalletManager cwm,
                                                               WKWallet wallet,
                                                               WKKey key);

typedef WKWalletSweeper
(*WKWalletManagerCreateWalletSweeperHandler) (WKWalletManager cwm,
                                                    WKWallet wallet,
                                                    WKKey key);

typedef struct {
    WKWalletManagerCreateHandler create;
    WKWalletManagerReleaseHandler release;
    WKWalletManagerCreateFileServiceHandler createFileService;
    WKWalletManagerGetEventTypesHandler getEventTypes;
    WKWalletManagerCreateP2PManagerHandler createP2PManager;
    WKWalletManagerCreateWalletHandler createWallet;
    WKWalletManagerSignTransactionWithSeedHandler signTransactionWithSeed;
    WKWalletManagerSignTransactionWithKeyHandler signTransactionWithKey;
    WKWalletManagerEstimateLimitHandler estimateLimit;
    WKWalletManagerEstimateFeeBasisHandler estimateFeeBasis;
    WKWalletManagerSaveTransactionBundleHandler saveTransactionBundle;
    WKWalletManagerSaveTransferBundleHandler    saveTransferBundle;
    WKWalletManagerRecoverTransfersFromTransactionBundleHandler recoverTransfersFromTransactionBundle;
    WKWalletManagerRecoverTransferFromTransferBundleHandler     recoverTransferFromTransferBundle;
    WKWalletManagerRecoverFeeBasisFromFeeEstimateHandler        recoverFeeBasisFromFeeEstimate;
    WKWalletManagerWalletSweeperValidateSupportedHandler validateSweeperSupported;
    WKWalletManagerCreateWalletSweeperHandler createSweeper;
} WKWalletManagerHandlers;

// MARK: - Wallet Manager State

private_extern WKWalletManagerState
wkWalletManagerStateInit(WKWalletManagerStateType type);

private_extern WKWalletManagerState
wkWalletManagerStateDisconnectedInit(WKWalletManagerDisconnectReason reason);

// MARK: - Wallet Manager

struct WKWalletManagerRecord {
    WKNetworkType type;
    const WKWalletManagerHandlers *handlers;
    WKRef ref;
    size_t sizeInBytes;

    pthread_mutex_t lock;
    WKClient client;
    WKNetwork network;
    WKAccount account;
    WKAddressScheme addressScheme;

    char *path;
    BRFileService fileService;

    BREventHandler handler;
 //   BREventHandler listenerHandler;

    WKClientP2PManager p2pManager;   // Null unless BTC, BCH, ETH, ...
    WKClientQRYManager qryManager;

    WKClientQRYByType byType;
    
    WKSyncMode syncMode;
    WKClientSync canSync;
    WKClientSend canSend;

    /// The primary wallet
    WKWallet wallet;

    /// All wallets (modifiable)
    BRArrayOf(WKWallet) wallets;

    /// The state (modifiable)
    WKWalletManagerState state;

    WKWalletManagerListener listener;
    WKWalletListener listenerWallet;

    /// The {Transfer,Transaction}Bundle (modifiable)
    Nullable BRArrayOf(WKClientTransferBundle) bundleTransfers;
    Nullable BRArrayOf(WKClientTransactionBundle) bundleTransactions;
};

typedef void *WKWalletManagerCreateContext;
typedef void (*WKWalletManagerCreateCallback) (WKWalletManagerCreateContext context,
                                                     WKWalletManager manager);

/**
 * @brief Create a wallet manager.  The result will be NULL if the manager's network has not been
 * initialized.
 *
 * @param listener the listener for wallet manager events
 * @param client the client for requesting data
 * @param account the account
 * @param network the network
 * @param mode the default mode
 * @param scheme the default scheme
 * @param path the file system path under which persistent account data will be stored.  Only public
 * data is ever stored in this location.
 *
 * @return A wallet manager for account on network.  May be NULL if `network` isn't initialized
 **/
extern WKWalletManager
wkWalletManagerCreate (WKWalletManagerListener listener,
                           WKClient client,
                           WKAccount account,
                           WKNetwork network,
                           WKSyncMode mode,
                           WKAddressScheme scheme,
                           const char *path);

extern WKWalletManager
wkWalletManagerAllocAndInit (size_t sizeInBytes,
                                 WKNetworkType type,
                                 WKWalletManagerListener listener,
                                 WKClient client,
                                 WKAccount account,
                                 WKNetwork network,
                                 WKAddressScheme scheme,
                                 const char *path,
                                 WKClientQRYByType byType,
                                 WKWalletManagerCreateContext createContext,
                                 WKWalletManagerCreateCallback createCallback);

private_extern WKNetworkType
wkWalletManagerGetType (WKWalletManager manager);

private_extern void
wkWalletManagerSetState (WKWalletManager cwm,
                             WKWalletManagerState state);

private_extern void
wkWalletManagerStop (WKWalletManager cwm);

private_extern void
wkWalletManagerAddWallet (WKWalletManager cwm,
                              WKWallet wallet);

private_extern void
wkWalletManagerRemWallet (WKWalletManager cwm,
                              WKWallet wallet);

private_extern void
wkWalletManagerSaveTransactionBundle (WKWalletManager manager,
                                          OwnershipKept WKClientTransactionBundle bundle);

private_extern void
wkWalletManagerSaveTransferBundle (WKWalletManager manager,
                                       OwnershipKept WKClientTransferBundle bundle);

private_extern WKWallet
wkWalletManagerCreateWalletInitialized (WKWalletManager cwm,
                                            WKCurrency currency,
                                            Nullable BRArrayOf(WKClientTransactionBundle) transactions,
                                            Nullable BRArrayOf(WKClientTransferBundle) transfers);

private_extern void
wkWalletManagerRecoverTransfersFromTransactionBundle (WKWalletManager cwm,
                                                          OwnershipKept WKClientTransactionBundle bundle);

// Is it possible that the transfers do not have the 'submitted' state?  In some race between
// the submit call and the included call?  Highly, highly unlikely but possible?
private_extern void
wkWalletManagerRecoverTransferFromTransferBundle (WKWalletManager cwm,
                                                      OwnershipKept WKClientTransferBundle bundle);

private_extern void
wkWalletManagerRecoverTransferAttributesFromTransferBundle (WKWallet wallet,
                                                                WKTransfer transfer,
                                                                OwnershipKept WKClientTransferBundle bundle);

private_extern WKFeeBasis
wkWalletManagerRecoverFeeBasisFromFeeEstimate (WKWalletManager cwm,
                                                   WKNetworkFee networkFee,
                                                   WKFeeBasis initialFeeBasis,
                                                   double costUnits,
                                                   size_t attributesCount,
                                                   OwnershipKept const char **attributeKeys,
                                                   OwnershipKept const char **attributeVals);

static inline void
wkWalletManagerGenerateEvent (WKWalletManager manager,
                                  WKWalletManagerEvent event) {
    wkListenerGenerateManagerEvent (&manager->listener, manager, event);
}

#ifdef __cplusplus
}
#endif

#endif /* WKWalletManagerP_h */
