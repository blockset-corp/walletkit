//
//  BRCryptoTransferEvent.h
//  BRCore
//
//  Created by Ed Gamble on 8/12/20.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoTransferEvent_h
#define BRCryptoTransferEvent_h

#include "BRCryptoBase.h"
#include "BRCryptoFeeBasis.h"

#ifdef __cplusplus
extern "C" {
#endif

    /// MARK: Transfer Submission Result

    typedef enum {
        CRYPTO_TRANSFER_SUBMIT_ERROR_UNKNOWN,
        CRYPTO_TRANSFER_SUBMIT_ERROR_POSIX,
    } BRCryptoTransferSubmitErrorType;

    typedef struct {
        BRCryptoTransferSubmitErrorType type;
        union {
            struct {
                int errnum;
            } posix;
        } u;
    } BRCryptoTransferSubmitError;

    extern BRCryptoTransferSubmitError
    cryptoTransferSubmitErrorUnknown(void);

    extern BRCryptoTransferSubmitError
    cryptoTransferSubmitErrorPosix(int errnum);

    extern bool
    cryptoTransferSubmitErrorIsEqual (const BRCryptoTransferSubmitError *e1,
                                      const BRCryptoTransferSubmitError *e2);
    /**
     * Return a descriptive message as to why the error occurred.
     *
     *@return the detailed reason as a string or NULL
     */
    extern char *
    cryptoTransferSubmitErrorGetMessage(BRCryptoTransferSubmitError *e);

    /// MARK: - Transfer State

    typedef enum {
        CRYPTO_TRANSFER_STATE_CREATED,
        CRYPTO_TRANSFER_STATE_SIGNED,
        CRYPTO_TRANSFER_STATE_SUBMITTED,
        CRYPTO_TRANSFER_STATE_INCLUDED,
        CRYPTO_TRANSFER_STATE_ERRORED,
        CRYPTO_TRANSFER_STATE_DELETED,
    } BRCryptoTransferStateType;

    extern const char *
    cryptoTransferStateTypeString (BRCryptoTransferStateType type);

    typedef struct BRCryptoTransferStateRecord *BRCryptoTransferState;

    extern BRCryptoTransferStateType
    cryptoTransferStateGetType (BRCryptoTransferState state);

    extern BRCryptoTransferState // Does not require cryptoTransferStateRelease
    cryptoTransferStateInit (BRCryptoTransferStateType type);

    extern BRCryptoTransferState // Requires cryptoTransferStateRelease
    cryptoTransferStateIncludedInit (uint64_t blockNumber,
                                     uint64_t transactionIndex,
                                     uint64_t timestamp,
                                     OwnershipKept BRCryptoFeeBasis feeBasis,
                                     BRCryptoBoolean success,
                                     const char *error);

    extern BRCryptoTransferState  // Does not require cryptoTransferStateRelease
    cryptoTransferStateErroredInit (BRCryptoTransferSubmitError error);

    extern bool
    cryptoTransferStateExtractIncluded (BRCryptoTransferState state,
                                        uint64_t *blockNumber,
                                        uint64_t *blockTimestamp,
                                        uint64_t *transactionIndex,
                                        BRCryptoFeeBasis *feeBasis,
                                        BRCryptoBoolean  *success,
                                        char **error);

    extern bool
    cryptoTransferStateExtractError (BRCryptoTransferState state,
                                     BRCryptoTransferSubmitError *error);

    DECLARE_CRYPTO_GIVE_TAKE (BRCryptoTransferState, cryptoTransferState);

    /// MARK: - Transfer Event

    typedef enum {
        /// Signaled when a transfer is *allocated*; the transfer may not, in fact generally is
        /// not, fully initialized.  Thus the transfer should only be used for 'identity' purposes.
        /// The initial TransferStateType could be CREATED, SUBMITTED, INCLUDED, ...
        CRYPTO_TRANSFER_EVENT_CREATED,

        /// Signaled when a transfer's state change - such as when the state transitions from
        /// SUBMITTED to INCLUDED.  Also signaled if the hash changes; which only applies to
        /// HBAR and only when the transfer is submitted (the HBAR hash depends on what node
        /// processes the transaction).
        CRYPTO_TRANSFER_EVENT_CHANGED,

        /// For some transfers, the hash is determined when the transfer is signed and submitted.
        /// CRYPTO_TRANSFER_EVENT_UPDATED_HASH,

        /// Signaled when a transfer is deleted; the transfer must not be 'dereferenced' and thus
        /// the pointer value can be used.  Surely the transfer's memory will be gone by the time
        /// that thread handling the event first sees the deleted transfer.  If any dereference
        /// occurs, the result will be an instant crash.
        CRYPTO_TRANSFER_EVENT_DELETED,
    } BRCryptoTransferEventType;

    extern const char *
    cryptoTransferEventTypeString (BRCryptoTransferEventType t);

    typedef struct {
        BRCryptoTransferEventType type;
        union {
            struct {
                BRCryptoTransferState old;
                BRCryptoTransferState new;
            } state;
        } u;
    } BRCryptoTransferEvent;

 #ifdef __cplusplus
}
#endif

#endif /* BRCryptoTransferEvent_h */
