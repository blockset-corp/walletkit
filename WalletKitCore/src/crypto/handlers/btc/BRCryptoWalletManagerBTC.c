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

// BRWallet Callbacks

// MARK: - Foward Declarations

// On: BRWalletRegisterTransaction, BRWalletRemoveTransaction
static void cryptoWalletManagerBTCBalanceChanged (void *info, uint64_t balanceInSatoshi);
// On: BRWalletRegisterTransaction
static void cryptoWalletManagerBTCTxAdded   (void *info, BRTransaction *tx);
// On: BRWalletUpdateTransactions, BRWalletSetTxUnconfirmedAfter (reorg)
static void cryptoWalletManagerBTCTxUpdated (void *info, const UInt256 *hashes, size_t count, uint32_t blockHeight, uint32_t timestamp);
// On: BRWalletRemoveTransaction
static void cryptoWalletManagerBTCTxDeleted (void *info, UInt256 hash, int notifyUser, int recommendRescan);

// MARK: - Events

static const BREventType *eventTypesBTC[] = {
    CRYPTO_CLIENT_EVENT_TYPES
};

static const unsigned int
eventTypesCountBTC = (sizeof (eventTypesBTC) / sizeof (BREventType*));

// MARK: - Wallet Manager

extern BRCryptoWalletManagerBTC
cryptoWalletManagerCoerceBTC (BRCryptoWalletManager manager, BRCryptoBlockChainType type) {
    assert (type == manager->type);
    return (BRCryptoWalletManagerBTC) manager;
}

static BRCryptoWalletManager
cryptoWalletManagerCreateBTC (BRCryptoWalletManagerListener listener,
                              BRCryptoClient client,
                              BRCryptoAccount account,
                              BRCryptoNetwork network,
                              BRCryptoSyncMode mode,
                              BRCryptoAddressScheme scheme,
                              const char *path) {
    return cryptoWalletManagerAllocAndInit (sizeof (struct BRCryptoWalletManagerBTCRecord),
                                            cryptoNetworkGetType(network),
                                            listener,
                                            client,
                                            account,
                                            network,
                                            scheme,
                                            path,
                                            CRYPTO_CLIENT_REQUEST_USE_TRANSACTIONS,
                                            NULL,
                                            NULL);
    
}

static void
cryptoWalletManagerReleaseBTC (BRCryptoWalletManager manager) {

}

static BRFileService
crytpWalletManagerCreateFileServiceBTC (BRCryptoWalletManager manager,
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
cryptoWalletManagerGetEventTypesBTC (BRCryptoWalletManager manager,
                                     size_t *eventTypesCount) {
    assert (NULL != eventTypesCount);
    *eventTypesCount = eventTypesCountBTC;
    return eventTypesBTC;
}

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
                                        BRCryptoNetworkFee networkFee,
                                        size_t attributesCount,
                                        OwnershipKept BRCryptoTransferAttribute *attributes) {
    BRWallet *btcWallet = cryptoWalletAsBTC(wallet);

    BRCryptoBoolean overflow = CRYPTO_FALSE;
    uint64_t btcFeePerKB = 1000 * cryptoNetworkFeeAsBTC (networkFee);
    uint64_t btcAmount   = cryptoAmountGetIntegerRaw (amount, &overflow);
    assert(CRYPTO_FALSE == overflow);

    uint64_t btcFee = (0 == btcAmount ? 0 : BRWalletFeeForTxAmountWithFeePerKb (btcWallet, btcFeePerKB, btcAmount));

    return cryptoFeeBasisCreateAsBTC (wallet->unitForFee, btcFee, btcFeePerKB, CRYPTO_FEE_BASIS_BTC_SIZE_UNKNOWN);
}

static BRCryptoWallet
cryptoWalletManagerCreateWalletBTC (BRCryptoWalletManager manager,
                                    BRCryptoCurrency currency,
                                    Nullable OwnershipKept BRArrayOf(BRCryptoClientTransactionBundle) initialTransactionsBundles,
                                    Nullable OwnershipKept BRArrayOf(BRCryptoClientTransferBundle) initialTransferBundles) {
    assert (NULL == manager->wallet);
    
    // Get the btcMasterPublicKey
    BRMasterPubKey btcMPK = cryptoAccountAsBTC(manager->account);

    // Get the btcChainParams
    const BRChainParams *btcChainParams = cryptoNetworkAsBTC(manager->network);

    assert (NULL == initialTransactionsBundles || 0 == array_count (initialTransactionsBundles));
    assert (NULL == initialTransferBundles     || 0 == array_count (initialTransferBundles));

    BRArrayOf(BRTransaction*) transactions = initialTransactionsLoadBTC(manager);

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
                          cryptoWalletManagerCoerceBTC(manager, manager->network->type),
                          cryptoWalletManagerBTCBalanceChanged,
                          cryptoWalletManagerBTCTxAdded,
                          cryptoWalletManagerBTCTxUpdated,
                          cryptoWalletManagerBTCTxDeleted);

    // Create the primary BRCryptoWallet
    BRCryptoNetwork  network       = manager->network;
    BRCryptoUnit     unitAsBase    = cryptoNetworkGetUnitAsBase    (network, currency);
    BRCryptoUnit     unitAsDefault = cryptoNetworkGetUnitAsDefault (network, currency);

    BRCryptoWallet wallet = cryptoWalletCreateAsBTC (manager->type,
                                                     manager->listenerWallet,
                                                     unitAsDefault,
                                                     unitAsDefault,
                                                     btcWallet);
    cryptoWalletManagerAddWallet (manager, wallet);

    // Process existing btcTransactions in the btcWallet into BRCryptoTransfers
    size_t btcTransactionsCount = BRWalletTransactions(btcWallet, NULL, 0);
    BRTransaction *btcTransactions[btcTransactionsCount > 0 ? btcTransactionsCount : 1]; // avoid a static analysis error
    BRWalletTransactions (btcWallet, btcTransactions, btcTransactionsCount);

    BRArrayOf(BRCryptoTransfer) transfers;
    array_new (transfers, btcTransactionsCount);

    for (size_t index = 0; index < btcTransactionsCount; index++) {
        array_add (transfers,
                   cryptoTransferCreateAsBTC (wallet->listenerTransfer,
                                              unitAsDefault,
                                              unitAsBase,
                                              btcWallet,
                                              BRTransactionCopy(btcTransactions[index]),
                                              manager->type));
    }
    cryptoWalletAddTransfers (wallet, transfers); // OwnershipGiven transfers

    cryptoUnitGive (unitAsDefault);
    cryptoUnitGive (unitAsBase);

    return wallet;
}

private_extern void
cryptoWalletManagerSaveTransactionBundleBTC (BRCryptoWalletManager manager,
                                             OwnershipKept BRCryptoClientTransactionBundle bundle) {
    size_t   serializationCount = 0;
    uint8_t *serialization = cryptoClientTransactionBundleGetSerialization (bundle, &serializationCount);

    BRTransaction *transaction = BRTransactionParse (serialization, serializationCount);
    if (NULL == transaction)
        printf ("BTC: SaveTransactionBundle: Missed @ Height %"PRIu64"\n", bundle->blockHeight);
    else {
        transaction->blockHeight = (uint32_t) bundle->blockHeight;
        transaction->timestamp   = (uint32_t) bundle->timestamp;

        fileServiceSave (manager->fileService, fileServiceTypeTransactionsBTC, transaction);
        BRTransactionFree(transaction);
    }
}

static void
cryptoWalletManagerRecoverTransfersFromTransactionBundleBTC (BRCryptoWalletManager manager,
                                                             OwnershipKept BRCryptoClientTransactionBundle bundle) {
    BRTransaction *btcTransaction = BRTransactionParse (bundle->serialization, bundle->serializationCount);

    bool error = CRYPTO_TRANSFER_STATE_ERRORED == bundle->status;
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

    // The transaction is in the wallet, this has generated more BRWallet EXTERNAL and INTERNAL
    // addresses.  Because the order of bundle arrival is not guaranteed to be by block number,
    // it is possible that some other transaction in the wallet now has inputs or outputs that are
    // now in BRWallet.  This changes the amount and fee, possibly.  Find those and replace them.
    else if (TX_UNCONFIRMED != btcBlockHeight) {
        for (size_t index = 0; index < array_count (manager->wallet->transfers); index++) {
            BRCryptoTransfer oldTransfer = manager->wallet->transfers[index];
            BRTransaction *tid = cryptoTransferCoerceBTC(oldTransfer)->tid;

            if (TX_UNCONFIRMED   != tid->blockHeight &&
                tid->blockHeight >= btcBlockHeight   &&
                CRYPTO_TRUE == cryptoTransferChangedAmountBTC (oldTransfer, btcWallet)) {
                cryptoTransferTake (oldTransfer);

                BRCryptoTransfer newTransfer  = cryptoTransferCreateAsBTC (oldTransfer->listener,
                                                                           oldTransfer->unit,
                                                                           oldTransfer->unitForFee,
                                                                           btcWallet,
                                                                           BRTransactionCopy (tid),
                                                                           oldTransfer->type);

                cryptoWalletReplaceTransfer (manager->wallet, oldTransfer, newTransfer);
                cryptoTransferGive (oldTransfer);
            }
        }
    }
}

static void
cryptoWalletManagerRecoverTransferFromTransferBundleBTC (BRCryptoWalletManager cwm,
                                                         OwnershipKept BRCryptoClientTransferBundle bundle) {
    // Not BTC functionality
    assert (0);
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

    pthread_mutex_lock (&manager->base.lock);
    BRCryptoWallet wallet = manager->base.wallet;
    BRWallet      *wid    = cryptoWalletAsBTC(wallet);

    BRCryptoWalletBTC walletBTC = cryptoWalletCoerceBTC(wallet);
    printf ("BTC: TxAdded  : %zu (Unresolved Count)\n", array_count (walletBTC->tidsUnresolved));

    // Save `tid` to the fileService.
    fileServiceSave (manager->base.fileService, fileServiceTypeTransactionsBTC, tid);

    // If `tid` is not resolved in `wid`, then add it as unresolved to `wid` and skip out.
    if (!BRWalletTransactionIsResolved (wid, tid)) {
        printf ("BTC: TxAdded  : %s (Not Resolved)\n", u256hex(UInt256Reverse(tid->txHash)));
        cryptoWalletAddUnresolvedAsBTC (wallet, tid);
        pthread_mutex_unlock (&manager->base.lock);
        return;
    }

    printf ("BTC: TxAdded  : %s\n", u256hex(UInt256Reverse(tid->txHash)));

    bool wasDeleted = false;
    bool wasCreated = false;

    BRCryptoTransferBTC transferBTC = cryptoWalletFindTransferByHashAsBTC (wallet, tid->txHash);
    BRCryptoTransfer    transfer    = (BRCryptoTransfer) transferBTC;
    cryptoTransferTake(transfer);

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
        wasCreated = true;
    }
    else {
        BRTransaction *oldTid = transferBTC->tid;
        BRTransaction *newTid = tid;

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
        BRTransactionFree (newTid);
        tid = oldTid;
    }
    assert (NULL != transfer);
    cryptoTransferGive(transfer);

    // Find other transations in `wallet` that are now resolved.
    size_t resolvedTransactionsCount = cryptoWalletRemResolvedAsBTC (wallet, NULL, 0);
    BRTransaction **resolvedTransactions = calloc (resolvedTransactionsCount, sizeof (BRTransaction*));
    resolvedTransactionsCount = cryptoWalletRemResolvedAsBTC (wallet, resolvedTransactions, resolvedTransactionsCount);
    // We now own `resolvedTransactions`

    pthread_mutex_unlock (&manager->base.lock);

    // If `transfer` wasCreated when we generated three events: TRANSFER_CREATED,
    // WALLET_ADDED_TRANSFER, WALLET_BALANCE_UPDATED.  If not created and not deleted and
    // now resolved, we'll generate a balance event.  This later case occurs if we've submitted
    // a transaction (I think); this event might be extaneous.
    if (!wasCreated && !wasDeleted) {
        BRCryptoAmount balance = cryptoWalletGetBalance (wallet);
        cryptoWalletGenerateEvent (wallet, cryptoWalletEventCreateBalanceUpdated (balance));
        cryptoAmountGive (balance);
    }

    for (size_t index = 0; index < resolvedTransactionsCount; index++) {
        BRTransaction *tid = resolvedTransactions[index];
        printf ("BTC: TxAdded  : %s (Resolved)\n", u256hex(UInt256Reverse(tid->txHash)));
        cryptoWalletManagerBTCTxAdded   (info, tid);
        cryptoWalletManagerBTCTxUpdated (info,
                                         &tid->txHash, 1,
                                         tid->blockHeight,
                                         tid->timestamp);
        BRTransactionFree(tid);
    }
    // Only one UPDATE BALANCE?

    free (resolvedTransactions);
}

// MARK: - BRWallet Callback TX Updated

static void cryptoWalletManagerBTCTxUpdated (void *info,
                                             const UInt256 *hashes, size_t count,
                                             uint32_t blockHeight,
                                             uint32_t timestamp) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;

    BRCryptoWallet wallet = manager->base.wallet;

    for (size_t index = 0; index < count; index++) {
        // TODO: This is here to allow events to flow; otherwise we'd block for too long??
        pthread_mutex_lock (&manager->base.lock);
        BRCryptoTransferBTC transfer = cryptoWalletFindTransferByHashAsBTC(wallet, hashes[index]);

        // If we don't know about `transfer` then it has not been added, typically because the
        // associated transaction is not resolved, so just skip this hash.
        if (NULL == transfer)
            cryptoWalletUpdUnresolvedAsBTC (wallet, &hashes[index], blockHeight, timestamp);
        else {
            assert (BRTransactionIsSigned (transfer->tid));

            printf ("BTC: TxUpdated: %s\n",u256hex(UInt256Reverse(transfer->tid->txHash)));

            if (!transfer->isDeleted) {
                transfer->tid->blockHeight = blockHeight;
                transfer->tid->timestamp   = timestamp;

                // Save the modified `tid` to the fileService.
                fileServiceSave (manager->base.fileService, fileServiceTypeTransactionsBTC, transfer->tid);
            }

            // Determine the transfer's state, as best we can.
            BRCryptoFeeBasis feeBasis = cryptoFeeBasisTake (transfer->base.feeBasisEstimated);

            BRCryptoTransferState newState = cryptoTransferInitializeStateBTC (transfer->tid,
                                                                               blockHeight,
                                                                               timestamp,
                                                                               feeBasis);
            if (BRTransactionIsSigned(transfer->tid)) {
                char *uids = NULL;
                asprintf (&uids, "%s:0", u256hex(transfer->tid->txHash));
                cryptoTransferSetUids (&transfer->base, uids);
                free (uids);
            }
            cryptoTransferSetState (&transfer->base, newState);

            cryptoFeeBasisGive (feeBasis);
            cryptoTransferStateGive (newState);
        }

        pthread_mutex_unlock (&manager->base.lock);
    }

    pthread_mutex_lock (&manager->base.lock);
    // Find other transations in `wallet` that are now resolved.
    size_t resolvedTransactionsCount = cryptoWalletRemResolvedAsBTC (wallet, NULL, 0);
    BRTransaction **resolvedTransactions = calloc (resolvedTransactionsCount, sizeof (BRTransaction*));
    resolvedTransactionsCount = cryptoWalletRemResolvedAsBTC (wallet, resolvedTransactions, resolvedTransactionsCount);
    // We now own `resolvedTransactions`
    pthread_mutex_unlock (&manager->base.lock);

    for (size_t index = 0; index < resolvedTransactionsCount; index++) {
        BRTransaction *tid = resolvedTransactions[index];
        cryptoWalletManagerBTCTxAdded   (info, tid);
        cryptoWalletManagerBTCTxUpdated (info,
                                         &tid->txHash, 1,
                                         tid->blockHeight,
                                         tid->timestamp);
        BRTransactionFree(tid);
    }
    // Only one UPDATE BALANCE?

    free (resolvedTransactions);

}

// MARK: - BRWallet Callback TX Deleted

static void cryptoWalletManagerBTCTxDeleted (void *info, UInt256 hash, int notifyUser, int recommendRescan) {
    BRCryptoWalletManagerBTC manager = info;
    BRCryptoWallet           wallet = manager->base.wallet;

    bool needEvents = true;

    pthread_mutex_lock (&manager->base.lock);
    BRCryptoTransferBTC transfer = cryptoWalletFindTransferByHashAsBTC (wallet, hash);

    if (NULL != transfer) {
        printf ("BTC: TxDeleted: %s\n",u256hex(UInt256Reverse(transfer->tid->txHash)));

        if (transfer->isDeleted) needEvents = false;
        else {
            transfer->isDeleted = true;
            cryptoTransferSetState (&transfer->base, cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_DELETED));
            fileServiceRemove (manager->base.fileService, fileServiceTypeTransactionsBTC, transfer->tid);
        }

        pthread_mutex_unlock (&manager->base.lock);

        if (needEvents) {
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
}

BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersBTC = {
    cryptoWalletManagerCreateBTC,
    cryptoWalletManagerReleaseBTC,
    crytpWalletManagerCreateFileServiceBTC,
    cryptoWalletManagerGetEventTypesBTC,
    cryptoWalletManagerCreateP2PManagerBTC,
    cryptoWalletManagerCreateWalletBTC,
    cryptoWalletManagerSignTransactionWithSeedBTC,
    cryptoWalletManagerSignTransactionWithKeyBTC,
    cryptoWalletManagerEstimateLimitBTC,
    cryptoWalletManagerEstimateFeeBasisBTC,
    cryptoWalletManagerSaveTransactionBundleBTC,
    NULL, // BRCryptoWalletManagerSaveTransactionBundleHandler
    cryptoWalletManagerRecoverTransfersFromTransactionBundleBTC,
    cryptoWalletManagerRecoverTransferFromTransferBundleBTC,
    NULL,//BRCryptoWalletManagerRecoverFeeBasisFromFeeEstimateHandler not supported
    cryptoWalletManagerWalletSweeperValidateSupportedBTC,
    cryptoWalletManagerCreateWalletSweeperBTC
};

BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersBCH = {
    cryptoWalletManagerCreateBTC,
    cryptoWalletManagerReleaseBTC,
    crytpWalletManagerCreateFileServiceBTC,
    cryptoWalletManagerGetEventTypesBTC,
    cryptoWalletManagerCreateP2PManagerBTC,
    cryptoWalletManagerCreateWalletBTC,
    cryptoWalletManagerSignTransactionWithSeedBTC,
    cryptoWalletManagerSignTransactionWithKeyBTC,
    cryptoWalletManagerEstimateLimitBTC,
    cryptoWalletManagerEstimateFeeBasisBTC,
    cryptoWalletManagerSaveTransactionBundleBTC,
    NULL, // BRCryptoWalletManagerSaveTransactionBundleHandler
    cryptoWalletManagerRecoverTransfersFromTransactionBundleBTC,
    cryptoWalletManagerRecoverTransferFromTransferBundleBTC,
    NULL,//BRCryptoWalletManagerRecoverFeeBasisFromFeeEstimateHandler not supported
    cryptoWalletManagerWalletSweeperValidateSupportedBTC,
    cryptoWalletManagerCreateWalletSweeperBTC
};

BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersBSV = {
    cryptoWalletManagerCreateBTC,
    cryptoWalletManagerReleaseBTC,
    crytpWalletManagerCreateFileServiceBTC,
    cryptoWalletManagerGetEventTypesBTC,
    cryptoWalletManagerCreateP2PManagerBTC,
    cryptoWalletManagerCreateWalletBTC,
    cryptoWalletManagerSignTransactionWithSeedBTC,
    cryptoWalletManagerSignTransactionWithKeyBTC,
    cryptoWalletManagerEstimateLimitBTC,
    cryptoWalletManagerEstimateFeeBasisBTC,
    cryptoWalletManagerSaveTransactionBundleBTC,
    NULL, // BRCryptoWalletManagerSaveTransactionBundleHandler
    cryptoWalletManagerRecoverTransfersFromTransactionBundleBTC,
    cryptoWalletManagerRecoverTransferFromTransferBundleBTC,
    NULL,//BRCryptoWalletManagerRecoverFeeBasisFromFeeEstimateHandler not supported
    cryptoWalletManagerWalletSweeperValidateSupportedBTC,
    cryptoWalletManagerCreateWalletSweeperBTC
};
