//
//  BRCryptoTransferP.h
//  BRCore
//
//  Created by Ed Gamble on 11/22/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoTransferP_h
#define BRCryptoTransferP_h

#include <pthread.h>
#include "support/BRArray.h"

#include "BRCryptoTransfer.h"
#include "BRCryptoNetwork.h"
#include "BRCryptoBaseP.h"


#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Transfer State

private_extern bool
cryptoTransferStateIsEqual (const BRCryptoTransferState *s1,
                            const BRCryptoTransferState *s2);

// MARK: - Transfer Confirmation

typedef struct {
    uint64_t blockNumber;
    uint64_t transactionIndex;
    uint64_t timestamp;
    BRCryptoAmount fee; // ouch; => cant be a struct
} BRCryptoTransferConfirmation;

// MARK: - Transfer Handlers

typedef void
(*BRCryptoTransferReleaseHandler) (BRCryptoTransfer transfer);

typedef BRCryptoHash
(*BRCryptoTransferGetHashHandler) (BRCryptoTransfer transfer);

typedef uint8_t *
(*BRCryptoTransferSerializeHandler) (BRCryptoTransfer transfer,
                                     BRCryptoNetwork  network,
                                     BRCryptoBoolean  requireSignature,
                                     size_t *serializationCount);

typedef int
(*BRCryptoTransferIsEqualHandler) (BRCryptoTransfer t1,
                                   BRCryptoTransfer t2);

typedef struct {
    BRCryptoTransferReleaseHandler release;
    BRCryptoTransferGetHashHandler getHash;
    BRCryptoTransferSerializeHandler serialize;
    BRCryptoTransferIsEqualHandler isEqual;
} BRCryptoTransferHandlers;

/// MARK: - Transfer Listener

typedef struct {
    BRCryptoListenerContext context;
    BRCryptoWalletManager manager;
    BRCryptoWallet wallet;
    BRCryptoTransferListenerCallback callback;
} BRCryptoTransferListener;

/// MARK: - Transfer

struct BRCryptoTransferRecord {
    BRCryptoBlockChainType type;
    const BRCryptoTransferHandlers *handlers;
    BRCryptoRef ref;
    size_t sizeInBytes;

    pthread_mutex_t lock;
    BRCryptoTransferListener listener;

    BRCryptoAddress sourceAddress;
    BRCryptoAddress targetAddress;
    BRCryptoTransferState state;

    /// The amount's unit.
    BRCryptoUnit unit;

    /// The fee's unit
    BRCryptoUnit unitForFee;

    /// The feeBasis.  We must include this here for at least the case of BTC where the fees
    /// encoded into the BTC-wire-transaction are based on the BRWalletFeePerKB value at the time
    /// that the transaction is created.  Sometime later, when the feeBasis is needed we can't
    /// go to the BTC wallet and expect the FeePerKB to be unchanged.

    /// Actually this can be derived from { btc.fee / txSize(btc.tid), txSize(btc.tid) }
    BRCryptoFeeBasis feeBasisEstimated;
    
    BRCryptoTransferDirection direction;
    
    /// The amount (unsigned value).
    BRCryptoAmount amount;

    BRArrayOf(BRCryptoTransferAttribute) attributes;
};

typedef void *BRCryptoTransferCreateContext;
typedef void (*BRCryptoTransferCreateCallback) (BRCryptoTransferCreateContext context,
                                                BRCryptoTransfer transfer);

extern BRCryptoTransfer
cryptoTransferAllocAndInit (size_t sizeInBytes,
                            BRCryptoBlockChainType type,
                            BRCryptoTransferListener listener,
                            BRCryptoUnit unit,
                            BRCryptoUnit unitForFee,
                            BRCryptoFeeBasis feeBasisEstimated,
                            BRCryptoAmount amount,
                            BRCryptoTransferDirection direction,
                            BRCryptoAddress sourceAddress,
                            BRCryptoAddress targetAddress,
                            BRCryptoTransferCreateContext  createContext,
                            BRCryptoTransferCreateCallback createCallback);

private_extern BRCryptoBlockChainType
cryptoTransferGetType (BRCryptoTransfer transfer);

private_extern void
cryptoTransferSetState (BRCryptoTransfer transfer,
                        BRCryptoTransferState state);

private_extern void
cryptoTransferSetAttributes (BRCryptoTransfer transfer,
                             OwnershipKept BRArrayOf(BRCryptoTransferAttribute) attributes);

private_extern void
cryptoTransferGenerateEvent (BRCryptoTransfer transfer,
                             BRCryptoTransferEvent event);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoTransferP_h */
