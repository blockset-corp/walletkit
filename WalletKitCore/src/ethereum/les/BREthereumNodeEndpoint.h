//
//  BREthereumNodeEndpoint.h
//  WalletKitCore
//
//  Created by Ed Gamble on 8/14/18.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BR_Ethereum_Node_Endpoint_H
#define BR_Ethereum_Node_Endpoint_H

#include <limits.h>
#include "support/BRInt.h"
#include "support/BRKey.h"
#include "BREthereumMessage.h"
#include "BREthereumLESRandom.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NODE_ROUTE_UDP,
    ETHEREUM_NODE_ROUTE_TCP
} BREthereumNodeEndpointRoute;

#define ETHEREUM_NUMBER_OF_NODE_ROUTES  (1 + ETHEREUM_NODE_ROUTE_TCP)

#define FOR_EACH_ROUTE( route ) \
    for (BREthereumNodeEndpointRoute route = NODE_ROUTE_UDP; route <= ETHEREUM_NODE_ROUTE_TCP; route++)

static inline const char *
ethNodeEndpointRouteGetName (BREthereumNodeEndpointRoute route) {
    return (ETHEREUM_NODE_ROUTE_TCP == route ? "TCP" : "UDP");
}

/**
 * A Node Endpoint is an IP:PORT pair over which UDP and TCP data transfer can occur.  A Node
 * Endpoint has a socket for each route (UDP or TCP) and one can send or recv data along those
 * routes.
 *
 * A NodeEndpoint has 'DIS Neighbor' which represents the Ethereum DIScovery protocol data
 * associated with this endpoint.  That data includes the 64 byte public key which represents
 * the NodeId
 *
 * A Node Endpoint includes the 'hello' and 'status' P2P messages that are received from this
 * node during the communication handshake.  These messages include information about that node
 * that is used to assess the viability of the node for use as a provier of Light Cliet data.
 * Notably, the 'hello' message includes the protocols support by the endpoint; we require some
 * version of LES (for Geth) or PIP (for Parity).  Additionally, the 'status' message includes
 * `networkId`,  `headNum`, `serveStateSince` and others the need to be assessd.
 *
 * Node Endpoint defines one half of a communction pair; a node includes a 'local' and a 'remote'
 * endpoint.
 */
 typedef struct BREthereumNodeEndpointRecord  *BREthereumNodeEndpoint;

extern BREthereumNodeEndpoint
ethNodeEndpointCreate (BREthereumDISNeighbor dis);

extern BREthereumNodeEndpoint
ethNodeEndpointCreateLocal (BREthereumLESRandomContext randomContext);

/**
 * Create a Node Endpoint from `enode`.  If `enode` cannot be parsed then NULL is returned.
 *
 * @param enode the node endpoint specification
 *
 * @return a new endpoint or NULL
 */
extern BREthereumNodeEndpoint
ethNodeEndpointCreateEnode (const char *enode);

extern void
ethNodeEndpointRelease (OwnershipGiven BREthereumNodeEndpoint endpoint);

extern BREthereumHash
ethNodeEndpointGetHash (BREthereumNodeEndpoint endpoint);

extern const char *
ethNodeEndpointGetHostname (BREthereumNodeEndpoint endpoint);

extern int // remote.dis.node.portTCP)
ethNodeEndpointGetPort (BREthereumNodeEndpoint endpoint,
                     BREthereumNodeEndpointRoute route);

extern int
ethNodeEndpointGetSocket (BREthereumNodeEndpoint endpoint,
                       BREthereumNodeEndpointRoute route);

extern BREthereumDISNeighbor
ethNodeEndpointGetDISNeighbor (BREthereumNodeEndpoint endpoint);

extern BRKey *
ethNodeEndpointGetKey (BREthereumNodeEndpoint endpoint);

extern BRKey *
ethNodeEndpointGetEphemeralKey (BREthereumNodeEndpoint endpoint);

extern UInt256 *
ethNodeEndpointGetNonce (BREthereumNodeEndpoint endpoint);

extern BREthereumP2PMessageHello
ethNodeEndpointGetHello (const BREthereumNodeEndpoint endpoint);

extern void
ethNodeEndpointSetHello (BREthereumNodeEndpoint endpoint,
                      OwnershipGiven BREthereumP2PMessageHello hello);

extern void
ethNodeEndpointDefineHello (BREthereumNodeEndpoint endpoint,
                         const char *name,
                         OwnershipGiven BRArrayOf(BREthereumP2PCapability) capabilities);

extern void
ethNodeEndpointShowHello (BREthereumNodeEndpoint endpoint);

/**
 * Return TRUE if `endpoint` has a capability with `name`
 */
extern BREthereumBoolean
ethNodeEndpointHasHelloCapability (BREthereumNodeEndpoint endpoint,
                                const char *name,
                                uint32_t version);

/**
 * Return the first `source` capability matched by `target`
 */
extern const BREthereumP2PCapability *
ethNodeEndpointHasHelloMatchingCapability (BREthereumNodeEndpoint source,
                                        BREthereumNodeEndpoint target);

extern BREthereumP2PMessageStatus
ethNodeEndpointGetStatus (const BREthereumNodeEndpoint endpoint);

extern void
ethNodeEndpointSetStatus (BREthereumNodeEndpoint endpoint,
                       OwnershipGiven BREthereumP2PMessageStatus status);

extern void
ethNodeEndpointShowStatus (BREthereumNodeEndpoint endpoint);

/// MARK: - Open/Close, Send/Recv

extern int
ethNodeEndpointOpen (BREthereumNodeEndpoint endpoint,
                  BREthereumNodeEndpointRoute route);

extern int
ethNodeEndpointClose (BREthereumNodeEndpoint endpoint,
                   BREthereumNodeEndpointRoute route,
                   int needShutdown);

extern int
ethNodeEndpointIsOpen (const BREthereumNodeEndpoint endpoint,
                    BREthereumNodeEndpointRoute route);

extern int
ethNodeEndpointRecvData (BREthereumNodeEndpoint endpoint,
                      BREthereumNodeEndpointRoute route,
                      uint8_t *bytes,
                      size_t *bytesCount,
                      int needBytesCount);

extern int
ethNodeEndpointSendData (BREthereumNodeEndpoint endpoint,
                      BREthereumNodeEndpointRoute route,
                      uint8_t *bytes,
                      size_t bytesCount);

// Support BRSet
extern size_t
ethNodeEndpointHashValue (const void *h);

// Support BRSet
extern int
ethNodeEndpointHashEqual (const void *h1, const void *h2);

//
// Enodes
//

// Mainnet
extern const char *bootstrapLCLEnodes[];

#ifdef __cplusplus
}
#endif

#endif /* BR_Ethereum_Node_Endpoint_H */
