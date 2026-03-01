//
//  WKTransferP.h
//  WalletKitCore
//
//  Created by Ed Gamble on 11/22/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKTransferP_h
#define WKTransferP_h

#include <pthread.h>
#include "support/BRArray.h"

#include "WKTransfer.h"
#include "WKNetwork.h"
#include "WKBaseP.h"
#include "WKListenerP.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Transfer Attribute

private_extern WKTransferAttribute
wkTransferAttributeCreate (const char *key,
                               const char *val, // nullable
                               WKBoolean isRequired);

// MARK: - Transfer State

#define WK_TRANSFER_INCLUDED_ERROR_SIZE     16

struct WKTransferStateRecord {
    WKTransferStateType type;
    union {
        struct {
            uint64_t blockNumber;
            uint64_t transactionIndex;
            // This is not assuredly the including block's timestamp; it is the transaction's
            // timestamp which varies depending on how the transaction was discovered.
            uint64_t timestamp;
            WKFeeBasis feeBasis;

            /// The included status; generally with type WK_TRANSFER_INCLUDE_STATUS_SUCCESS.  A few
            /// blockchains, notably Ethereum, can include a transaction on a failure, such as where
            ///  'gas' is paid in the fee (but the 'gas' wasn't enough to complete the transfer).
            WKTransferIncludeStatus status;
        } included;

        struct {
            WKTransferSubmitError error;
        } errored;
    } u;

    WKRef ref;
};

private_extern bool
wkTransferStateIsEqual (const WKTransferState s1,
                            const WKTransferState s2);

// MARK: - Transfer Confirmation

typedef struct {
    uint64_t blockNumber;
    uint64_t transactionIndex;
    uint64_t timestamp;
    WKAmount fee; // ouch; => cant be a struct
} WKTransferConfirmation;

// MARK: - Transfer Handlers

typedef void
(*WKTransferReleaseHandler) (WKTransfer transfer);

typedef void
(*WKTransferUpdateIdentifierHandler) (WKTransfer transfer);

typedef WKHash
(*WKTransferGetHashHandler) (WKTransfer transfer);

typedef bool // true if changed
(*WKTransferSetHashHandler) (WKTransfer transfer,
                                   WKHash hash);

typedef OwnershipGiven uint8_t *
(*WKTransferSerializeHandler) (WKTransfer transfer,
                               WKNetwork  network,
                               WKBoolean  requireSignature,
                               size_t *serializationCount);

typedef OwnershipGiven uint8_t *
(*WKTransferGetBytesForFeeEstimateHandler) (WKTransfer transfer,
                                            WKNetwork  network,
                                            size_t *bytesCount);

typedef int // 1 if equal, 0 if not
(*WKTransferIsEqualHandler) (WKTransfer t1,
                                   WKTransfer t2);

typedef struct {
    WKTransferReleaseHandler release;
    WKTransferGetHashHandler getHash;
    WKTransferSetHashHandler setHash;
    WKTransferUpdateIdentifierHandler updateIdentifier;
    WKTransferSerializeHandler serialize;
    WKTransferGetBytesForFeeEstimateHandler getBytesForFeeEstimate;
    WKTransferIsEqualHandler isEqual;
} WKTransferHandlers;

/// MARK: - Transfer

struct WKTransferRecord {
    WKNetworkType type;
    const WKTransferHandlers *handlers;
    WKRef ref;
    size_t sizeInBytes;

    pthread_mutex_t lock;
    WKTransferListener listener;

    /// The UIDS is globally unique.  It is derived from the transfer's originating
    /// transaction's identifier w/ the addition of an otherwise arbitrary index.  That is, one
    /// transaction can generate multiple transfers; the ordering of those generated transfers is
    /// arbitrary but each one can be assigned an index.
    ///
    /// In practice the UIDS is `<identifier>:<index>`.  The `identifier` is usually a hash.
    ///
    /// The UIDS can not exist until the transaction has been processed into its one or more
    /// transfers.  When a transfer is created this value will be NULL.
    char *uids;

    /// The identifier for this transfer's originating transaction.  This is usually the string
    /// representation of a hash; however some currencies, notably Hedera, have identifier that
    /// are not hashes.
    ///
    /// The identifier can be NULL.
    char *identifier;

    /// The source address sent the amount and paid the fee.
    WKAddress sourceAddress;

    /// The target address received the amount.
    WKAddress targetAddress;

    /// The state (modifiable)
    WKTransferState state;

    /// The amount's unit.
    WKUnit unit;

    /// The fee's unit
    WKUnit unitForFee;

    /// The feeBasis.  We must include this here for at least the case of BTC where the fees
    /// encoded into the BTC-wire-transaction are based on the BRWalletFeePerKB value at the time
    /// that the transaction is created.  Sometime later, when the feeBasis is needed we can't
    /// go to the BTC wallet and expect the FeePerKB to be unchanged.

    /// Actually this can be derived from { btc.fee / txSize(btc.tid), txSize(btc.tid) }
    WKFeeBasis feeBasisEstimated;
    
    WKTransferDirection direction;
    
    /// The amount (unsigned value).
    WKAmount amount;

    /// The attributes (modifiable)
    BRArrayOf(WKTransferAttribute) attributes;
};

typedef void *WKTransferCreateContext;
typedef void (*WKTransferCreateCallback) (WKTransferCreateContext context,
                                                WKTransfer transfer);

extern WKTransfer // OwnershipKept, all arguments
wkTransferAllocAndInit (size_t sizeInBytes,
                        WKNetworkType type,
                        WKTransferListener listener,
                        const char *uids,
                        WKUnit unit,
                        WKUnit unitForFee,
                        WKFeeBasis feeBasisEstimated,
                        WKAmount amount,
                        WKTransferDirection direction,
                        WKAddress sourceAddress,
                        WKAddress targetAddress,
                        WKTransferState state,
                        WKTransferCreateContext  createContext,
                        WKTransferCreateCallback createCallback);

private_extern WKNetworkType
wkTransferGetType (WKTransfer transfer);

private_extern void
wkTransferSetUids (WKTransfer transfer,
                   const char *uids);

private_extern void
wkTransferSetStateForced (WKTransfer transfer,
                              WKTransferState state,
                              bool forceEvent);

static inline void
wkTransferSetState (WKTransfer transfer,
                        WKTransferState state) {
    wkTransferSetStateForced (transfer, state, false);
}

// TODO: Are TransferAttributes not constant?
private_extern void
wkTransferSetAttributes (WKTransfer transfer,
                             size_t attributesCount,
                             OwnershipKept WKTransferAttribute *attributes);

private_extern void
wkTransferAttributeArrayRelease (BRArrayOf(WKTransferAttribute) attributes);

static inline void
wkTransferGenerateEvent (WKTransfer transfer,
                             WKTransferEvent event) {
    if (NULL == transfer->listener.listener) return;
    wkListenerGenerateTransferEvent(&transfer->listener, transfer, event);
}

private_extern WKAmount
wkTransferGetEstimatedFee (WKTransfer transfer);

private_extern WKAmount
wkTransferGetConfirmedFee (WKTransfer transfer);

private_extern WKFeeBasis
wkTransferGetFeeBasis (WKTransfer transfer);

private_extern WKAmount
wkTransferGetAmountDirectedInternal (WKTransfer transfer,
                                         WKBoolean  respectSuccess);

private_extern OwnershipGiven WKAmount
wkWalletGetTransferAmountDirectedNet (WKWallet wallet,
                                      WKTransfer transfer);

#ifdef __cplusplus
}
#endif

#endif /* WKTransferP_h */
