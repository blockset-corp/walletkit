//
//  BRPeerManager.h
//
//  Created by Aaron Voisine on 9/2/15.
//  Copyright (c) 2015 breadwallet LLC.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#ifndef BRPeerManager_h
#define BRPeerManager_h

#include "BRBitcoinPeer.h"
#include "BRBitcoinMerkleBlock.h"
#include "BRBitcoinTransaction.h"
#include "BRBitcoinWallet.h"
#include "BRBitcoinChainParams.h"
#include <stddef.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PEER_MAX_CONNECTIONS 3

typedef struct BRBitcoinPeerManagerStruct BRBitcoinPeerManager;

// returns a newly allocated BRPeerManager struct that must be freed by calling btcPeerManagerFree()
BRBitcoinPeerManager *btcPeerManagerNew(const BRBitcoinChainParams *params, BRBitcoinWallet *wallet, uint32_t earliestKeyTime,
                                BRBitcoinMerkleBlock *blocks[], size_t blocksCount, const BRBitcoinPeer peers[], size_t peersCount);

// not thread-safe, set callbacks once before calling btcPeerManagerConnect()
// info is a void pointer that will be passed along with each callback call
// void syncStarted(void *) - called when blockchain syncing starts
// void syncStopped(void *, int) - called when blockchain syncing stops, error is an errno.h code
// void txStatusUpdate(void *) - called when transaction status may have changed such as when a new block arrives
// void saveBlocks(void *, int, BRMerkleBlock *[], size_t) - called when blocks should be saved to the persistent store
// - if replace is true, remove any previously saved blocks first
// void savePeers(void *, int, const BRPeer[], size_t) - called when peers should be saved to the persistent store
// - if replace is true, remove any previously saved peers first
// int networkIsReachable(void *) - must return true when networking is available, false otherwise
// void threadCleanup(void *) - called before a thread terminates to faciliate any needed cleanup
void btcPeerManagerSetCallbacks(BRBitcoinPeerManager *manager, void *info,
                               void (*syncStarted)(void *info),
                               void (*syncStopped)(void *info, int error),
                               void (*txStatusUpdate)(void *info),
                               void (*saveBlocks)(void *info, int replace, BRBitcoinMerkleBlock *blocks[], size_t blocksCount),
                               void (*savePeers)(void *info, int replace, const BRBitcoinPeer peers[], size_t peersCount),
                               int (*networkIsReachable)(void *info),
                               void (*threadCleanup)(void *info));

// specifies a single fixed peer to use when connecting to the bitcoin network
// set address to UINT128_ZERO to revert to default behavior
void btcPeerManagerSetFixedPeer(BRBitcoinPeerManager *manager, UInt128 address, uint16_t port);

// current connect status
BRBitcoinPeerStatus btcPeerManagerConnectStatus(BRBitcoinPeerManager *manager);

// connect to bitcoin peer-to-peer network (also call this whenever networkIsReachable() status changes)
void btcPeerManagerConnect(BRBitcoinPeerManager *manager);

// disconnect from bitcoin peer-to-peer network (may cause syncFailed(), saveBlocks() or savePeers() callbacks to fire)
void btcPeerManagerDisconnect(BRBitcoinPeerManager *manager);

// rescans blocks and transactions after earliestKeyTime (a new random download peer is also selected due to the
// possibility that a malicious node might lie by omitting transactions that match the bloom filter)
void btcPeerManagerRescan(BRBitcoinPeerManager *manager);

// rescans blocks and transactions after the last hardcoded checkpoint (uses a new random download peer, see above comment)
void btcPeerManagerRescanFromLastHardcodedCheckpoint(BRBitcoinPeerManager *manager);

// rescans blocks and transactions from after the blockNumber.  If blockNumber is not known, then
// rescan from the just prior checkpoint (uses a new random download peer, see above comment).
void btcPeerManagerRescanFromBlockNumber(BRBitcoinPeerManager *manager, uint32_t blockNumber);

// the (unverified) best block height reported by connected peers
uint32_t btcPeerManagerEstimatedBlockHeight(BRBitcoinPeerManager *manager);

// current proof-of-work verified best block height
uint32_t btcPeerManagerLastBlockHeight(BRBitcoinPeerManager *manager);

// current proof-of-work verified best block timestamp (time interval since unix epoch)
uint32_t btcPeerManagerLastBlockTimestamp(BRBitcoinPeerManager *manager);

// current network sync progress from 0 to 1
// startHeight is the block height of the most recent fully completed sync
double btcPeerManagerSyncProgress(BRBitcoinPeerManager *manager, uint32_t startHeight);

// returns the number of currently connected peers
size_t btcPeerManagerPeerCount(BRBitcoinPeerManager *manager);

// description of the peer most recently used to sync blockchain data
const char *btcPeerManagerDownloadPeerName(BRBitcoinPeerManager *manager);

// publishes tx to bitcoin network (do not call btcTransactionFree() on tx afterward)
void btcPeerManagerPublishTx(BRBitcoinPeerManager *manager, BRBitcoinTransaction *tx, void *info,
                            void (*callback)(void *info, int error));

// number of connected peers that have relayed the given unconfirmed transaction
size_t btcPeerManagerRelayCount(BRBitcoinPeerManager *manager, UInt256 txHash);

// return the BRBitcoinChainParams used to create this peer manager
const BRBitcoinChainParams *btcPeerManagerChainParams(BRBitcoinPeerManager *manager);

// frees memory allocated for manager (call btcPeerManagerDisconnect() first if connected)
void btcPeerManagerFree(BRBitcoinPeerManager *manager);

#ifdef __cplusplus
}
#endif

#endif // BRPeerManager_h
