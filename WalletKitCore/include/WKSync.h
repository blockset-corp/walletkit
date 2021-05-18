//
//  WKSync.h
//  WalletKitCore
//
//  Created by Ed Gamble on 11/27/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKSync_h
#define WKSync_h

#include <stdint.h>
#include "WKBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: Sync Stopped Reason

/**
 * An enumeration of the reasons for stopping a sync
 */
typedef enum {
    WK_SYNC_STOPPED_REASON_COMPLETE,
    WK_SYNC_STOPPED_REASON_REQUESTED,
    WK_SYNC_STOPPED_REASON_UNKNOWN,
    WK_SYNC_STOPPED_REASON_POSIX
} WKSyncStoppedReasonType;

/**
 * A StoppedReason explains the reason for stopping a sync.
 */
typedef struct {
    WKSyncStoppedReasonType type;
    union {
        struct {
            int errnum;
        } posix;
    } u;
} WKSyncStoppedReason;

/**
 * Create a stopped reason as 'complete'
 */
extern WKSyncStoppedReason
wkSyncStoppedReasonComplete(void);

/**
 * Create a stopped reason as 'requested'
 */
extern WKSyncStoppedReason
wkSyncStoppedReasonRequested(void);

/**
 * Create a stopped reason as 'unknown'
 */
extern WKSyncStoppedReason
wkSyncStoppedReasonUnknown(void);

/**
 * Create a stopped reason as a posix error
 */
extern WKSyncStoppedReason
wkSyncStoppedReasonPosix(int errnum);

/**
 * Return a descriptive message as to why the sync stopped.
 *
 *@return the detailed reason as a string or NULL
 */
extern char *
wkSyncStoppedReasonGetMessage(WKSyncStoppedReason *reason);

/// MARK: Sync Mode

/**
 * The modes supported for syncing of a wallet's transactions.  These are the supported modes
 * but they are not necessarily available for any individual WalletKit execution.
 * Specifically, the API_ONLY mode may be supported but if the backend services are not
 * accessible, such as if a device is in 'airplane mode', then API_ONLY will not be available.
 */
typedef enum {
    /**
     * Use the BRD backend for all Core blockchain state.  The BRD backend includes a 'submit
     * transaction' interface.
     */
    WK_SYNC_MODE_API_ONLY,
    
    /**
     * Use the BRD backend for everything other than 'submit transaction'
     */
    WK_SYNC_MODE_API_WITH_P2P_SEND,
    
    /**
     * Use the BRD backend for an initial sync and then, once complete, use P2P.  If a sync
     * has not occurred in a while, use the BRD backend again before using P2P (so as to catch-up
     * quickly)
     */
    WK_SYNC_MODE_P2P_WITH_API_SYNC,
    
    /**
     * Use acomplete block chain sync, even starting at block zero (but usually from a block
     * derived from the accounts `earliestStartTime` (or the BIP-39 introduction block).
     */
    WK_SYNC_MODE_P2P_ONLY
} WKSyncMode;

#define NUMBER_OF_SYNC_MODES    (1 + WK_SYNC_MODE_P2P_ONLY)

extern const char *
wkSyncModeString (WKSyncMode m);

/// MARK: Sync Depth

typedef enum {
    /**
     * Sync from the block height of the last confirmed send transaction.
     */
    WK_SYNC_DEPTH_FROM_LAST_CONFIRMED_SEND,
    
    /**
     * Sync from the block height of the last trusted block; this is dependent on the
     * blockchain and mode as to how it determines trust.
     */
    WK_SYNC_DEPTH_FROM_LAST_TRUSTED_BLOCK,
    
    /**
     * Sync from the block height of the point in time when the account was created.
     */
    WK_SYNC_DEPTH_FROM_CREATION
} WKSyncDepth;

/// The Percent Complete (0...100.0) derived from the last block processed relative to the
/// full block range in a sync.
typedef float WKSyncPercentComplete;

#define AS_WK_SYNC_PERCENT_COMPLETE(number)    ((WKSyncPercentComplete) (number))

#ifdef __cplusplus
}
#endif

#endif /* WKSync_h */
