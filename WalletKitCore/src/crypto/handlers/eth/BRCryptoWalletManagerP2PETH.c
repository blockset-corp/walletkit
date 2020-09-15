//
//  BRCryptoWalletManagerP2PETH.c
//  Core
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoETH.h"

#include "ethereum/les/BREthereumLES.h"
#include "ethereum/bcs/BREthereumBCS.h"

// MARK: - Forward Declarations

static void
ewmReportTransferStatusAsEvent (BRCryptoWalletManager manager,
                                BRCryptoWallet wallet,
                                BRCryptoTransfer transfer,
                                BRCryptoTransferState oldState);


#if 0
static void
cryptoWalletManagerEstimateFeeBasisHandlerETH (BRCryptoWalletManager manager,
                                               BRCryptoWallet  wallet,
                                               BRCryptoCookie cookie,
                                               BRCryptoAddress target,
                                               BRCryptoAmount amount,
                                               BRCryptoNetworkFee networkFee) {
    BRCryptoTransfer transfer = ...;

    cryptoClientQRYEstimateTransferFee (cwm->qryManager, cookie, transfer, networkFee);
    return;
}
#endif

// MARK: - Client P2P

typedef struct BRCryptoClientP2PManagerRecordETH {
    struct BRCryptoClientP2PManagerRecord base;
    BRCryptoWalletManagerETH manager;
    BREthereumBCS bcs;
} *BRCryptoClientP2PManagerETH;

static BRCryptoClientP2PManagerETH
cryptoClientP2PManagerCoerce (BRCryptoClientP2PManager manager) {
    assert (CRYPTO_NETWORK_TYPE_ETH == manager->type);
    return (BRCryptoClientP2PManagerETH) manager;
}

static void
cryptoClientP2PManagerReleaseETH (BRCryptoClientP2PManager manager) {
    BRCryptoClientP2PManagerETH managerETH = cryptoClientP2PManagerCoerce (manager);
    bcsDestroy (managerETH->bcs);
}

static void
cryptoClientP2PManagerConnectETH (BRCryptoClientP2PManager manager,
                                  BRCryptoPeer peer) {
    BRCryptoClientP2PManagerETH managerETH = cryptoClientP2PManagerCoerce (manager);
    bcsStart (managerETH->bcs);
}

static void
cryptoClientP2PManagerDisconnectETH (BRCryptoClientP2PManager manager) {
    BRCryptoClientP2PManagerETH managerETH = cryptoClientP2PManagerCoerce (manager);
    bcsStop (managerETH->bcs);
}

// MARK: - Sync

static void
cryptoClientP2PManagerSyncETH (BRCryptoClientP2PManager manager,
                               BRCryptoSyncDepth depth,
                               BRCryptoBlockNumber height) {
    BRCryptoClientP2PManagerETH managerETH = cryptoClientP2PManagerCoerce (manager);
    bcsSync (managerETH->bcs, height);
}

typedef struct {
    BRCryptoWalletManager manager;
    BRCryptoTransfer transfer;
} BRCryptoClientP2PManagerPublishInfo;

static void
cryptoClientP2PManagerSendETH (BRCryptoClientP2PManager manager,
                               BRCryptoWallet   wallet,
                               BRCryptoTransfer transfer) {
    BRCryptoClientP2PManagerETH managerETH = cryptoClientP2PManagerCoerce (manager);
    BRCryptoTransferETH transferETH = cryptoTransferCoerceETH (transfer);

    bcsSendTransaction (managerETH->bcs, transferETH->originatingTransaction);
}

static BRCryptoClientP2PHandlers p2pHandlersETH = {
    cryptoClientP2PManagerReleaseETH,
    cryptoClientP2PManagerConnectETH,
    cryptoClientP2PManagerDisconnectETH,
    cryptoClientP2PManagerSyncETH,
    cryptoClientP2PManagerSendETH
};


/**
 * Handle the BCS BlockChain callback.  This should result in a 'client block event' callback.
 * However, that callback accepts a `bid` and we don't have one (in the same sense as a tid or
 * wid); perhaps the blockNumber is the `bid`?
 *
 * Additionally, this handler has no indication of the type of BCS data.  E.g is this block chained
 * or orphaned.
 *
 * @param ewm
 * @param headBlockHash
 * @param headBlockNumber
 * @param headBlockTimestamp
 */
static void
ewmHandleBlockChain (BREthereumBCSCallbackContext context,
                     BREthereumHash headBlockHash,
                     uint64_t headBlockNumber,
                     uint64_t headBlockTimestamp) {
    BRCryptoWalletManagerETH manager = context;

    // IF API_ONLY mode, do nothing
    if (CRYPTO_SYNC_MODE_API_ONLY          == manager->base.syncMode ||
        CRYPTO_SYNC_MODE_API_WITH_P2P_SEND == manager->base.syncMode) return;


    // TODO: Handle Callbacks Correctly (Racy)
    // if (NULL == manager->base.p2pManager) return;
    BRCryptoClientP2PManagerETH p2p = cryptoClientP2PManagerCoerce (manager->base.p2pManager);

    // Don't report during BCS sync.
    if (ETHEREUM_BOOLEAN_IS_FALSE(bcsSyncInProgress (p2p->bcs)))
        eth_log ("EWM", "BlockChain: %" PRIu64, headBlockNumber);

    // At least this - allows for: ewmGetBlockHeight
    cryptoNetworkSetHeight (p2p->manager->base.network, headBlockNumber);

    // TODO: Signal
    cryptoWalletManagerGenerateEvent (&p2p->manager->base,
                                             (BRCryptoWalletManagerEvent) {
        CRYPTO_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED,
        { .blockHeight = headBlockNumber }
    });

#if defined (NEVER_DEFINED)
    // TODO: Need a 'block id' - or axe the need of 'block id'?
    ewmSignalBlockEvent (ewm,
                         (BREthereumBlockId) 0,
                         BLOCK_EVENT_CHAINED,
                         SUCCESS,
                         NULL);
#endif
}
/**
 * Handle the BCS AccountState callback.
 *
 * @param ewm
 * @param accountState
 */
static void
ewmHandleAccountState (BREthereumBCSCallbackContext context,
                       BREthereumAccountState accountState) {
    BRCryptoWalletManagerETH manager = context;
    (void) manager;
    eth_log("EWM", "AccountState: Nonce: %" PRIu64, accountState.nonce);
#if 0
    ewmHandleAnnounceNonce (ewm, ethAccountGetPrimaryAddress(ewm->account), accountState.nonce, 0);
    ewmSignalBalance(ewm, ethAmountCreateEther(accountState.balance));
#endif
}

//
// We have `transaction` but we don't know if it originated a log.  If it did originate a log then
// we need to update that log's status.  We don't know what logs the transaction originated so
// we'll look through all wallets and all their transfers for any one transfer that matches the
// provided transaction.
//
// Note: that `transaction` is owned by another; thus we won't hold it.
//
static void
ewmHandleTransactionOriginatingLog (BRCryptoWalletManagerETH manager,
                                    BREthereumBCSCallbackTransactionType type,
                                    BRCryptoTransfer transfer) {

//    BREthereumHash hash = transactionGetHash(transaction);
    for (size_t wid = 0; wid < array_count(manager->base.wallets); wid++) {
        BRCryptoWallet wallet = manager->base.wallets[wid];

        // We already handle the ETH wallet.  See ewmHandleTransaction.
        if (wallet == manager->base.wallet) continue;

#if 0
        BREthereumTransfer transfer = walletGetTransferByOriginatingHash (wallet, hash);
        if (NULL != transfer) {
            // If this transaction is the transfer's originatingTransaction, then update the
            // originatingTransaction's status.
            BREthereumTransaction original = transferGetOriginatingTransaction (transfer);
            if (NULL != original && ETHEREUM_BOOLEAN_IS_TRUE(ethHashEqual (transactionGetHash(original),
                                                                        transactionGetHash(transaction))))
            transactionSetStatus (original, transactionGetStatus(transaction));

            //
            transferSetStatusForBasis (transfer, transactionGetStatus(transaction));

            // NOTE: So `transaction` applies to `transfer`.  If the transfer's basis is 'log'
            // then we'd like to update the log's identifier.... alas, we cannot because we need
            // the 'logIndex' and no way to get that from the originating transaction's status.

            ewmReportTransferStatusAsEvent(ewm, wallet, transfer);
        }
#endif
    }
}


static void
ewmHandleLogFeeBasis (BRCryptoWalletManagerETH manager,
                      BREthereumHash hash,
                      BRCryptoTransfer transferTransaction,
                      BRCryptoTransfer transferLog,
                      BRCryptoWallet walletLog) {
    BRCryptoWallet wallet = manager->base.wallet; // Wallet Holding ETH

    // Find the ETH transfer, if needed
    if (NULL == transferTransaction) {
        BRCryptoHash cryHash = cryptoHashCreateAsETH(hash);
        transferTransaction = cryptoWalletGetTransferByHash (wallet, cryHash); //  walletGetTransferByIdentifier (ewmGetWallet(ewm), hash);
        cryptoHashGive(cryHash);
    }
    // If none exists, then the transaction hasn't been 'synced' yet.
    if (NULL == transferTransaction) return;

    // If we have a TOK transfer, set the fee basis.
    if (NULL != transferLog) {
        BRCryptoTransferState oldState = cryptoTransferGetState (transferLog);
        cryptoTransferSetState (transferLog, cryptoTransferGetState(transferTransaction));
        ewmReportTransferStatusAsEvent (&manager->base, walletLog, transferLog, oldState);
        cryptoTransferStateRelease(&oldState);
    }

    // but if we don't have a TOK transfer, find every transfer referencing `hash` and set the basis.
    else
        for (size_t wid = 0; wid < array_count(manager->base.wallets); wid++) {
            BRCryptoWallet wallet = manager->base.wallets[wid];

            // We are only looking for TOK transfers (non-ETH).
            if (wallet == manager->base.wallet) continue;

            for (size_t tid = 0; tid < array_count(wallet->transfers); tid++) {
                transferLog = wallet->transfers[tid];

                // Look for a log that has a matching transaction hash
                BREthereumLog log = cryptoTransferCoerceETH(transferLog)->basis.u.log;
                if (NULL != log) {
                    BREthereumHash transactionHash;
                    if (ETHEREUM_BOOLEAN_TRUE == logExtractIdentifier (log, &transactionHash, NULL) &&
                        ETHEREUM_BOOLEAN_TRUE == ethHashEqual (transactionHash, hash)) {
                        ewmHandleLogFeeBasis (manager, hash, transferTransaction, transferLog, wallet);
                        break;
                    }
                }
            }
        }
}

static void
ewmHandleExchangeFeeBasis (BRCryptoWalletManagerETH manager,
                           BREthereumHash hash,
                           BRCryptoTransfer transferTransaction,
                           BRCryptoTransfer transferExchange,
                           BRCryptoWallet walletExchange) {
    BRCryptoWallet wallet = manager->base.wallet; // Wallet Holding ETH

    // Find the ETH transfer, if needed
    if (NULL == transferTransaction) {
        BRCryptoHash cryHash = cryptoHashCreateAsETH(hash);
        transferTransaction = cryptoWalletGetTransferByHash (wallet, cryHash); //  walletGetTransferByIdentifier (ewmGetWallet(ewm), hash);
        cryptoHashGive(cryHash);
    }


    // If none exists, then the transaction hasn't been 'synced' yet.
    if (NULL == transferTransaction) return;

    // If we have an exchange transfer, set the fee basis.
    if (NULL != transferExchange) {
        BRCryptoTransferState oldState = cryptoTransferGetState (transferExchange);
        cryptoTransferSetState (transferExchange, cryptoTransferGetState(transferTransaction));
        ewmReportTransferStatusAsEvent (&manager->base, walletExchange, transferExchange, oldState);
        cryptoTransferStateRelease(&oldState);
    }

    // but if we don't have an exchanage transfer, find every transfer referencing `hash` and set the basis.
    else
        for (size_t wid = 0; wid < array_count(manager->base.wallets); wid++) {
            BRCryptoWallet wallet = manager->base.wallets[wid];

            // We are only looking for TOK transfers (non-ETH).
            if (wallet == manager->base.wallet) continue;

            for (size_t tid = 0; tid < array_count(wallet->transfers); tid++) {
                transferExchange = wallet->transfers[tid];

                // Look for a log that has a matching transaction hash
                BREthereumExchange exchange = cryptoTransferCoerceETH(transferExchange)->basis.u.exchange;
                if (NULL != exchange) {
                    BREthereumHash transactionHash;
                    if (ETHEREUM_BOOLEAN_TRUE == ethExchangeExtractIdentifier (exchange, &transactionHash, NULL) &&
                        ETHEREUM_BOOLEAN_TRUE == ethHashEqual (transactionHash, hash)) {
                        ewmHandleExchangeFeeBasis (manager, hash, transferTransaction, transferExchange, wallet);
                        break;
                    }
                }
            }
        }
}

private_extern void
ewmHandleTransaction (BREthereumBCSCallbackContext context,
                      BREthereumBCSCallbackTransactionType type,
                      OwnershipGiven BREthereumTransaction transaction) {
    BRCryptoWalletManagerETH manager = context;
    (void) manager;

    BREthereumHash ethHash = transactionGetHash(transaction);
    BRCryptoHash      hash = cryptoHashCreateAsETH (ethHash);
    BRCryptoWallet  wallet = manager->base.wallet;

    BRCryptoTransfer transfer = cryptoWalletGetTransferByHash (wallet, hash);

    bool needStatusEvent = false;

    if (NULL == transfer) {
        transfer = cryptoTransferCreateWithTransactionAsETH (wallet->listenerTransfer,
                                                             wallet->unit,
                                                             wallet->unitForFee,
                                                             manager->account,
                                                             transaction);

        cryptoWalletAddTransfer (wallet, transfer);
    }

    else {

    }

    // If this transfer is referenced, fill out the referencer's fee basis.
    ewmHandleLogFeeBasis      (manager, ethHash, transfer, NULL, NULL);
    ewmHandleExchangeFeeBasis (manager, ethHash, transfer, NULL, NULL);

    if (needStatusEvent) {
        BREthereumHashString hashString;
        ethHashFillString (transactionGetHash(transaction), hashString);

        eth_log ("EWM", "Transaction: \"%s\", Change: %s, Status: %d", hashString,
                 BCS_CALLBACK_TRANSACTION_TYPE_NAME(type),
                 transactionGetStatus(transaction).type);

        //            ewmReportTransferStatusAsEvent(ewm, wallet, transfer);
    }

    ewmHandleTransactionOriginatingLog (manager, type, transfer);

#if 0
    BREthereumHash hash = transactionGetHash(transaction);

    // Find the wallet
    BREthereumWallet wallet = ewmGetWallet(ewm);
    assert (NULL != wallet);

    ///
    ///  What hash to use:
    ///     originating -> expecting a result
    ///     identifier  -> seen already.
    ///
    ///     originating should be good in one wallet?  [no, multiple logs?]
    ///        wallet will have transfers w/o a basis.
    ///        does a transfer with an ERC20 transfer fit in one wallet?
    ///        does a transfer with some smart contract fit in one wallet (no?)
    ///

    // Find a preexisting transfer
    BREthereumTransfer transfer = walletGetTransferByIdentifier (wallet, hash);
    if (NULL == transfer)
        transfer = walletGetTransferByOriginatingHash (wallet, hash);

    int needStatusEvent = 0;

    // If we've no transfer, then create one and save `transaction` as the basis
    if (NULL == transfer) {
        transfer = transferCreateWithTransaction (transaction); // transaction ownership given

        walletHandleTransfer (wallet, transfer);

        // We've added a transfer and arguably we should update the wallet's balance.  But don't.
        // Ethereum is 'account based'; we'll only update the balance based on a account state
        // change (based on a P2P or API callback).
        //
        // walletUpdateBalance (wallet);

        ewmSignalTransferEvent (ewm, wallet, transfer, (BREthereumTransferEvent) {
            TRANSFER_EVENT_CREATED,
            SUCCESS
        });

         // If this transfer is referenced by a log, fill out the log's fee basis.
        ewmHandleLogFeeBasis (ewm, hash, transfer, NULL);

        needStatusEvent = 1;
    }
    else {
        needStatusEvent = ewmReportTransferStatusAsEventIsNeeded (ewm, wallet, transfer,
                                                                  transactionGetStatus(transaction));


        // If this transaction is the transfer's originatingTransaction, then update the
        // originatingTransaction's status.
        BREthereumTransaction original = transferGetOriginatingTransaction (transfer);
        if (NULL != original && ETHEREUM_BOOLEAN_IS_TRUE(ethHashEqual (transactionGetHash(original),
                                                                    transactionGetHash(transaction))))
            transactionSetStatus (original, transactionGetStatus(transaction));

        transferSetBasisForTransaction (transfer, transaction); // transaction ownership given
    }

    if (needStatusEvent) {
        BREthereumHashString hashString;
        ethHashFillString(hash, hashString);
        eth_log ("EWM", "Transaction: \"%s\", Change: %s, Status: %d", hashString,
                 BCS_CALLBACK_TRANSACTION_TYPE_NAME(type),
                 transactionGetStatus(transaction).type);

        ewmReportTransferStatusAsEvent(ewm, wallet, transfer);
    }

    ewmHandleTransactionOriginatingLog (ewm, type, transaction);
#endif
}

static int
ewmReportTransferStatusAsEventIsNeeded (BRCryptoWalletManager manager,
                                        BRCryptoWallet wallet,
                                        BRCryptoTransfer transfer,
                                        BREthereumTransactionStatus status) {
    return 0;
#ifdef REFACTOR
    return (// If the status differs from the transfer's basis status...
            ETHEREUM_BOOLEAN_IS_FALSE (transactionStatusEqual (status, transferGetStatusForBasis(transfer))) ||
            // Otherwise, if the transfer's status differs.
            ETHEREUM_BOOLEAN_IS_FALSE (transferHasStatus (transfer, transferStatusCreate(status))));
#endif
}

static void
ewmReportTransferStatusAsEvent (BRCryptoWalletManager manager,
                                BRCryptoWallet wallet,
                                BRCryptoTransfer transfer,
                                BRCryptoTransferState oldState) {
    BRCryptoTransferState newState = cryptoTransferGetState (transfer);
    (void) newState;
#if 0
    if (!cryptoTransferStateIsEqual (&oldState, &newState)) {
        cryptoWalletManagerGenerateTransferEvent (manager, wallet, transfer, (BRCryptoTransferEvent) {
            CRYPTO_TRANSFER_EVENT_CHANGED,
            { .state = { oldState, newState }}
        });
    }
#endif
    
#if 0
    if (ETHEREUM_BOOLEAN_IS_TRUE (transferHasStatus (transfer, TRANSFER_STATUS_SUBMITTED)))
        ewmSignalTransferEvent(ewm, wallet, transfer, (BREthereumTransferEvent) {
            TRANSFER_EVENT_SUBMITTED,
            SUCCESS
        });

    else if (ETHEREUM_BOOLEAN_IS_TRUE (transferHasStatus (transfer, TRANSFER_STATUS_INCLUDED)))
        ewmSignalTransferEvent(ewm, wallet, transfer, (BREthereumTransferEvent) {
            TRANSFER_EVENT_INCLUDED,
            SUCCESS
        });

    else if (ETHEREUM_BOOLEAN_IS_TRUE (transferHasStatus (transfer, TRANSFER_STATUS_ERRORED))) {
        char *reason = NULL;
        transferExtractStatusError (transfer, &reason);
        ewmSignalTransferEvent (ewm, wallet, transfer,
                                ethTransferEventCreateError (TRANSFER_EVENT_ERRORED,
                                                          ERROR_TRANSACTION_SUBMISSION,
                                                          reason));

        if (NULL != reason) free (reason);
    }
#endif
}

private_extern void
ewmHandleLog (BREthereumBCSCallbackContext context,
              BREthereumBCSCallbackLogType type,
              OwnershipGiven BREthereumLog log) {
    BRCryptoWalletManagerETH managerETH = context;

    BREthereumHash logHash = logGetHash(log);

    BREthereumHash transactionHash;
    size_t logIndex;

    // Assert that we always have an identifier for `log`.
    BREthereumBoolean extractedIdentifier = logExtractIdentifier (log, &transactionHash, &logIndex);
    assert (ETHEREUM_BOOLEAN_IS_TRUE (extractedIdentifier));

    BREthereumAddress logAddress = logGetAddress(log);
    BREthereumToken   token      = BRSetGet (managerETH->tokens, &logAddress);
    if (NULL == token) { logRelease(log); return;}

    // TODO: Confirm LogTopic[0] is 'transfer'
    if (3 != logGetTopicsCount(log)) { logRelease(log); return; }

    BRCryptoWalletETH walletETH = cryptoWalletManagerEnsureWalletForToken (managerETH, token);
    assert (NULL != walletETH);

    BRCryptoTransferETH transferETH = cryptoWalletLookupTransferByIdentifier (walletETH, logHash);
    if (NULL == transferETH)
        transferETH = cryptoWalletLookupTransferByOriginatingHash (walletETH, transactionHash);

    int needStatusEvent = 0;

    BRCryptoTransferState oldState;

    // If we've no transfer, then create one and save `log` as the basis
    if (NULL == transferETH) {

        // Do we know that log->data is a number?
        BRRlpItem  item  = rlpDataGetItem (managerETH->coder, logGetDataShared(log));
        UInt256 amount = rlpDecodeUInt256 (managerETH->coder, item, 1);
        rlpItemRelease (managerETH->coder, item);

        transferETH = (BRCryptoTransferETH) cryptoTransferCreateWithLogAsETH (walletETH->base.listenerTransfer,
                                                                              walletETH->base.unit,
                                                                              walletETH->base.unitForFee,
                                                                              managerETH->account,
                                                                              amount,
                                                                              log);

        cryptoWalletAddTransfer (&walletETH->base, &transferETH->base);

        // We've added a transfer and arguably we should update the wallet's balance.  But don't.
        // Ethereum is 'account based'; we'll only update the balance based on a account state
        // change (based on a P2P or API callback).
        //
        // walletUpdateBalance (wallet);

        // If this transfer references a transaction, fill out this log's fee basis
        ewmHandleLogFeeBasis (managerETH, transactionHash, NULL, &transferETH->base, &walletETH->base);

        oldState = (BRCryptoTransferState) { CRYPTO_TRANSFER_STATE_CREATED };

        needStatusEvent = 1;
    }

    // We've got a transfer for log.  We'll update the transfer's basis and check if we need
    // to report a transfer status event.  We'll strive to only report events when the status has
    // actually changed.
    else {
        needStatusEvent = ewmReportTransferStatusAsEventIsNeeded (&managerETH->base, &walletETH->base, &transferETH->base,
                                                                  logGetStatus (log));

        // Log becomes the new basis for transfer
        assert  (TRANSFER_BASIS_LOG == transferETH->basis.type);
        assert (NULL != transferETH->basis.u.log);
        logRelease(transferETH->basis.u.log);

        transferETH->basis.type = TRANSFER_BASIS_LOG;
        transferETH->basis.u.log = log;               // ownership give
    }

    if (needStatusEvent) {
        BREthereumHashString logHashString;
        ethHashFillString(logHash, logHashString);

        BREthereumHashString transactionHashString;
        ethHashFillString(transactionHash, transactionHashString);

        eth_log ("EWM", "Log: %s { %8s @ %zu }, Change: %s, Status: %d",
                 logHashString, transactionHashString, logIndex,
                 BCS_CALLBACK_TRANSACTION_TYPE_NAME(type),
                 logGetStatus(log).type);

        ewmReportTransferStatusAsEvent (&managerETH->base, &walletETH->base, &transferETH->base, oldState);
    }
}

private_extern void
ewmHandleExchange (BREthereumBCSCallbackContext context,
                   BREthereumBCSCallbackExchangeType type,
                   OwnershipGiven BREthereumExchange exchange) {
    BRCryptoWalletManagerETH managerETH = context;

    BREthereumHash exchangeHash = ethExchangeGetHash(exchange);

    BREthereumHash transactionHash;
    size_t exchangeIndex;

    // Assert that we always have an identifier for `log`.
    BREthereumBoolean extractedIdentifier = ethExchangeExtractIdentifier (exchange, &transactionHash, &exchangeIndex);
    assert (ETHEREUM_BOOLEAN_IS_TRUE (extractedIdentifier));

    BREthereumAddress contract = ethExchangeGetContract(exchange);
    BREthereumToken   token    = BRSetGet (managerETH->tokens, &contract);
    BREthereumBoolean contractIsEmpty = ethAddressEqual (contract, EMPTY_ADDRESS_INIT);

    // If `token` is NULL, then we don't know about this currency.  If `contract` is not NULL,
    // then this is an exchange that we are NOT interested in.
    if (NULL == token && ETHEREUM_BOOLEAN_FALSE == contractIsEmpty) {
        ethExchangeRelease (exchange);
        return;
    }

    BRCryptoWalletETH walletETH = cryptoWalletManagerEnsureWalletForToken (managerETH, token);
    assert (NULL != walletETH);

    BRCryptoTransferETH transferETH = cryptoWalletLookupTransferByIdentifier (walletETH, exchangeHash);
    if (NULL == transferETH)
        transferETH = cryptoWalletLookupTransferByOriginatingHash (walletETH, transactionHash);

    bool needStatusEvent = false;

    BRCryptoTransferState oldState;

    if (NULL == transferETH) {
        transferETH = (BRCryptoTransferETH) cryptoTransferCreateWithExchangeAsETH (walletETH->base.listenerTransfer,
                                                                                   walletETH->base.unit,
                                                                                   walletETH->base.unitForFee,
                                                                                   managerETH->account,
                                                                                   ethExchangeGetAssetValue (exchange),
                                                                                   exchange);

        cryptoWalletAddTransfer (&walletETH->base, &transferETH->base);

        ewmHandleExchangeFeeBasis (managerETH, transactionHash, NULL, &transferETH->base, &walletETH->base);

        oldState = (BRCryptoTransferState) { CRYPTO_TRANSFER_STATE_CREATED };

        needStatusEvent = true;
    }
    else {
        needStatusEvent = ewmReportTransferStatusAsEventIsNeeded (&managerETH->base, &walletETH->base, &transferETH->base,
                                                                  ethExchangeGetStatus(exchange));

        transferETH->basis.type = TRANSFER_BASIS_EXCHANGE;
        transferETH->basis.u.exchange = exchange;
    }

    if (needStatusEvent) {
        BREthereumHashString exchnageHashString;
        ethHashFillString(exchangeHash, exchnageHashString);

        BREthereumHashString transactionHashString;
        ethHashFillString(transactionHash, transactionHashString);

        eth_log ("EWM", "Exchange: %s { %8s @ %zu }, Change: %s, Status: %d",
                 exchnageHashString, transactionHashString, exchangeIndex,
                 BCS_CALLBACK_TRANSACTION_TYPE_NAME(type),
                 ethExchangeGetStatus (exchange).type);

        ewmReportTransferStatusAsEvent (&managerETH->base, &walletETH->base, &transferETH->base, oldState);
    }
#if REFACTOR
    // We've added a transfer and should update the wallet's balance.  Ethereum is 'account based';
    // but in API modes we don't have the account information - so we'll update the balance
    // explicitly.  In P2P mode, we get the 'account'.
    //
    if (CRYPTO_SYNC_MODE_API_ONLY          == ewm->mode ||
        CRYPTO_SYNC_MODE_API_WITH_P2P_SEND == ewm->mode)
        ewmUpdateAndReportWalletState (ewm, wallet);
#endif
}

static void
ewmHandleSaveBlocks (BREthereumBCSCallbackContext context,
                     OwnershipGiven BRArrayOf(BREthereumBlock) blocks) {
    BRCryptoWalletManagerETH manager = context;
    size_t count = array_count(blocks);

    eth_log("EWM", "Save Blocks (Storage): %zu", count);
    fileServiceReplace (manager->base.fileService,
                        fileServiceTypeBlocksETH,
                        (const void **) blocks,
                        count);

    array_free (blocks);
}

static void
ewmHandleSaveNodes (BREthereumBCSCallbackContext context,
                    OwnershipGiven BRArrayOf(BREthereumNodeConfig) nodes) {
    BRCryptoWalletManagerETH manager = context;
    size_t count = array_count(nodes);

    eth_log("EWM", "Save Nodes (Storage): %zu", count);
    fileServiceReplace (manager->base.fileService,
                        fileServiceTypeNodesETH,
                        (const void **) nodes,
                        count);

    array_free (nodes);
}

//extern void
//ewmHandleSaveTransaction (BREthereumBCSCallbackContext context,
//                          BREthereumTransaction transaction,
//                          BREthereumClientChangeType type) {
//    BREthereumHash hash = transactionGetHash(transaction);
//    BREthereumHashString fileName;
//    ethHashFillString(hash, fileName);
//
//    eth_log("EWM", "Transaction: Save: %s: %s",
//            CLIENT_CHANGE_TYPE_NAME (type),
//            fileName);
//
//    if (CLIENT_CHANGE_REM == type || CLIENT_CHANGE_UPD == type)
//        fileServiceRemove (ewm->fs, ewmFileServiceTypeTransactions,
//                           fileServiceGetIdentifier(ewm->fs, ewmFileServiceTypeTransactions, transaction));
//
//    if (CLIENT_CHANGE_ADD == type || CLIENT_CHANGE_UPD == type)
//        fileServiceSave (ewm->fs, ewmFileServiceTypeTransactions, transaction);
//}
//
//extern void
//ewmHandleSaveLog (BREthereumBCSCallbackContext context,
//                  BREthereumLog log,
//                  BREthereumClientChangeType type) {
//    BREthereumHash hash = logGetHash(log);
//    BREthereumHashString filename;
//    ethHashFillString(hash, filename);
//
//    eth_log("EWM", "Log: Save: %s: %s",
//            CLIENT_CHANGE_TYPE_NAME (type),
//            filename);
//
//    if (CLIENT_CHANGE_REM == type || CLIENT_CHANGE_UPD == type)
//        fileServiceRemove (ewm->fs, ewmFileServiceTypeLogs,
//                           fileServiceGetIdentifier (ewm->fs, ewmFileServiceTypeLogs, log));
//
//    if (CLIENT_CHANGE_ADD == type || CLIENT_CHANGE_UPD == type)
//        fileServiceSave (ewm->fs, ewmFileServiceTypeLogs, log);
//}
//
//extern void
//ewmHandleSaveExchange (BREthereumEWM ewm,
//                       BREthereumExchange exchange,
//                       BREthereumClientChangeType type) {
//    BREthereumHash hash = ethExchangeGetHash(exchange);
//    BREthereumHashString filename;
//    ethHashFillString(hash, filename);
//
//    eth_log("EWM", "Exchange: Save: %s: %s",
//            CLIENT_CHANGE_TYPE_NAME (type),
//            filename);
//
//    if (CLIENT_CHANGE_REM == type || CLIENT_CHANGE_UPD == type)
//        fileServiceRemove (ewm->fs, ewmFileServiceTypeExchanges,
//                           fileServiceGetIdentifier (ewm->fs, ewmFileServiceTypeExchanges, exchange));
//
//    if (CLIENT_CHANGE_ADD == type || CLIENT_CHANGE_UPD == type)
//        fileServiceSave (ewm->fs, ewmFileServiceTypeExchanges, exchange);
//}
//extern void
//ewmHandleSaveWallet (BREthereumBCSCallbackContext context,
//                     BREthereumWallet wallet,
//                     BREthereumClientChangeType type) {
//    BREthereumWalletState state = walletStateCreate (wallet);
//
//    // If this is the primaryWallet, hack in the nonce
//    if (wallet == ewm->walletHoldingEther) {
//        walletStateSetNonce (state,
//                             ethAccountGetAddressNonce (ewm->account,
//                                                     ethAccountGetPrimaryAddress(ewm->account)));
//    }
//
//    BREthereumHash hash = walletStateGetHash(state);
//    BREthereumHashString filename;
//    ethHashFillString(hash, filename);
//
//    eth_log ("EWM", "Wallet: Save: %s: %s",
//             CLIENT_CHANGE_TYPE_NAME (type),
//             filename);
//
//    switch (type) {
//        case CLIENT_CHANGE_REM:
//            fileServiceRemove (ewm->fs, ewmFileServiceTypeWallets,
//                               fileServiceGetIdentifier (ewm->fs, ewmFileServiceTypeWallets, state));
//            break;
//
//        case CLIENT_CHANGE_ADD:
//        case CLIENT_CHANGE_UPD:
//            fileServiceSave (ewm->fs, ewmFileServiceTypeWallets, state);
//            break;
//    }
//
//    walletStateRelease (state);
//}

static void
ewmHandleSync (BREthereumBCSCallbackContext context,
               BREthereumBCSCallbackSyncType type,
               uint64_t blockNumberStart,
               uint64_t blockNumberCurrent,
               uint64_t blockNumberStop) {
    BRCryptoWalletManagerETH manager = context;

    //    assert (CRYPTO_SYNC_MODE_P2P_ONLY == ewm->mode || CRYPTO_SYNC_MODE_P2P_WITH_API_SYNC == ewm->mode);

    BRCryptoSyncPercentComplete syncCompletePercent = AS_CRYPTO_SYNC_PERCENT_COMPLETE (100.0 * (blockNumberCurrent - blockNumberStart) / (blockNumberStop - blockNumberStart));
    // We do not have blockTimestampCurrent

    if (blockNumberCurrent == blockNumberStart) {
        cryptoWalletManagerGenerateEvent (&manager->base,
                                                 (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_SYNC_STARTED
        });

        // TODO: connected -> syncing
    }
    else if (blockNumberCurrent == blockNumberStop) {
        cryptoWalletManagerGenerateEvent (&manager->base,
                                                 (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_SYNC_STOPPED
        });

        // TODO: syncing -> connected
    }
    else {
        cryptoWalletManagerGenerateEvent (&manager->base,
                                                 (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_SYNC_CONTINUES,
            { .syncContinues = { NO_CRYPTO_TIMESTAMP, syncCompletePercent }}
        });
    }

    eth_log ("EWM", "Sync: %d, %.2f%%", type, syncCompletePercent);
}

static void
ewmHandleGetBlocks (BREthereumBCSCallbackContext context,
                    BREthereumAddress address,
                    BREthereumSyncInterestSet interests,
                    uint64_t blockStart,
                    uint64_t blockStop) {
    BRCryptoWalletManagerETH manager = context;
    (void) manager;
#if 0
    char *strAddress = ethAddressGetEncodedString(address, 0);

    ewm->client.funcGetBlocks (ewm->client.context,
                               ewm,
                               strAddress,
                               interests,
                               blockStart,
                               blockStop,
                               ++ewm->requestId);

    free (strAddress);
#endif
}


extern BRCryptoClientP2PManager
cryptoWalletManagerCreateP2PManagerETH (BRCryptoWalletManager manager) {
    BRCryptoClientP2PManager p2pBase = cryptoClientP2PManagerCreate (sizeof (struct BRCryptoClientP2PManagerRecordETH),
                                                                     manager->type,
                                                                     &p2pHandlersETH);
    BRCryptoClientP2PManagerETH p2p = cryptoClientP2PManagerCoerce (p2pBase);
    p2p->manager = cryptoWalletManagerCoerceETH(manager);

    // TODO: Handle Callbacks Correctly
    manager->p2pManager = p2pBase;

    BREthereumBCSListener listener = {
        manager,
        ewmHandleBlockChain,
        ewmHandleAccountState,
        ewmHandleTransaction,
        ewmHandleLog,
        ewmHandleSaveBlocks,
        ewmHandleSaveNodes,
        ewmHandleSync,
        ewmHandleGetBlocks
    };

    BREthereumNetwork network = p2p->manager->network;
    BREthereumAddress address = ethAccountGetPrimaryAddress (p2p->manager->account);

    BRSetOf(BREthereumTransaction) transactions = initialTransactionsLoadETH (manager);
    BRSetOf(BREthereumLog)         logs         = initialLogsLoadETH         (manager);
    // Exchanges
    BRSetOf(BREthereumNodeConfig)  nodes        = initialNodesLoadETH        (manager);
    BRSetOf(BREthereumBlock)       blocks       = initialBlocksLoadETH       (manager);

    // If we have no blocks; then add a checkpoint
    if (0 == BRSetCount(blocks)) {
        const BREthereumBlockCheckpoint *checkpoint = blockCheckpointLookupByTimestamp (network, cryptoAccountGetTimestamp (manager->account));
        BREthereumBlock block = blockCreate (blockCheckpointCreatePartialBlockHeader (checkpoint));
        blockSetTotalDifficulty (block, checkpoint->u.td);
        BRSetAdd (blocks, block);
    }

    p2p->bcs = bcsCreate (network,
                          address,
                          listener,
                          p2p->manager->base.syncMode,
                          nodes,
                          blocks,
                          transactions,
                          logs);

    return p2pBase;
}
