//
//  WKWalletEvent.h
//  WalletKitCore
//
//  Created by Ed Gamble on 8/12/20.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKWalletEvent_h
#define WKWalletEvent_h

#include "WKNetwork.h"        // NetworkFee
#include "WKFeeBasis.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: Wallet Event

typedef enum {
    WK_WALLET_STATE_CREATED,
    WK_WALLET_STATE_DELETED
} WKWalletState;

typedef enum {
    /// Signaled when a wallet is *allocated*; the wallet may not, in fact generally is
    /// not, fully initialized.  Thus the wallet should only be used for 'identity' purposes.
    WK_WALLET_EVENT_CREATED,

    /// Signaled when a wallet's state change - such as when the state transitions from
    /// CREATED to DELETED.
    WK_WALLET_EVENT_CHANGED,

    /// Signaled when a wallet is deleted; the wallet must not be 'dereferenced' and thus
    /// the pointer value can be used.  Surely the wallets's memory will be gone by the time
    /// that thread handling the event first sees the deleted wallet.  If any dereference
    /// occurs, the result will be an instant crash.
    WK_WALLET_EVENT_DELETED,

    /// Signaled when a transfer is added to the wallet
    WK_WALLET_EVENT_TRANSFER_ADDED,

    /// Signaled when a transfer is changed.
    WK_WALLET_EVENT_TRANSFER_CHANGED,

    /// Signaled when a transfer is submitted.
    WK_WALLET_EVENT_TRANSFER_SUBMITTED,

    /// Signaled when a transfer is removed from the wallet.
    WK_WALLET_EVENT_TRANSFER_DELETED,

    /// Signaled when the wallet's balance changes.
    WK_WALLET_EVENT_BALANCE_UPDATED,

    /// Signaled when the wallet's default feeBasis changes.
    WK_WALLET_EVENT_FEE_BASIS_UPDATED,

    /// Signaled when the wallet's feeBaiss is estimated.
    WK_WALLET_EVENT_FEE_BASIS_ESTIMATED,
} WKWalletEventType;

extern const char *
wkWalletEventTypeString (WKWalletEventType t);

typedef struct WKWalletEventRecord *WKWalletEvent;

extern WKWalletEventType
wkWalletEventGetType (WKWalletEvent event);

extern WKBoolean
wkWalletEventExtractState (WKWalletEvent event,
                           WKWalletState *old,
                           WKWalletState *new);

extern WKBoolean
wkWalletEventExtractTransfer (WKWalletEvent event,
                              WKTransfer *transfer);

extern WKBoolean
wkWalletEventExtractTransferSubmit (WKWalletEvent event,
                                    WKTransfer *transfer);

extern WKBoolean
wkWalletEventExtractBalanceUpdate (WKWalletEvent event,
                                   WKAmount *balance);

extern WKBoolean
wkWalletEventExtractFeeBasisUpdate (WKWalletEvent event,
                                    WKFeeBasis *basis);

extern WKBoolean
wkWalletEventExtractFeeBasisEstimate (WKWalletEvent event,
                                      WKStatus *status,
                                      WKCookie *cookie,
                                      WKFeeBasis *basis);

extern WKBoolean
wkWalletEventIsEqual (WKWalletEvent event1,
                      WKWalletEvent event2);

DECLARE_WK_GIVE_TAKE (WKWalletEvent, wkWalletEvent);

#ifdef __cplusplus
}
#endif

#endif /* WKWalletEvent_h */
