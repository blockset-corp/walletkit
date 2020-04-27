//
//  BRCryptoWalletManagerP.h
//  BRCrypto
//
//  Created by Michael Carrara on 6/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoWalletManagerP_h
#define BRCryptoWalletManagerP_h

#include <pthread.h>
#include "support/BRArray.h"

#include "BRCryptoBase.h"
#include "BRCryptoNetwork.h"
#include "BRCryptoAccount.h"
#include "BRCryptoWallet.h"
#include "BRCryptoWalletManager.h"

#include "BRCryptoClientP.h"
#include "support/BRFileService.h"
#include "ethereum/event/BREvent.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - WalletManager Handlers

typedef void
(*BRWalletManagerReleaseHandler) (BRCryptoWalletManager manager);

typedef BRFileService
(*BRWalletManagerCreateFileService) (BRCryptoWalletManager manager,
                                     const char *basePath,
                                     const char *currency,
                                     const char *network,
                                     BRFileServiceContext context,
                                     BRFileServiceErrorHandler handler);

typedef BRArrayOf(BRCryptoWallet)
(*BRWalletManagerCreateWalletsHandler) (BRCryptoWalletManager manager,
                                        BRArrayOf(BRCryptoTransfer) transfers,
                                        BRCryptoWallet *primaryWallet);

typedef BRCryptoBoolean
(*BRWalletManagerSignTransactionWithSeedHandler) (BRCryptoWalletManager manager,
                                                  BRCryptoWallet wallet,
                                                  BRCryptoTransfer transfer,
                                                  UInt512 seed);

typedef BRCryptoBoolean
(*BRWalletManagerSignTransactionWithKeyHandler) (BRCryptoWalletManager manager,
                                                 BRCryptoWallet wallet,
                                                 BRCryptoTransfer transfer,
                                                 BRCryptoKey key);

typedef BRCryptoAmount
(*BRWalletManagerEstimateLimitHandler) (BRCryptoWalletManager cwm,
                                        BRCryptoWallet  wallet,
                                        BRCryptoBoolean asMaximum,
                                        BRCryptoAddress target,
                                        BRCryptoNetworkFee fee,
                                        BRCryptoBoolean *needEstimate,
                                        BRCryptoBoolean *isZeroIfInsuffientFunds,
                                        BRCryptoUnit unit);

typedef void
(*BRWalletManagerEstimateFeeBasisHandler) (BRCryptoWalletManager cwm,
                                           BRCryptoWallet  wallet,
                                           BRCryptoCookie cookie,
                                           BRCryptoAddress target,
                                           BRCryptoAmount amount,
                                           BRCryptoNetworkFee fee);

typedef BRCryptoClientP2PManager
(*BRWalletManagerCreateP2PManagerHandler) (BRCryptoWalletManager cwm);

typedef struct {
    BRWalletManagerReleaseHandler release;
    BRWalletManagerCreateFileService createFileService;
    BRWalletManagerCreateWalletsHandler createWallets;
    BRWalletManagerSignTransactionWithSeedHandler signTransactionWithSeed;
    BRWalletManagerSignTransactionWithKeyHandler signTransactionWithKey;
    BRWalletManagerEstimateLimitHandler estimateLimit;
    BRWalletManagerEstimateFeeBasisHandler estimateFeeBasis;
    BRWalletManagerCreateP2PManagerHandler createP2PManager;
} BRCryptoWalletManagerHandlers;


// MARK: - Wallet Manager

struct BRCryptoWalletManagerRecord {
    BRCryptoBlockChainType type;
    const BRCryptoWalletManagerHandlers *handlers;
    BRCryptoRef ref;

    pthread_mutex_t lock;

    BRCryptoListener listener;
    BRCryptoClient client;
    BRCryptoNetwork network;
    BRCryptoAccount account;
    BRCryptoAddressScheme addressScheme;

    char *path;
    BRFileService fileService;

    BREventHandler handler;

    BRCryptoClientP2PManager p2pManager;   // Null unless BTC, BCH, ETH, ...
    BRCryptoClientQRYManager qryManager;

    BRCryptoSyncMode syncMode;
    BRCryptoClientSync canSync;
    BRCryptoClientSend canSend;

    /// The primary wallet
    BRCryptoWallet wallet;

    /// All wallets
    BRArrayOf(BRCryptoWallet) wallets;

    BRCryptoWalletManagerState state;
};

 /// MARK: - WalletManager

private_extern BRCryptoWalletManagerState
cryptoWalletManagerStateInit(BRCryptoWalletManagerStateType type);

private_extern BRCryptoWalletManagerState
cryptoWalletManagerStateDisconnectedInit(BRCryptoWalletManagerDisconnectReason reason);

private_extern void
cryptoWalletManagerSetState (BRCryptoWalletManager cwm,
                             BRCryptoWalletManagerState state);

private_extern void
cryptoWalletManagerStop (BRCryptoWalletManager cwm);

private_extern void
cryptoWalletManagerAddWallet (BRCryptoWalletManager cwm,
                              BRCryptoWallet wallet);

private_extern void
cryptoWalletManagerRemWallet (BRCryptoWalletManager cwm,
                              BRCryptoWallet wallet);

#ifdef REFACTOR
private_extern BRWalletManagerClient
cryptoWalletManagerClientCreateBTCClient (OwnershipKept BRCryptoWalletManager cwm);

private_extern BREthereumClient
cryptoWalletManagerClientCreateETHClient (OwnershipKept BRCryptoWalletManager cwm);

private_extern BRGenericClient
cryptoWalletManagerClientCreateGENClient (OwnershipKept BRCryptoWalletManager cwm);

private_extern BRWalletManager
cryptoWalletManagerAsBTC (BRCryptoWalletManager manager);

private_extern BREthereumEWM
cryptoWalletManagerAsETH (BRCryptoWalletManager manager);

private_extern BRCryptoBoolean
cryptoWalletManagerHasBTC (BRCryptoWalletManager manager,
                           BRWalletManager bwm);

private_extern BRCryptoBoolean
cryptoWalletManagerHasETH (BRCryptoWalletManager manager,
                           BREthereumEWM ewm);

private_extern BRCryptoBoolean
cryptoWalletManagerHasGEN (BRCryptoWalletManager manager,
                           BRGenericManager gwm);

private_extern BRCryptoWallet
cryptoWalletManagerFindWalletAsBTC (BRCryptoWalletManager manager,
                                    BRWallet *btc);

private_extern BRCryptoWallet
cryptoWalletManagerFindWalletAsETH (BRCryptoWalletManager manager,
                                    BREthereumWallet eth);

private_extern BRCryptoWallet
cryptoWalletManagerFindWalletAsGEN (BRCryptoWalletManager cwm,
                                    BRGenericWallet gen);
extern void
cryptoWalletManagerHandleTransferGEN (BRCryptoWalletManager cwm,
                                      OwnershipGiven BRGenericTransfer transferGeneric);

private_extern void
cryptoWalletManagerSetTransferStateGEN (BRCryptoWalletManager cwm,
                                        BRCryptoWallet wallet,
                                        BRCryptoTransfer transfer,
                                        BRGenericTransferState newGenericState);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoWalletManagerP_h */
