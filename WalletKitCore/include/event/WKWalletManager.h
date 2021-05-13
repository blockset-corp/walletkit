//
//  WKWalletManagerEvent.h
//  WalletKitCore
//
//  Created by Ed Gamble on 8/12/20.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKWalletManagerEvent_h
#define WKWalletManagerEvent_h

#include "WKBase.h"
#include "WKSync.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: - Wallet Manager Disconnect Reason

typedef enum {
    WK_WALLET_MANAGER_DISCONNECT_REASON_REQUESTED,
    WK_WALLET_MANAGER_DISCONNECT_REASON_UNKNOWN,
    WK_WALLET_MANAGER_DISCONNECT_REASON_POSIX
} WKWalletManagerDisconnectReasonType;

typedef struct {
    WKWalletManagerDisconnectReasonType type;
    union {
        struct {
            int errnum;
        } posix;
    } u;
} WKWalletManagerDisconnectReason;

extern WKWalletManagerDisconnectReason
wkWalletManagerDisconnectReasonRequested (void);

extern WKWalletManagerDisconnectReason
wkWalletManagerDisconnectReasonUnknown (void);

extern WKWalletManagerDisconnectReason
wkWalletManagerDisconnectReasonPosix (int errnum);

/**
 * Return a descriptive message as to why the disconnect occurred.
 *
 *@return the detailed reason as a string or NULL
 */
extern char *
wkWalletManagerDisconnectReasonGetMessage (WKWalletManagerDisconnectReason *reason);

/// MARK: Wallet Manager Event

typedef enum {
    WK_WALLET_MANAGER_STATE_CREATED,
    WK_WALLET_MANAGER_STATE_DISCONNECTED,
    WK_WALLET_MANAGER_STATE_CONNECTED,
    WK_WALLET_MANAGER_STATE_SYNCING,
    WK_WALLET_MANAGER_STATE_DELETED
} WKWalletManagerStateType;

typedef struct {
    WKWalletManagerStateType type;
    union {
        struct {
            WKWalletManagerDisconnectReason reason;
        } disconnected;
    } u;
} WKWalletManagerState;

extern const WKWalletManagerState WK_WALLET_MANAGER_STATE_CREATED_INIT;

typedef enum {
    WK_WALLET_MANAGER_EVENT_CREATED,
    WK_WALLET_MANAGER_EVENT_CHANGED,
    WK_WALLET_MANAGER_EVENT_DELETED,
    
    WK_WALLET_MANAGER_EVENT_WALLET_ADDED,
    WK_WALLET_MANAGER_EVENT_WALLET_CHANGED,
    WK_WALLET_MANAGER_EVENT_WALLET_DELETED,
    
    // wallet: added, ...
    WK_WALLET_MANAGER_EVENT_SYNC_STARTED,
    WK_WALLET_MANAGER_EVENT_SYNC_CONTINUES,
    WK_WALLET_MANAGER_EVENT_SYNC_STOPPED,
    WK_WALLET_MANAGER_EVENT_SYNC_RECOMMENDED,
    
    WK_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED,
} WKWalletManagerEventType;

extern const char *
wkWalletManagerEventTypeString (WKWalletManagerEventType t);

typedef struct {
    WKWalletManagerEventType type;
    union {
        struct {
            WKWalletManagerState old;
            WKWalletManagerState new;
        } state;
        
        WKWallet wallet;
        
        struct {
            WKTimestamp timestamp;
            WKSyncPercentComplete percentComplete;
        } syncContinues;
        
        struct {
            WKSyncStoppedReason reason;
        } syncStopped;
        
        struct {
            WKSyncDepth depth;
        } syncRecommended;
        
        WKBlockNumber blockHeight;
    } u;
} WKWalletManagerEvent;

#ifdef __cplusplus
}
#endif

#endif /* WKWalletManagerEvent_h */
