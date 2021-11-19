//
//  BRPeer.h
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

#ifndef BRPeer_h
#define BRPeer_h

#include "BRBitcoinTransaction.h"
#include "BRBitcoinMerkleBlock.h"
#include "support/BRAddress.h"
#include "support/BRInt.h"
#include "support/BROSCompat.h"
#include <stddef.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SERVICES_NODE_NETWORK 0x01 // services value indicating a node carries full blocks, not just headers
#define SERVICES_NODE_BLOOM   0x04 // BIP111: https://github.com/bitcoin/bips/blob/master/bip-0111.mediawiki
#define SERVICES_NODE_WITNESS 0x08 // BIP144: https://github.com/bitcoin/bips/blob/master/bip-0144.mediawiki
#define SERVICES_NODE_BCASH   0x20 // https://github.com/Bitcoin-UAHF/spec/blob/master/uahf-technical-spec.md
    
#define BR_VERSION "2.1"
#define USER_AGENT "/bread:" BR_VERSION "/"

// explanation of message types at: https://en.bitcoin.it/wiki/Protocol_specification
#define MSG_VERSION     "version"
#define MSG_VERACK      "verack"
#define MSG_ADDR        "addr"
#define MSG_INV         "inv"
#define MSG_GETDATA     "getdata"
#define MSG_NOTFOUND    "notfound"
#define MSG_GETBLOCKS   "getblocks"
#define MSG_GETHEADERS  "getheaders"
#define MSG_TX          "tx"
#define MSG_BLOCK       "block"
#define MSG_HEADERS     "headers"
#define MSG_GETADDR     "getaddr"
#define MSG_MEMPOOL     "mempool"
#define MSG_PING        "ping"
#define MSG_PONG        "pong"
#define MSG_FILTERLOAD  "filterload"
#define MSG_FILTERADD   "filteradd"
#define MSG_FILTERCLEAR "filterclear"
#define MSG_MERKLEBLOCK "merkleblock"
#define MSG_ALERT       "alert"
#define MSG_REJECT      "reject"   // described in BIP61: https://github.com/bitcoin/bips/blob/master/bip-0061.mediawiki
#define MSG_FEEFILTER   "feefilter"// described in BIP133 https://github.com/bitcoin/bips/blob/master/bip-0133.mediawiki

#define REJECT_INVALID     0x10 // transaction is invalid for some reason (invalid signature, output value > input, etc)
#define REJECT_SPENT       0x12 // an input is already spent
#define REJECT_NONSTANDARD 0x40 // not mined/relayed because it is "non-standard" (type or version unknown by server)
#define REJECT_DUST        0x41 // one or more output amounts are below the 'dust' threshold
#define REJECT_LOWFEE      0x42 // transaction does not have enough fee/priority to be relayed or mined

typedef enum {
    BRPeerStatusDisconnected = 0,
    BRPeerStatusConnecting,
    BRPeerStatusConnected
} BRBitcoinPeerStatus;

typedef struct {
    UInt128 address; // IPv6 address of peer
    uint16_t port; // port number for peer connection
    uint64_t services; // bitcoin network services supported by peer
    uint64_t timestamp; // timestamp reported by peer
    uint8_t flags; // scratch variable
} BRBitcoinPeer;

#define BR_PEER_NONE ((const BRPeer) { UINT128_ZERO, 0, 0, 0, 0 })

// NOTE: BRBitcoinPeer functions are not thread-safe

// returns a newly allocated BRBitcoinPeer struct that must be freed by calling btcPeerFree()
BRBitcoinPeer *btcPeerNew(uint32_t magicNumber);

// info is a void pointer that will be passed along with each callback call
// void connected(void *) - called when peer handshake completes successfully
// void disconnected(void *, int) - called when peer connection is closed, error is an errno.h code
// void relayedPeers(void *, const BRPeer[], size_t) - called when an "addr" message is received from peer
// void relayedTx(void *, BRBitcoinTransaction *) - called when a "tx" message is received from peer
// void hasTx(void *, UInt256 txHash) - called when an "inv" message with an already-known tx hash is received from peer
// void rejectedTx(void *, UInt256 txHash, uint8_t) - called when a "reject" message is received from peer
// void relayedBlock(void *, BRMerkleBlock *) - called when a "merkleblock" or "headers" message is received from peer
// void notfound(void *, const UInt256[], size_t, const UInt256[], size_t) - called when "notfound" message is received
// BRBitcoinTransaction *requestedTx(void *, UInt256) - called when "getdata" message with a tx hash is received from peer
// int networkIsReachable(void *) - must return true when networking is available, false otherwise
// void threadCleanup(void *) - called before a thread terminates to faciliate any needed cleanup    
void btcPeerSetCallbacks(BRBitcoinPeer *peer, void *info,
                        void (*connected)(void *info),
                        void (*disconnected)(void *info, int error),
                        void (*relayedPeers)(void *info, const BRBitcoinPeer peers[], size_t peersCount),
                        void (*relayedTx)(void *info, BRBitcoinTransaction *tx),
                        void (*hasTx)(void *info, UInt256 txHash),
                        void (*rejectedTx)(void *info, UInt256 txHash, uint8_t code),
                        void (*relayedBlock)(void *info, BRBitcoinMerkleBlock *block),
                        void (*notfound)(void *info, const UInt256 txHashes[], size_t txCount,
                                         const UInt256 blockHashes[], size_t blockCount),
                        void (*setFeePerKb)(void *info, uint64_t feePerKb),
                        BRBitcoinTransaction *(*requestedTx)(void *info, UInt256 txHash),
                        int (*networkIsReachable)(void *info),
                        void (*threadCleanup)(void *info));

// set earliestKeyTime to wallet creation time in order to speed up initial sync
void btcPeerSetEarliestKeyTime(BRBitcoinPeer *peer, uint32_t earliestKeyTime);

// call this when local best block height changes (helps detect tarpit nodes)
void btcPeerSetCurrentBlockHeight(BRBitcoinPeer *peer, uint32_t currentBlockHeight);

// current connection status
BRBitcoinPeerStatus btcPeerConnectStatus(BRBitcoinPeer *peer);

// open connection to peer and perform handshake
void btcPeerConnect(BRBitcoinPeer *peer);

// close connection to peer
void btcPeerDisconnect(BRBitcoinPeer *peer);

// call this to (re)schedule a disconnect in the given number of seconds, or < 0 to cancel (useful for sync timeout)
void btcPeerScheduleDisconnect(BRBitcoinPeer *peer, double seconds);

// set this to true when wallet addresses need to be added to bloom filter
void btcPeerSetNeedsFilterUpdate(BRBitcoinPeer *peer, int needsFilterUpdate);

// display name of peer address
const char *btcPeerHost(BRBitcoinPeer *peer);

// connected peer version number
uint32_t btcPeerVersion(BRBitcoinPeer *peer);

// connected peer user agent string
const char *btcPeerUserAgent(BRBitcoinPeer *peer);

// best block height reported by connected peer
uint32_t btcPeerLastBlock(BRBitcoinPeer *peer);

// minimum tx fee rate peer will accept
uint64_t btcPeerFeePerKb(BRBitcoinPeer *peer);

// average ping time for connected peer
double btcPeerPingTime(BRBitcoinPeer *peer);

// sends a bitcoin protocol message to peer
void btcPeerSendMessage(BRBitcoinPeer *peer, const uint8_t *msg, size_t msgLen, const char *type);
void btcPeerSendFilterload(BRBitcoinPeer *peer, const uint8_t *filter, size_t filterLen);
void btcPeerSendMempool(BRBitcoinPeer *peer, const UInt256 knownTxHashes[], size_t knownTxCount, void *info,
                        void (*completionCallback)(void *info, int success));
void btcPeerSendGetheaders(BRBitcoinPeer *peer, const UInt256 locators[], size_t locatorsCount, UInt256 hashStop);
void btcPeerSendGetblocks(BRBitcoinPeer *peer, const UInt256 locators[], size_t locatorsCount, UInt256 hashStop);
void btcPeerSendInv(BRBitcoinPeer *peer, const UInt256 txHashes[], size_t txCount);
void btcPeerSendGetdata(BRBitcoinPeer *peer, const UInt256 txHashes[], size_t txCount, const UInt256 blockHashes[],
                        size_t blockCount);
void btcPeerSendGetaddr(BRBitcoinPeer *peer);
void btcPeerSendPing(BRBitcoinPeer *peer, void *info, void (*pongCallback)(void *info, int success));

// useful to get additional tx after a bloom filter update
void btcPeerRerequestBlocks(BRBitcoinPeer *peer, UInt256 fromBlock);

// returns a hash value for peer suitable for use in a hashtable
inline static size_t btcPeerHash(const void *peer)
{
    uint32_t address = ((const BRBitcoinPeer *)peer)->address.u32[3], port = ((const BRBitcoinPeer *)peer)->port;
 
    // (((FNV_OFFSET xor address)*FNV_PRIME) xor port)*FNV_PRIME
    return (size_t)((((0x811C9dc5 ^ address)*0x01000193) ^ port)*0x01000193);
}

// true if a and b have the same address and port
inline static int btcPeerEq(const void *peer, const void *otherPeer)
{
    return (peer == otherPeer ||
            (UInt128Eq(((const BRBitcoinPeer *)peer)->address, ((const BRBitcoinPeer *)otherPeer)->address) &&
             ((const BRBitcoinPeer *)peer)->port == ((const BRBitcoinPeer *)otherPeer)->port));
}

// frees memory allocated for peer
void btcPeerFree(BRBitcoinPeer *peer);

#ifdef __cplusplus
}
#endif

#endif // BRPeer_h
