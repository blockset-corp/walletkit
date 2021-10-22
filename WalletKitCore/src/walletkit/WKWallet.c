//
//  WKWallet.c
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <strings.h>

#include "WKWalletP.h"

#include "WKAmountP.h"
#include "WKFeeBasisP.h"
#include "WKKeyP.h"
#include "WKTransferP.h"
#include "WKAddressP.h"
#include "WKNetworkP.h"
#include "WKPaymentP.h"

#include "WKHandlersP.h"

static void
wkWalletUpdTransfer (WKWallet wallet,
                     WKTransfer transfer,
                     OwnershipKept WKTransferState oldState);

static void
wkWalletUpdBalanceOnTransferConfirmation (WKWallet wallet,
                                              WKTransfer transfer);

// MARK: - Wallet Event

struct WKWalletEventRecord {
    WKWalletEventType type;
    union {
        struct {
            WKWalletState old;
            WKWalletState new;
        } state;

        WKTransfer transfer;

        struct {
            /// Handler must 'give'
            WKAmount amount;
        } balanceUpdated;

        struct {
            /// Handler must 'give'
            WKFeeBasis basis;
        } feeBasisUpdated;

        struct {
            /// Handler must 'give' basis
            WKStatus status;
            WKCookie cookie;
            WKFeeBasis basis;
        } feeBasisEstimated;
    } u;

    WKRef ref;
};

IMPLEMENT_WK_GIVE_TAKE (WKWalletEvent, wkWalletEvent)

private_extern WKWalletEvent
wkWalletEventCreate (WKWalletEventType type) {
    WKWalletEvent event = calloc (1, sizeof (struct WKWalletEventRecord));

    event->type = type;
    event->ref  = WK_REF_ASSIGN (wkWalletEventRelease);

    return event;
}

static void
wkWalletEventRelease (WKWalletEvent event) {
    switch (event->type) {
        case WK_WALLET_EVENT_CREATED:
        case WK_WALLET_EVENT_DELETED:
            break;

        case WK_WALLET_EVENT_CHANGED:
            // WKWalletState old, new
            break;

        case WK_WALLET_EVENT_TRANSFER_ADDED:
        case WK_WALLET_EVENT_TRANSFER_CHANGED:
        case WK_WALLET_EVENT_TRANSFER_DELETED:
            wkTransferGive (event->u.transfer);
            break;

        case WK_WALLET_EVENT_TRANSFER_SUBMITTED:
            wkTransferGive (event->u.transfer);
            break;

        case WK_WALLET_EVENT_BALANCE_UPDATED:
            wkAmountGive (event->u.balanceUpdated.amount);
            break;

        case WK_WALLET_EVENT_FEE_BASIS_UPDATED:
            wkFeeBasisGive (event->u.feeBasisUpdated.basis);
            break;

        case WK_WALLET_EVENT_FEE_BASIS_ESTIMATED:
            // WKStatus status
            // WKCookie cookie
            wkFeeBasisGive (event->u.feeBasisEstimated.basis);
            break;
    }

    memset (event, 0, sizeof(*event));
    free (event);
}

extern WKWalletEventType
wkWalletEventGetType (WKWalletEvent event) {
    return event->type;
}

private_extern WKWalletEvent
wkWalletEventCreateState (WKWalletState old,
                              WKWalletState new) {
    WKWalletEvent event = wkWalletEventCreate (WK_WALLET_EVENT_CHANGED);

    
    event->u.state.old = old;
    event->u.state.new = new;

    return event;
}

extern WKBoolean
wkWalletEventExtractState (WKWalletEvent event,
                               WKWalletState *old,
                               WKWalletState *new) {
    if (WK_WALLET_EVENT_CHANGED != event->type) return WK_FALSE;

    if (NULL != old) *old = event->u.state.old;
    if (NULL != new) *new = event->u.state.new;

    return WK_TRUE;
}

static bool
wkWalletEventTypeIsTransfer (WKWalletEventType type) {
    return (WK_WALLET_EVENT_TRANSFER_ADDED   == type ||
            WK_WALLET_EVENT_TRANSFER_CHANGED == type ||
            WK_WALLET_EVENT_TRANSFER_DELETED == type);
}

private_extern WKWalletEvent
wkWalletEventCreateTransfer (WKWalletEventType type,
                                 WKTransfer transfer) {
    assert (wkWalletEventTypeIsTransfer(type));
    WKWalletEvent event = wkWalletEventCreate (type);

    event->u.transfer = wkTransferTake (transfer);

    return event;
}

extern WKBoolean
wkWalletEventExtractTransfer (WKWalletEvent event,
                                  WKTransfer *transfer) {

    if (!wkWalletEventTypeIsTransfer (event->type)) return WK_FALSE;

    if (NULL != transfer) *transfer = wkTransferTake (event->u.transfer);

    return WK_TRUE;
}

private_extern WKWalletEvent
wkWalletEventCreateTransferSubmitted (WKTransfer transfer) {
    WKWalletEvent event = wkWalletEventCreate (WK_WALLET_EVENT_TRANSFER_SUBMITTED);

    event->u.transfer = wkTransferTake (transfer);

    return event;
}

extern WKBoolean
wkWalletEventExtractTransferSubmit (WKWalletEvent event,
                                        WKTransfer *transfer) {

    if (WK_WALLET_EVENT_TRANSFER_SUBMITTED != event->type) return WK_FALSE;

    if (NULL != transfer) *transfer = wkTransferTake (event->u.transfer);

    return WK_TRUE;
}

private_extern WKWalletEvent
wkWalletEventCreateBalanceUpdated (OwnershipGiven WKAmount balance) {
    WKWalletEvent event = wkWalletEventCreate (WK_WALLET_EVENT_BALANCE_UPDATED);

    event->u.balanceUpdated.amount = wkAmountTake (balance);

    return event;
}

extern WKBoolean
wkWalletEventExtractBalanceUpdate (WKWalletEvent event,
                                       WKAmount *balance) {
    if (WK_WALLET_EVENT_BALANCE_UPDATED != event->type) return WK_FALSE;

    if (NULL != balance) *balance = wkAmountTake (event->u.balanceUpdated.amount);

    return WK_TRUE;
}

private_extern WKWalletEvent
wkWalletEventCreateFeeBasisUpdated (WKFeeBasis basis) {
    WKWalletEvent event = wkWalletEventCreate (WK_WALLET_EVENT_FEE_BASIS_UPDATED);

    event->u.feeBasisUpdated.basis = wkFeeBasisTake (basis);

    return event;
}

extern WKBoolean
wkWalletEventExtractFeeBasisUpdate (WKWalletEvent event,
                                        WKFeeBasis *basis) {
    if (WK_WALLET_EVENT_FEE_BASIS_UPDATED != event->type) return WK_FALSE;

    if (NULL != basis) *basis = wkFeeBasisTake (event->u.feeBasisUpdated.basis);

    return WK_TRUE;
}

private_extern WKWalletEvent
wkWalletEventCreateFeeBasisEstimated (WKStatus status,
                                          WKCookie cookie,
                                          OwnershipKept WKFeeBasis basis) {
    WKWalletEvent event = wkWalletEventCreate (WK_WALLET_EVENT_FEE_BASIS_ESTIMATED);

    event->u.feeBasisEstimated.status = status;
    event->u.feeBasisEstimated.cookie = cookie;
    event->u.feeBasisEstimated.basis  = wkFeeBasisTake (basis);

    return event;
}

extern WKBoolean
wkWalletEventExtractFeeBasisEstimate (WKWalletEvent event,
                                          WKStatus *status,
                                          WKCookie *cookie,
                                          WKFeeBasis *basis) {
    if (WK_WALLET_EVENT_FEE_BASIS_ESTIMATED != event->type) return WK_FALSE;

    if (NULL != status) *status = event->u.feeBasisEstimated.status;
    if (NULL != cookie) *cookie = event->u.feeBasisEstimated.cookie;
    if (NULL != basis ) *basis  = wkFeeBasisTake (event->u.feeBasisEstimated.basis);

    return WK_TRUE;
}

extern WKBoolean
wkWalletEventIsEqual (WKWalletEvent event1,
                          WKWalletEvent event2) {
    if (event1->type != event2->type) return WK_FALSE;

    switch (event1->type) {
        case WK_WALLET_EVENT_CREATED:
        case WK_WALLET_EVENT_DELETED:
            return WK_TRUE;

        case WK_WALLET_EVENT_CHANGED:
            return AS_WK_BOOLEAN (event1->u.state.old == event2->u.state.new &&
                                      event1->u.state.new == event2->u.state.new);

        case WK_WALLET_EVENT_TRANSFER_ADDED:
        case WK_WALLET_EVENT_TRANSFER_CHANGED:
        case WK_WALLET_EVENT_TRANSFER_DELETED:
            return wkTransferEqual(event1->u.transfer, event2->u.transfer);

        case WK_WALLET_EVENT_TRANSFER_SUBMITTED:
            return wkTransferEqual(event1->u.transfer, event2->u.transfer);

        case WK_WALLET_EVENT_BALANCE_UPDATED:
            return WK_COMPARE_EQ == wkAmountCompare (event1->u.balanceUpdated.amount, event2->u.balanceUpdated.amount);

        case WK_WALLET_EVENT_FEE_BASIS_UPDATED:
            return wkFeeBasisIsEqual (event1->u.feeBasisUpdated.basis, event2->u.feeBasisUpdated.basis);

        case WK_WALLET_EVENT_FEE_BASIS_ESTIMATED:
            return AS_WK_BOOLEAN (event1->u.feeBasisEstimated.status == event2->u.feeBasisEstimated.status &&
                                      event1->u.feeBasisEstimated.cookie == event2->u.feeBasisEstimated.cookie &&
                                      WK_TRUE == wkFeeBasisIsEqual (event1->u.feeBasisEstimated.basis,
                                                                            event2->u.feeBasisEstimated.basis));
    }
}

// MARK: - Wallet

IMPLEMENT_WK_GIVE_TAKE (WKWallet, wkWallet)

extern WKWallet
wkWalletAllocAndInit (size_t sizeInBytes,
                          WKNetworkType type,
                          WKWalletListener listener,
                          WKUnit unit,
                          WKUnit unitForFee,
                          WKAmount balanceMinimum,
                          WKAmount balanceMaximum,
                          WKFeeBasis defaultFeeBasis,
                          WKWalletCreateContext createContext,
                          WKWalletCreateCallbak createCallback) {
    assert (sizeInBytes >= sizeof (struct WKWalletRecord));

    WKWallet wallet = calloc (1, sizeInBytes);

    wallet->sizeInBytes = sizeInBytes;
    wallet->type  = type;
    wallet->handlers = wkHandlersLookup(type)->wallet;

    wallet->listener   = listener;
    wallet->state      = WK_WALLET_STATE_CREATED;
    wallet->unit       = wkUnitTake (unit);
    wallet->unitForFee = wkUnitTake (unitForFee);

    WKCurrency currency = wkUnitGetCurrency(unit);
    assert (NULL == balanceMinimum || wkAmountHasCurrency (balanceMinimum, currency));
    assert (NULL == balanceMaximum || wkAmountHasCurrency (balanceMaximum, currency));
    wkCurrencyGive (currency);

    wallet->balanceMinimum = wkAmountTake (balanceMinimum);
    wallet->balanceMaximum = wkAmountTake (balanceMaximum);
    wallet->balance = wkAmountCreateInteger(0, unit);

    wallet->defaultFeeBasis = wkFeeBasisTake (defaultFeeBasis);

    array_new (wallet->transfers, 5);

    wallet->ref = WK_REF_ASSIGN (wkWalletRelease);

    wallet->listenerTransfer = wkListenerCreateTransferListener (&wallet->listener, wallet, wkWalletUpdTransfer);

    pthread_mutex_init_brd (&wallet->lock, PTHREAD_MUTEX_NORMAL);  // PTHREAD_MUTEX_RECURSIVE

    if (NULL != createCallback) createCallback (createContext, wallet);

    wkWalletGenerateEvent (wallet, wkWalletEventCreate(WK_WALLET_EVENT_CREATED));

    return wallet;
}

static void
wkWalletRelease (WKWallet wallet) {
    pthread_mutex_lock (&wallet->lock);
    wkWalletSetState (wallet, WK_WALLET_STATE_DELETED);

    wkUnitGive (wallet->unit);
    wkUnitGive (wallet->unitForFee);

    wkAmountGive (wallet->balanceMinimum);
    wkAmountGive (wallet->balanceMaximum);
    wkAmountGive (wallet->balance);

    wkFeeBasisGive (wallet->defaultFeeBasis);

    for (size_t index = 0; index < array_count(wallet->transfers); index++)
        wkTransferGive (wallet->transfers[index]);
    array_free (wallet->transfers);

    wallet->handlers->release (wallet);

    pthread_mutex_unlock  (&wallet->lock);
    pthread_mutex_destroy (&wallet->lock);

    wkWalletGenerateEvent (wallet, wkWalletEventCreate(WK_WALLET_EVENT_DELETED));

    memset (wallet, 0, sizeof(*wallet));
    free (wallet);
}

private_extern WKNetworkType
wkWalletGetType (WKWallet wallet) {
    return wallet->type;
}

extern WKCurrency
wkWalletGetCurrency (WKWallet wallet) {
    return wkUnitGetCurrency(wallet->unit);
}

extern WKUnit
wkWalletGetUnit (WKWallet wallet) {
    return wkUnitTake (wallet->unit);
}

extern WKBoolean
wkWalletHasCurrency (WKWallet wallet,
                         WKCurrency currency) {
    return wkUnitHasCurrency (wallet->unit, currency);
}

extern WKWalletState
wkWalletGetState (WKWallet wallet) {
    pthread_mutex_lock (&wallet->lock);
    WKWalletState state = wallet->state;
    pthread_mutex_unlock (&wallet->lock);

    return state;
}

private_extern void
wkWalletSetState (WKWallet wallet,
                      WKWalletState state) {
    WKWalletState newState = state;

    pthread_mutex_lock (&wallet->lock);
    WKWalletState oldState = wallet->state;
    wallet->state = state;
    pthread_mutex_unlock (&wallet->lock);

    if (oldState != newState)
        wkWalletGenerateEvent (wallet, wkWalletEventCreateState (oldState, newState));
}

extern WKCurrency
wkWalletGetCurrencyForFee (WKWallet wallet) {
    return wkUnitGetCurrency (wallet->unitForFee);
}

extern WKUnit
wkWalletGetUnitForFee (WKWallet wallet) {
    return wkUnitTake (wallet->unitForFee);
}

extern WKBoolean
wkWalletHasCurrencyForFee (WKWallet wallet,
                               WKCurrency currency) {
    return wkUnitHasCurrency (wallet->unitForFee, currency);
}

extern WKAmount
wkWalletGetBalance (WKWallet wallet) {
    pthread_mutex_lock (&wallet->lock);
    WKAmount amount = wkAmountTake (wallet->balance);
    pthread_mutex_unlock (&wallet->lock);

    return amount;
}

static void // called wtih wallet->lock
wkWalletSetBalance (WKWallet wallet,
                        OwnershipGiven WKAmount newBalance) {
    WKAmount oldBalance = wallet->balance;

    wallet->balance = newBalance;

    if (WK_COMPARE_EQ != wkAmountCompare (oldBalance, newBalance)) {
        wkWalletGenerateEvent (wallet, wkWalletEventCreateBalanceUpdated (newBalance));
    }

    wkAmountGive(oldBalance);
}

static void // called wtih wallet->lock
wkWalletIncBalance (WKWallet wallet,
                        OwnershipGiven WKAmount amount) {
    wkWalletSetBalance (wallet, wkAmountAdd (wallet->balance, amount));
    wkAmountGive(amount);
}

static void // called wtih wallet->lock
wkWalletDecBalance (WKWallet wallet,
                        OwnershipGiven WKAmount amount) {
    wkWalletSetBalance (wallet, wkAmountSub (wallet->balance, amount));
    wkAmountGive(amount);
}

/**
 * Return the amount from `transfer` that applies to the balance of `wallet`.  The result must
 * be in the wallet's unit
 */
private_extern OwnershipGiven WKAmount // called wtih wallet->lock
wkWalletGetTransferAmountDirectedNet (WKWallet wallet,
                                      WKTransfer transfer) {
    // If the wallet and transfer units are compatible, use the transfer's amount
    WKAmount transferAmount = (WK_TRUE == wkUnitIsCompatible(wallet->unit, transfer->unit)
                               ? wkTransferGetAmountDirectedInternal (transfer, WK_TRUE)
                               : wkAmountCreateInteger (0, wallet->unit));

    // If the wallet unit and the transfer unitForFee are compatible and if we did not
    // receive the transfer then use the transfer's fee
    WKAmount transferFee    = (WK_TRUE == wkUnitIsCompatible(wallet->unit, transfer->unitForFee) &&
                               WK_TRANSFER_RECEIVED != wkTransferGetDirection(transfer)
                               ? wkTransferGetFee (transfer)
                               : NULL);

    WKAmount transferNet = (NULL != transferFee
                            ? wkAmountSub  (transferAmount, transferFee)
                            : wkAmountTake (transferAmount));

    wkAmountGive(transferFee);
    wkAmountGive(transferAmount);

    return transferNet;
}


/**
 * Recompute the balance by iterating over all transfers and summing the 'amount directed net'.
 * This is appropriately used when the 'amount directed net' has changed; typically when a
 * transfer's state is become 'included' and thus the fee has been finalized.  Note, however, we
 *  handle an estimated vs confirmed fee explicitly in the subsequent function.
*/
static WKAmount
wkWalletComputeBalance (WKWallet wallet, bool needLock) {
    if (needLock) pthread_mutex_lock (&wallet->lock);
    WKAmount balance = wkAmountCreateInteger (0, wallet->unit);

    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        // If the transfer has ERRORED, ignore it immediately
        if (WK_TRANSFER_STATE_ERRORED != wkTransferGetStateType (wallet->transfers[index])) {
            WKAmount amount     = wkWalletGetTransferAmountDirectedNet (wallet, wallet->transfers[index]);
            WKAmount newBalance = wkAmountAdd (balance, amount);

            wkAmountGive(amount);
            wkAmountGive(balance);

            balance = newBalance;
        }
    }
    if (needLock) pthread_mutex_unlock (&wallet->lock);

    return balance;
}

private_extern void
wkWalletUpdBalance (WKWallet wallet, bool needLock) {
    if (needLock) pthread_mutex_lock (&wallet->lock);
    wkWalletSetBalance (wallet, wkWalletComputeBalance (wallet, false));
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
wkWalletUpdBalanceOnTransferConfirmation (WKWallet wallet,
                                              WKTransfer transfer) {
    // if this wallet does not pay fees, then there is nothing to do
    if (WK_FALSE == wkUnitIsCompatible (wallet->unit, wallet->unitForFee))
        return;

    WKAmount feeEstimated = wkTransferGetEstimatedFee (transfer);
    WKAmount feeConfirmed = wkTransferGetConfirmedFee (transfer);
    assert (NULL == feeConfirmed || NULL == feeEstimated || WK_TRUE == wkAmountIsCompatible (feeEstimated, feeConfirmed));
    assert (NULL == feeConfirmed || WK_TRUE == wkAmountIsCompatible (feeConfirmed, wallet->balance));
    // TODO: assert (NULL != feeConfirmed)

    WKAmount change = NULL;

    if (NULL != feeConfirmed && NULL != feeEstimated)
        change = wkAmountSub (feeConfirmed, feeEstimated);
    else if (NULL != feeConfirmed)
        change = wkAmountTake (feeConfirmed);
    else if (NULL != feeEstimated)
        change = wkAmountNegate (feeEstimated);

    if (NULL != change && WK_FALSE == wkAmountIsZero(change))
        wkWalletIncBalance (wallet, wkAmountTake(change));

    wkAmountGive (change);
    wkAmountGive (feeEstimated);
    wkAmountGive (feeConfirmed);
}

extern WKAmount /* nullable */
wkWalletGetBalanceMinimum (WKWallet wallet) {
    return wkAmountTake (wallet->balanceMinimum);
}

extern WKAmount /* nullable */
wkWalletGetBalanceMaximum (WKWallet wallet) {
    return wkAmountTake (wallet->balanceMaximum);
}

static WKBoolean
wkWalletHasTransferLock (WKWallet wallet,
                             WKTransfer transfer,
                             bool needLock) {
    WKBoolean r = WK_FALSE;
    if (needLock) pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; index < array_count(wallet->transfers) && WK_FALSE == r; index++) {
        r = wkTransferEqual (transfer, wallet->transfers[index]);
    }
    if (needLock) pthread_mutex_unlock (&wallet->lock);
    return r;
}

extern WKBoolean
wkWalletHasTransfer (WKWallet wallet,
                         WKTransfer transfer) {
    return wkWalletHasTransferLock (wallet, transfer, true);
}

static void
wkWalletAnnounceTransfer (WKWallet wallet,
                              WKTransfer transfer,
                              WKWalletEventType type) {  // TRANSFER_{ADDED,UPDATED,DELETED}
    assert (WK_WALLET_EVENT_TRANSFER_ADDED   == type ||
            WK_WALLET_EVENT_TRANSFER_CHANGED == type ||
            WK_WALLET_EVENT_TRANSFER_DELETED == type);
    if (NULL != wallet->handlers->announceTransfer)
        wallet->handlers->announceTransfer (wallet, transfer, type);
}

private_extern void
wkWalletAddTransfer (WKWallet wallet,
                         WKTransfer transfer) {
    pthread_mutex_lock (&wallet->lock);
    if (WK_FALSE == wkWalletHasTransferLock (wallet, transfer, false)) {
        array_add (wallet->transfers, wkTransferTake(transfer));
        wkWalletAnnounceTransfer (wallet, transfer, WK_WALLET_EVENT_TRANSFER_ADDED);
        wkWalletGenerateEvent (wallet, wkWalletEventCreateTransfer (WK_WALLET_EVENT_TRANSFER_ADDED, transfer));
        wkWalletIncBalance (wallet, wkWalletGetTransferAmountDirectedNet(wallet, transfer));
     }
    pthread_mutex_unlock (&wallet->lock);
}

private_extern void
wkWalletAddTransfers (WKWallet wallet,
                          OwnershipGiven BRArrayOf(WKTransfer) transfers) {
    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; index < array_count(transfers); index++) {
        WKTransfer transfer = transfers[index];
        if (WK_FALSE == wkWalletHasTransferLock (wallet, transfer, false)) {
            array_add (wallet->transfers, wkTransferTake(transfer));
            wkWalletAnnounceTransfer (wallet, transfer, WK_WALLET_EVENT_TRANSFER_ADDED);
            // Must announce

            // TODO: replace w/ bulk announcement
            wkWalletGenerateEvent (wallet, wkWalletEventCreateTransfer (WK_WALLET_EVENT_TRANSFER_ADDED, transfer));

//            wkWalletIncBalance (wallet, wkTransferGetAmountDirectedNet(transfer));
        }
    }

    // generate event

    // new balance
    wkWalletUpdBalance(wallet, false);

    array_free_all (transfers, wkTransferGive);
    pthread_mutex_unlock (&wallet->lock);
}

private_extern void
wkWalletRemTransfer (WKWallet wallet, WKTransfer transfer) {
    WKTransfer walletTransfer = NULL;
    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        if (WK_TRUE == wkTransferEqual (wallet->transfers[index], transfer)) {
            walletTransfer = wallet->transfers[index];
            array_rm (wallet->transfers, index);
            wkWalletAnnounceTransfer (wallet, transfer, WK_WALLET_EVENT_TRANSFER_DELETED);
            wkWalletGenerateEvent (wallet, wkWalletEventCreateTransfer (WK_WALLET_EVENT_TRANSFER_DELETED, transfer));
            wkWalletDecBalance (wallet, wkWalletGetTransferAmountDirectedNet(wallet, transfer));
            break;
        }
    }
    pthread_mutex_unlock (&wallet->lock);

    // drop reference outside of lock to avoid potential case where release function runs
    if (NULL != walletTransfer) wkTransferGive (walletTransfer);
}

private_extern void
wkWalletReplaceTransfer (WKWallet wallet,
                             OwnershipKept  WKTransfer oldTransfer,
                             OwnershipGiven WKTransfer newTransfer) {
    WKTransfer walletTransfer = NULL;
    
    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        if (oldTransfer == wallet->transfers[index] ||
            WK_TRUE == wkTransferEqual (wallet->transfers[index], oldTransfer)) {

            walletTransfer = wallet->transfers[index];
            wallet->transfers[index] = wkTransferTake (newTransfer);

            wkWalletAnnounceTransfer (wallet, oldTransfer, WK_WALLET_EVENT_TRANSFER_DELETED);
            wkWalletGenerateEvent (wallet, wkWalletEventCreateTransfer (WK_WALLET_EVENT_TRANSFER_DELETED, oldTransfer));
            wkWalletDecBalance (wallet, wkWalletGetTransferAmountDirectedNet(wallet, oldTransfer));

            wkWalletAnnounceTransfer (wallet, newTransfer, WK_WALLET_EVENT_TRANSFER_ADDED);
            wkWalletGenerateEvent (wallet, wkWalletEventCreateTransfer (WK_WALLET_EVENT_TRANSFER_ADDED, newTransfer));
            wkWalletIncBalance (wallet, wkWalletGetTransferAmountDirectedNet(wallet, newTransfer));

            break;
        }
    }
    pthread_mutex_unlock (&wallet->lock);

    // drop reference outside of lock to avoid potential case where release function runs
    wkTransferGive (walletTransfer);
    wkTransferGive (newTransfer);
}

// This is called as the 'transferListener' by WKTransfer from `cryptoTransferSetState`.  It
// is a way for a wallet to listen in on transfer changes.
static void
wkWalletUpdTransfer (WKWallet wallet,
                         WKTransfer transfer,
                         WKTransferState oldState) {
    // The transfer's state has changed.  This implies a possible amount/fee change as well as
    // perhaps other wallet changes, such a nonce change.
    pthread_mutex_lock (&wallet->lock);
    if (WK_TRUE == wkWalletHasTransferLock (wallet, transfer, false)) {
        switch (transfer->state->type) {
            case WK_TRANSFER_STATE_CREATED:
            case WK_TRANSFER_STATE_SIGNED:
            case WK_TRANSFER_STATE_SUBMITTED:
            case WK_TRANSFER_STATE_DELETED:
                break; // nothing
            case WK_TRANSFER_STATE_INCLUDED:
                // If the `oldState` is INCLUDED, then this is a re-org and (somehow) the oldState
                // and the newState can have completely different fees.  Just recompute the wallet
                // balance with `transfer` (and its `newState`).  This case is highly uncommon.
                if (WK_TRANSFER_STATE_INCLUDED == oldState->type)
                    wkWalletUpdBalance (wallet, false);
                else
                    // If `oldState` is not INCLUDED, the updae the balance based on a different
                    // between the estimated and confirmed fees. This case is common.
                    wkWalletUpdBalanceOnTransferConfirmation (wallet, transfer);
                break;
            case WK_TRANSFER_STATE_ERRORED:
                // Recompute the balance
                wkWalletUpdBalance (wallet, false);
                break;
        }

        // Announce a 'TRANSFER_CHANGED'; each currency might respond differently.
        wkWalletAnnounceTransfer (wallet, transfer, WK_WALLET_EVENT_TRANSFER_CHANGED);
    }
    pthread_mutex_unlock (&wallet->lock);
}

extern WKTransfer *
wkWalletGetTransfers (WKWallet wallet, size_t *count) {
    pthread_mutex_lock (&wallet->lock);
    *count = array_count (wallet->transfers);
    WKTransfer *transfers = NULL;
    if (0 != *count) {
        transfers = calloc (*count, sizeof(WKTransfer));
        for (size_t index = 0; index < *count; index++) {
            transfers[index] = wkTransferTake(wallet->transfers[index]);
        }
    }
    pthread_mutex_unlock (&wallet->lock);
    return transfers;
}

private_extern WKTransfer
wkWalletGetTransferByHash (WKWallet wallet, WKHash hashToMatch) {
    WKTransfer transfer = NULL;

    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; NULL == transfer && index < array_count(wallet->transfers); index++) {
        WKHash hash = wkTransferGetHash (wallet->transfers[index]);
        if (WK_TRUE == wkHashEqual(hash, hashToMatch))
            transfer = wallet->transfers[index];
        wkHashGive(hash);
    }
    pthread_mutex_unlock (&wallet->lock);

    return wkTransferTake (transfer);
}

private_extern WKTransfer
wkWalletGetTransferByUIDS (WKWallet wallet, const char *uids) {
    WKTransfer transfer = NULL;

    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; NULL == transfer && index < array_count(wallet->transfers); index++) {
        const char *transferUIDS = wallet->transfers[index]->uids;
        if (NULL != transferUIDS && 0 == strcmp (transferUIDS, uids))
            transfer = wallet->transfers[index];
    }
    pthread_mutex_unlock (&wallet->lock);

    return wkTransferTake (transfer);
}

private_extern WKTransfer
wkWalletGetTransferByHashOrUIDS (WKWallet wallet, WKHash hash, const char *uids) {
    WKTransfer transfer = NULL;

    // This is quite a special match...

    // 1) If we've nothing to match, then we are done.
    if (NULL == hash && NULL == uids) return NULL;

    // 2) If we find a uids match, we are done.
    transfer = (NULL == uids
                ? NULL
                : wkWalletGetTransferByUIDS (wallet, uids));
    if (NULL != transfer) return transfer;

    // 3) If we don't find a hash match, we are done
    transfer = (NULL == hash
                ? NULL
                : wkWalletGetTransferByHash (wallet, hash));
    if (NULL == transfer) return NULL;

    // 4) If we wanted a UIDS but then transfer doesn't have one, then we are done.  This is a
    // transfer that we created... and it is waiting for a uids
    if (NULL != uids && NULL == transfer->uids) return transfer;

    // 5) We are done.
    return NULL;

    // WHY?
}

extern WKAddress
wkWalletGetAddress (WKWallet wallet,
                        WKAddressScheme addressScheme) {
    return wallet->handlers->getAddress (wallet, addressScheme);
}

extern bool
wkWalletHasAddress (WKWallet wallet,
                        WKAddress address) {

    return (wallet->type != wkAddressGetType(address)
            ? false
            : wallet->handlers->hasAddress (wallet, address));
}

private_extern OwnershipGiven BRSetOf(BRCyptoAddress)
wkWalletGetAddressesForRecovery (WKWallet wallet) {
    return wallet->handlers->getAddressesForRecovery (wallet);
}

extern WKFeeBasis
wkWalletGetDefaultFeeBasis (WKWallet wallet) {
    pthread_mutex_lock (&wallet->lock);
    WKFeeBasis feeBasis = wkFeeBasisTake (wallet->defaultFeeBasis);
    pthread_mutex_unlock (&wallet->lock);

    return feeBasis;
}

extern void
wkWalletSetDefaultFeeBasis (WKWallet wallet,
                                WKFeeBasis feeBasis) {
    pthread_mutex_lock (&wallet->lock);
    if (NULL != wallet->defaultFeeBasis) wkFeeBasisGive (wallet->defaultFeeBasis);
    wallet->defaultFeeBasis = wkFeeBasisTake(feeBasis);
    WKFeeBasis newFeeBasis = wallet->defaultFeeBasis;
    pthread_mutex_unlock (&wallet->lock);

    wkWalletGenerateEvent (wallet, wkWalletEventCreateFeeBasisUpdated (newFeeBasis));
}

extern size_t
wkWalletGetTransferAttributeCount (WKWallet wallet,
                                       WKAddress target) {
    return wallet->handlers->getTransferAttributeCount (wallet, target);
}

extern WKTransferAttribute
wkWalletGetTransferAttributeAt (WKWallet wallet,
                                    WKAddress target,
                                    size_t index) {
    return wallet->handlers->getTransferAttributeAt (wallet, target, index);
}

extern WKTransferAttributeValidationError
wkWalletValidateTransferAttribute (WKWallet wallet,
                                       OwnershipKept WKTransferAttribute attribute,
                                       WKBoolean *validates) {
    return wallet->handlers->validateTransferAttribute (wallet, attribute, validates);
}

extern WKTransferAttributeValidationError
wkWalletValidateTransferAttributes (WKWallet wallet,
                                        size_t attributesCount,
                                        OwnershipKept WKTransferAttribute *attributes,
                                        WKBoolean *validates) {
    *validates = WK_TRUE;

    for (size_t index = 0; index <attributesCount; index++) {
        WKTransferAttributeValidationError error = wkWalletValidateTransferAttribute (wallet, attributes[index], validates);
        if (WK_FALSE == *validates)
            return error;
    }

    return (WKTransferAttributeValidationError) 0;
}

extern WKTransferAttribute
wkWalletGetTransferAttributeForKey (WKWallet wallet,
                                        WKAddress target,
                                        const char *key) {

    size_t count = wkWalletGetTransferAttributeCount (wallet, target);
    for (size_t index = 0; index < count; index++) {
        WKTransferAttribute attribute = wkWalletGetTransferAttributeAt (wallet, target, index);
        if (0 == strcasecmp (key, wkTransferAttributeGetKey (attribute)))
            return attribute;
        wkTransferAttributeGive (attribute);
    }
    
    return NULL;
}

extern WKTransfer
wkWalletCreateTransferMultiple (WKWallet wallet,
                                    size_t outputsCount,
                                    WKTransferOutput *outputs,
                                    WKFeeBasis estimatedFeeBasis) {
    //    assert (wkWalletGetType(wallet) == wkFeeBasisGetType(estimatedFeeBasis));
    if (0 == outputsCount) return NULL;

    WKUnit unit         = wkWalletGetUnit (wallet);
    WKUnit unitForFee   = wkWalletGetUnitForFee(wallet);
    WKCurrency currency = wkUnitGetCurrency(unit);

    WKTransfer transfer = wallet->handlers->createTransferMultiple (wallet,
                                                                          outputsCount,
                                                                          outputs,
                                                                          estimatedFeeBasis,
                                                                          currency,
                                                                          unit,
                                                                          unitForFee);

    wkCurrencyGive(currency);
    wkUnitGive (unitForFee);
    wkUnitGive (unit);

    return transfer;
}

extern WKTransfer
wkWalletCreateTransfer (WKWallet  wallet,
                            WKAddress target,
                            WKAmount  amount,
                            WKFeeBasis estimatedFeeBasis,
                            size_t attributesCount,
                            OwnershipKept WKTransferAttribute *attributes) {
    assert (wkWalletGetType(wallet) == wkAddressGetType(target));
    //    assert (wkWalletGetType(wallet) == wkFeeBasisGetType(estimatedFeeBasis));


    WKUnit unit       = wkWalletGetUnit (wallet);
    WKUnit unitForFee = wkWalletGetUnitForFee(wallet);

    WKCurrency currency = wkUnitGetCurrency(unit);
    assert (wkAmountHasCurrency (amount, currency));

    WKTransfer transfer = wallet->handlers->createTransfer (wallet,
                                                                  target,
                                                                  amount,
                                                                  estimatedFeeBasis,
                                                                  attributesCount,
                                                                  attributes,
                                                                  currency,
                                                                  unit,
                                                                  unitForFee);

    if (NULL != transfer && attributesCount > 0)
        wkTransferSetAttributes (transfer, attributesCount, attributes);

    wkCurrencyGive(currency);
    wkUnitGive (unitForFee);
    wkUnitGive (unit);

    return transfer;
}

extern WKTransfer
wkWalletCreateTransferForPaymentProtocolRequest (WKWallet wallet,
                                                     WKPaymentProtocolRequest request,
                                                     WKFeeBasis estimatedFeeBasis) {
    const WKPaymentProtocolHandlers * paymentHandlers = wkHandlersLookup(wkWalletGetType(wallet))->payment;
    
    assert (NULL != paymentHandlers);
    
    return paymentHandlers->createTransfer (request,
                                            wallet,
                                            estimatedFeeBasis);
}

extern WKBoolean
wkWalletEqual (WKWallet w1, WKWallet w2) {
    return AS_WK_BOOLEAN (w1 == w2 ||
                              (w1->type == w2->type &&
                               w1->handlers->isEqual (w1, w2)));
}

extern const char *
wkWalletEventTypeString (WKWalletEventType t) {
    switch (t) {
        case WK_WALLET_EVENT_CREATED:
        return "WK_WALLET_EVENT_CREATED";

        case WK_WALLET_EVENT_CHANGED:
        return "WK_WALLET_EVENT_CHANGED";

        case WK_WALLET_EVENT_DELETED:
        return "WK_WALLET_EVENT_DELETED";

        case WK_WALLET_EVENT_TRANSFER_ADDED:
        return "WK_WALLET_EVENT_TRANSFER_ADDED";

        case WK_WALLET_EVENT_TRANSFER_CHANGED:
        return "WK_WALLET_EVENT_TRANSFER_CHANGED";

        case WK_WALLET_EVENT_TRANSFER_SUBMITTED:
        return "WK_WALLET_EVENT_TRANSFER_SUBMITTED";

        case WK_WALLET_EVENT_TRANSFER_DELETED:
        return "WK_WALLET_EVENT_TRANSFER_DELETED";

        case WK_WALLET_EVENT_BALANCE_UPDATED:
        return "WK_WALLET_EVENT_BALANCE_UPDATED";

        case WK_WALLET_EVENT_FEE_BASIS_UPDATED:
        return "WK_WALLET_EVENT_FEE_BASIS_UPDATED";

        case WK_WALLET_EVENT_FEE_BASIS_ESTIMATED:
        return "WK_WALLET_EVENT_FEE_BASIS_ESTIMATED";
    }
    return "<WK_WALLET_EVENT_TYPE_UNKNOWN>";
}

