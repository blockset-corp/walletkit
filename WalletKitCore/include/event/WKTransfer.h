//
//  WKTransferEvent.h
//  WalletKitCore
//
//  Created by Ed Gamble on 8/12/20.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKTransferEvent_h
#define WKTransferEvent_h

#include "WKBase.h"
#include "WKFeeBasis.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Transfer Include Error

#define WK_TRANSFER_STATUS_DETAILS_LENGTH      (31)

///
/// An 'Included Error' occurs for some currencies, none of the BTC-alikes, whereby a transaction
/// can be included on the blockchain but not complete successfully - e.g. the asset is not
/// transferred.
///
typedef enum {
    WK_TRANSFER_INCLUDED_STATUS_SUCCESS,

    /// Not enough 'gas' paid to complete the calculations required to include the transaction.  The
    /// transaction is in the blockchain, the sender paid a fee, but the assert transfer did not
    /// complete.
    WK_TRANSFER_INCLUDED_STATUS_FAILURE_INSUFFICIENT_NETWORK_COST_UNIT, // out of gas

    /// An error occurred during the processing of the transaction.  This is typically something
    /// specific to the Smart Contract processing the transaction.  This error won't be seen for
    /// a simple transfer of the native currency; nor, in the case of Ethereum, for an ERC-20
    /// exchange. Examples:
    ///    'Comp::_transferTokens: transfer amount exceeds balance
    ///    'insufficient funds'
    ///    'Too little received'
    ///    'UniswapV2Router: EXCESSIVE_INPUT_AMOUNT'
    WK_TRANSFER_INCLUDED_STATUS_FAILURE_REVERTED,

    /// The resons for the included failure is unknown
    WK_TRANSFER_INCLUDED_STATUS_FAILURE_UNKNOWN,


} WKTransferIncludeStatusType;

#define TRANSFER_INCLUDED_ERROR_UNDEFINED  ((WKTransferIncludeErrorType) -1)

typedef struct {
    /// The type of include error.
    WKTransferIncludeStatusType type;

    /// Additional details, if available
    char details [WK_TRANSFER_STATUS_DETAILS_LENGTH + 1];
} WKTransferIncludeStatus;

extern WKTransferIncludeStatus
wkTransferIncludeStatusCreateSuccess (void);

extern WKTransferIncludeStatus
wkTransferIncludeStatusCreateFailure (WKTransferIncludeStatusType type, const char *details);

extern const char *
wkTransferIncludeStatusGetDetails (const WKTransferIncludeStatus *status);

extern bool
wkTransferIncludeStatusIsEqual (const WKTransferIncludeStatus *status1,
                                const WKTransferIncludeStatus *status2);

typedef enum {
    WK_TRANSFER_SUBMIT_ERROR_UNKNOWN,
    WK_TRANSFER_SUBMIT_ERROR_POSIX,
} WKTransferSubmitErrorType;

typedef struct {
    WKTransferSubmitErrorType type;
    union {
        struct {
            int errnum;
        } posix;
    } u;
} WKTransferSubmitError;

extern WKTransferSubmitError
wkTransferSubmitErrorUnknown(void);

extern WKTransferSubmitError
wkTransferSubmitErrorPosix(int errnum);

extern bool
wkTransferSubmitErrorIsEqual (const WKTransferSubmitError *e1,
                              const WKTransferSubmitError *e2);
/**
 * Return a descriptive message as to why the error occurred.
 *
 *@return the detailed reason as a string or NULL
 */
extern char *
wkTransferSubmitErrorGetMessage(WKTransferSubmitError *e);

/// MARK: - Transfer State

typedef enum {
    WK_TRANSFER_STATE_CREATED,
    WK_TRANSFER_STATE_SIGNED,
    WK_TRANSFER_STATE_SUBMITTED,
    WK_TRANSFER_STATE_INCLUDED,
    WK_TRANSFER_STATE_ERRORED,
    WK_TRANSFER_STATE_DELETED,
} WKTransferStateType;

extern const char *
wkTransferStateTypeString (WKTransferStateType type);

typedef struct WKTransferStateRecord *WKTransferState;

extern WKTransferStateType
wkTransferStateGetType (WKTransferState state);

extern WKTransferState // Does not require wkTransferStateRelease
wkTransferStateInit (WKTransferStateType type);

extern WKTransferState // Requires wkTransferStateRelease
wkTransferStateIncludedInit (uint64_t blockNumber,
                             uint64_t transactionIndex,
                             uint64_t timestamp,
                             OwnershipKept WKFeeBasis feeBasis,
                             WKTransferIncludeStatus status);

extern WKTransferState  // Does not require wkTransferStateRelease
wkTransferStateErroredInit (WKTransferSubmitError error);

extern bool
wkTransferStateExtractIncluded (WKTransferState state,
                                uint64_t *blockNumber,
                                uint64_t *blockTimestamp,
                                uint64_t *transactionIndex,
                                WKFeeBasis *feeBasis,
                                WKTransferIncludeStatus *status);

extern bool
wkTransferStateExtractError (WKTransferState state,
                             WKTransferSubmitError *error);

DECLARE_WK_GIVE_TAKE (WKTransferState, wkTransferState);

/// MARK: - Transfer Event

typedef enum {
    /// Signaled when a transfer is *allocated*; the transfer may not, in fact generally is
    /// not, fully initialized.  Thus the transfer should only be used for 'identity' purposes.
    /// The initial TransferStateType could be CREATED, SUBMITTED, INCLUDED, ...
    WK_TRANSFER_EVENT_CREATED,
    
    /// Signaled when a transfer's state change - such as when the state transitions from
    /// SUBMITTED to INCLUDED.  Also signaled if the hash changes; which only applies to
    /// HBAR and only when the transfer is submitted (the HBAR hash depends on what node
    /// processes the transaction).
    WK_TRANSFER_EVENT_CHANGED,
    
    /// For some transfers, the hash is determined when the transfer is signed and submitted.
    /// WK_TRANSFER_EVENT_UPDATED_HASH,
    
    /// Signaled when a transfer is deleted; the transfer must not be 'dereferenced' and thus
    /// the pointer value can be used.  Surely the transfer's memory will be gone by the time
    /// that thread handling the event first sees the deleted transfer.  If any dereference
    /// occurs, the result will be an instant crash.
    WK_TRANSFER_EVENT_DELETED,
} WKTransferEventType;

extern const char *
wkTransferEventTypeString (WKTransferEventType t);

typedef struct {
    WKTransferEventType type;
    union {
        struct {
            WKTransferState old;
            WKTransferState new;
        } state;
    } u;
} WKTransferEvent;

#ifdef __cplusplus
}
#endif

#endif /* WKTransferEvent_h */
