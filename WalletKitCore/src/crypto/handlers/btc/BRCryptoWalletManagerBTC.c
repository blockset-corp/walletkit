//
//  BRCryptoWalletManagerBTC.c
//  Core
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoBTC.h"

#include "crypto/BRCryptoAccountP.h"
#include "crypto/BRCryptoNetworkP.h"
#include "crypto/BRCryptoKeyP.h"
#include "crypto/BRCryptoClientP.h"
#include "crypto/BRCryptoWalletManagerP.h"
#include "crypto/BRCryptoWalletSweeperP.h"

#include "bitcoin/BRWallet.h"
#include "bitcoin/BRPeerManager.h"

// BRWallet Callbacks

// On: BRWalletRegisterTransaction, BRWalletRemoveTransaction
static void cryptoWalletManagerBTCBalanceChanged (void *info, uint64_t balanceInSatoshi);
// On: BRWalletRegisterTransaction
static void cryptoWalletManagerBTCTxAdded   (void *info, BRTransaction *tx);
// On: BRWalletUpdateTransactions, BRWalletSetTxUnconfirmedAfter (reorg)
static void cryptoWalletManagerBTCTxUpdated (void *info, const UInt256 *hashes, size_t count, uint32_t blockHeight, uint32_t timestamp);
// On: BRWalletRemoveTransaction
static void cryptoWalletManagerBTCTxDeleted (void *info, UInt256 hash, int notifyUser, int recommendRescan);

// BRPeerManager Callbacks
static void cryptoWalletManagerBTCSyncStarted (void *info);
static void cryptoWalletManagerBTCSyncStopped (void *info, int reason);
static void cryptoWalletManagerBTCTxStatusUpdate (void *info);
static void cryptoWalletManagerBTCSaveBlocks (void *info, int replace, BRMerkleBlock **blocks, size_t count);
static void cryptoWalletManagerBTCSavePeers  (void *info, int replace, const BRPeer *peers, size_t count);
static int  cryptoWalletManagerBTCNetworkIsReachable (void *info);
static void cryptoWalletManagerBTCThreadCleanup (void *info);
static void cryptoWalletManagerBTCTxPublished (void *info, int error);

static BRCryptoWalletManagerBTC
cryptoWalletManagerCoerce (BRCryptoWalletManager manager, BRCryptoBlockChainType type) {
    assert (type == manager->type);
    return (BRCryptoWalletManagerBTC) manager;
}

static BRCryptoWalletManager
cryptoWalletManagerCreateBTC (BRCryptoListener listener,
                              BRCryptoClient client,
                              BRCryptoAccount account,
                              BRCryptoNetwork network,
                              BRCryptoSyncMode mode,
                              BRCryptoAddressScheme scheme,
                              const char *path) {
    BRCryptoWalletManager manager = cryptoWalletManagerAllocAndInit (sizeof (struct BRCryptoWalletManagerBTCRecord),
                                                                     cryptoNetworkGetType(network),
                                                                     listener,
                                                                     client,
                                                                     account,
                                                                     network,
                                                                     scheme,
                                                                     path,
                                                                     CRYPTO_CLIENT_REQUEST_USE_TRANSACTIONS);
    BRCryptoWalletManagerBTC managerBTC = cryptoWalletManagerCoerce (manager, cryptoNetworkGetType(network));

    // BTC Stuff
    managerBTC->ignoreTBD = 0;

    return manager;
}

static void
cryptoWalletManagerReleaseBTC (BRCryptoWalletManager manager) {

}

static void
cryptoWalletManagerInitializeBTC (BRCryptoWalletManager manager) {
    // Get the btcMasterPublicKey
    BRMasterPubKey btcMPK = cryptoAccountAsBTC(manager->account);

    // Get the btcChainParams
    const BRChainParams *btcChainParams = cryptoNetworkAsBTC(manager->network);

    // Load the BTC transactions from the fileService
    BRArrayOf(BRTransaction*) transactions = initialTransactionsLoadBTC (manager);
    if (NULL == transactions) array_new (transactions, 1);

    // Create the BTC wallet
    //
    // Since the BRWallet callbacks are not set, none of these transactions generate callbacks.
    // And, in fact, looking at BRWalletNew(), there is not even an attempt to generate callbacks
    // even if they could have been specified.
    BRWallet *btcWallet = BRWalletNew (btcChainParams->addrParams, transactions, array_count(transactions), btcMPK);
    assert (NULL != btcWallet);

    // The btcWallet now should include *all* the transactions
    array_free (transactions);

    // Set the callbacks
    BRWalletSetCallbacks (btcWallet,
                          cryptoWalletManagerCoerce(manager, manager->network->type),
                          cryptoWalletManagerBTCBalanceChanged,
                          cryptoWalletManagerBTCTxAdded,
                          cryptoWalletManagerBTCTxUpdated,
                          cryptoWalletManagerBTCTxDeleted);

    // Create the primary BRCryptoWallet
    BRCryptoNetwork  network       = manager->network;
    BRCryptoCurrency currency      = cryptoNetworkGetCurrency (network);
    BRCryptoUnit     unitAsBase    = cryptoNetworkGetUnitAsBase    (network, currency);
    BRCryptoUnit     unitAsDefault = cryptoNetworkGetUnitAsDefault (network, currency);

    manager->wallet = cryptoWalletCreateAsBTC (manager->type,
                                               manager->listenerWallet,
                                               unitAsDefault,
                                               unitAsDefault,
                                               btcWallet);
    array_add (manager->wallets, manager->wallet);

    // Process existing btcTransactions in the btcWallet into BRCryptoTransfers
    size_t btcTransactionsCount = BRWalletTransactions(btcWallet, NULL, 0);
    BRTransaction *btcTransactions[btcTransactionsCount > 0 ? btcTransactionsCount : 1]; // avoid a static analysis error
    BRWalletTransactions (btcWallet, btcTransactions, btcTransactionsCount);

    for (size_t index = 0; index < btcTransactionsCount; index++) {
        BRCryptoTransfer transfer = cryptoTransferCreateAsBTC (manager->wallet->listenerTransfer,
                                                               unitAsDefault,
                                                               unitAsBase,
                                                               btcWallet,
                                                               BRTransactionCopy(btcTransactions[index]),
                                                               manager->type);
        cryptoWalletAddTransfer(manager->wallet, transfer);
    }

    cryptoUnitGive (unitAsDefault);
    cryptoUnitGive (unitAsBase);
    cryptoCurrencyGive (currency);

    return;
}

static BRFileService
crytpWalletManagerCreateFileServiceBTC (BRCryptoWalletManager manager,
                                        const char *basePath,
                                        const char *currency,
                                        const char *network,
                                        BRFileServiceContext context,
                                        BRFileServiceErrorHandler handler) {
    return fileServiceCreateFromTypeSpecfications (basePath, currency, network,
                                                   context, handler,
                                                   fileServiceSpecificationsCountBTC,
                                                   fileServiceSpecificationsBTC);
}

static const BREventType **
cryptoWalletManagerGetEventTypesBTC (BRCryptoWalletManager manager,
                                     size_t *eventTypesCount) {
    assert (NULL != eventTypesCount);
    *eventTypesCount = eventTypesCountBTC;
    return eventTypesBTC;
}

#if 0
static BRArrayOf(BRCryptoWallet)
cryptoWalletManagerCreateWalletsBTC (BRCryptoWalletManager manager,
                                            BRArrayOf(BRCryptoTransfer) transfers,
                                            BRCryptoWallet *primaryWallet) {
    assert (NULL != primaryWallet);
    *primaryWallet = NULL;

    BRArrayOf(BRCryptoWallet) wallets;
    array_new (wallets, 1);

    // Get the btcMasterPublicKey
    BRCryptoAccount account = cryptoWalletManagerGetAccount(manager);
    BRMasterPubKey btcMPK = cryptoAccountAsBTC(account);
    
    // Get the btcChainParams
    BRCryptoNetwork network = cryptoWalletManagerGetNetwork(manager);
    const BRChainParams *btcChainParams = cryptoNetworkAsBTC(network);

    // Get the btcTransactions
    BRArrayOf(BRTransaction*) transactions;
    array_new (transactions, array_count(transfers));
    for (size_t index = 0; index < array_count(transfers); index++) {
        array_add (transactions, cryptoTransferAsBTC(transfers[index]));
    }

    // Since the BRWallet callbacks are not set, none of these transactions generate callbacks.
    // And, in fact, looking at BRWalletNew(), there is not even an attempt to generate callbacks
    // even if they could have been specified.
    BRWallet *btcWallet = BRWalletNew (btcChainParams->addrParams, transactions, array_count(transactions), btcMPK);

    // Free `transactions` before any non-local exists.
    array_free (transactions);

    if (NULL == btcWallet) {
        return wallets;  // bwmCreateErrorHandler (bwm, 0, "wallet");
    }

    // Set the callbacks if the wallet has been created successfully
    BRWalletSetCallbacks (btcWallet,
                          cryptoWalletManagerCoerce(manager),
                          cryptoWalletManagerBTCBalanceChanged,
                          cryptoWalletManagerBTCTxAdded,
                          cryptoWalletManagerBTCTxUpdated,
                          cryptoWalletManagerBTCTxDeleted);

    BRCryptoCurrency currency      = cryptoNetworkGetCurrency (network);
    BRCryptoUnit     unitAsBase    = cryptoNetworkGetUnitAsBase    (network, currency);
    BRCryptoUnit     unitAsDefault = cryptoNetworkGetUnitAsDefault (network, currency);

    *primaryWallet = cryptoWalletCreateAsBTC (unitAsDefault, unitAsBase, btcWallet);
    array_add (wallets, *primaryWallet);

    cryptoUnitGive (unitAsDefault);
    cryptoUnitGive (unitAsBase);
    cryptoCurrencyGive (currency);
    cryptoNetworkGive  (network);

    return wallets;
}
#endif

static BRCryptoBoolean
cryptoWalletManagerSignTransactionWithSeedBTC (BRCryptoWalletManager manager,
                                                      BRCryptoWallet wallet,
                                                      BRCryptoTransfer transfer,
                                                      UInt512 seed) {
    BRWallet      *btcWallet       = cryptoWalletAsBTC   (wallet);
    BRTransaction *btcTransaction  = cryptoTransferAsBTC (transfer);         // OWN/REF ?
    const BRChainParams *btcParams = cryptoNetworkAsBTC  (manager->network);

    return AS_CRYPTO_BOOLEAN (1 == BRWalletSignTransaction (btcWallet, btcTransaction, btcParams->forkId, seed.u8, sizeof(UInt512)));
}

static BRCryptoBoolean
cryptoWalletManagerSignTransactionWithKeyBTC (BRCryptoWalletManager manager,
                                                     BRCryptoWallet wallet,
                                                     BRCryptoTransfer transfer,
                                                     BRCryptoKey key) {
    BRTransaction *btcTransaction  = cryptoTransferAsBTC (transfer);         // OWN/REF ?
    BRKey         *btcKey          = cryptoKeyGetCore (key);
    const BRChainParams *btcParams = cryptoNetworkAsBTC  (manager->network);

    return AS_CRYPTO_BOOLEAN (1 == BRTransactionSign (btcTransaction, btcParams->forkId, btcKey, 1));
}

static BRCryptoAmount
cryptoWalletManagerEstimateLimitBTC (BRCryptoWalletManager cwm,
                                            BRCryptoWallet  wallet,
                                            BRCryptoBoolean asMaximum,
                                            BRCryptoAddress target,
                                            BRCryptoNetworkFee networkFee,
                                            BRCryptoBoolean *needEstimate,
                                            BRCryptoBoolean *isZeroIfInsuffientFunds,
                                            BRCryptoUnit unit) {
    BRWallet *btcWallet = cryptoWalletAsBTC (wallet);

    // Amount may be zero if insufficient fees
    *isZeroIfInsuffientFunds = CRYPTO_TRUE;

    // NOTE: We know BTC/BCH has a minimum balance of zero.

    uint64_t balance     = BRWalletBalance (btcWallet);
    uint64_t feePerKB    = 1000 * cryptoNetworkFeeAsBTC (networkFee);
    uint64_t amountInSAT = (CRYPTO_FALSE == asMaximum
                            ? BRWalletMinOutputAmountWithFeePerKb (btcWallet, feePerKB)
                            : BRWalletMaxOutputAmountWithFeePerKb (btcWallet, feePerKB));
    uint64_t fee         = (amountInSAT > 0
                            ? BRWalletFeeForTxAmountWithFeePerKb (btcWallet, feePerKB, amountInSAT)
                            : 0);

    //            if (CRYPTO_TRUE == asMaximum)
    //                assert (balance == amountInSAT + fee);

    if (amountInSAT + fee > balance)
        amountInSAT = 0;

    return cryptoAmountCreateInteger ((int64_t) amountInSAT, unit);
}

static BRCryptoFeeBasis
cryptoWalletManagerEstimateFeeBasisBTC (BRCryptoWalletManager cwm,
                                        BRCryptoWallet wallet,
                                        BRCryptoCookie cookie,
                                        BRCryptoAddress target,
                                        BRCryptoAmount amount,
                                        BRCryptoNetworkFee networkFee) {
    BRWallet *btcWallet = cryptoWalletAsBTC(wallet);

    BRCryptoBoolean overflow = CRYPTO_FALSE;
    uint64_t btcFeePerKB = 1000 * cryptoNetworkFeeAsBTC (networkFee);
    uint64_t btcAmount   = cryptoAmountGetIntegerRaw (amount, &overflow);
    assert(CRYPTO_FALSE == overflow);

    uint64_t btcFee = (0 == btcAmount ? 0 : BRWalletFeeForTxAmountWithFeePerKb (btcWallet, btcFeePerKB, btcAmount));
    uint32_t btcSizeInBytes = (uint32_t) ((1000 * btcFee) / btcFeePerKB);

    return cryptoFeeBasisCreateAsBTC (wallet->unitForFee, btcFeePerKB, btcSizeInBytes);
}

static BRCryptoWallet
cryptoWalletManagerCreateWalletBTC (BRCryptoWalletManager manager,
                                    BRCryptoCurrency currency) {
    // Get the btcMasterPublicKey
    BRMasterPubKey btcMPK = cryptoAccountAsBTC(manager->account);

    // Get the btcChainParams
    const BRChainParams *btcChainParams = cryptoNetworkAsBTC(manager->network);

    // Load the BTC transactions from the fileService
    BRArrayOf(BRTransaction*) transactions = initialTransactionsLoadBTC (manager);
    if (NULL == transactions) array_new (transactions, 1);

    // Create the BTC wallet
    //
    // Since the BRWallet callbacks are not set, none of these transactions generate callbacks.
    // And, in fact, looking at BRWalletNew(), there is not even an attempt to generate callbacks
    // even if they could have been specified.
    BRWallet *btcWallet = BRWalletNew (btcChainParams->addrParams, transactions, array_count(transactions), btcMPK);
    assert (NULL != btcWallet);

    // The btcWallet now should include *all* the transactions
    array_free (transactions);

    // Set the callbacks
    BRWalletSetCallbacks (btcWallet,
                          cryptoWalletManagerCoerce(manager, manager->network->type),
                          cryptoWalletManagerBTCBalanceChanged,
                          cryptoWalletManagerBTCTxAdded,
                          cryptoWalletManagerBTCTxUpdated,
                          cryptoWalletManagerBTCTxDeleted);

    // Create the primary BRCryptoWallet
    BRCryptoNetwork  network       = manager->network;
    BRCryptoUnit     unitAsBase    = cryptoNetworkGetUnitAsBase    (network, currency);
    BRCryptoUnit     unitAsDefault = cryptoNetworkGetUnitAsDefault (network, currency);

    manager->wallet = cryptoWalletCreateAsBTC (manager->type,
                                               manager->listenerWallet,
                                               unitAsDefault,
                                               unitAsDefault,
                                               btcWallet);
    array_add (manager->wallets, manager->wallet);

    // Process existing btcTransactions in the btcWallet into BRCryptoTransfers
    size_t btcTransactionsCount = BRWalletTransactions(btcWallet, NULL, 0);
    BRTransaction *btcTransactions[btcTransactionsCount > 0 ? btcTransactionsCount : 1]; // avoid a static analysis error
    BRWalletTransactions (btcWallet, btcTransactions, btcTransactionsCount);

    for (size_t index = 0; index < btcTransactionsCount; index++) {
        BRCryptoTransfer transfer = cryptoTransferCreateAsBTC (manager->wallet->listenerTransfer,
                                                               unitAsDefault,
                                                               unitAsBase,
                                                               btcWallet,
                                                               BRTransactionCopy(btcTransactions[index]),
                                                               manager->type);
        cryptoWalletAddTransfer(manager->wallet, transfer);
    }

    cryptoUnitGive (unitAsDefault);
    cryptoUnitGive (unitAsBase);

    return NULL;
}

static void
cryptoWalletManagerRecoverTransfersFromTransactionBundleBTC (BRCryptoWalletManager manager,
                                                             OwnershipKept BRCryptoClientTransactionBundle bundle) {
    BRTransaction *btcTransaction = BRTransactionParse (bundle->serialization, bundle->serializationCount);

    bool error = TRANSFER_STATUS_ERRORED != bundle->status;
    bool needRegistration = (!error && NULL != btcTransaction && BRTransactionIsSigned (btcTransaction));
    bool needFree = true;

    BRWallet *btcWallet = cryptoWalletAsBTC(manager->wallet);

    // Convert from `uint64_t` to `uint32_t` with a bit of care regarding BLOCK_HEIGHT_UNBOUND
    // and TX_UNCONFIRMED - they are directly coercible but be explicit about it.
    uint32_t btcBlockHeight = (BLOCK_HEIGHT_UNBOUND == bundle->blockHeight ? TX_UNCONFIRMED : (uint32_t) bundle->blockHeight);
    uint32_t btcTimestamp   = (uint32_t) bundle->timestamp;


    //     if (needRegistration) {
    //         if (0 == pthread_mutex_lock (&manager->lock)) {
    //             // confirm completion is for in-progress sync
    //             needRegistration &= (rid == BRClientSyncManagerScanStateGetRequestId (&manager->scanState) && manager->isConnected);
    //             pthread_mutex_unlock (&manager->lock);
    //         } else {
    //             assert (0);
    //         }
    //     }

    if (needRegistration) {
        if (NULL == BRWalletTransactionForHash (btcWallet, btcTransaction->txHash)) {
            // BRWalletRegisterTransaction doesn't reliably report if the txn was added to the wallet.
            BRWalletRegisterTransaction (btcWallet, btcTransaction);
            if (btcTransaction == BRWalletTransactionForHash (btcWallet, btcTransaction->txHash)) {
                // If our transaction made it into the wallet, do not deallocate it
                needFree = false;
            }
        }
    }

    // Check if the wallet knows about transaction.  This is an important check.  If the wallet
    // does not know about the tranaction then the subsequent BRWalletUpdateTransactions will
    // free the transaction (with BRTransactionFree()).
    if (BRWalletContainsTransaction (btcWallet, btcTransaction)) {
        if (error) {
            // On an error, remove the transaction.  This will cascade through BRWallet callbacks
            // to produce `balanceUpdated` and `txDeleted`.  The later will be handled by removing
            // a BRTransactionWithState from the BRWalletManager.
            BRWalletRemoveTransaction (btcWallet, btcTransaction->txHash);
        }
        else {
            // If the transaction has transitioned from 'included' back to 'submitted' (like when
            // there is a blockchain reord), the blockHeight will be TX_UNCONFIRMED and the
            // timestamp will be 0.  This will cascade through BRWallet callbacks to produce
            // 'balanceUpdated' and 'txUpdated'.
            //
            // If no longer 'included' this might cause dependent transactions to go to 'invalid'.
            BRWalletUpdateTransactions (btcWallet,
                                        &btcTransaction->txHash, 1,
                                        btcBlockHeight,
                                        btcTimestamp);
        }
    }

    // Free if ownership hasn't been passed
    if (needFree) {
        BRTransactionFree (btcTransaction);
    }
}

static void
cryptoWalletManagerRecoverTransferFromTransferBundleBTC (BRCryptoWalletManager cwm,
                                                                OwnershipKept BRCryptoClientTransferBundle bundle) {
    // Not BTC functionality
    assert (0);
}


// MARK: - Client P2P

typedef struct BRCryptoClientP2PManagerRecordBTC {
    struct BRCryptoClientP2PManagerRecord base;
    BRCryptoWalletManagerBTC manager;
    BRPeerManager *btcPeerManager;
} *BRCryptoClientP2PManagerBTC;

static BRCryptoClientP2PManagerBTC
cryptoClientP2PManagerCoerce (BRCryptoClientP2PManager manager) {
    assert (CRYPTO_NETWORK_TYPE_BTC == manager->type ||
            CRYPTO_NETWORK_TYPE_BCH == manager->type ||
            CRYPTO_NETWORK_TYPE_BSV == manager->type);
    return (BRCryptoClientP2PManagerBTC) manager;
}

static void
cryptoClientP2PManagerReleaseBTC (BRCryptoClientP2PManager baseManager) {
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);
    BRPeerManagerFree (manager->btcPeerManager);
}

static void
cryptoClientP2PManagerConnectBTC (BRCryptoClientP2PManager baseManager,
                                         BRCryptoPeer peer) {
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);

    // Assume `peer` is NULL; UINT128_ZERO will restore BRPeerManager peer discovery
    UInt128  address = UINT128_ZERO;
    uint16_t port    = 0;

    if (NULL != peer) {
        BRCryptoData16 addrAsInt = cryptoPeerGetAddrAsInt(peer);
        memcpy (address.u8, addrAsInt.data, sizeof (addrAsInt.data));
        port = cryptoPeerGetPort (peer);
    }

    // Calling `SetFixedPeer` will 100% disconnect.  We could avoid calling SetFixedPeer
    // if we kept a reference to `peer` and checked if it differs.
    BRPeerManagerSetFixedPeer (manager->btcPeerManager, address, port);
    BRPeerManagerConnect(manager->btcPeerManager);
}

static void
cryptoClientP2PManagerDisconnectBTC (BRCryptoClientP2PManager baseManager) {
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);

    BRPeerManagerDisconnect (manager->btcPeerManager);
}

// MARK: - Sync

static uint32_t
cryptoClientP2PManagerCalculateSyncDepthHeight(BRCryptoSyncDepth depth,
                                               const BRChainParams *chainParams,
                                               uint64_t networkBlockHeight,
                                               OwnershipKept BRTransaction *lastConfirmedSendTx) {
    switch (depth) {
        case CRYPTO_SYNC_DEPTH_FROM_LAST_CONFIRMED_SEND:
            return NULL == lastConfirmedSendTx ? 0 : lastConfirmedSendTx->blockHeight;

        case CRYPTO_SYNC_DEPTH_FROM_LAST_TRUSTED_BLOCK: {
            const BRCheckPoint *checkpoint = BRChainParamsGetCheckpointBeforeBlockNumber (chainParams,
                                                                                          (uint32_t) MIN (networkBlockHeight, UINT32_MAX));
            return NULL == checkpoint ? 0 : checkpoint->height;
        }

        case CRYPTO_SYNC_DEPTH_FROM_CREATION: {
            return 0;
        }
    }
}

static void
cryptoClientP2PManagerSyncBTC (BRCryptoClientP2PManager baseManager,
                                      BRCryptoSyncDepth depth,
                                      BRCryptoBlockNumber height) {
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);

    uint32_t calcHeight = cryptoClientP2PManagerCalculateSyncDepthHeight (depth,
                                                                          BRPeerManagerChainParams(manager->btcPeerManager),
                                                                          height,
                                                                          NULL /* lastConfirmedSendTx */);
    uint32_t lastHeight = BRPeerManagerLastBlockHeight (manager->btcPeerManager);
    uint32_t scanHeight = MIN (calcHeight, lastHeight);

    if (0 != scanHeight) {
        BRPeerManagerRescanFromBlockNumber (manager->btcPeerManager, scanHeight);
    } else {
        BRPeerManagerRescan (manager->btcPeerManager);
    }
}

typedef struct {
    BRCryptoWalletManager manager;
    BRCryptoTransfer transfer;
} BRCryptoClientP2PManagerPublishInfo;

static void
cryptoClientP2PManagerSendBTC (BRCryptoClientP2PManager baseManager, BRCryptoTransfer transfer) {
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);

    BRTransaction *btcTransaction = cryptoTransferAsBTC (transfer);

    BRCryptoClientP2PManagerPublishInfo *btcInfo = calloc (1, sizeof (BRCryptoClientP2PManagerPublishInfo));
    btcInfo->manager  = cryptoWalletManagerTake(&manager->manager->base);
    btcInfo->transfer = cryptoTransferTake (transfer);

    BRPeerManagerPublishTx (manager->btcPeerManager,
                            btcTransaction,
                            btcInfo,
                            cryptoWalletManagerBTCTxPublished);
}

static BRCryptoClientP2PHandlers p2pHandlersBTC = {
    cryptoClientP2PManagerReleaseBTC,
    cryptoClientP2PManagerConnectBTC,
    cryptoClientP2PManagerDisconnectBTC,
    cryptoClientP2PManagerSyncBTC,
    cryptoClientP2PManagerSendBTC
};

static BRCryptoClientP2PManager
crytpWalletManagerCreateP2PManagerBTC (BRCryptoWalletManager cwm) {
    // load blocks, load peers

    BRCryptoClientP2PManager baseManager = cryptoClientP2PManagerCreate (sizeof (struct BRCryptoClientP2PManagerRecordBTC),
                                                                         cwm->type,
                                                                         &p2pHandlersBTC);
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);
    manager->manager = cryptoWalletManagerCoerce(cwm, baseManager->type);

    const BRChainParams *btcChainParams = cryptoNetworkAsBTC(cwm->network);
    BRWallet *btcWallet = cryptoWalletAsBTC(cwm->wallet);
    uint32_t btcEarliestKeyTime = (uint32_t) cryptoAccountGetTimestamp(cwm->account);

    BRArrayOf(BRMerkleBlock*) blocks = initialBlocksLoadBTC (cwm);
    BRArrayOf(BRPeer)         peers  = initialPeersLoadBTC  (cwm);

    manager->btcPeerManager = BRPeerManagerNew (btcChainParams,
                                                btcWallet,
                                                btcEarliestKeyTime,
                                                blocks, (NULL == blocks ? 0 : array_count (blocks)),
                                                peers,  (NULL == peers  ? 0 : array_count (peers)));


    assert (NULL != manager->btcPeerManager);

    BRPeerManagerSetCallbacks (manager->btcPeerManager,
                               cryptoWalletManagerCoerce(cwm, baseManager->type),
                               cryptoWalletManagerBTCSyncStarted,
                               cryptoWalletManagerBTCSyncStopped,
                               cryptoWalletManagerBTCTxStatusUpdate,
                               cryptoWalletManagerBTCSaveBlocks,
                               cryptoWalletManagerBTCSavePeers,
                               cryptoWalletManagerBTCNetworkIsReachable,
                               cryptoWalletManagerBTCThreadCleanup);

    if (NULL != blocks) array_free (blocks);
    if (NULL != peers ) array_free (peers);

    return baseManager;
}

/// MARK: - Wallet Sweeper

extern BRCryptoWalletSweeperStatus
cryptoWalletManagerWalletSweeperValidateSupportedBTC (BRCryptoWalletManager cwm,
                                                      BRCryptoWallet wallet,
                                                      BRCryptoKey key) {
    BRWallet * wid          = cryptoWalletAsBTC (wallet);
    BRKey * keyCore            = cryptoKeyGetCore (key);
    BRAddressParams addrParams = cryptoNetworkAsBTC (cwm->network)->addrParams;

    // encode using legacy format (only supported method for BTC)
    size_t addrLength = BRKeyLegacyAddr (keyCore, NULL, 0, addrParams);
    char  *addr = malloc (addrLength + 1);
    BRKeyLegacyAddr (keyCore, addr, addrLength, addrParams);
    addr[addrLength] = '\0';

    // check if we are trying to sweep ourselves
    int containsAddr = BRWalletContainsAddress (wid, addr);
    free (addr);

    if (containsAddr) {
        return CRYPTO_WALLET_SWEEPER_INVALID_SOURCE_WALLET;
    }

    return CRYPTO_WALLET_SWEEPER_SUCCESS;
}

extern BRCryptoWalletSweeper
cryptoWalletManagerCreateWalletSweeperBTC (BRCryptoWalletManager cwm,
                                           BRCryptoWallet wallet,
                                           BRCryptoKey key) {
    BRCryptoCurrency currency = cryptoWalletGetCurrency (wallet);
    BRCryptoUnit unit = cryptoNetworkGetUnitAsBase (cwm->network, currency);

    BRCryptoWalletSweeper sweeper = cryptoWalletSweeperAllocAndInit (sizeof (struct BRCryptoWalletSweeperBTCRecord),
                                                                         cwm->type,
                                                                         key,
                                                                         unit);

    BRCryptoWalletSweeperBTC sweeperBTC = (BRCryptoWalletSweeperBTC) sweeper;

    BRKey *keyCore = cryptoKeyGetCore (key);
    BRAddressParams addrParams = cryptoNetworkAsBTC (cwm->network)->addrParams;

    size_t addressLength = BRKeyLegacyAddr (keyCore, NULL, 0, addrParams);
    char  *address = malloc (addressLength + 1);
    BRKeyLegacyAddr (keyCore, address, addressLength, addrParams);
    address[addressLength] = '\0';

    sweeperBTC->addrParams = addrParams;
    sweeperBTC->isSegwit = CRYPTO_ADDRESS_SCHEME_BTC_SEGWIT == cwm->addressScheme;
    sweeperBTC->sourceAddress = address;
    array_new (sweeperBTC->txns, 100);

    cryptoUnitGive (unit);
    cryptoCurrencyGive (currency);

    return sweeper;
}

// MARK: BRWallet Callback Balance Changed

static void
cryptoWalletManagerUpdateTransferBTC (BRCryptoWalletManager manager,
                                      BRCryptoWallet wallet,
                                      BRCryptoTransfer transfer,
                                      bool needCreate,
                                      bool needAdded,
                                      bool needBalance) {
#if 0
    if (needCreate)
        cryptoWalletManagerGenerateTransferEvent (manager, wallet, transfer, (BRCryptoTransferEvent) {
            CRYPTO_TRANSFER_EVENT_CREATED
        });


    if (needAdded)
        cryptoWalletGenerateEvent (wallet, (BRCryptoWalletEvent) {
            CRYPTO_WALLET_EVENT_TRANSFER_ADDED,
            { .transfer = { cryptoTransferTake (transfer) }}
        });


    if (needBalance)
        cryptoWalletGenerateEvent (wallet, (BRCryptoWalletEvent) {
            CRYPTO_WALLET_EVENT_BALANCE_UPDATED,
            { .balanceUpdated = { cryptoWalletGetBalance (wallet) }}
        });
#endif
}

static void cryptoWalletManagerBTCBalanceChanged (void *info, uint64_t balanceInSatoshi) {
    BRCryptoWalletManagerBTC manager = info;
    // printf ("BTC: BalanceChanged\n");
    (void) manager;
}

// MARK: - BRWallet Callback TX Added

static void cryptoWalletManagerBTCTxAdded   (void *info, BRTransaction *tid) {
    BRCryptoWalletManagerBTC manager = info;

    // We have the possibility that the TID argument ceases to exist by the time this `TxAdded`
    // function is invoked.  From the Bitcoin code: BRWalletRegisterTransaction is called but
    // is interrupted just before wallet->txAdded in invoked; then, somehow, wallet->txDeleted
    // is called (and assume BRTranactionFree() got invoked).  Gone.
    tid = BRTransactionCopy (tid);

    printf ("BTC: TxAdded\n");

    pthread_mutex_lock (&manager->base.lock);
    bool needEvents = true;
    bool needCreateEvent = false;

    BRCryptoWallet  wallet = manager->base.wallet;
    BRWallet       *wid    = cryptoWalletAsBTC(wallet);

    BRCryptoTransferBTC transferBTC = cryptoWalletFindTransferByHashAsBTC (wallet, tid->txHash);
    BRCryptoTransfer    transfer    = (BRCryptoTransfer) transferBTC;

    if (NULL == transfer) {
        // first we've seen it, so it came from the network; add it to our list
        transfer = cryptoTransferCreateAsBTC (wallet->listenerTransfer,
                                              wallet->unit,
                                              wallet->unitForFee,
                                              wid,
                                              tid,
                                              wallet->type);
        transferBTC = cryptoTransferCoerceBTC (transfer);
        cryptoWalletAddTransfer (wallet, transfer);
        needCreateEvent = true;
    }
    else {
        BRTransaction *oldTid = transferBTC->tid;
        BRTransaction *newTid = tid;

        if (transferBTC->isDeleted) {
            // We've seen it before but has already been deleted, somewhow?  We are quietly going
            // to skip out and avoid signalling any events. Perhaps should assert(0) here.
            needEvents = false;
        }
        else {
            // this is a transaction we've submitted; set the reference transaction from the wallet
            oldTid->blockHeight = newTid->blockHeight;
            oldTid->timestamp   = newTid->timestamp;
        }

        // we already have an owned copy of this transaction; free up the passed one
        BRTransactionFree (newTid);
        tid = oldTid;
    }
    assert (NULL != transfer);

    transferBTC->isResolved = BRWalletTransactionIsResolved (wid, tid);

    // Find other transfers that are now resolved.

    size_t transfersCount = array_count (wallet->transfers);
    BRCryptoTransferBTC *resolvedTransfers = calloc (transfersCount, sizeof (BRCryptoTransferBTC));

    size_t resolvedTransactionIndex = 0;
    for (size_t index = 0; index < transfersCount; index++) {
        BRCryptoTransferBTC transferBTC = cryptoTransferCoerceBTC (wallet->transfers[index]);

        bool nowResolved = BRWalletTransactionIsResolved (wid, transferBTC->tid);

        if (nowResolved && !transferBTC->isResolved) {
            transferBTC->isResolved = true;
            resolvedTransfers[resolvedTransactionIndex++] = transferBTC;
        }
    }

    pthread_mutex_unlock (&manager->base.lock);

    if (transferBTC->isResolved)
        cryptoWalletManagerUpdateTransferBTC (&manager->base, wallet, transfer, needCreateEvent, needCreateEvent, true);

    for (size_t index = 0; index < resolvedTransactionIndex; index++)
        cryptoWalletManagerUpdateTransferBTC (&manager->base, wallet, &resolvedTransfers[index]->base, false, false, true);

    // Only one UPDATE BALANCE?

    free (resolvedTransfers);
}

// MARK: - BRWallet Callback TX Updated

static void cryptoWalletManagerBTCTxUpdated (void *info,
                                             const UInt256 *hashes, size_t count,
                                             uint32_t blockHeight,
                                             uint32_t timestamp) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;

    BRCryptoWallet wallet = manager->base.wallet;

    printf ("BTC: TxUpdated\n");
    for (size_t index = 0; index < count; index++) {
        bool needEvents = true;

        pthread_mutex_lock (&manager->base.lock);
        BRCryptoTransferBTC transfer = cryptoWalletFindTransferByHashAsBTC(wallet, hashes[index]);
        assert (NULL != transfer);

        assert (BRTransactionIsSigned (transfer->tid));

        if (transfer->isDeleted) needEvents = false;
        else {
            transfer->tid->blockHeight = blockHeight;
            transfer->tid->timestamp   = timestamp;
        }

        if (transfer->isResolved && TX_UNCONFIRMED != blockHeight) {
            BRCryptoFeeBasis      feeBasis = cryptoFeeBasisTake (transfer->base.feeBasisEstimated);
            BRCryptoTransferState oldState = cryptoTransferGetState (&transfer->base);
            BRCryptoTransferState newState = cryptoTransferStateIncludedInit (blockHeight,
                                                                              0,
                                                                              timestamp,
                                                                              feeBasis,
                                                                              CRYPTO_TRUE,
                                                                              NULL);
            cryptoFeeBasisGive (feeBasis);
            cryptoTransferSetState (&transfer->base, newState);
            pthread_mutex_unlock (&manager->base.lock);

            cryptoTransferStateRelease (&oldState);
            cryptoTransferStateRelease (&newState);
        }
        else pthread_mutex_unlock (&manager->base.lock);
    }
}

// MARK: - BRWallet Callback TX Deleted

static void cryptoWalletManagerBTCTxDeleted (void *info, UInt256 hash, int notifyUser, int recommendRescan) {
    BRCryptoWalletManagerBTC manager = info;
    BRCryptoWallet           wallet = manager->base.wallet;

    bool needEvents = true;
    printf ("BTC: TxDeleted\n");

    pthread_mutex_lock (&manager->base.lock);
    BRCryptoTransferBTC transfer = cryptoWalletFindTransferByHashAsBTC (wallet, hash);

    if (transfer->isDeleted) needEvents = false;
    else {
        transfer->isDeleted = true;
        cryptoTransferSetState (&transfer->base, cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_DELETED));
    }

    pthread_mutex_unlock (&manager->base.lock);
    if (needEvents) {
        if (transfer->isResolved)
            cryptoTransferGenerateEvent (&transfer->base, (BRCryptoTransferEvent) {
                CRYPTO_TRANSFER_EVENT_DELETED
            });

        if (recommendRescan)
            cryptoWalletManagerGenerateEvent (&manager->base, (BRCryptoWalletManagerEvent) {
                CRYPTO_WALLET_MANAGER_EVENT_SYNC_RECOMMENDED,
                { .syncRecommended = { CRYPTO_SYNC_DEPTH_FROM_LAST_CONFIRMED_SEND } }
            });
    }
}

// MARK: BRPeerManager Callbacks

static void cryptoWalletManagerBTCSyncStarted (void *info) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
#ifdef REFACTOR
    BRPeerSyncManager manager = (BRPeerSyncManager) info;

    // This callback occurs when a sync has started. The behaviour of this function is
    // defined as:
    //   - If we are not in a connected state, signal that we are now connected.
    //   - If we were already in a (full scan) syncing state, signal the termination of that
    //     sync
    //   - Always signal the start of a sync

    if (0 == pthread_mutex_lock (&manager->lock)) {
        uint32_t startBlockHeight = BRPeerManagerLastBlockHeight (manager->peerManager);

        uint8_t needConnectionEvent = !manager->isConnected;
        uint8_t needSyncStartedEvent = 1; // syncStarted callback always indicates a full scan
        uint8_t needSyncStoppedEvent = manager->isFullScan;

        manager->isConnected = needConnectionEvent ? 1 : manager->isConnected;
        manager->isFullScan = needSyncStartedEvent ? 1 : manager->isFullScan;
        manager->successfulScanBlockHeight = MIN (startBlockHeight, manager->successfulScanBlockHeight);

        _peer_log ("BSM: syncStarted needConnect:%"PRIu8", needStart:%"PRIu8", needStop:%"PRIu8"\n",
                   needConnectionEvent, needSyncStartedEvent, needSyncStoppedEvent);

        // Send event while holding the state lock so that we
        // don't broadcast a events out of order.

        if (needSyncStoppedEvent) {
            manager->eventCallback (manager->eventContext,
                                    BRPeerSyncManagerAsSyncManager (manager),
                                    (BRSyncManagerEvent) {
                SYNC_MANAGER_SYNC_STOPPED,
                { .syncStopped = { cryptoSyncStoppedReasonRequested() } }
            });
        }

        if (needConnectionEvent) {
            manager->eventCallback (manager->eventContext,
                                    BRPeerSyncManagerAsSyncManager (manager),
                                    (BRSyncManagerEvent) {
                SYNC_MANAGER_CONNECTED,
            });
        }

        if (needSyncStartedEvent) {
            manager->eventCallback (manager->eventContext,
                                    BRPeerSyncManagerAsSyncManager (manager),
                                    (BRSyncManagerEvent) {
                SYNC_MANAGER_SYNC_STARTED,
            });
        }

        pthread_mutex_unlock (&manager->lock);
    } else {
        assert (0);
    }
#endif
}

static void cryptoWalletManagerBTCSyncStopped (void *info, int reason) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
#ifdef REFACTOR
    BRPeerSyncManager manager = (BRPeerSyncManager) info;

    // This callback occurs when a sync has stopped. This MAY mean we have disconnected or it
    // may mean that we have "caught up" to the blockchain. So, we need to first get the connectivity
    // state of the `BRPeerManager`. The behaviour of this function is defined as:
    //   - If we were in a (full scan) syncing state, signal the termination of that
    //     sync
    //   - If we were connected and are now disconnected, signal that we are now disconnected.

    if (0 == pthread_mutex_lock (&manager->lock)) {
        uint8_t isConnected = BRPeerStatusDisconnected != BRPeerManagerConnectStatus (manager->peerManager);

        uint8_t needSyncStoppedEvent = manager->isFullScan;
        uint8_t needDisconnectionEvent = !isConnected && manager->isConnected;
        uint8_t needSuccessfulScanBlockHeightUpdate = manager->isFullScan && isConnected && !reason;

        manager->isConnected = needDisconnectionEvent ? 0 : isConnected;
        manager->isFullScan = needSyncStoppedEvent ? 0 : manager->isFullScan;
        manager->successfulScanBlockHeight =  needSuccessfulScanBlockHeightUpdate ? BRPeerManagerLastBlockHeight (manager->peerManager) : manager->successfulScanBlockHeight;

        _peer_log ("BSM: syncStopped needStop:%"PRIu8", needDisconnect:%"PRIu8"\n",
                   needSyncStoppedEvent, needDisconnectionEvent);

        // Send event while holding the state lock so that we
        // don't broadcast a events out of order.

        if (needSyncStoppedEvent) {
            manager->eventCallback (manager->eventContext,
                                    BRPeerSyncManagerAsSyncManager (manager),
                                    (BRSyncManagerEvent) {
                SYNC_MANAGER_SYNC_STOPPED,
                { .syncStopped = {
                    (reason ?
                     cryptoSyncStoppedReasonPosix(reason) :
                     (isConnected ?
                      cryptoSyncStoppedReasonComplete() :
                      cryptoSyncStoppedReasonRequested()))
                }
                }
            });
        }

        if (needDisconnectionEvent) {
            manager->eventCallback (manager->eventContext,
                                    BRPeerSyncManagerAsSyncManager (manager),
                                    (BRSyncManagerEvent) {
                SYNC_MANAGER_DISCONNECTED,
                { .disconnected = {
                    (reason ?
                     cryptoWalletManagerDisconnectReasonPosix(reason) :
                     cryptoWalletManagerDisconnectReasonRequested())
                }
                }
            });
        }
        pthread_mutex_unlock (&manager->lock);
    } else {
        assert (0);
    }
#endif
}

static void cryptoWalletManagerBTCTxStatusUpdate (void *info) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
#ifdef REFACTOR
    BRPeerSyncManager manager = (BRPeerSyncManager) info;

    // This callback occurs under a number of scenarios.
    //
    // One of those scenario is when a block has been relayed by the P2P network. Thus, it provides an
    // opportunity to get the current block height and update accordingly.
    //
    // The behaviour of this function is defined as:
    //   - If the block height has changed, signal the new value

    if (0 == pthread_mutex_lock (&manager->lock)) {
        uint64_t blockHeight = BRPeerManagerLastBlockHeight (manager->peerManager);

        uint8_t needBlockHeightEvent = blockHeight > manager->networkBlockHeight;

        // Never move the block height "backwards"; always maintain our knowledge
        // of the maximum height observed
        manager->networkBlockHeight = MAX (blockHeight, manager->networkBlockHeight);

        // Send event while holding the state lock so that we
        // don't broadcast a events out of order.

        if (needBlockHeightEvent) {
            manager->eventCallback (manager->eventContext,
                                    BRPeerSyncManagerAsSyncManager (manager),
                                    (BRSyncManagerEvent) {
                SYNC_MANAGER_BLOCK_HEIGHT_UPDATED,
                { .blockHeightUpdated = { blockHeight }}
            });
        }

        manager->eventCallback (manager->eventContext,
                                BRPeerSyncManagerAsSyncManager (manager),
                                (BRSyncManagerEvent) {
            SYNC_MANAGER_TXNS_UPDATED
        });

        pthread_mutex_unlock (&manager->lock);
    } else {
        assert (0);
    }
#endif
}

static void cryptoWalletManagerBTCSaveBlocks (void *info, int replace, BRMerkleBlock **blocks, size_t count) {
    BRCryptoWalletManagerBTC manager = info;

    if (replace) {
        fileServiceReplace (manager->base.fileService, fileServiceTypeBlocksBTC, (const void **) blocks, count);
    }
    else {
        for (size_t index = 0; index < count; index++)
            fileServiceSave (manager->base.fileService, fileServiceTypeBlocksBTC, blocks[index]);
    }
}

static void cryptoWalletManagerBTCSavePeers  (void *info, int replace, const BRPeer *peers, size_t count) {
    BRCryptoWalletManagerBTC manager = info;

    // filesystem changes are NOT queued; they are acted upon immediately

    if (!replace) {
        // save each peer, one-by-one
        for (size_t index = 0; index < count; index++)
            fileServiceSave (manager->base.fileService, fileServiceTypePeersBTC, &peers[index]);
    }

    else if (0 == count) {
        // no peers to set, just do a clear
        fileServiceClear (manager->base.fileService, fileServiceTypePeersBTC);
    }

    else {
        // fileServiceReplace expects an array of pointers to entities, instead of an array of
        // structures so let's do the conversion here
        const BRPeer **peerRefs = calloc (count, sizeof(BRPeer *));

        for (size_t i = 0; i < count; i++) {
            peerRefs[i] = &peers[i];
        }

        fileServiceReplace (manager->base.fileService, fileServiceTypePeersBTC, (const void **) peerRefs, count);
        free (peerRefs);
    }
}

static int  cryptoWalletManagerBTCNetworkIsReachable (void *info) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
    return 1;
#ifdef REFACTOR
    BRPeerSyncManager manager = (BRPeerSyncManager) info;
    return BRPeerSyncManagerGetNetworkReachable (manager);
#endif
}

static void cryptoWalletManagerBTCThreadCleanup (void *info) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
#ifdef REFACTOR
#endif
}

static void cryptoWalletManagerBTCTxPublished (void *info, int error) {
    BRCryptoClientP2PManagerPublishInfo *btcInfo = info;
    BRCryptoWalletManager manager = btcInfo->manager;
    BRCryptoTransfer     transfer = btcInfo->transfer;

    pthread_mutex_lock (&manager->lock);

    BRCryptoWallet wallet = manager->wallet;
    assert (cryptoWalletHasTransfer (wallet, transfer));

    BRCryptoTransferState oldState = cryptoTransferGetState (transfer);
    assert (CRYPTO_TRANSFER_STATE_SUBMITTED != oldState.type);

    BRCryptoTransferState newState = cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_SUBMITTED);
    cryptoTransferSetState (transfer, newState);

    pthread_mutex_unlock (&manager->lock);
#if 0
    cryptoWalletManagerGenerateTransferEvent (manager, wallet, transfer,
                                              (BRCryptoTransferEvent) {
        CRYPTO_TRANSFER_EVENT_CHANGED,
        { .state = { oldState, newState }}
    });
#endif
    cryptoWalletManagerGive(manager);
    cryptoTransferGive (transfer);
    free (info);
}

const BREventType *eventTypesBTC[] = {
#if 0
    &bwmSignalTxAddedEventType,
    &bwmSignalTxUpdatedEventType,
    &bwmSignalTxDeletedEventType,

    &bwmWalletManagerEventType,
    &bwmWalletEventType,
    &bwmTransactionEventType,

    &bwmClientAnnounceBlockNumberEventType,
    &bwmClientAnnounceTransactionEventType,
    &bwmClientAnnounceTransactionCompleteEventType,
    &bwmClientAnnounceSubmitEventType,
#endif
};

const unsigned int
eventTypesCountBTC = (sizeof (eventTypesBTC) / sizeof (BREventType*));

BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersBTC = {
    cryptoWalletManagerCreateBTC,
    cryptoWalletManagerReleaseBTC,
    cryptoWalletManagerInitializeBTC,
    crytpWalletManagerCreateFileServiceBTC,
    cryptoWalletManagerGetEventTypesBTC,
    cryptoWalletManagerCreateWalletBTC,
    cryptoWalletManagerSignTransactionWithSeedBTC,
    cryptoWalletManagerSignTransactionWithKeyBTC,
    cryptoWalletManagerEstimateLimitBTC,
    cryptoWalletManagerEstimateFeeBasisBTC,
    crytpWalletManagerCreateP2PManagerBTC,
    cryptoWalletManagerRecoverTransfersFromTransactionBundleBTC,
    cryptoWalletManagerRecoverTransferFromTransferBundleBTC,
    cryptoWalletManagerWalletSweeperValidateSupportedBTC,
    cryptoWalletManagerCreateWalletSweeperBTC
};

BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersBCH = {
    cryptoWalletManagerCreateBTC,
    cryptoWalletManagerReleaseBTC,
    cryptoWalletManagerInitializeBTC,
    crytpWalletManagerCreateFileServiceBTC,
    cryptoWalletManagerGetEventTypesBTC,
    cryptoWalletManagerCreateWalletBTC,
    cryptoWalletManagerSignTransactionWithSeedBTC,
    cryptoWalletManagerSignTransactionWithKeyBTC,
    cryptoWalletManagerEstimateLimitBTC,
    cryptoWalletManagerEstimateFeeBasisBTC,
    crytpWalletManagerCreateP2PManagerBTC,
    cryptoWalletManagerRecoverTransfersFromTransactionBundleBTC,
    cryptoWalletManagerRecoverTransferFromTransferBundleBTC,
    cryptoWalletManagerWalletSweeperValidateSupportedBTC,
    cryptoWalletManagerCreateWalletSweeperBTC
};

BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersBSV = {
    cryptoWalletManagerCreateBTC,
    cryptoWalletManagerReleaseBTC,
    cryptoWalletManagerInitializeBTC,
    crytpWalletManagerCreateFileServiceBTC,
    cryptoWalletManagerGetEventTypesBTC,
    cryptoWalletManagerCreateWalletBTC,
    cryptoWalletManagerSignTransactionWithSeedBTC,
    cryptoWalletManagerSignTransactionWithKeyBTC,
    cryptoWalletManagerEstimateLimitBTC,
    cryptoWalletManagerEstimateFeeBasisBTC,
    crytpWalletManagerCreateP2PManagerBTC,
    cryptoWalletManagerRecoverTransfersFromTransactionBundleBTC,
    cryptoWalletManagerRecoverTransferFromTransferBundleBTC,
    cryptoWalletManagerWalletSweeperValidateSupportedBTC,
    cryptoWalletManagerCreateWalletSweeperBTC
};
