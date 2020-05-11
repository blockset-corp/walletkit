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
#include "BRCryptoBaseP.h"


#ifdef __cplusplus
extern "C" {
#endif

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

typedef BRCryptoAmount
(*BRCryptoTransferGetAmountAsSignHandler) (BRCryptoTransfer transfer,
                                           BRCryptoBoolean isNegative);

typedef BRCryptoTransferDirection
(*BRCryptoTransferGetDirectionHandler) (BRCryptoTransfer transfer);

typedef BRCryptoHash
(*BRCryptoTransferGetHashHandler) (BRCryptoTransfer transfer);

typedef uint8_t *
(*BRCryptoTransferSerializeForSubmission) (BRCryptoTransfer transfer,
                                           size_t *serializationCount);

typedef int
(*BRCryptoTransferIsEqualHandler) (BRCryptoTransfer t1,
                                   BRCryptoTransfer t2);

typedef struct {
    BRCryptoTransferReleaseHandler release;
    BRCryptoTransferGetAmountAsSignHandler getAmountAsSign;
    BRCryptoTransferGetDirectionHandler getDirection;
    BRCryptoTransferGetHashHandler getHash;
    BRCryptoTransferSerializeForSubmission serializeForSubmission;
    BRCryptoTransferIsEqualHandler isEqual;
} BRCryptoTransferHandlers;

/// MARK: - Transfer

struct BRCryptoTransferRecord {
    BRCryptoBlockChainType type;
    const BRCryptoTransferHandlers *handlers;
    BRCryptoRef ref;
    size_t sizeInBytes;

    pthread_mutex_t lock;

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

    BRArrayOf(BRCryptoTransferAttribute) attributes;
};

extern BRCryptoTransfer
cryptoTransferAllocAndInit (size_t sizeInBytes,
                            BRCryptoBlockChainType type,
                            BRCryptoUnit unit,
                            BRCryptoUnit unitForFee);

private_extern BRCryptoBlockChainType
cryptoTransferGetType (BRCryptoTransfer transfer);

private_extern void
cryptoTransferSetState (BRCryptoTransfer transfer,
                        BRCryptoTransferState state);

private_extern void
cryptoTransferSetConfirmedFeeBasis (BRCryptoTransfer transfer,
                                    BRCryptoFeeBasis feeBasisConfirmed);

private_extern void
cryptoTransferSetAttributes (BRCryptoTransfer transfer,
                             OwnershipKept BRArrayOf(BRCryptoTransferAttribute) attributes);

#if 0
private_extern BRCryptoTransfer
cryptoTransferCreateAsBTC (BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BRWallet *wid,
                           OwnershipKept BRTransaction *tid,
                           BRCryptoBoolean isBTC); // TRUE if BTC; FALSE if BCH

private_extern BRCryptoTransfer
cryptoTransferCreateAsETH (BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BREthereumEWM ewm,
                           BREthereumTransfer tid,
                           BRCryptoFeeBasis feeBasisEstimated);

extern BRCryptoTransfer
cryptoTransferCreateAsGEN (BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BRGenericTransfer tid);

private_extern BRTransaction *
cryptoTransferAsBTC (BRCryptoTransfer transfer);

private_extern BREthereumTransfer
cryptoTransferAsETH (BRCryptoTransfer transfer);

private_extern BRGenericTransfer
cryptoTransferAsGEN (BRCryptoTransfer transfer);

private_extern BRCryptoBoolean
cryptoTransferHasBTC (BRCryptoTransfer transfer,
                      BRTransaction *btc);

private_extern BRCryptoBoolean
cryptoTransferHasETH (BRCryptoTransfer transfer,
                      BREthereumTransfer eth);

private_extern BRCryptoBoolean
cryptoTransferHasGEN (BRCryptoTransfer transfer,
                      BRGenericTransfer gen);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoTransferP_h */
