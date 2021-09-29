//
//  WKWalletP.h
//  WalletKitCore
//
//  Created by Ed Gamble on 11/22/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKWalletP_h
#define WKWalletP_h

#include <pthread.h>
#include <stdbool.h>
#include <assert.h>
#include "support/BRArray.h"
#include "support/BRSet.h"

#include "event/WKWallet.h"
#include "WKWallet.h"
#include "WKBaseP.h"
#include "WKClient.h"
#include "WKTransferP.h"
#include "WKListenerP.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Wallet Event

private_extern WKWalletEvent
wkWalletEventCreate (WKWalletEventType type);

private_extern WKWalletEvent
wkWalletEventCreateState (WKWalletState old,
                              WKWalletState new);

private_extern WKWalletEvent
wkWalletEventCreateTransfer (WKWalletEventType type,
                                 WKTransfer transfer);

private_extern WKWalletEvent
wkWalletEventCreateTransferSubmitted (WKTransfer transfer);

private_extern WKWalletEvent
wkWalletEventCreateBalanceUpdated (WKAmount balance);

private_extern WKWalletEvent
wkWalletEventCreateFeeBasisUpdated (WKFeeBasis basis);

private_extern WKWalletEvent
wkWalletEventCreateFeeBasisEstimated (WKStatus status,
                                          WKCookie cookie,
                                          WKFeeBasis basis);

// MARK: - Wallet Handlers

typedef void
(*WKWalletReleaseHandler) (WKWallet address);

typedef WKAddress
(*WKWalletGetAddressHandler) (WKWallet wallet,
                                    WKAddressScheme addressScheme);

typedef bool
(*WKWalletHasAddressHandler) (WKWallet wallet,
                                    WKAddress address);

typedef size_t
(*WKWalletGetTransferAttributeCountHandler) (WKWallet wallet,
                                                   WKAddress target);

typedef WKTransferAttribute
(*WKWalletGetTransferAttributeAtHandler) (WKWallet wallet,
                                                WKAddress target,
                                                size_t index);

typedef WKTransferAttributeValidationError
(*WKWalletValidateTransferAttributeHandler) (WKWallet wallet,
                                                   OwnershipKept WKTransferAttribute attribute,
                                                   WKBoolean *validates);

typedef WKTransfer
(*WKWalletCreateTransferHandler) (WKWallet  wallet,
                                        WKAddress target,
                                        WKAmount  amount,
                                        WKFeeBasis estimatedFeeBasis,
                                        size_t attributesCount,
                                        OwnershipKept WKTransferAttribute *attributes,
                                        WKCurrency currency,
                                        WKUnit unit,
                                        WKUnit unitForFee);

typedef WKTransfer
(*WKWalletCreateTransferMultipleHandler) (WKWallet wallet,
                                                size_t outputsCount,
                                                WKTransferOutput *outputs,
                                                WKFeeBasis estimatedFeeBasis,
                                                WKCurrency currency,
                                                WKUnit unit,
                                                WKUnit unitForFee);

typedef OwnershipGiven BRSetOf(WKAddress)
(*WKWalletGetAddressesForRecoveryHandler) (WKWallet wallet);

typedef void
(*WKWalletAnnounceTransfer) (WKWallet wallet,
                                   WKTransfer transfer,
                                   WKWalletEventType type); // TRANSFER_{ADDED,DELETED}

typedef bool
(*WKWalletIsEqualHandler) (WKWallet wallet1, WKWallet wallet2);

typedef struct {
    WKWalletReleaseHandler release;
    WKWalletGetAddressHandler getAddress;
    WKWalletHasAddressHandler hasAddress;
    WKWalletGetTransferAttributeCountHandler getTransferAttributeCount;
    WKWalletGetTransferAttributeAtHandler getTransferAttributeAt;
    WKWalletValidateTransferAttributeHandler validateTransferAttribute;
    WKWalletCreateTransferHandler createTransfer;
    WKWalletCreateTransferMultipleHandler createTransferMultiple;
    WKWalletGetAddressesForRecoveryHandler getAddressesForRecovery;
    WKWalletAnnounceTransfer announceTransfer; // May be NULL
    WKWalletIsEqualHandler isEqual;
} WKWalletHandlers;


// MARK: - Wallet

struct WKWalletRecord {
    WKNetworkType type;
    const WKWalletHandlers *handlers;
    WKRef ref;
    size_t sizeInBytes;

    pthread_mutex_t lock;
    WKWalletListener listener;

    /// The state (modifiable)
    WKWalletState state;

    WKUnit unit;
    WKUnit unitForFee;

    /// The transfers (modifiable)
    BRArrayOf (WKTransfer) transfers;

    /// The balance (modifiable)
    WKAmount balance;
    WKAmount balanceMinimum;
    WKAmount balanceMaximum;

    /// The defaultFeeBaiss (modifiable)
    WKFeeBasis defaultFeeBasis;

    WKTransferListener listenerTransfer;
};

typedef void  *WKWalletCreateContext;
typedef void (*WKWalletCreateCallbak) (WKWalletCreateContext context,
                                             WKWallet wallet);

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
                          WKWalletCreateCallbak createCallback);

private_extern WKNetworkType
wkWalletGetType (WKWallet wallet);

private_extern void
wkWalletSetState (WKWallet wallet,
                      WKWalletState state);

private_extern WKTransfer
wkWalletGetTransferByHash (WKWallet wallet, WKHash hashToMatch);

private_extern WKTransfer
wkWalletGetTransferByUIDS (WKWallet wallet, const char *identifier);

private_extern WKTransfer
wkWalletGetTransferByHashOrUIDS (WKWallet wallet, WKHash hash, const char *uids);

private_extern void
wkWalletAddTransfer (WKWallet wallet, WKTransfer transfer);

private_extern void
wkWalletAddTransfers (WKWallet wallet,
                          OwnershipGiven BRArrayOf(WKTransfer) transfers);

private_extern void
wkWalletRemTransfer (WKWallet wallet, WKTransfer transfer);

private_extern void
wkWalletReplaceTransfer (WKWallet wallet,
                             OwnershipKept  WKTransfer oldTransfer,
                             OwnershipGiven WKTransfer newTransfer);

private_extern OwnershipGiven BRSetOf(BRCyptoAddress)
wkWalletGetAddressesForRecovery (WKWallet wallet);

private_extern void
wkWalletUpdBalance (WKWallet wallet, bool needLock);

static inline void
wkWalletGenerateEvent (WKWallet wallet,
                           OwnershipGiven WKWalletEvent event) {
    wkListenerGenerateWalletEvent (&wallet->listener, wallet, event);
}

#ifdef __cplusplus
}
#endif

#endif /* WKWalletP_h */
