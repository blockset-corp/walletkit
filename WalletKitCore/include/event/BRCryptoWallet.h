//
//  BRCryptoWalletEvent.h
//  BRCore
//
//  Created by Ed Gamble on 8/12/20.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoWalletEvent_h
#define BRCryptoWalletEvent_h

#include "BRCryptoNetwork.h"        // NetworkFee
#include "BRCryptoFeeBasis.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: Wallet Event

typedef enum {
    CRYPTO_WALLET_STATE_CREATED,
    CRYPTO_WALLET_STATE_DELETED
} BRCryptoWalletState;

typedef enum {
    /// Signaled when a wallet is *allocated*; the wallet may not, in fact generally is
    /// not, fully initialized.  Thus the wallet should only be used for 'identity' purposes.
    CRYPTO_WALLET_EVENT_CREATED,

    /// Signaled when a wallet's state change - such as when the state transitions from
    /// CREATED to DELETED.
    CRYPTO_WALLET_EVENT_CHANGED,

    /// Signaled when a wallet is deleted; the wallet must not be 'dereferenced' and thus
    /// the pointer value can be used.  Surely the wallets's memory will be gone by the time
    /// that thread handling the event first sees the deleted wallet.  If any dereference
    /// occurs, the result will be an instant crash.
    CRYPTO_WALLET_EVENT_DELETED,

    /// Signaled when a transfer is added to the wallet
    CRYPTO_WALLET_EVENT_TRANSFER_ADDED,

    /// Signaled when a transfer is changed.
    CRYPTO_WALLET_EVENT_TRANSFER_CHANGED,

    /// Signaled when a transfer is submitted.
    CRYPTO_WALLET_EVENT_TRANSFER_SUBMITTED,

    /// Signaled when a transfer is removed from the wallet.
    CRYPTO_WALLET_EVENT_TRANSFER_DELETED,

    /// Signaled when the wallet's balance changes.
    CRYPTO_WALLET_EVENT_BALANCE_UPDATED,

    /// Signaled when the wallet's default feeBasis changes.
    CRYPTO_WALLET_EVENT_FEE_BASIS_UPDATED,

    /// Signaled when the wallet's feeBaiss is estimated.
    CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED,
} BRCryptoWalletEventType;

extern const char *
cryptoWalletEventTypeString (BRCryptoWalletEventType t);

typedef struct BRCryptoWalletEventRecord *BRCryptoWalletEvent;

extern BRCryptoWalletEventType
cryptoWalletEventGetType (BRCryptoWalletEvent event);

extern BRCryptoBoolean
cryptoWalletEventExtractState (BRCryptoWalletEvent event,
                               BRCryptoWalletState *old,
                               BRCryptoWalletState *new);

extern BRCryptoBoolean
cryptoWalletEventExtractTransfer (BRCryptoWalletEvent event,
                                  BRCryptoTransfer *transfer);

extern BRCryptoBoolean
cryptoWalletEventExtractTransferSubmit (BRCryptoWalletEvent event,
                                        BRCryptoTransfer *transfer);

extern BRCryptoBoolean
cryptoWalletEventExtractBalanceUpdate (BRCryptoWalletEvent event,
                                       BRCryptoAmount *balance);

extern BRCryptoBoolean
cryptoWalletEventExtractFeeBasisUpdate (BRCryptoWalletEvent event,
                                        BRCryptoFeeBasis *basis);

extern BRCryptoBoolean
cryptoWalletEventExtractFeeBasisEstimate (BRCryptoWalletEvent event,
                                          BRCryptoStatus *status,
                                          BRCryptoCookie *cookie,
                                          BRCryptoFeeBasis *basis);

extern BRCryptoBoolean
cryptoWalletEventIsEqual (BRCryptoWalletEvent event1,
                          BRCryptoWalletEvent event2);

DECLARE_CRYPTO_GIVE_TAKE (BRCryptoWalletEvent, cryptoWalletEvent);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoWalletEvent_h */
