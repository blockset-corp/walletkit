//
//  WKWalletManagerBTC.c
//  WalletKitCore
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKBTC.h"

#include "walletkit/WKAccountP.h"
#include "walletkit/WKNetworkP.h"
#include "walletkit/WKKeyP.h"
#include "walletkit/WKClientP.h"
#include "walletkit/WKWalletManagerP.h"
#include "walletkit/WKWalletSweeperP.h"

// BRBitcoinWallet Callbacks

// MARK: - Foward Declarations

// On: BRWalletRegisterTransaction, BRWalletRemoveTransaction
static void wkWalletManagerBTCBalanceChanged (void *info, uint64_t balanceInSatoshi);
// On: BRWalletRegisterTransaction
static void wkWalletManagerBTCTxAdded   (void *info, BRBitcoinTransaction *tx);
// On: BRWalletUpdateTransactions, btcWalletSetTxUnconfirmedAfter (reorg)
static void wkWalletManagerBTCTxUpdated (void *info, const UInt256 *hashes, size_t count, uint32_t blockHeight, uint32_t timestamp);
// On: BRWalletRemoveTransaction
static void wkWalletManagerBTCTxDeleted (void *info, UInt256 hash, int notifyUser, int recommendRescan);

// MARK: - Events

static const BREventType *eventTypesBTC[] = {
    WK_CLIENT_EVENT_TYPES
};

static const unsigned int
eventTypesCountBTC = (sizeof (eventTypesBTC) / sizeof (BREventType*));

// MARK: - Wallet Manager

extern WKWalletManagerBTC
wkWalletManagerCoerceBTC (WKWalletManager manager, WKNetworkType type) {
    assert (type == manager->type);
    return (WKWalletManagerBTC) manager;
}

static WKWalletManager
wkWalletManagerCreateBTC (WKWalletManagerListener listener,
                              WKClient client,
                              WKAccount account,
                              WKNetwork network,
                              WKSyncMode mode,
                              WKAddressScheme scheme,
                              const char *path) {
    return wkWalletManagerAllocAndInit (sizeof (struct WKWalletManagerBTCRecord),
                                            wkNetworkGetType(network),
                                            listener,
                                            client,
                                            account,
                                            network,
                                            scheme,
                                            path,
                                            WK_CLIENT_REQUEST_USE_TRANSACTIONS,
                                            NULL,
                                            NULL);
    
}

static void
wkWalletManagerReleaseBTC (WKWalletManager manager) {

}

static BRFileService
crytpWalletManagerCreateFileServiceBTC (WKWalletManager manager,
                                        const char *basePath,
                                        const char *currency,
                                        const char *network,
                                        BRFileServiceContext context,
                                        BRFileServiceErrorHandler handler) {
    return fileServiceCreateFromTypeSpecifications (basePath, currency, network,
                                                    context, handler,
                                                    fileServiceSpecificationsCountBTC,
                                                    fileServiceSpecificationsBTC);
}

static const BREventType **
wkWalletManagerGetEventTypesBTC (WKWalletManager manager,
                                     size_t *eventTypesCount) {
    assert (NULL != eventTypesCount);
    *eventTypesCount = eventTypesCountBTC;
    return eventTypesBTC;
}

static WKBoolean
wkWalletManagerSignTransactionWithSeedBTC (WKWalletManager manager,
                                           WKWallet wallet,
                                           WKTransfer transfer,
                                           UInt512 seed) {
    BRBitcoinWallet      *btcWallet       = wkWalletAsBTC   (wallet);
    BRBitcoinTransaction *btcTransaction  = wkTransferAsBTC (transfer);         // OWN/REF ?
    const BRBitcoinChainParams *btcParams = wkNetworkAsBTC  (manager->network);

    return AS_WK_BOOLEAN (1 == btcWalletSignTransaction (btcWallet, btcTransaction, btcParams->forkId,
                                                         btcParams->bip32depth, btcParams->bip32child,
                                                         seed.u8, sizeof(UInt512)));
}

static WKBoolean
wkWalletManagerSignTransactionWithKeyBTC (WKWalletManager manager,
                                                     WKWallet wallet,
                                                     WKTransfer transfer,
                                                     WKKey key) {
    BRBitcoinTransaction *btcTransaction  = wkTransferAsBTC (transfer);         // OWN/REF ?
    BRKey         *btcKey          = wkKeyGetCore (key);
    const BRBitcoinChainParams *btcParams = wkNetworkAsBTC  (manager->network);

    return AS_WK_BOOLEAN (1 == btcTransactionSign (btcTransaction, btcParams->forkId, btcKey, 1));
}

static WKAmount
wkWalletManagerEstimateLimitBTC (WKWalletManager cwm,
                                            WKWallet  wallet,
                                            WKBoolean asMaximum,
                                            WKAddress target,
                                            WKNetworkFee networkFee,
                                            WKBoolean *needEstimate,
                                            WKBoolean *isZeroIfInsuffientFunds,
                                            WKUnit unit) {
    BRBitcoinWallet *btcWallet = wkWalletAsBTC (wallet);

    // Amount may be zero if insufficient fees
    *isZeroIfInsuffientFunds = WK_TRUE;

    // NOTE: We know BTC/BCH has a minimum balance of zero.

    uint64_t balance     = btcWalletBalance (btcWallet);
    uint64_t feePerKB    = 1000 * wkNetworkFeeAsBTC (networkFee);
    uint64_t amountInSAT = (WK_FALSE == asMaximum
                            ? btcWalletMinOutputAmountWithFeePerKb (btcWallet, feePerKB)
                            : btcWalletMaxOutputAmountWithFeePerKb (btcWallet, feePerKB));
    uint64_t fee         = (amountInSAT > 0
                            ? btcWalletFeeForTxAmountWithFeePerKb (btcWallet, feePerKB, amountInSAT)
                            : 0);

    //            if (WK_TRUE == asMaximum)
    //                assert (balance == amountInSAT + fee);

    if (amountInSAT + fee > balance)
        amountInSAT = 0;

    return wkAmountCreateInteger ((int64_t) amountInSAT, unit);
}

static WKFeeBasis
wkWalletManagerEstimateFeeBasisBTC (WKWalletManager cwm,
                                        WKWallet wallet,
                                        WKCookie cookie,
                                        WKAddress target,
                                        WKAmount amount,
                                        WKNetworkFee networkFee,
                                        size_t attributesCount,
                                        OwnershipKept WKTransferAttribute *attributes) {
    BRBitcoinWallet *btcWallet = wkWalletAsBTC(wallet);

    WKBoolean overflow = WK_FALSE;
    uint64_t btcFeePerKB = 1000 * wkNetworkFeeAsBTC (networkFee);
    uint64_t btcAmount   = wkAmountGetIntegerRaw (amount, &overflow);
    assert(WK_FALSE == overflow);

    uint64_t btcFee = (0 == btcAmount ? 0 : btcWalletFeeForTxAmountWithFeePerKb (btcWallet, btcFeePerKB, btcAmount));

    return wkFeeBasisCreateAsBTC (wallet->unitForFee, btcFee, btcFeePerKB, WK_FEE_BASIS_BTC_SIZE_UNKNOWN);
}

static BRMasterPubKey
wkWalletManagerGetMPK (WKWalletManager manager) {
    assert (wkNetworkTypeIsBitcoinBased(manager->type));
    switch (manager->type) {
        case WK_NETWORK_TYPE_BTC:
        case WK_NETWORK_TYPE_BCH:
        case WK_NETWORK_TYPE_BSV:
            return wkAccountAsBTC (manager->account);
        case WK_NETWORK_TYPE_LTC:
            return wkAccountAsLTC (manager->account);
        case WK_NETWORK_TYPE_DOGE:
            return wkAccountAsDOGE (manager->account);
        default:
            assert (false);
            return (BRMasterPubKey) { 0 };
    }
}

static WKWallet
wkWalletManagerCreateWalletBTC (WKWalletManager manager,
                                WKCurrency currency,
                                Nullable OwnershipKept BRArrayOf(WKClientTransactionBundle) initialTransactionsBundles,
                                Nullable OwnershipKept BRArrayOf(WKClientTransferBundle) initialTransferBundles) {
    assert (NULL == manager->wallet);
    
    // Get the btcMasterPublicKey
    BRMasterPubKey btcMPK = wkWalletManagerGetMPK (manager);

    // Get the btcChainParams
    const BRBitcoinChainParams *btcChainParams = wkNetworkAsBTC(manager->network);

    assert (NULL == initialTransactionsBundles || 0 == array_count (initialTransactionsBundles));
    assert (NULL == initialTransferBundles     || 0 == array_count (initialTransferBundles));

    BRArrayOf(BRBitcoinTransaction*) transactions = initialTransactionsLoadBTC(manager);

    // Create the BTC wallet
    //
    // Since the BRBitcoinWallet callbacks are not set, none of these transactions generate callbacks.
    // And, in fact, looking at btcWalletNew(), there is not even an attempt to generate callbacks
    // even if they could have been specified.
    BRBitcoinWallet *btcWallet = btcWalletNew (btcChainParams->addrParams, transactions, array_count(transactions), btcMPK);
    assert (NULL != btcWallet);

    // The btcWallet now should include *all* the transactions
    array_free (transactions);

    // Set the callbacks
    btcWalletSetCallbacks (btcWallet,
                          wkWalletManagerCoerceBTC(manager, manager->network->type),
                          wkWalletManagerBTCBalanceChanged,
                          wkWalletManagerBTCTxAdded,
                          wkWalletManagerBTCTxUpdated,
                          wkWalletManagerBTCTxDeleted);

    // Create the primary WKWallet
    WKNetwork  network       = manager->network;
    WKUnit     unitAsBase    = wkNetworkGetUnitAsBase    (network, currency);
    WKUnit     unitAsDefault = wkNetworkGetUnitAsDefault (network, currency);

    WKWallet wallet = wkWalletCreateAsBTC (manager->type,
                                                     manager->listenerWallet,
                                                     unitAsDefault,
                                                     unitAsDefault,
                                                     btcWallet);
    wkWalletManagerAddWallet (manager, wallet);

    // Process existing btcTransactions in the btcWallet into WKTransfers
    size_t btcTransactionsCount = btcWalletTransactions(btcWallet, NULL, 0);
    BRBitcoinTransaction *btcTransactions[btcTransactionsCount > 0 ? btcTransactionsCount : 1]; // avoid a static analysis error
    btcWalletTransactions (btcWallet, btcTransactions, btcTransactionsCount);

    BRArrayOf(WKTransfer) transfers;
    array_new (transfers, btcTransactionsCount);

    for (size_t index = 0; index < btcTransactionsCount; index++) {
        array_add (transfers,
                   wkTransferCreateAsBTC (wallet->listenerTransfer,
                                              unitAsDefault,
                                              unitAsBase,
                                              btcWallet,
                                              btcTransactionCopy(btcTransactions[index]),
                                              manager->type));
    }
    wkWalletAddTransfers (wallet, transfers); // OwnershipGiven transfers

    wkUnitGive (unitAsDefault);
    wkUnitGive (unitAsBase);

    return wallet;
}

private_extern void
wkWalletManagerSaveTransactionBundleBTC (WKWalletManager manager,
                                             OwnershipKept WKClientTransactionBundle bundle) {
    size_t   serializationCount = 0;
    uint8_t *serialization = wkClientTransactionBundleGetSerialization (bundle, &serializationCount);

    BRBitcoinTransaction *transaction = btcTransactionParse (serialization, serializationCount);
    if (NULL == transaction)
        printf ("BTC: SaveTransactionBundle: Missed @ Height %"PRIu64"\n", bundle->blockHeight);
    else {
        transaction->blockHeight = (uint32_t) bundle->blockHeight;
        transaction->timestamp   = (uint32_t) bundle->timestamp;

        fileServiceSave (manager->fileService, fileServiceTypeTransactionsBTC, transaction);
        btcTransactionFree(transaction);
    }
}

static void
wkWalletManagerRecoverTransfersFromTransactionBundleBTC (WKWalletManager manager,
                                                             OwnershipKept WKClientTransactionBundle bundle) {
    BRBitcoinTransaction *btcTransaction = btcTransactionParse (bundle->serialization, bundle->serializationCount);

    bool error = WK_TRANSFER_STATE_ERRORED == bundle->status;
    bool needRegistration = (!error && NULL != btcTransaction && btcTransactionIsSigned (btcTransaction));
    bool needFree = true;

    BRBitcoinWallet *btcWallet = wkWalletAsBTC(manager->wallet);

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
        if (NULL == btcWalletTransactionForHash (btcWallet, btcTransaction->txHash)) {
            // BRWalletRegisterTransaction doesn't reliably report if the txn was added to the wallet.
            btcWalletRegisterTransaction (btcWallet, btcTransaction);
            if (btcTransaction == btcWalletTransactionForHash (btcWallet, btcTransaction->txHash)) {
                // If our transaction made it into the wallet, do not deallocate it
                needFree = false;
            }
        }
    }

    // Check if the wallet knows about transaction.  This is an important check.  If the wallet
    // does not know about the tranaction then the subsequent BRWalletUpdateTransactions will
    // free the transaction (with btcTransactionFree()).
    if (btcWalletContainsTransaction (btcWallet, btcTransaction)) {
        if (error) {
            // On an error, remove the transaction.  This will cascade through BRBitcoinWallet callbacks
            // to produce `balanceUpdated` and `txDeleted`.  The later will be handled by removing
            // a BRTransactionWithState from the BRWalletManager.
            btcWalletRemoveTransaction (btcWallet, btcTransaction->txHash);
        }
        else {
            // If the transaction has transitioned from 'included' back to 'submitted' (like when
            // there is a blockchain reord), the blockHeight will be TX_UNCONFIRMED and the
            // timestamp will be 0.  This will cascade through BRBitcoinWallet callbacks to produce
            // 'balanceUpdated' and 'txUpdated'.
            //
            // If no longer 'included' this might cause dependent transactions to go to 'invalid'.
            btcWalletUpdateTransactions (btcWallet,
                                        &btcTransaction->txHash, 1,
                                        btcBlockHeight,
                                        btcTimestamp);
        }
    }

    // Free if ownership hasn't been passed
    if (needFree) {
        btcTransactionFree (btcTransaction);
    }

    // The transaction is in the wallet, this has generated more BRBitcoinWallet EXTERNAL and INTERNAL
    // addresses.  Because the order of bundle arrival is not guaranteed to be by block number,
    // it is possible that some other transaction in the wallet now has inputs or outputs that are
    // now in BRBitcoinWallet.  This changes the amount and fee, possibly.  Find those and replace them.
    else if (TX_UNCONFIRMED != btcBlockHeight) {
        for (size_t index = 0; index < array_count (manager->wallet->transfers); index++) {
            WKTransfer oldTransfer = manager->wallet->transfers[index];
            BRBitcoinTransaction *tid = wkTransferCoerceBTC(oldTransfer)->tid;

            if (TX_UNCONFIRMED   != tid->blockHeight &&
                tid->blockHeight >= btcBlockHeight   &&
                WK_TRUE == wkTransferChangedAmountBTC (oldTransfer, btcWallet)) {
                wkTransferTake (oldTransfer);

                WKTransfer newTransfer  = wkTransferCreateAsBTC (oldTransfer->listener,
                                                                           oldTransfer->unit,
                                                                           oldTransfer->unitForFee,
                                                                           btcWallet,
                                                                           btcTransactionCopy (tid),
                                                                           oldTransfer->type);

                wkWalletReplaceTransfer (manager->wallet, oldTransfer, newTransfer);
                wkTransferGive (oldTransfer);
            }
        }
    }
}

static void
wkWalletManagerRecoverTransferFromTransferBundleBTC (WKWalletManager cwm,
                                                         OwnershipKept WKClientTransferBundle bundle) {
    // Not BTC functionality
    assert (0);
}

/// MARK: - Wallet Sweeper

extern WKWalletSweeperStatus
wkWalletManagerWalletSweeperValidateSupportedBTC (WKWalletManager cwm,
                                                      WKWallet wallet,
                                                      WKKey key) {
    BRBitcoinWallet * wid          = wkWalletAsBTC (wallet);
    BRKey * keyCore            = wkKeyGetCore (key);
    BRAddressParams addrParams = wkNetworkAsBTC (cwm->network)->addrParams;

    // encode using legacy format (only supported method for BTC)
    size_t addrLength = BRKeyLegacyAddr (keyCore, NULL, 0, addrParams);
    char  *addr = malloc (addrLength + 1);
    BRKeyLegacyAddr (keyCore, addr, addrLength, addrParams);
    addr[addrLength] = '\0';

    // check if we are trying to sweep ourselves
    int containsAddr = btcWalletContainsAddress (wid, addr);
    free (addr);

    if (containsAddr) {
        return WK_WALLET_SWEEPER_INVALID_SOURCE_WALLET;
    }

    return WK_WALLET_SWEEPER_SUCCESS;
}

extern WKWalletSweeper
wkWalletManagerCreateWalletSweeperBTC (WKWalletManager cwm,
                                           WKWallet wallet,
                                           WKKey key) {
    WKCurrency currency = wkWalletGetCurrency (wallet);
    WKUnit unit = wkNetworkGetUnitAsBase (cwm->network, currency);

    WKWalletSweeper sweeper = wkWalletSweeperAllocAndInit (sizeof (struct WKWalletSweeperBTCRecord),
                                                                         cwm->type,
                                                                         key,
                                                                         unit);

    WKWalletSweeperBTC sweeperBTC = (WKWalletSweeperBTC) sweeper;

    BRKey *keyCore = wkKeyGetCore (key);
    BRAddressParams addrParams = wkNetworkAsBTC (cwm->network)->addrParams;

    size_t addressLength = BRKeyLegacyAddr (keyCore, NULL, 0, addrParams);
    char  *address = malloc (addressLength + 1);
    BRKeyLegacyAddr (keyCore, address, addressLength, addrParams);
    address[addressLength] = '\0';

    sweeperBTC->addrParams = addrParams;
    sweeperBTC->isSegwit = WK_ADDRESS_SCHEME_BTC_SEGWIT == cwm->addressScheme;
    sweeperBTC->sourceAddress = address;
    array_new (sweeperBTC->txns, 100);

    wkUnitGive (unit);
    wkCurrencyGive (currency);

    return sweeper;
}

// MARK: BRBitcoinWallet Callback Balance Changed

static void wkWalletManagerBTCBalanceChanged (void *info, uint64_t balanceInSatoshi) {
    WKWalletManagerBTC manager = info;
    // printf ("BTC: BalanceChanged\n");
    (void) manager;
}

// MARK: - BRBitcoinWallet Callback TX Added

static void wkWalletManagerBTCTxAdded   (void *info, BRBitcoinTransaction *tid) {
    WKWalletManagerBTC manager = info;

    // We have the possibility that the TID argument ceases to exist by the time this `TxAdded`
    // function is invoked.  From the Bitcoin code: BRWalletRegisterTransaction is called but
    // is interrupted just before wallet->txAdded in invoked; then, somehow, wallet->txDeleted
    // is called (and assume BRTranactionFree() got invoked).  Gone.
    tid = btcTransactionCopy (tid);

    pthread_mutex_lock (&manager->base.lock);
    WKWallet wallet = manager->base.wallet;
    BRBitcoinWallet      *wid    = wkWalletAsBTC(wallet);

    WKWalletBTC walletBTC = wkWalletCoerceBTC(wallet);
    printf ("BTC: TxAdded  : %zu (Unresolved Count)\n", array_count (walletBTC->tidsUnresolved));

    // Save `tid` to the fileService.
    fileServiceSave (manager->base.fileService, fileServiceTypeTransactionsBTC, tid);

    // If `tid` is not resolved in `wid`, then add it as unresolved to `wid` and skip out.
    if (!btcWalletTransactionIsResolved (wid, tid)) {
        printf ("BTC: TxAdded  : %s (Not Resolved)\n", u256hex(UInt256Reverse(tid->txHash)));
        wkWalletAddUnresolvedAsBTC (wallet, tid);
        pthread_mutex_unlock (&manager->base.lock);
        return;
    }

    printf ("BTC: TxAdded  : %s\n", u256hex(UInt256Reverse(tid->txHash)));

    bool wasDeleted = false;
    bool wasCreated = false;

    WKTransferBTC transferBTC = wkWalletFindTransferByHashAsBTC (wallet, tid->txHash);
    WKTransfer    transfer    = (WKTransfer) transferBTC;
    wkTransferTake(transfer);

    if (NULL == transfer) {
        // first we've seen it, so it came from the network; add it to our list
        transfer = wkTransferCreateAsBTC (wallet->listenerTransfer,
                                              wallet->unit,
                                              wallet->unitForFee,
                                              wid,
                                              tid,
                                              wallet->type);
        transferBTC = wkTransferCoerceBTC (transfer);
        wkWalletAddTransfer (wallet, transfer);
        wasCreated = true;
    }
    else {
        BRBitcoinTransaction *oldTid = transferBTC->tid;
        BRBitcoinTransaction *newTid = tid;

        if (transferBTC->isDeleted) {
            // We've seen it before but has already been deleted, somewhow?  We are quietly going
            // to skip out and avoid signalling any events. Perhaps should assert(0) here.
            wasDeleted = true;
        }
        else {
            // this is a transaction we've submitted; set the reference transaction from the wallet
            oldTid->blockHeight = newTid->blockHeight;
            oldTid->timestamp   = newTid->timestamp;
        }

        // we already have an owned copy of this transaction; free up the passed one
        btcTransactionFree (newTid);
        tid = oldTid;
    }
    assert (NULL != transfer);
    wkTransferGive(transfer);

    // Find other transations in `wallet` that are now resolved.
    size_t resolvedTransactionsCount = wkWalletRemResolvedAsBTC (wallet, NULL, 0);
    BRBitcoinTransaction **resolvedTransactions = calloc (resolvedTransactionsCount, sizeof (BRBitcoinTransaction*));
    resolvedTransactionsCount = wkWalletRemResolvedAsBTC (wallet, resolvedTransactions, resolvedTransactionsCount);
    // We now own `resolvedTransactions`

    pthread_mutex_unlock (&manager->base.lock);

    // If `transfer` wasCreated when we generated three events: TRANSFER_CREATED,
    // WALLET_ADDED_TRANSFER, WALLET_BALANCE_UPDATED.  If not created and not deleted and
    // now resolved, we'll generate a balance event.  This later case occurs if we've submitted
    // a transaction (I think); this event might be extaneous.
    if (!wasCreated && !wasDeleted) {
        WKAmount balance = wkWalletGetBalance (wallet);
        wkWalletGenerateEvent (wallet, wkWalletEventCreateBalanceUpdated (balance));
        wkAmountGive (balance);
    }

    for (size_t index = 0; index < resolvedTransactionsCount; index++) {
        BRBitcoinTransaction *tid = resolvedTransactions[index];
        printf ("BTC: TxAdded  : %s (Resolved)\n", u256hex(UInt256Reverse(tid->txHash)));
        wkWalletManagerBTCTxAdded   (info, tid);
        wkWalletManagerBTCTxUpdated (info,
                                         &tid->txHash, 1,
                                         tid->blockHeight,
                                         tid->timestamp);
        btcTransactionFree(tid);
    }
    // Only one UPDATE BALANCE?

    free (resolvedTransactions);
}

// MARK: - BRBitcoinWallet Callback TX Updated

static void wkWalletManagerBTCTxUpdated (void *info,
                                             const UInt256 *hashes, size_t count,
                                             uint32_t blockHeight,
                                             uint32_t timestamp) {
    WKWalletManagerBTC manager = info;
    (void) manager;

    WKWallet wallet = manager->base.wallet;

    for (size_t index = 0; index < count; index++) {
        // TODO: This is here to allow events to flow; otherwise we'd block for too long??
        pthread_mutex_lock (&manager->base.lock);
        WKTransferBTC transfer = wkWalletFindTransferByHashAsBTC(wallet, hashes[index]);

        // If we don't know about `transfer` then it has not been added, typically because the
        // associated transaction is not resolved, so just skip this hash.
        if (NULL == transfer)
            wkWalletUpdUnresolvedAsBTC (wallet, &hashes[index], blockHeight, timestamp);
        else {
            assert (btcTransactionIsSigned (transfer->tid));

            printf ("BTC: TxUpdated: %s\n",u256hex(UInt256Reverse(transfer->tid->txHash)));

            if (!transfer->isDeleted) {
                transfer->tid->blockHeight = blockHeight;
                transfer->tid->timestamp   = timestamp;

                // Save the modified `tid` to the fileService.
                fileServiceSave (manager->base.fileService, fileServiceTypeTransactionsBTC, transfer->tid);
            }

            // Determine the transfer's state, as best we can.
            WKFeeBasis feeBasis = wkFeeBasisTake (transfer->base.feeBasisEstimated);

            WKTransferState newState = wkTransferInitializeStateBTC (transfer->tid,
                                                                               blockHeight,
                                                                               timestamp,
                                                                               feeBasis);

            wkTransferSetState (&transfer->base, newState);

            wkFeeBasisGive (feeBasis);
            wkTransferStateGive (newState);
        }

        pthread_mutex_unlock (&manager->base.lock);
    }

    pthread_mutex_lock (&manager->base.lock);
    // Find other transations in `wallet` that are now resolved.
    size_t resolvedTransactionsCount = wkWalletRemResolvedAsBTC (wallet, NULL, 0);
    BRBitcoinTransaction **resolvedTransactions = calloc (resolvedTransactionsCount, sizeof (BRBitcoinTransaction*));
    resolvedTransactionsCount = wkWalletRemResolvedAsBTC (wallet, resolvedTransactions, resolvedTransactionsCount);
    // We now own `resolvedTransactions`
    pthread_mutex_unlock (&manager->base.lock);

    for (size_t index = 0; index < resolvedTransactionsCount; index++) {
        BRBitcoinTransaction *tid = resolvedTransactions[index];
        wkWalletManagerBTCTxAdded   (info, tid);
        wkWalletManagerBTCTxUpdated (info,
                                         &tid->txHash, 1,
                                         tid->blockHeight,
                                         tid->timestamp);
        btcTransactionFree(tid);
    }
    // Only one UPDATE BALANCE?

    free (resolvedTransactions);

}

// MARK: - BRBitcoinWallet Callback TX Deleted

static void wkWalletManagerBTCTxDeleted (void *info, UInt256 hash, int notifyUser, int recommendRescan) {
    WKWalletManagerBTC manager = info;
    WKWallet           wallet = manager->base.wallet;

    bool needEvents = true;

    pthread_mutex_lock (&manager->base.lock);
    WKTransferBTC transfer = wkWalletFindTransferByHashAsBTC (wallet, hash);

    if (NULL != transfer) {
        printf ("BTC: TxDeleted: %s\n",u256hex(UInt256Reverse(transfer->tid->txHash)));

        if (transfer->isDeleted) needEvents = false;
        else {
            transfer->isDeleted = true;
            wkTransferSetState (&transfer->base, wkTransferStateInit (WK_TRANSFER_STATE_DELETED));
            fileServiceRemove (manager->base.fileService, fileServiceTypeTransactionsBTC, transfer->tid);
        }

        pthread_mutex_unlock (&manager->base.lock);

        if (needEvents) {
            wkTransferGenerateEvent (&transfer->base, (WKTransferEvent) {
                WK_TRANSFER_EVENT_DELETED
            });

            if (recommendRescan)
                wkWalletManagerGenerateEvent (&manager->base, (WKWalletManagerEvent) {
                    WK_WALLET_MANAGER_EVENT_SYNC_RECOMMENDED,
                    { .syncRecommended = { WK_SYNC_DEPTH_FROM_LAST_CONFIRMED_SEND } }
                });
        }
    }
}

WKWalletManagerHandlers wkWalletManagerHandlersBTC = {
    wkWalletManagerCreateBTC,
    wkWalletManagerReleaseBTC,
    crytpWalletManagerCreateFileServiceBTC,
    wkWalletManagerGetEventTypesBTC,
    wkWalletManagerCreateP2PManagerBTC,
    wkWalletManagerCreateWalletBTC,
    wkWalletManagerSignTransactionWithSeedBTC,
    wkWalletManagerSignTransactionWithKeyBTC,
    wkWalletManagerEstimateLimitBTC,
    wkWalletManagerEstimateFeeBasisBTC,
    wkWalletManagerSaveTransactionBundleBTC,
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    wkWalletManagerRecoverTransfersFromTransactionBundleBTC,
    wkWalletManagerRecoverTransferFromTransferBundleBTC,
    NULL,//WKWalletManagerRecoverFeeBasisFromFeeEstimateHandler not supported
    wkWalletManagerWalletSweeperValidateSupportedBTC,
    wkWalletManagerCreateWalletSweeperBTC
};

WKWalletManagerHandlers wkWalletManagerHandlersBCH = {
    wkWalletManagerCreateBTC,
    wkWalletManagerReleaseBTC,
    crytpWalletManagerCreateFileServiceBTC,
    wkWalletManagerGetEventTypesBTC,
    wkWalletManagerCreateP2PManagerBTC,
    wkWalletManagerCreateWalletBTC,
    wkWalletManagerSignTransactionWithSeedBTC,
    wkWalletManagerSignTransactionWithKeyBTC,
    wkWalletManagerEstimateLimitBTC,
    wkWalletManagerEstimateFeeBasisBTC,
    wkWalletManagerSaveTransactionBundleBTC,
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    wkWalletManagerRecoverTransfersFromTransactionBundleBTC,
    wkWalletManagerRecoverTransferFromTransferBundleBTC,
    NULL,//WKWalletManagerRecoverFeeBasisFromFeeEstimateHandler not supported
    wkWalletManagerWalletSweeperValidateSupportedBTC,
    wkWalletManagerCreateWalletSweeperBTC
};

WKWalletManagerHandlers wkWalletManagerHandlersBSV = {
    wkWalletManagerCreateBTC,
    wkWalletManagerReleaseBTC,
    crytpWalletManagerCreateFileServiceBTC,
    wkWalletManagerGetEventTypesBTC,
    wkWalletManagerCreateP2PManagerBTC,
    wkWalletManagerCreateWalletBTC,
    wkWalletManagerSignTransactionWithSeedBTC,
    wkWalletManagerSignTransactionWithKeyBTC,
    wkWalletManagerEstimateLimitBTC,
    wkWalletManagerEstimateFeeBasisBTC,
    wkWalletManagerSaveTransactionBundleBTC,
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    wkWalletManagerRecoverTransfersFromTransactionBundleBTC,
    wkWalletManagerRecoverTransferFromTransferBundleBTC,
    NULL,//WKWalletManagerRecoverFeeBasisFromFeeEstimateHandler not supported
    wkWalletManagerWalletSweeperValidateSupportedBTC,
    wkWalletManagerCreateWalletSweeperBTC
};

WKWalletManagerHandlers wkWalletManagerHandlersLTC = {
    wkWalletManagerCreateBTC,
    wkWalletManagerReleaseBTC,
    crytpWalletManagerCreateFileServiceBTC,
    wkWalletManagerGetEventTypesBTC,
    wkWalletManagerCreateP2PManagerBTC,
    wkWalletManagerCreateWalletBTC,
    wkWalletManagerSignTransactionWithSeedBTC,
    wkWalletManagerSignTransactionWithKeyBTC,
    wkWalletManagerEstimateLimitBTC,
    wkWalletManagerEstimateFeeBasisBTC,
    wkWalletManagerSaveTransactionBundleBTC,
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    wkWalletManagerRecoverTransfersFromTransactionBundleBTC,
    wkWalletManagerRecoverTransferFromTransferBundleBTC,
    NULL,//WKWalletManagerRecoverFeeBasisFromFeeEstimateHandler not supported
    wkWalletManagerWalletSweeperValidateSupportedBTC,
    wkWalletManagerCreateWalletSweeperBTC
};

WKWalletManagerHandlers wkWalletManagerHandlersDOGE = {
    wkWalletManagerCreateBTC,
    wkWalletManagerReleaseBTC,
    crytpWalletManagerCreateFileServiceBTC,
    wkWalletManagerGetEventTypesBTC,
    wkWalletManagerCreateP2PManagerBTC,
    wkWalletManagerCreateWalletBTC,
    wkWalletManagerSignTransactionWithSeedBTC,
    wkWalletManagerSignTransactionWithKeyBTC,
    wkWalletManagerEstimateLimitBTC,
    wkWalletManagerEstimateFeeBasisBTC,
    wkWalletManagerSaveTransactionBundleBTC,
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    wkWalletManagerRecoverTransfersFromTransactionBundleBTC,
    wkWalletManagerRecoverTransferFromTransferBundleBTC,
    NULL,//WKWalletManagerRecoverFeeBasisFromFeeEstimateHandler not supported
    wkWalletManagerWalletSweeperValidateSupportedBTC,
    wkWalletManagerCreateWalletSweeperBTC
};
