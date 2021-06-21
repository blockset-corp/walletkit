//
//  BRCryptoWalletP.h
//  BRCore
//
//  Created by Ed Gamble on 11/22/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoWalletP_h
#define BRCryptoWalletP_h

#include <pthread.h>
#include <stdbool.h>
#include <assert.h>
#include "support/BRArray.h"
#include "support/BRSet.h"

#include "event/BRCryptoWallet.h"
#include "BRCryptoWallet.h"
#include "BRCryptoBaseP.h"
#include "BRCryptoClient.h"
#include "BRCryptoTransferP.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Wallet Event

private_extern BRCryptoWalletEvent
cryptoWalletEventCreate (BRCryptoWalletEventType type);

private_extern BRCryptoWalletEvent
cryptoWalletEventCreateState (BRCryptoWalletState old,
                              BRCryptoWalletState new);

private_extern BRCryptoWalletEvent
cryptoWalletEventCreateTransfer (BRCryptoWalletEventType type,
                                 BRCryptoTransfer transfer);

private_extern BRCryptoWalletEvent
cryptoWalletEventCreateTransferSubmitted (BRCryptoTransfer transfer);

private_extern BRCryptoWalletEvent
cryptoWalletEventCreateBalanceUpdated (BRCryptoAmount balance);

private_extern BRCryptoWalletEvent
cryptoWalletEventCreateFeeBasisUpdated (BRCryptoFeeBasis basis);

private_extern BRCryptoWalletEvent
cryptoWalletEventCreateFeeBasisEstimated (BRCryptoStatus status,
                                          BRCryptoCookie cookie,
                                          BRCryptoFeeBasis basis);

// MARK: - Wallet Handlers

typedef void
(*BRCryptoWalletReleaseHandler) (BRCryptoWallet address);

typedef BRCryptoAddress
(*BRCryptoWalletGetAddressHandler) (BRCryptoWallet wallet,
                                    BRCryptoAddressScheme addressScheme);

typedef bool
(*BRCryptoWalletHasAddressHandler) (BRCryptoWallet wallet,
                                    BRCryptoAddress address);

typedef size_t
(*BRCryptoWalletGetTransferAttributeCountHandler) (BRCryptoWallet wallet,
                                                   BRCryptoAddress target);

typedef BRCryptoTransferAttribute
(*BRCryptoWalletGetTransferAttributeAtHandler) (BRCryptoWallet wallet,
                                                BRCryptoAddress target,
                                                size_t index);

typedef BRCryptoTransferAttributeValidationError
(*BRCryptoWalletValidateTransferAttributeHandler) (BRCryptoWallet wallet,
                                                   OwnershipKept BRCryptoTransferAttribute attribute,
                                                   BRCryptoBoolean *validates);

typedef BRCryptoTransfer
(*BRCryptoWalletCreateTransferHandler) (BRCryptoWallet  wallet,
                                        BRCryptoAddress target,
                                        BRCryptoAmount  amount,
                                        BRCryptoFeeBasis estimatedFeeBasis,
                                        size_t attributesCount,
                                        OwnershipKept BRCryptoTransferAttribute *attributes,
                                        BRCryptoCurrency currency,
                                        BRCryptoUnit unit,
                                        BRCryptoUnit unitForFee);

typedef BRCryptoTransfer
(*BRCryptoWalletCreateTransferMultipleHandler) (BRCryptoWallet wallet,
                                                size_t outputsCount,
                                                BRCryptoTransferOutput *outputs,
                                                BRCryptoFeeBasis estimatedFeeBasis,
                                                BRCryptoCurrency currency,
                                                BRCryptoUnit unit,
                                                BRCryptoUnit unitForFee);

typedef OwnershipGiven BRSetOf(BRCryptoAddress)
(*BRCryptoWalletGetAddressesForRecoveryHandler) (BRCryptoWallet wallet);

typedef void
(*BRCryptoWalletAnnounceTransfer) (BRCryptoWallet wallet,
                                   BRCryptoTransfer transfer,
                                   BRCryptoWalletEventType type); // TRANSFER_{ADDED,DELETED}

typedef bool
(*BRCryptoWalletIsEqualHandler) (BRCryptoWallet wallet1, BRCryptoWallet wallet2);

typedef struct {
    BRCryptoWalletReleaseHandler release;
    BRCryptoWalletGetAddressHandler getAddress;
    BRCryptoWalletHasAddressHandler hasAdress;
    BRCryptoWalletGetTransferAttributeCountHandler getTransferAttributeCount;
    BRCryptoWalletGetTransferAttributeAtHandler getTransferAttributeAt;
    BRCryptoWalletValidateTransferAttributeHandler validateTransferAttribute;
    BRCryptoWalletCreateTransferHandler createTransfer;
    BRCryptoWalletCreateTransferMultipleHandler createTransferMultiple;
    BRCryptoWalletGetAddressesForRecoveryHandler getAddressesForRecovery;
    BRCryptoWalletAnnounceTransfer announceTransfer; // May be NULL
    BRCryptoWalletIsEqualHandler isEqual;
} BRCryptoWalletHandlers;


// MARK: - Wallet

struct BRCryptoWalletRecord {
    BRCryptoBlockChainType type;
    const BRCryptoWalletHandlers *handlers;
    BRCryptoRef ref;
    size_t sizeInBytes;

    pthread_mutex_t lock;
    BRCryptoWalletListener listener;

    /// The state (modifiable)
    BRCryptoWalletState state;

    BRCryptoUnit unit;
    BRCryptoUnit unitForFee;

    /// The transfers (modifiable)
    BRArrayOf (BRCryptoTransfer) transfers;

    /// The balance (modifiable)
    BRCryptoAmount balance;
    BRCryptoAmount balanceMinimum;
    BRCryptoAmount balanceMaximum;

    /// The defaultFeeBaiss (modifiable)
    BRCryptoFeeBasis defaultFeeBasis;

    BRCryptoTransferListener listenerTransfer;
};

typedef void  *BRCryptoWalletCreateContext;
typedef void (*BRCryptoWalletCreateCallbak) (BRCryptoWalletCreateContext context,
                                             BRCryptoWallet wallet);

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
                          BRCryptoWalletCreateCallbak createCallback);

private_extern BRCryptoBlockChainType
cryptoWalletGetType (BRCryptoWallet wallet);

private_extern void
cryptoWalletSetState (BRCryptoWallet wallet,
                      BRCryptoWalletState state);

private_extern BRCryptoTransfer
cryptoWalletGetTransferByHash (BRCryptoWallet wallet, BRCryptoHash hashToMatch);

private_extern BRCryptoTransfer
cryptoWalletGetTransferByUIDS (BRCryptoWallet wallet, const char *identifier);

private_extern BRCryptoTransfer
cryptoWalletGetTransferByHashOrUIDS (BRCryptoWallet wallet, BRCryptoHash hash, const char *uids);

private_extern void
cryptoWalletAddTransfer (BRCryptoWallet wallet, BRCryptoTransfer transfer);

private_extern void
cryptoWalletAddTransfers (BRCryptoWallet wallet,
                          OwnershipGiven BRArrayOf(BRCryptoTransfer) transfers);

private_extern void
cryptoWalletRemTransfer (BRCryptoWallet wallet, BRCryptoTransfer transfer);

private_extern void
cryptoWalletReplaceTransfer (BRCryptoWallet wallet,
                             OwnershipKept  BRCryptoTransfer oldTransfer,
                             OwnershipGiven BRCryptoTransfer newTransfer);

private_extern OwnershipGiven BRSetOf(BRCyptoAddress)
cryptoWalletGetAddressesForRecovery (BRCryptoWallet wallet);

private_extern void
cryptoWalletUpdBalance (BRCryptoWallet wallet, bool needLock);

static inline void
cryptoWalletGenerateEvent (BRCryptoWallet wallet,
                           OwnershipGiven BRCryptoWalletEvent event) {
    cryptoListenerGenerateWalletEvent (&wallet->listener, wallet, event);
}

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoWalletP_h */
