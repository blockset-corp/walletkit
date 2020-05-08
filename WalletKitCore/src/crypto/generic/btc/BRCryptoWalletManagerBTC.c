#include "BRCryptoBTC.h"

#include "../../BRCryptoAccountP.h"
#include "../../BRCryptoNetworkP.h"
#include "../../BRCryptoKeyP.h"
#include "../../BRCryptoClientP.h"
#include "../../BRCryptoWalletManagerP.h"

#include "bitcoin/BRWallet.h"
#include "bitcoin/BRPeerManager.h"

// BRFileService

static BRArrayOf(BRTransaction*) initialTransactionsLoad (BRCryptoWalletManager manager);
static BRArrayOf(BRPeer)         initialPeersLoad (BRCryptoWalletManager manager);
static BRArrayOf(BRMerkleBlock*) initialBlocksLoad (BRCryptoWalletManager manager);
static BRFileServiceTypeSpecification fileServiceSpecifications[];
static size_t fileServiceSpecificationsCount;

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

struct BRCryptoWalletManagerBTCRecord {
    struct BRCryptoWalletManagerRecord base;

    int ignoreTBD;
};

static BRCryptoWalletManagerBTC
cryptoWalletManagerCoerce (BRCryptoWalletManager wallet) {
    assert (CRYPTO_NETWORK_TYPE_BTC == wallet->type);
    return (BRCryptoWalletManagerBTC) wallet;
}

static BRCryptoWalletManager
cryptoWalletManagerCreateHandlerBTC (BRCryptoListener listener,
                                     BRCryptoClient client,
                                     BRCryptoAccount account,
                                     BRCryptoNetwork network,
                                     BRCryptoSyncMode mode,
                                     BRCryptoAddressScheme scheme,
                                     const char *path) {
    BRCryptoWalletManager managerBase = cryptoWalletManagerAllocAndInit (sizeof (struct BRCryptoWalletManagerBTCRecord),
                                                                         cryptoNetworkGetType(network),
                                                                         listener,
                                                                         client,
                                                                         account,
                                                                         network,
                                                                         scheme,
                                                                         path,
                                                                         CRYPTO_CLIENT_QRY_GET_TRANSACTIONS);
    BRCryptoWalletManagerBTC manager = cryptoWalletManagerCoerce (managerBase);

    // BTC Stuff
    manager->ignoreTBD = 0;

    return managerBase;
}

static void
cryptoWalletManagerReleaseHandlerBTC (BRCryptoWalletManager manager) {

}

static void
cryptoWalletManagerInitializeHandlerBTC (BRCryptoWalletManager manager) {
    // Get the btcMasterPublicKey
    BRMasterPubKey btcMPK = cryptoAccountAsBTC(manager->account);

    // Get the btcChainParams
    const BRChainParams *btcChainParams = cryptoNetworkAsBTC(manager->network);

    // Load the BTC transactions from the fileService
    BRArrayOf(BRTransaction*) transactions = initialTransactionsLoad (manager);
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
                          cryptoWalletManagerCoerce(manager),
                          cryptoWalletManagerBTCBalanceChanged,
                          cryptoWalletManagerBTCTxAdded,
                          cryptoWalletManagerBTCTxUpdated,
                          cryptoWalletManagerBTCTxDeleted);

    // Create the primary BRCryptoWallet
    BRCryptoNetwork  network       = manager->network;
    BRCryptoCurrency currency      = cryptoNetworkGetCurrency (network);
    BRCryptoUnit     unitAsBase    = cryptoNetworkGetUnitAsBase    (network, currency);
    BRCryptoUnit     unitAsDefault = cryptoNetworkGetUnitAsDefault (network, currency);

    manager->wallet = cryptoWalletCreateAsBTC (unitAsDefault, unitAsBase, btcWallet);
    array_add (manager->wallets, manager->wallet);

    // Process existing btcTransactions in the btcWallet into BRCryptoTransfers
    size_t btcTransactionsCount = BRWalletTransactions(btcWallet, NULL, 0);
    BRTransaction *btcTransactions[btcTransactionsCount > 0 ? btcTransactionsCount : 1]; // avoid a static analysis error
    BRWalletTransactions (btcWallet, btcTransactions, btcTransactionsCount);

    for (size_t index = 0; index < btcTransactionsCount; index++) {
        BRCryptoTransfer transfer = cryptoTransferCreateAsBTC (unitAsDefault,
                                                               unitAsBase,
                                                               btcWallet,
                                                               BRTransactionCopy(btcTransactions[index]),
                                                               btcTransactions[index],
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
                                                   fileServiceSpecificationsCount,
                                                   fileServiceSpecifications);
}

static const BREventType **
cryptoWalletManagerGetEventTypesBTC (BRCryptoWalletManager manager,
                                     size_t *eventTypesCount) {
    assert (NULL != eventTypesCount);
    *eventTypesCount = bwmEventTypesCount;
    return bwmEventTypes;
}

#if 0
static BRArrayOf(BRCryptoWallet)
cryptoWalletManagerCreateWalletsHandlerBTC (BRCryptoWalletManager manager,
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
cryptoWalletManagerSignTransactionWithSeedHandlerBTC (BRCryptoWalletManager manager,
                                                      BRCryptoWallet wallet,
                                                      BRCryptoTransfer transfer,
                                                      UInt512 seed) {
    BRWallet      *btcWallet       = cryptoWalletAsBTC   (wallet);
    BRTransaction *btcTransaction  = cryptoTransferAsBTC (transfer);         // OWN/REF ?
    const BRChainParams *btcParams = cryptoNetworkAsBTC  (manager->network);

    return AS_CRYPTO_BOOLEAN (1 == BRWalletSignTransaction (btcWallet, btcTransaction, btcParams->forkId, seed.u8, sizeof(UInt512)));
}

static BRCryptoBoolean
cryptoWalletManagerSignTransactionWithKeyHandlerBTC (BRCryptoWalletManager manager,
                                                     BRCryptoWallet wallet,
                                                     BRCryptoTransfer transfer,
                                                     BRCryptoKey key) {
    BRTransaction *btcTransaction  = cryptoTransferAsBTC (transfer);         // OWN/REF ?
    BRKey         *btcKey          = cryptoKeyGetCore (key);
    const BRChainParams *btcParams = cryptoNetworkAsBTC  (manager->network);

    return AS_CRYPTO_BOOLEAN (1 == BRTransactionSign (btcTransaction, btcParams->forkId, btcKey, 1));
}

static BRCryptoAmount
cryptoWalletManagerEstimateLimitHandlerBTC (BRCryptoWalletManager cwm,
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

static void
cryptoWalletManagerEstimateFeeBasisHandlerBTC (BRCryptoWalletManager cwm,
                                               BRCryptoWallet  wallet,
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

    (void) btcSizeInBytes;
#ifdef REFACTOR
    bwmSignalWalletEvent(manager,
                         wallet,
                         (BRWalletEvent) {
        BITCOIN_WALLET_FEE_ESTIMATED,
        { .feeEstimated = { cookie, btcFeePerKB, btcSizeInBytes }}
    });
#endif
    return;
}

static void
cryptoWalletManagerRecoverTransfersFromTransactionBundleHandlerBTC (BRCryptoWalletManager manager,
                                                                    OwnershipKept BRCryptoClientTransactionBundle bundle) {
    BRTransaction *btcTransaction = BRTransactionParse (bundle->serialization, bundle->serializationCount);

    bool error = TRANSFER_STATUS_ERRORED != bundle->status;
    bool needRegistration = (!error && NULL != btcTransaction && BRTransactionIsSigned (btcTransaction));
    bool needFree = true;

    BRWallet *btcWallet = cryptoWalletAsBTC(manager->wallet);

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
                                        (uint32_t) bundle->blockHeight,
                                        bundle->timestamp);
        }
    }

    // Free if ownership hasn't been passed
    if (needFree) {
        BRTransactionFree (btcTransaction);
    }
}

static void
cryptoWalletManagerRecoverTransferFromTransferBundleHandlerBTC (BRCryptoWalletManager cwm,
                                                                OwnershipKept BRCryptoClientTransferBundle bundle) {
    // Not BTC functionality
    assert (0);
    return NULL;
}


// MARK: - Client P2P

typedef struct BRCryptoClientP2PManagerRecordBTC {
    struct BRCryptoClientP2PManagerRecord base;
    BRCryptoWalletManagerBTC manager;
    BRPeerManager *btcPeerManager;
} *BRCryptoClientP2PManagerBTC;

static BRCryptoClientP2PManagerBTC
cryptoClientP2PManagerCoerce (BRCryptoClientP2PManager managerBase) {
    assert (CRYPTO_NETWORK_TYPE_BTC == managerBase->type);
    return (BRCryptoClientP2PManagerBTC) managerBase;
}

static void
cryptoClientP2PManagerReleaseHandlerBTC (BRCryptoClientP2PManager baseManager) {
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);
    BRPeerManagerFree (manager->btcPeerManager);
}

static void
cryptoClientP2PManagerConnectHandlerBTC (BRCryptoClientP2PManager baseManager,
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
cryptoClientP2PManagerDisconnectHandlerBTC (BRCryptoClientP2PManager baseManager) {
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
cryptoClientP2PManagerSyncHandlerBTC (BRCryptoClientP2PManager baseManager,
                                      BRCryptoSyncDepth depth,
                                      BRCryptoBlockChainHeight height) {
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
cryptoClientP2PManagerSendHandlerBTC (BRCryptoClientP2PManager baseManager, BRCryptoTransfer transfer) {
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
    cryptoClientP2PManagerReleaseHandlerBTC,
    cryptoClientP2PManagerConnectHandlerBTC,
    cryptoClientP2PManagerDisconnectHandlerBTC,
    cryptoClientP2PManagerSyncHandlerBTC,
    cryptoClientP2PManagerSendHandlerBTC
};

static BRCryptoClientP2PManager
crytpWalletManagerCreateP2PManagerHandlerBTC (BRCryptoWalletManager cwm) {
    // load blocks, load peers

    BRCryptoClientP2PManager baseManager = cryptoClientP2PManagerCreate (sizeof (struct BRCryptoClientP2PManagerRecordBTC),
                                                                         CRYPTO_NETWORK_TYPE_BTC,
                                                                         &p2pHandlersBTC);
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);
    manager->manager = cryptoWalletManagerCoerce(cwm);

    const BRChainParams *btcChainParams = cryptoNetworkAsBTC(cwm->network);
    BRWallet *btcWallet = cryptoWalletAsBTC(cwm->wallet);
    uint32_t btcEarliestKeyTime = (uint32_t) cryptoAccountGetTimestamp(cwm->account);

    BRArrayOf(BRMerkleBlock*) blocks = initialBlocksLoad (cwm);
    BRArrayOf(BRPeer)         peers  = initialPeersLoad  (cwm);

    manager->btcPeerManager = BRPeerManagerNew (btcChainParams,
                                                btcWallet,
                                                btcEarliestKeyTime,
                                                blocks, (NULL == blocks ? 0 : array_count (blocks)),
                                                peers,  (NULL == peers  ? 0 : array_count (peers)));


    assert (NULL != manager->btcPeerManager);

    BRPeerManagerSetCallbacks (manager->btcPeerManager,
                               cryptoWalletManagerCoerce(cwm),
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

/// MARK: - Transaction File Service

#define fileServiceTypeTransactions     "transactions"

enum {
    WALLET_MANAGER_TRANSACTION_VERSION_1
};

static UInt256
fileServiceTypeTransactionV1Identifier (BRFileServiceContext context,
                                        BRFileService fs,
                                        const void *entity) {
    const BRTransaction *transaction = entity;
    return transaction->txHash;
}

static uint8_t *
fileServiceTypeTransactionV1Writer (BRFileServiceContext context,
                                    BRFileService fs,
                                    const void* entity,
                                    uint32_t *bytesCount) {
    const BRTransaction *transaction = entity;

    size_t txTimestampSize  = sizeof (uint32_t);
    size_t txBlockHeightSize = sizeof (uint32_t);
    size_t txSize = BRTransactionSerialize (transaction, NULL, 0);

    assert (txTimestampSize   == sizeof(transaction->timestamp));
    assert (txBlockHeightSize == sizeof(transaction->blockHeight));

    *bytesCount = (uint32_t) (txSize + txBlockHeightSize + txTimestampSize);

    uint8_t *bytes = calloc (*bytesCount, 1);

    size_t bytesOffset = 0;

    BRTransactionSerialize (transaction, &bytes[bytesOffset], txSize);
    bytesOffset += txSize;

    UInt32SetLE (&bytes[bytesOffset], transaction->blockHeight);
    bytesOffset += txBlockHeightSize;

    UInt32SetLE(&bytes[bytesOffset], transaction->timestamp);

    return bytes;
}

static void *
fileServiceTypeTransactionV1Reader (BRFileServiceContext context,
                                    BRFileService fs,
                                    uint8_t *bytes,
                                    uint32_t bytesCount) {
    size_t txTimestampSize  = sizeof (uint32_t);
    size_t txBlockHeightSize = sizeof (uint32_t);
    if (bytesCount < (txTimestampSize + txBlockHeightSize)) return NULL;

    BRTransaction *transaction = BRTransactionParse (bytes, bytesCount - txTimestampSize - txBlockHeightSize);
    if (NULL == transaction) return NULL;

    transaction->blockHeight = UInt32GetLE (&bytes[bytesCount - txTimestampSize - txBlockHeightSize]);
    transaction->timestamp   = UInt32GetLE (&bytes[bytesCount - txTimestampSize]);

    return transaction;
}

static BRArrayOf(BRTransaction*)
initialTransactionsLoad (BRCryptoWalletManager manager) {
    BRSetOf(BRTransaction*) transactionSet = BRSetNew(BRTransactionHash, BRTransactionEq, 100);
    if (1 != fileServiceLoad (manager->fileService, transactionSet, fileServiceTypeTransactions, 1)) {
        BRSetFreeAll(transactionSet, (void (*) (void*)) BRTransactionFree);
        _peer_log ("BWM: failed to load transactions");
        return NULL;
    }

    size_t transactionsCount = BRSetCount(transactionSet);

    BRArrayOf(BRTransaction*) transactions;
    array_new (transactions, transactionsCount);
    array_set_count(transactions, transactionsCount);

    BRSetAll(transactionSet, (void**) transactions, transactionsCount);
    BRSetFree(transactionSet);

    _peer_log ("BWM: loaded %zu transactions\n", transactionsCount);
    return transactions;
}

/// MARK: - Block File Service

#define fileServiceTypeBlocks       "blocks"
enum {
    WALLET_MANAGER_BLOCK_VERSION_1
};

static UInt256
fileServiceTypeBlockV1Identifier (BRFileServiceContext context,
                                  BRFileService fs,
                                  const void *entity) {
    const BRMerkleBlock *block = (BRMerkleBlock*) entity;
    return block->blockHash;
}

static uint8_t *
fileServiceTypeBlockV1Writer (BRFileServiceContext context,
                              BRFileService fs,
                              const void* entity,
                              uint32_t *bytesCount) {
    const BRMerkleBlock *block = entity;

    // The serialization of a block does not include the block height.  Thus, we'll need to
    // append the height.

    // These are serialization sizes
    size_t blockHeightSize = sizeof (uint32_t);
    size_t blockSize = BRMerkleBlockSerialize(block, NULL, 0);

    // Confirm.
    assert (blockHeightSize == sizeof (block->height));

    // Update bytesCound with the total of what is written.
    *bytesCount = (uint32_t) (blockSize + blockHeightSize);

    // Get our bytes
    uint8_t *bytes = calloc (*bytesCount, 1);

    // We'll serialize the block itself first
    BRMerkleBlockSerialize(block, bytes, blockSize);

    // And then the height.
    UInt32SetLE(&bytes[blockSize], block->height);

    return bytes;
}

static void *
fileServiceTypeBlockV1Reader (BRFileServiceContext context,
                              BRFileService fs,
                              uint8_t *bytes,
                              uint32_t bytesCount) {
    size_t blockHeightSize = sizeof (uint32_t);
    if (bytesCount < blockHeightSize) return NULL;

    BRMerkleBlock *block = BRMerkleBlockParse (bytes, bytesCount - blockHeightSize);
    if (NULL == block) return NULL;

    block->height = UInt32GetLE(&bytes[bytesCount - blockHeightSize]);

    return block;
}

static BRArrayOf(BRMerkleBlock*)
initialBlocksLoad (BRCryptoWalletManager manager) {
    BRSetOf(BRMerkleBlock*) blockSet = BRSetNew(BRMerkleBlockHash, BRMerkleBlockEq, 100);
    if (1 != fileServiceLoad (manager->fileService, blockSet, fileServiceTypeBlocks, 1)) {
        BRSetFreeAll(blockSet, (void (*) (void*)) BRMerkleBlockFree);
        _peer_log ("BWM: failed to load blocks");
        return NULL;
    }

    size_t blocksCount = BRSetCount(blockSet);

    BRArrayOf(BRMerkleBlock*) blocks;
    array_new (blocks, blocksCount);
    array_set_count(blocks, blocksCount);

    BRSetAll(blockSet, (void**) blocks, blocksCount);
    BRSetFree(blockSet);

    _peer_log ("BWM: loaded %zu blocks\n", blocksCount);
    return blocks;
}

/// MARK: - Peer File Service

#define fileServiceTypePeers        "peers"
enum {
    WALLET_MANAGER_PEER_VERSION_1
};

static UInt256
fileServiceTypePeerV1Identifier (BRFileServiceContext context,
                                 BRFileService fs,
                                 const void *entity) {
    const BRPeer *peer = entity;

    UInt256 hash;
    BRSHA256 (&hash, peer, sizeof(BRPeer));

    return hash;
}

static uint8_t *
fileServiceTypePeerV1Writer (BRFileServiceContext context,
                             BRFileService fs,
                             const void* entity,
                             uint32_t *bytesCount) {
    const BRPeer *peer = entity;
    size_t offset = 0;

    *bytesCount = sizeof (BRPeer);
    uint8_t *bytes = malloc (*bytesCount);

    memcpy (&bytes[offset], peer->address.u8, sizeof (UInt128));
    offset += sizeof (UInt128);

    UInt16SetBE (&bytes[offset], peer->port);
    offset += sizeof (uint16_t);

    UInt64SetBE (&bytes[offset], peer->services);
    offset += sizeof (uint64_t);

    UInt64SetBE (&bytes[offset], peer->timestamp);
    offset += sizeof (uint64_t);

    bytes[offset] = peer->flags;
    offset += sizeof(uint8_t); (void) offset;

    return bytes;
}

static void *
fileServiceTypePeerV1Reader (BRFileServiceContext context,
                             BRFileService fs,
                             uint8_t *bytes,
                             uint32_t bytesCount) {
    assert (bytesCount == sizeof (BRPeer));

    size_t offset = 0;

    BRPeer *peer = malloc (bytesCount);

    memcpy (peer->address.u8, &bytes[offset], sizeof (UInt128));
    offset += sizeof (UInt128);

    peer->port = UInt16GetBE (&bytes[offset]);
    offset += sizeof (uint16_t);

    peer->services = UInt64GetBE(&bytes[offset]);
    offset += sizeof (uint64_t);

    peer->timestamp = UInt64GetBE(&bytes[offset]);
    offset += sizeof (uint64_t);

    peer->flags = bytes[offset];
    offset += sizeof(uint8_t); (void) offset;

    return peer;
}

static BRArrayOf(BRPeer)
initialPeersLoad (BRCryptoWalletManager manager) {
    /// Load peers for the wallet manager.
    BRSetOf(BRPeer*) peerSet = BRSetNew(BRPeerHash, BRPeerEq, 100);
    if (1 != fileServiceLoad (manager->fileService, peerSet, fileServiceTypePeers, 1)) {
        BRSetFreeAll(peerSet, free);
        _peer_log ("BWM: failed to load peers");
        return NULL;
    }

    size_t peersCount = BRSetCount(peerSet);

    BRArrayOf(BRPeer) peers;
    array_new (peers, peersCount);

    FOR_SET (BRPeer*, peer, peerSet) array_add (peers, *peer);
    BRSetFreeAll(peerSet, free);

    _peer_log ("BWM: loaded %zu peers\n", peersCount);
    return peers;
}

static BRFileServiceTypeSpecification fileServiceSpecifications[] = {
    {
        fileServiceTypeTransactions,
        WALLET_MANAGER_TRANSACTION_VERSION_1,
        1,
        {
            {
                WALLET_MANAGER_TRANSACTION_VERSION_1,
                fileServiceTypeTransactionV1Identifier,
                fileServiceTypeTransactionV1Reader,
                fileServiceTypeTransactionV1Writer
            }
        }
    },

    {
        fileServiceTypeBlocks,
        WALLET_MANAGER_BLOCK_VERSION_1,
        1,
        {
            {
                WALLET_MANAGER_BLOCK_VERSION_1,
                fileServiceTypeBlockV1Identifier,
                fileServiceTypeBlockV1Reader,
                fileServiceTypeBlockV1Writer
            }
        }
    },

    {
        fileServiceTypePeers,
        WALLET_MANAGER_PEER_VERSION_1,
        1,
        {
            {
                WALLET_MANAGER_PEER_VERSION_1,
                fileServiceTypePeerV1Identifier,
                fileServiceTypePeerV1Reader,
                fileServiceTypePeerV1Writer
            }
        }
    }
};
static size_t fileServiceSpecificationsCount = (sizeof (fileServiceSpecifications) /
                                                sizeof (BRFileServiceTypeSpecification));



// MARK: BRWallet Callback Balance Changed

static void cryptoWalletManagerBTCBalanceChanged (void *info, uint64_t balanceInSatoshi) {
    BRCryptoWalletManagerBTC manager = info;
    printf ("BTC: BalanceChanged");
    (void) manager;
}

// MARK: - BRWallet Callback TX Added

static void
bwmHandleTxAdded (BRCryptoWalletManager manager,
                  OwnershipGiven BRTransaction *ownedTransaction,
                  OwnershipKept BRTransaction *refedTransaction) {
    printf ("BTC: TxAdded");
// #ifdef REFACTOR
    pthread_mutex_lock (&manager->lock);
    bool needEvents = true;

    BRCryptoWallet  wallet    = manager->wallet;
    BRWallet       *btcWallet = cryptoWalletAsBTC(wallet);

    BRCryptoTransfer    transfer    = NULL;
    BRCryptoTransferBTC transferBTC = NULL;

    // Look for an existing transfer matching `txHash`
    BRCryptoHash hash = cryptoHashCreateAsBTC (ownedTransaction->txHash);
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        if (CRYPTO_TRUE == cryptoHashEqual (hash, cryptoTransferGetHash(wallet->transfers[index]))) {
            transfer = wallet->transfers[index];
            transferBTC = cryptoTransferCoerceBTC (transfer);
            break;
        }
    }

    if (NULL == transfer) {
        // first we've seen it, so it came from the network; add it to our list
        transfer = cryptoTransferCreateAsBTC (wallet->unit,
                                              wallet->unitForFee,
                                              btcWallet,
                                              ownedTransaction,
                                              refedTransaction,
                                              wallet->type);
        transferBTC = cryptoTransferCoerceBTC (transfer);

        // TODO: Add to Wallet
    }
    else {
        if (transferBTC->isDeleted) {
            // We've seen it before but has already been deleted, somewhow?  We are quietly going
            // to skip out and avoid signalling any events. Perhaps should assert(0) here.
            needEvents = false;
        }
        else {
            // this is a transaction we've submitted; set the reference transaction from the wallet
            transferBTC->refedTransaction = refedTransaction;
            // TODO: This v
            // BRTransactionWithStateSetBlock (txnWithState, ownedTransaction->blockHeight, ownedTransaction->timestamp);
        }

        // we already have an owned copy of this transaction; free up the passed one
        BRTransactionFree (ownedTransaction);
    }
    assert (NULL != transfer);

    transferBTC->isResolved = BRWalletTransactionIsResolved (btcWallet, refedTransaction);

    // Find other transfers that are now resolved.

    size_t transfersCount = array_count (wallet->transfers);
    BRCryptoTransferBTC *resolvedTransfers = calloc (transfersCount, sizeof (BRCryptoTransferBTC));

    size_t resolvedTransactionIndex = 0;
    for (size_t index = 0; index < transfersCount; index++) {
        BRCryptoTransferBTC transferBTC = cryptoTransferCoerceBTC (wallet->transfers[index]);

        bool nowResolved = BRWalletTransactionIsResolved (btcWallet, transferBTC->refedTransaction);

        if (nowResolved && !transferBTC->isResolved) {
            transferBTC->isResolved = true;
            resolvedTransfers[resolvedTransactionIndex++] = transferBTC;
        }
    }

    pthread_mutex_unlock (&manager->lock);

#if REFACTOR
    if (transferBTC->isResolved && needEvents)
        bwmGenerateAddedEvents (manager, txnWithState);

    for (size_t index = 0; index < resolvedTransactionIndex; index++)
        bwmGenerateAddedEvents (manager, resolvedTransactions[index]);
#endif

    free (resolvedTransfers);

//#endif
}

typedef struct {
    struct BREventRecord base;
    BRCryptoWalletManager manager;
    BRTransaction *ownedTransaction;
    BRTransaction *refedTransaction;
} BRCryptoWalletManagerWalletTxAddedEvent;

static void
bwmSignalTxAddedDispatcher (BREventHandler ignore,
                            BRCryptoWalletManagerWalletTxAddedEvent *event) {
    bwmHandleTxAdded(event->manager, event->ownedTransaction, event->refedTransaction);
}

static void
bwmSignalTxAddedDestroyer (BRCryptoWalletManagerWalletTxAddedEvent *event) {
    BRTransactionFree (event->ownedTransaction);
}

static BREventType bwmSignalTxAddedEventType = {
    "BTC: Wallet TX Added Event",
    sizeof (BRCryptoWalletManagerWalletTxAddedEvent),
    (BREventDispatcher) bwmSignalTxAddedDispatcher,
    (BREventDestroyer) bwmSignalTxAddedDestroyer
};

extern void
bwmSignalTxAdded (BRCryptoWalletManager manager,
                  OwnershipGiven BRTransaction *ownedTransaction,
                  OwnershipKept BRTransaction *refedTransaction) {
    BRCryptoWalletManagerWalletTxAddedEvent message =
    { { NULL, &bwmSignalTxAddedEventType}, manager, ownedTransaction, refedTransaction};
    eventHandlerSignalEvent (manager->handler, (BREvent*) &message);
}

static void cryptoWalletManagerBTCTxAdded   (void *info, BRTransaction *tx) {
    BRCryptoWalletManagerBTC manager = info;
    bwmSignalTxAdded (&manager->base, BRTransactionCopy (tx), tx);

}

// MARK: - BRWallet Callback TX Updated

static void
bwmHandleTxUpdated (BRCryptoWalletManager manager,
                    UInt256 hash,
                    uint32_t blockHeight,
                    uint32_t timestamp) {
    printf ("BTC: TxUpdated");
#ifdef REFACTOR
    pthread_mutex_lock (&manager->lock);

    // We anticapte a case whereby a transaction has seen `txDeleted` but then `txUpdated`.  At
    // least we postulate such a case because IOS crashes seem to indicate that it has occurred.
    //
    // A txDeleted is currently only produced in a BRSyncMode of P2P; but this might change as
    // BlockSet deletes transactions from the mempool.
    int needEvents = 1;

    // Find the transaction by `hash` but in the lookup ignore the deleted flag.
    BRTransactionWithState txnWithState = BRWalletManagerFindTransactionByHash (manager, hash, 1);

    // A transaction must have been found, even if it is flagged as deleted.
    assert (NULL != txnWithState);

    // The transaction must be signed to be updated
    assert (BRTransactionIsSigned (BRTransactionWithStateGetOwned (txnWithState)));

    // If the transaction is deleted.... we'll avoid setting the block and the subsequent event.
    if (txnWithState->isDeleted) needEvents = 0;
    else {
        BRTransactionWithStateSetBlock (txnWithState, blockHeight, timestamp);
    }
    pthread_mutex_unlock (&manager->lock);

    if (txnWithState->isResolved && needEvents) {
        bwmSignalTransactionEvent(manager,
                                  manager->wallet,
                                  BRTransactionWithStateGetOwned (txnWithState),
                                  (BRTransactionEvent) {
            BITCOIN_TRANSACTION_UPDATED,
            { .updated = { blockHeight, timestamp }}
        });
    }
#endif
}

typedef struct {
    struct BREventRecord base;
    BRCryptoWalletManager manager;
    UInt256 hash;
    uint32_t blockHeight;
    uint32_t timestamp;
} BRCryptoWalletManagerWalletTxUpdatedEvent;

static void
bwmSignalTxUpdatedDispatcher (BREventHandler ignore,
                              BRCryptoWalletManagerWalletTxUpdatedEvent *event) {
    bwmHandleTxUpdated(event->manager, event->hash, event->blockHeight, event->timestamp);
}

static BREventType bwmSignalTxUpdatedEventType = {
    "BTC: Wallet TX Updated Event",
    sizeof (BRCryptoWalletManagerWalletTxUpdatedEvent),
    (BREventDispatcher) bwmSignalTxUpdatedDispatcher
};

static void
bwmSignalTxUpdated (BRCryptoWalletManager manager,
                    UInt256 hash,
                    uint32_t blockHeight,
                    uint32_t timestamp) {
    BRCryptoWalletManagerWalletTxUpdatedEvent message =
    { { NULL, &bwmSignalTxUpdatedEventType}, manager, hash, blockHeight, timestamp};
    eventHandlerSignalEvent (manager->handler, (BREvent*) &message);
}

static void cryptoWalletManagerBTCTxUpdated (void *info, const UInt256 *hashes, size_t count, uint32_t blockHeight, uint32_t timestamp) {
    BRCryptoWalletManagerBTC manager = info;
    for (size_t index = 0; index < count; index++)
        bwmSignalTxUpdated (&manager->base, hashes[index], blockHeight, timestamp);
}

// MARK: - BRWallet Callback TX Deleted

static void
bwmHandleTxDeleted (BRCryptoWalletManager manager,
                    UInt256 hash,
                    int recommendRescan) {
    printf ("BTC: TxDeleted");
#ifdef REFACTOR
    pthread_mutex_lock (&manager->lock);

    // We anticapte a case whereby a transaction has seen `txDeleted` but then `txDeleted` again.
    // At least we postulate such a case because IOS crashes seem to indicate that it has occurred.
    //
    // See comment above in `bwmHandleTxUpdated()`
    int needEvents = 1;

    // Find the transaction by `hash` but in the lookup ignore the deleted flag.
    BRTransactionWithState txnWithState = BRWalletManagerFindTransactionByHash (manager, hash, 1);

    // A transaction must have been found, even if it is flagged as deleted.
    assert (NULL != txnWithState);

    // The transaction must be signed to be deleted
    assert (BRTransactionIsSigned (BRTransactionWithStateGetOwned (txnWithState)));

    // If the transaction is deleted.... we'll avoid setting the deleted flag (again) and also
    // avoid the subsequent events.
    if (txnWithState->isDeleted) needEvents = 0;
    else {
        BRTransactionWithStateSetDeleted (txnWithState);
    }
    pthread_mutex_unlock (&manager->lock);

    if (needEvents) {
        if (txnWithState->isResolved)
            bwmSignalTransactionEvent(manager,
                                      manager->wallet,
                                      BRTransactionWithStateGetOwned (txnWithState),
                                      (BRTransactionEvent) {
                BITCOIN_TRANSACTION_DELETED
            });

        if (recommendRescan) {
            // When that happens it's because the wallet believes its missing spend tx, causing new tx to get
            // rejected as a double spend because of how the input selection works. You should only have to
            // scan from the most recent successful spend.
            bwmSignalWalletManagerEvent(manager,
                                        (BRWalletManagerEvent) {
                BITCOIN_WALLET_MANAGER_SYNC_RECOMMENDED,
                { .syncRecommended = { CRYPTO_SYNC_DEPTH_FROM_LAST_CONFIRMED_SEND } }
            });
        }
    }

#endif
}

typedef struct {
    struct BREventRecord base;
    BRCryptoWalletManager manager;
    UInt256 hash;
    int recommendRescan;
} BRCryptoWalletManagerWalletTxDeletedEvent;

static void
bwmSignalTxDeletedDispatcher (BREventHandler ignore,
                              BRCryptoWalletManagerWalletTxDeletedEvent *event) {
    bwmHandleTxDeleted(event->manager, event->hash,  event->recommendRescan);
}

static BREventType bwmSignalTxDeletedEventType = {
    "BTC: Wallet TX Deleted Event",
    sizeof (BRCryptoWalletManagerWalletTxDeletedEvent),
    (BREventDispatcher) bwmSignalTxDeletedDispatcher
};

extern void
bwmSignalTxDeleted (BRCryptoWalletManager manager,
                    UInt256 hash,
                    int recommendRescan) {
    BRCryptoWalletManagerWalletTxDeletedEvent message =
    { { NULL, &bwmSignalTxDeletedEventType}, manager, hash, recommendRescan};
    eventHandlerSignalEvent (manager->handler, (BREvent*) &message);
}


static void cryptoWalletManagerBTCTxDeleted (void *info, UInt256 hash, int notifyUser, int recommendRescan) {
    BRCryptoWalletManagerBTC manager = info;
    bwmSignalTxDeleted (&manager->base, hash, recommendRescan);
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
        fileServiceReplace (manager->base.fileService, fileServiceTypeBlocks, (const void **) blocks, count);
    }
    else {
        for (size_t index = 0; index < count; index++)
            fileServiceSave (manager->base.fileService, fileServiceTypeBlocks, blocks[index]);
    }
}

static void cryptoWalletManagerBTCSavePeers  (void *info, int replace, const BRPeer *peers, size_t count) {
    BRCryptoWalletManagerBTC manager = info;

    // filesystem changes are NOT queued; they are acted upon immediately

    if (!replace) {
        // save each peer, one-by-one
        for (size_t index = 0; index < count; index++)
            fileServiceSave (manager->base.fileService, fileServiceTypePeers, &peers[index]);
    }

    else if (0 == count) {
        // no peers to set, just do a clear
        fileServiceClear (manager->base.fileService, fileServiceTypePeers);
    }

    else {
        // fileServiceReplace expects an array of pointers to entities, instead of an array of
        // structures so let's do the conversion here
        const BRPeer **peerRefs = calloc (count, sizeof(BRPeer *));

        for (size_t i = 0; i < count; i++) {
            peerRefs[i] = &peers[i];
        }

        fileServiceReplace (manager->base.fileService, fileServiceTypePeers, (const void **) peerRefs, count);
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

    cryptoWalletManagerGenerateTransferEvent (manager, wallet, transfer,
                                              (BRCryptoTransferEvent) {
        CRYPTO_TRANSFER_EVENT_CHANGED,
        { .state = { oldState, newState }}
    });

    cryptoWalletManagerGive(manager);
    cryptoTransferGive (transfer);
    free (info);
}

const BREventType *bwmEventTypes[] = {
    &bwmSignalTxAddedEventType,
    &bwmSignalTxUpdatedEventType,
    &bwmSignalTxDeletedEventType,

#if 0
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
bwmEventTypesCount = (sizeof (bwmEventTypes) / sizeof (BREventType*));

BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersBTC = {
    cryptoWalletManagerCreateHandlerBTC,
    cryptoWalletManagerReleaseHandlerBTC,
    cryptoWalletManagerInitializeHandlerBTC,
    crytpWalletManagerCreateFileServiceBTC,
    cryptoWalletManagerGetEventTypesBTC,
    cryptoWalletManagerSignTransactionWithSeedHandlerBTC,
    cryptoWalletManagerSignTransactionWithKeyHandlerBTC,
    cryptoWalletManagerEstimateLimitHandlerBTC,
    cryptoWalletManagerEstimateFeeBasisHandlerBTC,
    crytpWalletManagerCreateP2PManagerHandlerBTC,
    cryptoWalletManagerRecoverTransfersFromTransactionBundleHandlerBTC,
    cryptoWalletManagerRecoverTransferFromTransferBundleHandlerBTC
};
