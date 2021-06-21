//
//  BRCryptoWallet.c
//  BRCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoWalletP.h"

#include "BRCryptoAmountP.h"
#include "BRCryptoFeeBasisP.h"
#include "BRCryptoKeyP.h"
#include "BRCryptoTransferP.h"
#include "BRCryptoAddressP.h"
#include "BRCryptoNetworkP.h"
#include "BRCryptoPaymentP.h"

#include "BRCryptoHandlersP.h"

static void
cryptoWalletUpdTransfer (BRCryptoWallet wallet,
                                  BRCryptoTransfer transfer,
                                  OwnershipKept BRCryptoTransferState newState);

static void
cryptoWalletUpdBalanceOnTransferConfirmation (BRCryptoWallet wallet,
                                              BRCryptoTransfer transfer);

// MARK: - Wallet Event

struct BRCryptoWalletEventRecord {
    BRCryptoWalletEventType type;
    union {
        struct {
            BRCryptoWalletState old;
            BRCryptoWalletState new;
        } state;

        BRCryptoTransfer transfer;

        struct {
            /// Handler must 'give'
            BRCryptoAmount amount;
        } balanceUpdated;

        struct {
            /// Handler must 'give'
            BRCryptoFeeBasis basis;
        } feeBasisUpdated;

        struct {
            /// Handler must 'give' basis
            BRCryptoStatus status;
            BRCryptoCookie cookie;
            BRCryptoFeeBasis basis;
        } feeBasisEstimated;
    } u;

    BRCryptoRef ref;
};

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoWalletEvent, cryptoWalletEvent)

private_extern BRCryptoWalletEvent
cryptoWalletEventCreate (BRCryptoWalletEventType type) {
    BRCryptoWalletEvent event = calloc (1, sizeof (struct BRCryptoWalletEventRecord));

    event->type = type;
    event->ref  = CRYPTO_REF_ASSIGN (cryptoWalletEventRelease);

    return event;
}

static void
cryptoWalletEventRelease (BRCryptoWalletEvent event) {
    switch (event->type) {
        case CRYPTO_WALLET_EVENT_CREATED:
        case CRYPTO_WALLET_EVENT_DELETED:
            break;

        case CRYPTO_WALLET_EVENT_CHANGED:
            // BRCryptoWalletState old, new
            break;

        case CRYPTO_WALLET_EVENT_TRANSFER_ADDED:
        case CRYPTO_WALLET_EVENT_TRANSFER_CHANGED:
        case CRYPTO_WALLET_EVENT_TRANSFER_DELETED:
            cryptoTransferGive (event->u.transfer);
            break;

        case CRYPTO_WALLET_EVENT_TRANSFER_SUBMITTED:
            cryptoTransferGive (event->u.transfer);
            break;

        case CRYPTO_WALLET_EVENT_BALANCE_UPDATED:
            cryptoAmountGive (event->u.balanceUpdated.amount);
            break;

        case CRYPTO_WALLET_EVENT_FEE_BASIS_UPDATED:
            cryptoFeeBasisGive (event->u.feeBasisUpdated.basis);
            break;

        case CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED:
            // BRCryptoStatus status
            // BRCryptoCookie cookie
            cryptoFeeBasisGive (event->u.feeBasisEstimated.basis);
            break;
    }

    memset (event, 0, sizeof(*event));
    free (event);
}

extern BRCryptoWalletEventType
cryptoWalletEventGetType (BRCryptoWalletEvent event) {
    return event->type;
}

private_extern BRCryptoWalletEvent
cryptoWalletEventCreateState (BRCryptoWalletState old,
                              BRCryptoWalletState new) {
    BRCryptoWalletEvent event = cryptoWalletEventCreate (CRYPTO_WALLET_EVENT_CHANGED);

    
    event->u.state.old = old;
    event->u.state.new = new;

    return event;
}

extern BRCryptoBoolean
cryptoWalletEventExtractState (BRCryptoWalletEvent event,
                               BRCryptoWalletState *old,
                               BRCryptoWalletState *new) {
    if (CRYPTO_WALLET_EVENT_CHANGED != event->type) return CRYPTO_FALSE;

    if (NULL != old) *old = event->u.state.old;
    if (NULL != new) *new = event->u.state.new;

    return CRYPTO_TRUE;
}

static bool
cryptoWalletEventTypeIsTransfer (BRCryptoWalletEventType type) {
    return (CRYPTO_WALLET_EVENT_TRANSFER_ADDED   == type ||
            CRYPTO_WALLET_EVENT_TRANSFER_CHANGED == type ||
            CRYPTO_WALLET_EVENT_TRANSFER_DELETED == type);
}

private_extern BRCryptoWalletEvent
cryptoWalletEventCreateTransfer (BRCryptoWalletEventType type,
                                 BRCryptoTransfer transfer) {
    assert (cryptoWalletEventTypeIsTransfer(type));
    BRCryptoWalletEvent event = cryptoWalletEventCreate (type);

    event->u.transfer = cryptoTransferTake (transfer);

    return event;
}

extern BRCryptoBoolean
cryptoWalletEventExtractTransfer (BRCryptoWalletEvent event,
                                  BRCryptoTransfer *transfer) {

    if (!cryptoWalletEventTypeIsTransfer (event->type)) return CRYPTO_FALSE;

    if (NULL != transfer) *transfer = cryptoTransferTake (event->u.transfer);

    return CRYPTO_TRUE;
}

private_extern BRCryptoWalletEvent
cryptoWalletEventCreateTransferSubmitted (BRCryptoTransfer transfer) {
    BRCryptoWalletEvent event = cryptoWalletEventCreate (CRYPTO_WALLET_EVENT_TRANSFER_SUBMITTED);

    event->u.transfer = cryptoTransferTake (transfer);

    return event;
}

extern BRCryptoBoolean
cryptoWalletEventExtractTransferSubmit (BRCryptoWalletEvent event,
                                        BRCryptoTransfer *transfer) {

    if (CRYPTO_WALLET_EVENT_TRANSFER_SUBMITTED != event->type) return CRYPTO_FALSE;

    if (NULL != transfer) *transfer = cryptoTransferTake (event->u.transfer);

    return CRYPTO_TRUE;
}

private_extern BRCryptoWalletEvent
cryptoWalletEventCreateBalanceUpdated (OwnershipGiven BRCryptoAmount balance) {
    BRCryptoWalletEvent event = cryptoWalletEventCreate (CRYPTO_WALLET_EVENT_BALANCE_UPDATED);

    event->u.balanceUpdated.amount = cryptoAmountTake (balance);

    return event;
}

extern BRCryptoBoolean
cryptoWalletEventExtractBalanceUpdate (BRCryptoWalletEvent event,
                                       BRCryptoAmount *balance) {
    if (CRYPTO_WALLET_EVENT_BALANCE_UPDATED != event->type) return CRYPTO_FALSE;

    if (NULL != balance) *balance = cryptoAmountTake (event->u.balanceUpdated.amount);

    return CRYPTO_TRUE;
}

private_extern BRCryptoWalletEvent
cryptoWalletEventCreateFeeBasisUpdated (BRCryptoFeeBasis basis) {
    BRCryptoWalletEvent event = cryptoWalletEventCreate (CRYPTO_WALLET_EVENT_FEE_BASIS_UPDATED);

    event->u.feeBasisUpdated.basis = cryptoFeeBasisTake (basis);

    return event;
}

extern BRCryptoBoolean
cryptoWalletEventExtractFeeBasisUpdate (BRCryptoWalletEvent event,
                                        BRCryptoFeeBasis *basis) {
    if (CRYPTO_WALLET_EVENT_FEE_BASIS_UPDATED != event->type) return CRYPTO_FALSE;

    if (NULL != basis) *basis = cryptoFeeBasisTake (event->u.feeBasisUpdated.basis);

    return CRYPTO_TRUE;
}

private_extern BRCryptoWalletEvent
cryptoWalletEventCreateFeeBasisEstimated (BRCryptoStatus status,
                                          BRCryptoCookie cookie,
                                          OwnershipGiven BRCryptoFeeBasis basis) {
    BRCryptoWalletEvent event = cryptoWalletEventCreate (CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED);

    event->u.feeBasisEstimated.status = status;
    event->u.feeBasisEstimated.cookie = cookie;
    event->u.feeBasisEstimated.basis  = cryptoFeeBasisTake (basis);

    return event;
}

extern BRCryptoBoolean
cryptoWalletEventExtractFeeBasisEstimate (BRCryptoWalletEvent event,
                                          BRCryptoStatus *status,
                                          BRCryptoCookie *cookie,
                                          BRCryptoFeeBasis *basis) {
    if (CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED != event->type) return CRYPTO_FALSE;

    if (NULL != status) *status = event->u.feeBasisEstimated.status;
    if (NULL != cookie) *cookie = event->u.feeBasisEstimated.cookie;
    if (NULL != basis ) *basis  = cryptoFeeBasisTake (event->u.feeBasisEstimated.basis);

    return CRYPTO_TRUE;
}

extern BRCryptoBoolean
cryptoWalletEventIsEqual (BRCryptoWalletEvent event1,
                          BRCryptoWalletEvent event2) {
    if (event1->type != event2->type) return CRYPTO_FALSE;

    switch (event1->type) {
        case CRYPTO_WALLET_EVENT_CREATED:
        case CRYPTO_WALLET_EVENT_DELETED:
            return CRYPTO_TRUE;

        case CRYPTO_WALLET_EVENT_CHANGED:
            return AS_CRYPTO_BOOLEAN (event1->u.state.old == event2->u.state.new &&
                                      event1->u.state.new == event2->u.state.new);

        case CRYPTO_WALLET_EVENT_TRANSFER_ADDED:
        case CRYPTO_WALLET_EVENT_TRANSFER_CHANGED:
        case CRYPTO_WALLET_EVENT_TRANSFER_DELETED:
            return cryptoTransferEqual(event1->u.transfer, event2->u.transfer);

        case CRYPTO_WALLET_EVENT_TRANSFER_SUBMITTED:
            return cryptoTransferEqual(event1->u.transfer, event2->u.transfer);

        case CRYPTO_WALLET_EVENT_BALANCE_UPDATED:
            return CRYPTO_COMPARE_EQ == cryptoAmountCompare (event1->u.balanceUpdated.amount, event2->u.balanceUpdated.amount);

        case CRYPTO_WALLET_EVENT_FEE_BASIS_UPDATED:
            return cryptoFeeBasisIsEqual (event1->u.feeBasisUpdated.basis, event2->u.feeBasisUpdated.basis);

        case CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED:
            return AS_CRYPTO_BOOLEAN (event1->u.feeBasisEstimated.status == event2->u.feeBasisEstimated.status &&
                                      event1->u.feeBasisEstimated.cookie == event2->u.feeBasisEstimated.cookie &&
                                      CRYPTO_TRUE == cryptoFeeBasisIsEqual (event1->u.feeBasisEstimated.basis,
                                                                            event2->u.feeBasisEstimated.basis));
    }
}

// MARK: - Wallet

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoWallet, cryptoWallet)

extern BRCryptoWallet
cryptoWalletAllocAndInit (size_t sizeInBytes,
                          BRCryptoBlockChainType type,
                          BRCryptoWalletListener listener,
                          BRCryptoUnit unit,
                          BRCryptoUnit unitForFee,
                          BRCryptoAmount balanceMinimum,
                          BRCryptoAmount balanceMaximum,
                          BRCryptoFeeBasis defaultFeeBasis,
                          BRCryptoWalletCreateContext createContext,
                          BRCryptoWalletCreateCallbak createCallback) {
    assert (sizeInBytes >= sizeof (struct BRCryptoWalletRecord));

    BRCryptoWallet wallet = calloc (1, sizeInBytes);

    wallet->sizeInBytes = sizeInBytes;
    wallet->type  = type;
    wallet->handlers = cryptoHandlersLookup(type)->wallet;

    wallet->listener   = listener;
    wallet->state      = CRYPTO_WALLET_STATE_CREATED;
    wallet->unit       = cryptoUnitTake (unit);
    wallet->unitForFee = cryptoUnitTake (unitForFee);

    BRCryptoCurrency currency = cryptoUnitGetCurrency(unit);
    assert (NULL == balanceMinimum || cryptoAmountHasCurrency (balanceMinimum, currency));
    assert (NULL == balanceMaximum || cryptoAmountHasCurrency (balanceMaximum, currency));
    cryptoCurrencyGive (currency);

    wallet->balanceMinimum = cryptoAmountTake (balanceMinimum);
    wallet->balanceMaximum = cryptoAmountTake (balanceMaximum);
    wallet->balance = cryptoAmountCreateInteger(0, unit);

    wallet->defaultFeeBasis = cryptoFeeBasisTake (defaultFeeBasis);

    array_new (wallet->transfers, 5);

    wallet->ref = CRYPTO_REF_ASSIGN (cryptoWalletRelease);

    wallet->listenerTransfer = cryptoListenerCreateTransferListener (&wallet->listener, wallet, cryptoWalletUpdTransfer);

    pthread_mutex_init_brd (&wallet->lock, PTHREAD_MUTEX_NORMAL);  // PTHREAD_MUTEX_RECURSIVE

    if (NULL != createCallback) createCallback (createContext, wallet);

    cryptoWalletGenerateEvent (wallet, cryptoWalletEventCreate(CRYPTO_WALLET_EVENT_CREATED));

    return wallet;
}

static void
cryptoWalletRelease (BRCryptoWallet wallet) {
    pthread_mutex_lock (&wallet->lock);
    cryptoWalletSetState (wallet, CRYPTO_WALLET_STATE_DELETED);

    cryptoUnitGive (wallet->unit);
    cryptoUnitGive (wallet->unitForFee);

    cryptoAmountGive (wallet->balanceMinimum);
    cryptoAmountGive (wallet->balanceMaximum);
    cryptoAmountGive (wallet->balance);

    cryptoFeeBasisGive (wallet->defaultFeeBasis);

    for (size_t index = 0; index < array_count(wallet->transfers); index++)
        cryptoTransferGive (wallet->transfers[index]);
    array_free (wallet->transfers);

    wallet->handlers->release (wallet);

    pthread_mutex_unlock  (&wallet->lock);
    pthread_mutex_destroy (&wallet->lock);

    cryptoWalletGenerateEvent (wallet, cryptoWalletEventCreate(CRYPTO_WALLET_EVENT_DELETED));

    memset (wallet, 0, sizeof(*wallet));
    free (wallet);
}

private_extern BRCryptoBlockChainType
cryptoWalletGetType (BRCryptoWallet wallet) {
    return wallet->type;
}

extern BRCryptoCurrency
cryptoWalletGetCurrency (BRCryptoWallet wallet) {
    return cryptoUnitGetCurrency(wallet->unit);
}

extern BRCryptoUnit
cryptoWalletGetUnit (BRCryptoWallet wallet) {
    return cryptoUnitTake (wallet->unit);
}

extern BRCryptoBoolean
cryptoWalletHasCurrency (BRCryptoWallet wallet,
                         BRCryptoCurrency currency) {
    return cryptoUnitHasCurrency (wallet->unit, currency);
}

extern BRCryptoWalletState
cryptoWalletGetState (BRCryptoWallet wallet) {
    pthread_mutex_lock (&wallet->lock);
    BRCryptoWalletState state = wallet->state;
    pthread_mutex_unlock (&wallet->lock);

    return state;
}

private_extern void
cryptoWalletSetState (BRCryptoWallet wallet,
                      BRCryptoWalletState state) {
    BRCryptoWalletState newState = state;

    pthread_mutex_lock (&wallet->lock);
    BRCryptoWalletState oldState = wallet->state;
    wallet->state = state;
    pthread_mutex_unlock (&wallet->lock);

    if (oldState != newState)
        cryptoWalletGenerateEvent (wallet, cryptoWalletEventCreateState (oldState, newState));
}

extern BRCryptoCurrency
cryptoWalletGetCurrencyForFee (BRCryptoWallet wallet) {
    return cryptoUnitGetCurrency (wallet->unitForFee);
}

extern BRCryptoUnit
cryptoWalletGetUnitForFee (BRCryptoWallet wallet) {
    return cryptoUnitTake (wallet->unitForFee);
}

extern BRCryptoBoolean
cryptoWalletHasCurrencyForFee (BRCryptoWallet wallet,
                               BRCryptoCurrency currency) {
    return cryptoUnitHasCurrency (wallet->unitForFee, currency);
}

extern BRCryptoAmount
cryptoWalletGetBalance (BRCryptoWallet wallet) {
    pthread_mutex_lock (&wallet->lock);
    BRCryptoAmount amount = cryptoAmountTake (wallet->balance);
    pthread_mutex_unlock (&wallet->lock);

    return amount;
}

static void // called wtih wallet->lock
cryptoWalletSetBalance (BRCryptoWallet wallet,
                        OwnershipGiven BRCryptoAmount newBalance) {
    BRCryptoAmount oldBalance = wallet->balance;

    wallet->balance = newBalance;

    if (CRYPTO_COMPARE_EQ != cryptoAmountCompare (oldBalance, newBalance)) {
        cryptoWalletGenerateEvent (wallet, cryptoWalletEventCreateBalanceUpdated (newBalance));
    }

    cryptoAmountGive(oldBalance);
}

static void // called wtih wallet->lock
cryptoWalletIncBalance (BRCryptoWallet wallet,
                        OwnershipGiven BRCryptoAmount amount) {
    cryptoWalletSetBalance (wallet, cryptoAmountAdd (wallet->balance, amount));
    cryptoAmountGive(amount);
}

static void // called wtih wallet->lock
cryptoWalletDecBalance (BRCryptoWallet wallet,
                        OwnershipGiven BRCryptoAmount amount) {
    cryptoWalletSetBalance (wallet, cryptoAmountSub (wallet->balance, amount));
    cryptoAmountGive(amount);
}

/**
 * Return the amount from `transfer` that applies to the balance of `wallet`.  The result must
 * be in the wallet's unit
 */
static OwnershipGiven BRCryptoAmount // called wtih wallet->lock
cryptoWalletGetTransferAmountDirectedNet (BRCryptoWallet wallet,
                                          BRCryptoTransfer transfer) {
    // If the wallet and transfer units are compatible, use the transfer's amount
    BRCryptoAmount transferAmount = (CRYPTO_TRUE == cryptoUnitIsCompatible(wallet->unit, transfer->unit)
                                     ? cryptoTransferGetAmountDirectedInternal (transfer, CRYPTO_TRUE)
                                     : cryptoAmountCreateInteger (0, wallet->unit));

    // If the wallet unit and the transfer unitForFee are compatible and if we did not
    // receive the transfer then use the transfer's fee
    BRCryptoAmount transferFee    = (CRYPTO_TRUE == cryptoUnitIsCompatible(wallet->unit, transfer->unitForFee) &&
                                     CRYPTO_TRANSFER_RECEIVED != cryptoTransferGetDirection(transfer)
                                     ? cryptoTransferGetFee (transfer)
                                     : NULL);

    BRCryptoAmount transferNet = (NULL != transferFee
                                  ? cryptoAmountSub  (transferAmount, transferFee)
                                  : cryptoAmountTake (transferAmount));

    cryptoAmountGive(transferFee);
    cryptoAmountGive(transferAmount);

    return transferNet;
}


/**
 * Recompute the balance by iterating over all transfers and summing the 'amount directed net'.
 * This is appropriately used when the 'amount directed net' has changed; typically when a
 * transfer's state is become 'included' and thus the fee has been finalized.  Note, however, we
 *  handle an estimated vs confirmed fee explicitly in the subsequent function.
*/
static BRCryptoAmount
cryptoWalletComputeBalance (BRCryptoWallet wallet, bool needLock) {
    if (needLock) pthread_mutex_lock (&wallet->lock);
    BRCryptoAmount balance = cryptoAmountCreateInteger (0, wallet->unit);

    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        // If the transfer has ERRORED, ignore it immediately
        if (CRYPTO_TRANSFER_STATE_ERRORED != cryptoTransferGetStateType (wallet->transfers[index])) {
            BRCryptoAmount amount     = cryptoWalletGetTransferAmountDirectedNet (wallet, wallet->transfers[index]);
            BRCryptoAmount newBalance = cryptoAmountAdd (balance, amount);

            cryptoAmountGive(amount);
            cryptoAmountGive(balance);

            balance = newBalance;
        }
    }
    if (needLock) pthread_mutex_unlock (&wallet->lock);

    return balance;
}

private_extern void
cryptoWalletUpdBalance (BRCryptoWallet wallet, bool needLock) {
    if (needLock) pthread_mutex_lock (&wallet->lock);
    cryptoWalletSetBalance (wallet, cryptoWalletComputeBalance (wallet, false));
    if (needLock) pthread_mutex_unlock (&wallet->lock);
}

//
// When a transfer is confirmed, the fee can change.  A typical example is that for ETH a transfer
// has an estimated fee based on `gasLimit` but an actual, included fee based on `gasUsed`.  The
// following code handles a fee change - and only a fee change.
//
// There are BTC cases where the amount changes when *another* transfer is confirmed.  Specifically,
// BRWallet has a BRTransaction 'in the future' where one of the inputs is an early UTXO.  The
// other inputs and the outputs of the BRTransaction aren't resolved until early transactions are
// added to the BRWallet, and then addresses are generated upto the gap_limit.  Only then can the
// later transactions inputs and outputs be resovled.  We don't handle that case in this function.
//
static void // called wtih wallet->lock
cryptoWalletUpdBalanceOnTransferConfirmation (BRCryptoWallet wallet,
                                              BRCryptoTransfer transfer) {
    // if this wallet does not pay fees, then there is nothing to do
    if (CRYPTO_FALSE == cryptoUnitIsCompatible (wallet->unit, wallet->unitForFee))
        return;

    BRCryptoAmount feeEstimated = cryptoTransferGetEstimatedFee (transfer);
    BRCryptoAmount feeConfirmed = cryptoTransferGetConfirmedFee (transfer);
    assert (NULL == feeConfirmed || NULL == feeEstimated || CRYPTO_TRUE == cryptoAmountIsCompatible (feeEstimated, feeConfirmed));
    assert (NULL == feeConfirmed || CRYPTO_TRUE == cryptoAmountIsCompatible (feeConfirmed, wallet->balance));
    // TODO: assert (NULL != feeConfirmed)

    BRCryptoAmount change = NULL;

    if (NULL != feeConfirmed && NULL != feeEstimated)
        change = cryptoAmountSub (feeConfirmed, feeEstimated);
    else if (NULL != feeConfirmed)
        change = cryptoAmountTake (feeConfirmed);
    else if (NULL != feeEstimated)
        change = cryptoAmountNegate (feeEstimated);

    if (NULL != change && CRYPTO_FALSE == cryptoAmountIsZero(change))
        cryptoWalletIncBalance (wallet, cryptoAmountTake(change));

    cryptoAmountGive (change);
    cryptoAmountGive (feeEstimated);
    cryptoAmountGive (feeConfirmed);
}

extern BRCryptoAmount /* nullable */
cryptoWalletGetBalanceMinimum (BRCryptoWallet wallet) {
    return cryptoAmountTake (wallet->balanceMinimum);
}

extern BRCryptoAmount /* nullable */
cryptoWalletGetBalanceMaximum (BRCryptoWallet wallet) {
    return cryptoAmountTake (wallet->balanceMaximum);
}

static BRCryptoBoolean
cryptoWalletHasTransferLock (BRCryptoWallet wallet,
                             BRCryptoTransfer transfer,
                             bool needLock) {
    BRCryptoBoolean r = CRYPTO_FALSE;
    if (needLock) pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; index < array_count(wallet->transfers) && CRYPTO_FALSE == r; index++) {
        r = cryptoTransferEqual (transfer, wallet->transfers[index]);
    }
    if (needLock) pthread_mutex_unlock (&wallet->lock);
    return r;
}

extern BRCryptoBoolean
cryptoWalletHasTransfer (BRCryptoWallet wallet,
                         BRCryptoTransfer transfer) {
    return cryptoWalletHasTransferLock (wallet, transfer, true);
}

static void
cryptoWalletAnnounceTransfer (BRCryptoWallet wallet,
                              BRCryptoTransfer transfer,
                              BRCryptoWalletEventType type) {  // TRANSFER_{ADDED,UPDATED,DELETED}
    assert (CRYPTO_WALLET_EVENT_TRANSFER_ADDED   == type ||
            CRYPTO_WALLET_EVENT_TRANSFER_CHANGED == type ||
            CRYPTO_WALLET_EVENT_TRANSFER_DELETED == type);
    if (NULL != wallet->handlers->announceTransfer)
        wallet->handlers->announceTransfer (wallet, transfer, type);
}

extern void
cryptoWalletAddTransfer (BRCryptoWallet wallet,
                         BRCryptoTransfer transfer) {
    pthread_mutex_lock (&wallet->lock);
    if (CRYPTO_FALSE == cryptoWalletHasTransferLock (wallet, transfer, false)) {
        array_add (wallet->transfers, cryptoTransferTake(transfer));
        cryptoWalletAnnounceTransfer (wallet, transfer, CRYPTO_WALLET_EVENT_TRANSFER_ADDED);
        cryptoWalletGenerateEvent (wallet, cryptoWalletEventCreateTransfer (CRYPTO_WALLET_EVENT_TRANSFER_ADDED, transfer));
        cryptoWalletIncBalance (wallet, cryptoWalletGetTransferAmountDirectedNet(wallet, transfer));
     }
    pthread_mutex_unlock (&wallet->lock);
}

extern void
cryptoWalletAddTransfers (BRCryptoWallet wallet,
                          OwnershipGiven BRArrayOf(BRCryptoTransfer) transfers) {
    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; index < array_count(transfers); index++) {
        BRCryptoTransfer transfer = transfers[index];
        if (CRYPTO_FALSE == cryptoWalletHasTransferLock (wallet, transfer, false)) {
            array_add (wallet->transfers, cryptoTransferTake(transfer));
            cryptoWalletAnnounceTransfer (wallet, transfer, CRYPTO_WALLET_EVENT_TRANSFER_ADDED);
            // Must announce

            // TODO: replace w/ bulk announcement
            cryptoWalletGenerateEvent (wallet, cryptoWalletEventCreateTransfer (CRYPTO_WALLET_EVENT_TRANSFER_ADDED, transfer));

//            cryptoWalletIncBalance (wallet, cryptoTransferGetAmountDirectedNet(transfer));
        }
    }

    // generate event

    // new balance
    cryptoWalletUpdBalance(wallet, false);

    array_free_all (transfers, cryptoTransferGive);
    pthread_mutex_unlock (&wallet->lock);
}

extern void
cryptoWalletRemTransfer (BRCryptoWallet wallet, BRCryptoTransfer transfer) {
    BRCryptoTransfer walletTransfer = NULL;
    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        if (CRYPTO_TRUE == cryptoTransferEqual (wallet->transfers[index], transfer)) {
            walletTransfer = wallet->transfers[index];
            array_rm (wallet->transfers, index);
            cryptoWalletAnnounceTransfer (wallet, transfer, CRYPTO_WALLET_EVENT_TRANSFER_DELETED);
            cryptoWalletGenerateEvent (wallet, cryptoWalletEventCreateTransfer (CRYPTO_WALLET_EVENT_TRANSFER_DELETED, transfer));
            cryptoWalletDecBalance (wallet, cryptoWalletGetTransferAmountDirectedNet(wallet, transfer));
            break;
        }
    }
    pthread_mutex_unlock (&wallet->lock);

    // drop reference outside of lock to avoid potential case where release function runs
    if (NULL != walletTransfer) cryptoTransferGive (walletTransfer);
}

private_extern void
cryptoWalletReplaceTransfer (BRCryptoWallet wallet,
                             OwnershipKept  BRCryptoTransfer oldTransfer,
                             OwnershipGiven BRCryptoTransfer newTransfer) {
    BRCryptoTransfer walletTransfer = NULL;
    
    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        if (oldTransfer == wallet->transfers[index] ||
            CRYPTO_TRUE == cryptoTransferEqual (wallet->transfers[index], oldTransfer)) {
            walletTransfer = wallet->transfers[index];
            wallet->transfers[index] = cryptoTransferTake (newTransfer);

            cryptoWalletAnnounceTransfer (wallet, oldTransfer, CRYPTO_WALLET_EVENT_TRANSFER_DELETED);
            cryptoWalletGenerateEvent (wallet, cryptoWalletEventCreateTransfer (CRYPTO_WALLET_EVENT_TRANSFER_DELETED, oldTransfer));
            cryptoWalletDecBalance (wallet, cryptoWalletGetTransferAmountDirectedNet(wallet, oldTransfer));

            cryptoWalletAnnounceTransfer (wallet, newTransfer, CRYPTO_WALLET_EVENT_TRANSFER_ADDED);
            cryptoWalletGenerateEvent (wallet, cryptoWalletEventCreateTransfer (CRYPTO_WALLET_EVENT_TRANSFER_ADDED, newTransfer));
            cryptoWalletIncBalance (wallet, cryptoWalletGetTransferAmountDirectedNet(wallet, newTransfer));

            break;
        }
    }
    pthread_mutex_unlock (&wallet->lock);

    // drop reference outside of lock to avoid potential case where release function runs
    cryptoTransferGive (walletTransfer);
    cryptoTransferGive (newTransfer);
}

// This is called as the 'transferListener' by BRCryptoTransfer from `cryptoTransferSetState`.  It
// is a way for a wallet to listen in on transfer changes.
static void
cryptoWalletUpdTransfer (BRCryptoWallet wallet,
                         BRCryptoTransfer transfer,
                         BRCryptoTransferState newState) {
    // The transfer's state has changed.  This implies a possible amount/fee change as well as
    // perhaps other wallet changes, such a nonce change.
    pthread_mutex_lock (&wallet->lock);
    if (CRYPTO_TRUE == cryptoWalletHasTransferLock (wallet, transfer, false)) {
        switch (newState->type) {
            case CRYPTO_TRANSFER_STATE_CREATED:
            case CRYPTO_TRANSFER_STATE_SIGNED:
            case CRYPTO_TRANSFER_STATE_SUBMITTED:
            case CRYPTO_TRANSFER_STATE_DELETED:
                break; // nothing
            case CRYPTO_TRANSFER_STATE_INCLUDED:
                cryptoWalletUpdBalanceOnTransferConfirmation (wallet, transfer);
                break;
            case CRYPTO_TRANSFER_STATE_ERRORED:
                // Recompute the balance
                cryptoWalletUpdBalance (wallet, false);
                break;
        }

        // Announce a 'TRANSFER_CHANGED'; each currency might respond differently.
        cryptoWalletAnnounceTransfer (wallet, transfer, CRYPTO_WALLET_EVENT_TRANSFER_CHANGED);
    }
    pthread_mutex_unlock (&wallet->lock);
}

extern BRCryptoTransfer *
cryptoWalletGetTransfers (BRCryptoWallet wallet, size_t *count) {
    pthread_mutex_lock (&wallet->lock);
    *count = array_count (wallet->transfers);
    BRCryptoTransfer *transfers = NULL;
    if (0 != *count) {
        transfers = calloc (*count, sizeof(BRCryptoTransfer));
        for (size_t index = 0; index < *count; index++) {
            transfers[index] = cryptoTransferTake(wallet->transfers[index]);
        }
    }
    pthread_mutex_unlock (&wallet->lock);
    return transfers;
}

private_extern BRCryptoTransfer
cryptoWalletGetTransferByHash (BRCryptoWallet wallet, BRCryptoHash hashToMatch) {
    BRCryptoTransfer transfer = NULL;

    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; NULL == transfer && index < array_count(wallet->transfers); index++) {
        BRCryptoHash hash = cryptoTransferGetHash (wallet->transfers[index]);
        if (CRYPTO_TRUE == cryptoHashEqual(hash, hashToMatch))
            transfer = wallet->transfers[index];
        cryptoHashGive(hash);
    }
    pthread_mutex_unlock (&wallet->lock);

    return cryptoTransferTake (transfer);
}

private_extern BRCryptoTransfer
cryptoWalletGetTransferByUIDS (BRCryptoWallet wallet, const char *uids) {
    BRCryptoTransfer transfer = NULL;

    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; NULL == transfer && index < array_count(wallet->transfers); index++) {
        const char *transferUIDS = wallet->transfers[index]->uids;
        if (NULL != transferUIDS && 0 == strcmp (transferUIDS, uids))
            transfer = wallet->transfers[index];
    }
    pthread_mutex_unlock (&wallet->lock);

    return cryptoTransferTake (transfer);
}

private_extern BRCryptoTransfer
cryptoWalletGetTransferByHashOrUIDS (BRCryptoWallet wallet, BRCryptoHash hash, const char *uids) {
    BRCryptoTransfer transfer = NULL;

    // This is quite a special match...

    // 1) If we've nothing to match, then we are done.
    if (NULL == hash && NULL == uids) return NULL;

    // 2) If we find a uids match, we are done.
    transfer = (NULL == uids
                ? NULL
                : cryptoWalletGetTransferByUIDS (wallet, uids));
    if (NULL != transfer) return transfer;

    // 3) If we don't find a hash match, we are done
    transfer = (NULL == hash
                ? NULL
                : cryptoWalletGetTransferByHash (wallet, hash));
    if (NULL == transfer) return NULL;

    // 4) If we wanted a UIDS but then transfer doesn't have one, then we are done.  This is a
    // transfer that we created... and it is waiting for a uids
    if (NULL != uids && NULL == transfer->uids) return transfer;

    // 5) We are done.
    return NULL;

    // WHY?
}

extern BRCryptoAddress
cryptoWalletGetAddress (BRCryptoWallet wallet,
                        BRCryptoAddressScheme addressScheme) {
    return wallet->handlers->getAddress (wallet, addressScheme);
}

extern bool
cryptoWalletHasAddress (BRCryptoWallet wallet,
                        BRCryptoAddress address) {

    return (wallet->type != cryptoAddressGetType(address)
            ? false
            : wallet->handlers->hasAdress (wallet, address));
}

private_extern OwnershipGiven BRSetOf(BRCyptoAddress)
cryptoWalletGetAddressesForRecovery (BRCryptoWallet wallet) {
    return wallet->handlers->getAddressesForRecovery (wallet);
}

extern BRCryptoFeeBasis
cryptoWalletGetDefaultFeeBasis (BRCryptoWallet wallet) {
    pthread_mutex_lock (&wallet->lock);
    BRCryptoFeeBasis feeBasis = cryptoFeeBasisTake (wallet->defaultFeeBasis);
    pthread_mutex_unlock (&wallet->lock);

    return feeBasis;
}

extern void
cryptoWalletSetDefaultFeeBasis (BRCryptoWallet wallet,
                                BRCryptoFeeBasis feeBasis) {
    pthread_mutex_lock (&wallet->lock);
    if (NULL != wallet->defaultFeeBasis) cryptoFeeBasisGive (wallet->defaultFeeBasis);
    wallet->defaultFeeBasis = cryptoFeeBasisTake(feeBasis);
    BRCryptoFeeBasis newFeeBasis = wallet->defaultFeeBasis;
    pthread_mutex_unlock (&wallet->lock);

    cryptoWalletGenerateEvent (wallet, cryptoWalletEventCreateFeeBasisUpdated (newFeeBasis));
}

extern size_t
cryptoWalletGetTransferAttributeCount (BRCryptoWallet wallet,
                                       BRCryptoAddress target) {
    return wallet->handlers->getTransferAttributeCount (wallet, target);
}

extern BRCryptoTransferAttribute
cryptoWalletGetTransferAttributeAt (BRCryptoWallet wallet,
                                    BRCryptoAddress target,
                                    size_t index) {
    return wallet->handlers->getTransferAttributeAt (wallet, target, index);
}

extern BRCryptoTransferAttributeValidationError
cryptoWalletValidateTransferAttribute (BRCryptoWallet wallet,
                                       OwnershipKept BRCryptoTransferAttribute attribute,
                                       BRCryptoBoolean *validates) {
    return wallet->handlers->validateTransferAttribute (wallet, attribute, validates);
}

extern BRCryptoTransferAttributeValidationError
cryptoWalletValidateTransferAttributes (BRCryptoWallet wallet,
                                        size_t attributesCount,
                                        OwnershipKept BRCryptoTransferAttribute *attributes,
                                        BRCryptoBoolean *validates) {
    *validates = CRYPTO_TRUE;

    for (size_t index = 0; index <attributesCount; index++) {
        BRCryptoTransferAttributeValidationError error = cryptoWalletValidateTransferAttribute (wallet, attributes[index], validates);
        if (CRYPTO_FALSE == *validates)
            return error;
    }

    return (BRCryptoTransferAttributeValidationError) 0;
}

extern BRCryptoTransferAttribute
cryptoWalletGetTransferAttributeForKey (BRCryptoWallet wallet,
                                        BRCryptoAddress target,
                                        const char *key) {

    size_t count = cryptoWalletGetTransferAttributeCount (wallet, target);
    for (size_t index = 0; index < count; index++) {
        BRCryptoTransferAttribute attribute = cryptoWalletGetTransferAttributeAt (wallet, target, index);
        if (0 == strcasecmp (key, cryptoTransferAttributeGetKey (attribute)))
            return attribute;
        cryptoTransferAttributeGive (attribute);
    }
    
    return NULL;
}

extern BRCryptoTransfer
cryptoWalletCreateTransferMultiple (BRCryptoWallet wallet,
                                    size_t outputsCount,
                                    BRCryptoTransferOutput *outputs,
                                    BRCryptoFeeBasis estimatedFeeBasis) {
    //    assert (cryptoWalletGetType(wallet) == cryptoFeeBasisGetType(estimatedFeeBasis));
    if (0 == outputsCount) return NULL;

    BRCryptoUnit unit         = cryptoWalletGetUnit (wallet);
    BRCryptoUnit unitForFee   = cryptoWalletGetUnitForFee(wallet);
    BRCryptoCurrency currency = cryptoUnitGetCurrency(unit);

    BRCryptoTransfer transfer = wallet->handlers->createTransferMultiple (wallet,
                                                                          outputsCount,
                                                                          outputs,
                                                                          estimatedFeeBasis,
                                                                          currency,
                                                                          unit,
                                                                          unitForFee);

    cryptoCurrencyGive(currency);
    cryptoUnitGive (unitForFee);
    cryptoUnitGive (unit);

    return transfer;
}

extern BRCryptoTransfer
cryptoWalletCreateTransfer (BRCryptoWallet  wallet,
                            BRCryptoAddress target,
                            BRCryptoAmount  amount,
                            BRCryptoFeeBasis estimatedFeeBasis,
                            size_t attributesCount,
                            OwnershipKept BRCryptoTransferAttribute *attributes) {
    assert (cryptoWalletGetType(wallet) == cryptoAddressGetType(target));
    //    assert (cryptoWalletGetType(wallet) == cryptoFeeBasisGetType(estimatedFeeBasis));


    BRCryptoUnit unit       = cryptoWalletGetUnit (wallet);
    BRCryptoUnit unitForFee = cryptoWalletGetUnitForFee(wallet);

    BRCryptoCurrency currency = cryptoUnitGetCurrency(unit);
    assert (cryptoAmountHasCurrency (amount, currency));

    BRCryptoTransfer transfer = wallet->handlers->createTransfer (wallet,
                                                                  target,
                                                                  amount,
                                                                  estimatedFeeBasis,
                                                                  attributesCount,
                                                                  attributes,
                                                                  currency,
                                                                  unit,
                                                                  unitForFee);

    if (NULL != transfer && attributesCount > 0)
        cryptoTransferSetAttributes (transfer, attributesCount, attributes);

    cryptoCurrencyGive(currency);
    cryptoUnitGive (unitForFee);
    cryptoUnitGive (unit);

    return transfer;
}

extern BRCryptoTransfer
cryptoWalletCreateTransferForPaymentProtocolRequest (BRCryptoWallet wallet,
                                                     BRCryptoPaymentProtocolRequest request,
                                                     BRCryptoFeeBasis estimatedFeeBasis) {
    const BRCryptoPaymentProtocolHandlers * paymentHandlers = cryptoHandlersLookup(cryptoWalletGetType(wallet))->payment;
    
    assert (NULL != paymentHandlers);
    
    return paymentHandlers->createTransfer (request,
                                            wallet,
                                            estimatedFeeBasis);
}

extern BRCryptoBoolean
cryptoWalletEqual (BRCryptoWallet w1, BRCryptoWallet w2) {
    return AS_CRYPTO_BOOLEAN (w1 == w2 ||
                              (w1->type == w2->type &&
                               w1->handlers->isEqual (w1, w2)));
}

extern const char *
cryptoWalletEventTypeString (BRCryptoWalletEventType t) {
    switch (t) {
        case CRYPTO_WALLET_EVENT_CREATED:
        return "CRYPTO_WALLET_EVENT_CREATED";

        case CRYPTO_WALLET_EVENT_CHANGED:
        return "CRYPTO_WALLET_EVENT_CHANGED";

        case CRYPTO_WALLET_EVENT_DELETED:
        return "CRYPTO_WALLET_EVENT_DELETED";

        case CRYPTO_WALLET_EVENT_TRANSFER_ADDED:
        return "CRYPTO_WALLET_EVENT_TRANSFER_ADDED";

        case CRYPTO_WALLET_EVENT_TRANSFER_CHANGED:
        return "CRYPTO_WALLET_EVENT_TRANSFER_CHANGED";

        case CRYPTO_WALLET_EVENT_TRANSFER_SUBMITTED:
        return "CRYPTO_WALLET_EVENT_TRANSFER_SUBMITTED";

        case CRYPTO_WALLET_EVENT_TRANSFER_DELETED:
        return "CRYPTO_WALLET_EVENT_TRANSFER_DELETED";

        case CRYPTO_WALLET_EVENT_BALANCE_UPDATED:
        return "CRYPTO_WALLET_EVENT_BALANCE_UPDATED";

        case CRYPTO_WALLET_EVENT_FEE_BASIS_UPDATED:
        return "CRYPTO_WALLET_EVENT_FEE_BASIS_UPDATED";

        case CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED:
        return "CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED";
    }
    return "<CRYPTO_WALLET_EVENT_TYPE_UNKNOWN>";
}

