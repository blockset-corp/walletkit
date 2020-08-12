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

    #define CRYPTO_TRANSFER_INCLUDED_ERROR_SIZE     16

    typedef struct {
        BRCryptoTransferStateType type;
        union {
            struct {
                uint64_t blockNumber;
                uint64_t transactionIndex;
                // This is not assuredly the including block's timestamp; it is the transaction's
                // timestamp which varies depending on how the transaction was discovered.
                uint64_t timestamp;
                BRCryptoFeeBasis feeBasis;

                // transfer that has failed can be included too
                BRCryptoBoolean success;
                char error[CRYPTO_TRANSFER_INCLUDED_ERROR_SIZE + 1];
            } included;

            struct {
                BRCryptoTransferSubmitError error;
            } errored;
        } u;
    } BRCryptoTransferState;

    extern BRCryptoTransferState
    cryptoTransferStateInit (BRCryptoTransferStateType type);

    extern BRCryptoTransferState
    cryptoTransferStateIncludedInit (uint64_t blockNumber,
                                     uint64_t transactionIndex,
                                     uint64_t timestamp,
                                     BRCryptoFeeBasis feeBasis,
                                     BRCryptoBoolean success,
                                     const char *error);

    extern BRCryptoTransferState
    cryptoTransferStateErroredInit (BRCryptoTransferSubmitError error);

    extern BRCryptoTransferState
    cryptoTransferStateCopy (BRCryptoTransferState *state);

    extern void
    cryptoTransferStateRelease (BRCryptoTransferState *state);

    /// MARK: - Transfer Event

    typedef enum {
        /// Signaled when a transfer is *allocated*; the transfer may not, in fact generally is
        /// not, fully initialized.  Thus the transfer should only be used for 'identity' purposes.
        CRYPTO_TRANSFER_EVENT_CREATED,

        /// Signaled when a transfer's state change - such as when the state transitions from
        /// SUBMITTED to INCLUDED.
        CRYPTO_TRANSFER_EVENT_CHANGED,

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
