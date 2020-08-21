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

#include "BRCryptoWallet.h"
#include "BRCryptoBaseP.h"
#include "BRCryptoClient.h"
#include "BRCryptoTransferP.h"

#ifdef __cplusplus
extern "C" {
#endif

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
    BRCryptoWalletIsEqualHandler isEqual;
} BRCryptoWalletHandlers;

// MARK: - Wallet Listener

typedef struct {
    BRCryptoListenerContext context;
    BRCryptoWalletManager manager;
    BRCryptoWalletListenerCallback walletCallback;
    BRCryptoTransferListenerCallback transferCallback;
} BRCryptoWalletListener;

// MARK: - Wallet

struct BRCryptoWalletRecord {
    BRCryptoBlockChainType type;
    const BRCryptoWalletHandlers *handlers;
    BRCryptoRef ref;
    size_t sizeInBytes;

    pthread_mutex_t lock;
    BRCryptoWalletListener listener;

    BRCryptoWalletState state;

    BRCryptoUnit unit;
    BRCryptoUnit unitForFee;

    //
    // Do we hold transfers here?  The BRWallet and the BREthereumWallet already hold transfers.
    // Shouldn't we defer to those to get transfers (and then wrap them in BRCryptoTransfer)?
    // Then we avoid caching trouble (in part).  For a newly created transaction (not yet signed),
    // the BRWallet will not hold a BRTransaction however, BREthereumWallet will hold a new
    // BREthereumTransaction. From BRWalet: `assert(tx != NULL && BRTransactionIsSigned(tx));`
    //
    // We are going to have the same
    //
    BRArrayOf (BRCryptoTransfer) transfers;

    BRCryptoAmount balance;
    BRCryptoAmount balanceMinimum;
    BRCryptoAmount balanceMaximum;

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

private_extern void
cryptoWalletAddTransfer (BRCryptoWallet wallet, BRCryptoTransfer transfer);

private_extern void
cryptoWalletRemTransfer (BRCryptoWallet wallet, BRCryptoTransfer transfer);

private_extern OwnershipGiven BRSetOf(BRCyptoAddress)
cryptoWalletGetAddressesForRecovery (BRCryptoWallet wallet);

private_extern void
cryptoWalletGenerateEvent (BRCryptoWallet wallet,
                           BRCryptoWalletEvent event);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoWalletP_h */
