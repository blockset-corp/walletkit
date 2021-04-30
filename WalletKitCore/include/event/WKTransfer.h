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

    /// MARK: Transfer Submission Result

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
                                     WKBoolean success,
                                     const char *error);

    extern WKTransferState  // Does not require wkTransferStateRelease
    wkTransferStateErroredInit (WKTransferSubmitError error);

    extern bool
    wkTransferStateExtractIncluded (WKTransferState state,
                                        uint64_t *blockNumber,
                                        uint64_t *blockTimestamp,
                                        uint64_t *transactionIndex,
                                        WKFeeBasis *feeBasis,
                                        WKBoolean  *success,
                                        char **error);

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
