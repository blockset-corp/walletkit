//
//  BRCryptoWalletManagerEvent.h
//  BRCore
//
//  Created by Ed Gamble on 8/12/20.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoWalletManagerEvent_h
#define BRCryptoWalletManagerEvent_h

#include "BRCryptoBase.h"
#include "BRCryptoSync.h"

#ifdef __cplusplus
extern "C" {
#endif

    /// MARK: - Wallet Manager Disconnect Reason

    typedef enum {
        CRYPTO_WALLET_MANAGER_DISCONNECT_REASON_REQUESTED,
        CRYPTO_WALLET_MANAGER_DISCONNECT_REASON_UNKNOWN,
        CRYPTO_WALLET_MANAGER_DISCONNECT_REASON_POSIX
    } BRCryptoWalletManagerDisconnectReasonType;

    typedef struct {
        BRCryptoWalletManagerDisconnectReasonType type;
        union {
            struct {
                int errnum;
            } posix;
        } u;
    } BRCryptoWalletManagerDisconnectReason;

    extern BRCryptoWalletManagerDisconnectReason
    cryptoWalletManagerDisconnectReasonRequested (void);

    extern BRCryptoWalletManagerDisconnectReason
    cryptoWalletManagerDisconnectReasonUnknown (void);

    extern BRCryptoWalletManagerDisconnectReason
    cryptoWalletManagerDisconnectReasonPosix (int errnum);

    /**
     * Return a descriptive message as to why the disconnect occurred.
     *
     *@return the detailed reason as a string or NULL
     */
    extern char *
    cryptoWalletManagerDisconnectReasonGetMessage (BRCryptoWalletManagerDisconnectReason *reason);

    /// MARK: Wallet Manager Event

    typedef enum {
        CRYPTO_WALLET_MANAGER_STATE_CREATED,
        CRYPTO_WALLET_MANAGER_STATE_DISCONNECTED,
        CRYPTO_WALLET_MANAGER_STATE_CONNECTED,
        CRYPTO_WALLET_MANAGER_STATE_SYNCING,
        CRYPTO_WALLET_MANAGER_STATE_DELETED
    } BRCryptoWalletManagerStateType;

    typedef struct {
        BRCryptoWalletManagerStateType type;
        union {
            struct {
                BRCryptoWalletManagerDisconnectReason reason;
            } disconnected;
        } u;
    } BRCryptoWalletManagerState;

    extern const BRCryptoWalletManagerState CRYPTO_WALLET_MANAGER_STATE_CREATED_INIT;

    typedef enum {
        CRYPTO_WALLET_MANAGER_EVENT_CREATED,
        CRYPTO_WALLET_MANAGER_EVENT_CHANGED,
        CRYPTO_WALLET_MANAGER_EVENT_DELETED,

        CRYPTO_WALLET_MANAGER_EVENT_WALLET_ADDED,
        CRYPTO_WALLET_MANAGER_EVENT_WALLET_CHANGED,
        CRYPTO_WALLET_MANAGER_EVENT_WALLET_DELETED,

        // wallet: added, ...
        CRYPTO_WALLET_MANAGER_EVENT_SYNC_STARTED,
        CRYPTO_WALLET_MANAGER_EVENT_SYNC_CONTINUES,
        CRYPTO_WALLET_MANAGER_EVENT_SYNC_STOPPED,
        CRYPTO_WALLET_MANAGER_EVENT_SYNC_RECOMMENDED,

        CRYPTO_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED,
    } BRCryptoWalletManagerEventType;

    extern const char *
    cryptoWalletManagerEventTypeString (BRCryptoWalletManagerEventType t);

    typedef struct {
        BRCryptoWalletManagerEventType type;
        union {
            struct {
                BRCryptoWalletManagerState old;
                BRCryptoWalletManagerState new;
            } state;

            BRCryptoWallet wallet;

            struct {
                BRCryptoTimestamp timestamp;
                BRCryptoSyncPercentComplete percentComplete;
            } syncContinues;

            struct {
                BRCryptoSyncStoppedReason reason;
            } syncStopped;

            struct {
                BRCryptoSyncDepth depth;
            } syncRecommended;

            BRCryptoBlockNumber blockHeight;
        } u;
    } BRCryptoWalletManagerEvent;

 #ifdef __cplusplus
}
#endif

#endif /* BRCryptoWalletManagerEvent_h */
