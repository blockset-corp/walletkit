//
//  BREthereumNode.h
//  WalletKitCore
//
//  Created by Ed Gamble on 8/13/18.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BR_Ethereum_Node_H
#define BR_Ethereum_Node_H

#include "BREthereumMessage.h"
#include "BREthereumNodeEndpoint.h"
#include "BREthereumProvision.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A Node is a proxy for an Ethereum Node.  This Core Ethereum code uses nodes to connect
 * to Ethereum nodes supporting a light client protocol (for GETH, LESv2; for Parity, PIPv1),
 * to send requests, and to recv respones.
 *
 * The Node interface abstracts over the specific light client protocol (LESv2, PIPv1). Thus,
 * the Node interface provides for Status, Block Header, Block Body, Transaction Receipt,
 * Accounts and others - even though, for example LESv2 and PIPv1 offer very different
 * messages interfaces to send/recv Ethereum data.
 *
 * A Node connects two endpoints - a local one and a remote one.    A Node is not connected
 * unless commanded and if the handshake process succeeds - including capatibility between the
 * local and remote nodes.  Specifically the local and remote nodes must share one of the
 * LESv2 or PIPv1 Ethereum supprotocols.
 *
 * There can be two routes to the remote node - a TCP route and an UDP route; these routes are
 * used for different message types (underlying Node interfaces) - the UDP route only supports
 * Node Discovery; TCP supports other messages types for P2P, ETH, LES and PIP.
 *
 * The connection between local and remote endpoints, whether UDP or TCP, uses a Unix socket
 * for send/recv interactions.  The Node interface allows for a select() call w/ read and write
 * file descriptors.  For a write descriptor, the Node must know if data/messages are pending
 * to be sent to a remote endpoint.
 *
 * When connected, a Node announces the extension to the Ethereum block chain.  As this
 * announcement can occur at any time, once connected the select() read descriptor must be
 * set.
 */
typedef struct BREthereumNodeRecord *BREthereumNode;

/**
 * A Node has a type of either GETH or PARITY.  The node's interface will be implemented
 * according to the particulars of the type.  For GETH, we'll use the LESv2 subprotocol; for
 * Parity we'll use PIPv1.
 */
typedef enum {
    NODE_TYPE_UNKNOWN,
    NODE_TYPE_GETH,
    NODE_TYPE_PARITY
} BREthereumNodeType;

extern const char *
ethNodeTypeGetName (BREthereumNodeType type);

/**
 * A Node has a priority as one of LCL, BRD or DIS.  When comparing nodes, such as for the LES
 * ordering of available nodes, we'll bias the ordering based on this priority.  LCL > BRD > DIS.
 * Note: higher priority corresponds to a lower enum value.
 */
typedef enum {
    NODE_PRIORITY_LCL,
    NODE_PRIORITY_BRD,
    NODE_PRIORITY_DIS,
} BREthereumNodePriority;

/**
 * A Node will callback on: state changes, announcements (of block), and results.
 * The callback includes a User context.
 */
typedef void *BREthereumNodeContext;

typedef void
(*BREthereumNodeCallbackStatus) (BREthereumNodeContext context,
                                 BREthereumNode node,
                                 BREthereumHash headHash,
                                 uint64_t headNumber);

typedef void
(*BREthereumNodeCallbackAnnounce) (BREthereumNodeContext context,
                                   BREthereumNode node,
                                   BREthereumHash headHash,
                                   uint64_t headNumber,
                                   UInt256 headTotalDifficulty,
                                   uint64_t reorgDepth);

typedef void
(*BREthereumNodeCallbackProvide) (BREthereumNodeContext context,
                                  BREthereumNode node,
                                  BREthereumProvisionResult result);

typedef void
(*BREthereumNodeCallbackNeighbor) (BREthereumNodeContext context,
                                   BREthereumNode node,
                                   BRArrayOf(BREthereumDISNeighbor) neighbors);

/// MARK: - LES Node State

typedef enum {
    NODE_AVAILABLE,
    NODE_CONNECTING,
    NODE_CONNECTED,
    NODE_ERROR,
} BREthereumNodeStateType;

typedef enum {
    NODE_ERROR_UNIX,
    NODE_ERROR_DISCONNECT,
    NODE_ERROR_PROTOCOL
} BREthereumNodeErrorType;

typedef enum  {
    NODE_CONNECT_OPEN,

    NODE_CONNECT_AUTH,
    NODE_CONNECT_AUTH_ACK,

    NODE_CONNECT_HELLO,
    NODE_CONNECT_HELLO_ACK,

    NODE_CONNECT_PRE_STATUS_PING_RECV,
    NODE_CONNECT_PRE_STATUS_PONG_SEND,

    NODE_CONNECT_STATUS,
    NODE_CONNECT_STATUS_ACK,

    NODE_CONNECT_PING,
    NODE_CONNECT_PING_ACK,
    NODE_CONNECT_PING_ACK_DISCOVER,
    NODE_CONNECT_PING_ACK_DISCOVER_ACK,
    NODE_CONNECT_DISCOVER,
    NODE_CONNECT_DISCOVER_ACK,
    NODE_CONNECT_DISCOVER_ACK_TOO
} BREthereumNodeConnectType;

typedef enum {
    NODE_PROTOCOL_EXHAUSTED,
    NODE_PROTOCOL_NONSTANDARD_PORT,
    NODE_PROTOCOL_PING_PONG_MISSED,
    NODE_PROTOCOL_UDP_EXCESSIVE_BYTE_COUNT,
    NODE_PROTOCOL_TCP_AUTHENTICATION,
    NODE_PROTOCOL_TCP_HELLO_MISSED,
    NODE_PROTOCOL_TCP_STATUS_MISSED,
    NODE_PROTOCOL_CAPABILITIES_MISMATCH,
    NODE_PROTOCOL_STATUS_MISMATCH,
    NODE_PROTOCOL_RLP_PARSE
} BREthereumNodeProtocolReason;

typedef struct {
    BREthereumNodeStateType type;
    union {
        struct {
            BREthereumNodeConnectType type;
        } connecting;

        struct {
            BREthereumNodeErrorType type;
            union {
                int unx;
                BREthereumP2PDisconnectReason disconnect;
                BREthereumNodeProtocolReason protocol;
            } u;
        } error;
    } u;
} BREthereumNodeState;

/** Fills description, returns description */
extern const char *
ethNodeStateDescribe (const BREthereumNodeState *state,
                   char description[128]);

extern BRRlpItem
ethNodeStateEncode (const BREthereumNodeState *state,
                 BRRlpCoder coder);

extern BREthereumNodeState
ethNodeStateDecode (BRRlpItem item,
                 BRRlpCoder coder);

// connect
// disconnect
// network reachable

/**
 * Create a node
 *
 * @param network
 * @param remote
 * @param remote
 * @param local
 * @param context
 * @param callbackMessage
 * @param callbackStatus
 * @return
 */
extern BREthereumNode // add 'message id offset'?
ethNodeCreate (BREthereumNodePriority priority,
            BREthereumNetwork network,
            OwnershipKept const BREthereumNodeEndpoint local,
            OwnershipGiven BREthereumNodeEndpoint remote,
            BREthereumNodeContext context,
            BREthereumNodeCallbackStatus callbackStatus,
            BREthereumNodeCallbackAnnounce callbackAnnounce,
            BREthereumNodeCallbackProvide callbackProvide,
            BREthereumNodeCallbackNeighbor callbackNeighbor,
            BREthereumBoolean handleSync);

extern void
ethNodeRelease (BREthereumNode node);

static inline void
ethNodeReleaseForSet (void *ignore, void *item) {
    ethNodeRelease ((BREthereumNode) item);
}

extern void
ethNodeClean (BREthereumNode node);

extern BREthereumBoolean
ethNodeUpdatedLocalStatus (BREthereumNode node,
                        BREthereumNodeEndpointRoute route);

extern BREthereumNodeType
ethNodeGetType (BREthereumNode node);

extern BREthereumNodePriority
ethNodeGetPriority (BREthereumNode node);

extern BREthereumNodeState
ethNodeConnect (BREthereumNode node,
             BREthereumNodeEndpointRoute route,
             time_t now);

extern BREthereumNodeState
ethNodeDisconnect (BREthereumNode node,
                BREthereumNodeEndpointRoute route,
                BREthereumNodeState stateToAnnounce,
                BREthereumBoolean returnToAvailable);

extern int
ethNodeUpdateDescriptors (BREthereumNode node,
                       BREthereumNodeEndpointRoute route,
                       fd_set *recv,   // read
                       fd_set *send);  // write

extern BREthereumNodeState
ethNodeProcess (BREthereumNode node,
             BREthereumNodeEndpointRoute route,
             time_t now,
             fd_set *recv,   // read
             fd_set *send);  // write

extern BREthereumBoolean
ethNodeCanHandleProvision (BREthereumNode node,
                        BREthereumProvision provision);

extern void
ethNodeHandleProvision (BREthereumNode node,
                     BREthereumProvision provision);

extern BRArrayOf(BREthereumProvision)
ethNodeUnhandleProvisions (BREthereumNode node);

extern const BREthereumNodeEndpoint
ethNodeGetRemoteEndpoint (BREthereumNode node);

extern const BREthereumNodeEndpoint
ethNodeGetLocalEndpoint (BREthereumNode node);

/** Compare nodes based on their priority and DIS neighbor distance */
extern BREthereumComparison
ethNodeCompare (BREthereumNode node1,
             BREthereumNode node2);

extern int
ethNodeHasState (BREthereumNode node,
              BREthereumNodeEndpointRoute route,
              BREthereumNodeStateType type);

extern BREthereumNodeState
ethNodeGetState (BREthereumNode node,
              BREthereumNodeEndpointRoute route);

extern void
ethNodeSetStateInitial (BREthereumNode node,
                     BREthereumNodeEndpointRoute route,
                     BREthereumNodeState state);

extern BREthereumBoolean
ethNodeGetDiscovered (BREthereumNode node);

extern void
ethNodeSetDiscovered (BREthereumNode node,
                   BREthereumBoolean discovered);

extern BREthereumBoolean
ethNodeHandleTime (BREthereumNode node,
                BREthereumNodeEndpointRoute route,
                time_t now,
                BREthereumBoolean tryPing);

extern size_t
ethNodeHashValue (const void *node);

extern int
ethNodeHashEqual (const void *node1,
               const void *node2);

/// MARK: - Node Message Send/Recv
    
typedef enum {
    NODE_STATUS_SUCCESS,
    NODE_STATUS_ERROR
} BREthereumNodeStatus;

extern BREthereumNodeStatus
ethNodeDiscover (BREthereumNode node,
              const BREthereumNodeEndpoint endpoint);

typedef struct {
    BREthereumNodeStatus status;
    union {
        struct {
            BREthereumMessage message;
        } success;

        struct {
        } error;
    } u;
} BREthereumNodeMessageResult;

extern void
ethNodeShow (BREthereumNode node);

#ifdef __cplusplus
}
#endif

#endif /* BR_Ethereum_Node_H */
