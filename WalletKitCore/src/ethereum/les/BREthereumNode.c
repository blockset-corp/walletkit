//
//  BREthereumNode.c
//  WalletKitCore
//
//  Created by Ed Gamble on 8/13/18.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include "support/BRCrypto.h"
#include "support/BRKeyECIES.h"
#include "support/BRAssert.h"
#include "BREthereumNode.h"
#include "BREthereumLESFrameCoder.h"
#include "ethereum/util/BREthereumLog.h"

/// MARK: - Forward Declarations

// #define NODE_SHOW_RLP_ITEMS
// #define NODE_SHOW_RECV_RLP_ITEMS
// #define NODE_SHOW_SEND_RLP_ITEMS

#define NODE_SHOW_SEND_TX_ALWAYS

// #define NEED_TO_PRINT_SEND_RECV_DATA
// #define NEED_TO_AVOID_PROOFS_LOGGING

// Chose above; leave the following
#if defined (NODE_SHOW_RLP_ITEMS)
#define NODE_SHOW_RECV_RLP_ITEMS
#define NODE_SHOW_SEND_RLP_ITEMS
#endif

#undef NODE_DEBUG_SOCKETS

#if defined (NODE_DEBUG_SOCKETS)
static int socketOpenCount = 0;
#endif

static BREthereumNodeState
ethNodeStateAnnounce (BREthereumNode node,
                   BREthereumNodeEndpointRoute route,
                   BREthereumNodeState state);

static size_t
ethNodeGetThenIncrementMessageIdentifier (BREthereumNode node,
                                       size_t byIncrement);

static BREthereumNodeMessageResult
ethNodeRecv (BREthereumNode node,
          BREthereumNodeEndpointRoute route);

static BREthereumNodeStatus
ethNodeSend (BREthereumNode node,
          BREthereumNodeEndpointRoute route,
          BREthereumMessage message);   // BRRlpData/BRRlpItem *optionalMessageData/Item

#define DEFAULT_SEND_DATA_BUFFER_SIZE    (16 * 1024)
#define DEFAULT_RECV_DATA_BUFFER_SIZE    (64 * 1024)

#define DEFAULT_NODE_TIMEOUT_IN_SECONDS       (10)
#define DEFAULT_NODE_TIMEOUT_IN_SECONDS_RECV  (60)      // 1 minute

//
// Frame Coder Stuff
//
#define SIG_SIZE_BYTES      65
#define PUBLIC_SIZE_BYTES   64
#define HEPUBLIC_BYTES      32
#define NONCE_BYTES         32

#define AUTH_BUF_LEN        (SIG_SIZE_BYTES + HEPUBLIC_BYTES + PUBLIC_SIZE_BYTES + NONCE_BYTES + 1)
#define AUTH_CIPHER_BUF_LEN (AUTH_BUF_LEN + 65 + 16 + 32)

#define ACK_BUF_LEN         (PUBLIC_SIZE_BYTES + NONCE_BYTES + 1)
#dwfine ACK_CIPHER_BUF_LEN  (ACK_BUF_LEN + 65 + 16 + 32)

//
static int _sendAuthInitiator(BREthereumNode node);
static int _readAuthAckFromRecipient(BREthereumNode node);

/// MARK: - Node Type

extern const char *
ethNodeTypeGetName (BREthereumNodeType type) {
    static const char *nodeTypeNames[] = {
        "Unknown",
        "Geth",
        "Parity"
    };
    return nodeTypeNames[type];
}

/// MARK: - Node State Create ...

static inline BREthereumNodeState
ethNodeStateCreate (BREthereumNodeStateType type) {
    return (BREthereumNodeState) { type };
}

static BREthereumNodeState
ethNodeStateCreateConnecting (BREthereumNodeConnectType type) {
    return (BREthereumNodeState) {
        NODE_CONNECTING,
        { .connecting = { type }}
    };
}

static BREthereumNodeState
ethNodeStateCreateConnected (void) {
    return ethNodeStateCreate (NODE_CONNECTED);
}

static BREthereumNodeState
ethNodeStateCreateErrorUnix (int error) {
    return (BREthereumNodeState) {
        NODE_ERROR,
        { .error = {
            NODE_ERROR_UNIX,
            { .unx = error }}}
    };
}

static BREthereumNodeState
ethNodeStateCreateErrorDisconnect (BREthereumP2PDisconnectReason reason) {
    return (BREthereumNodeState) {
        NODE_ERROR,
        { .error = {
            NODE_ERROR_DISCONNECT,
            { .disconnect = reason }}}
    };
}

static BREthereumNodeState
ethNodeStateCreateErrorProtocol (BREthereumNodeProtocolReason reason) {
    return (BREthereumNodeState) {
        NODE_ERROR,
        { .error = {
            NODE_ERROR_PROTOCOL,
            { .protocol = reason }}}
    };
}

const char *
ethNodeProtocolReasonDescription (BREthereumNodeProtocolReason reason) {
    static const char *
    protocolReasonDescriptions [] = {
        "Exhausted",
        "Non-Standard Port",
        "Ping_Pong Missed",
        "UDP Excessive Byte Count",
        "TCP Authentication",
        "TCP Hello Missed",
        "TCP Status Missed",
        "Capabilities Mismatch",
        "Status Mismatch",
        "RLP Parse"
    };
    return protocolReasonDescriptions [reason];
}

static const char *
ethNodeConnectTypeDescription (BREthereumNodeConnectType type) {
    static const char *
    connectTypeDescriptions [] = {
        "Open",
        "Auth",
        "Auth Ack",
        "Hello",
        "Hello Ack",
        "PreStatus Ping Recv",
        "PreStatus Pong Send",
        "Status",
        "Status Ack",
        "Ping",
        "Ping Ack",
        "Ping Ack Discover",
        "Ping Ack Discover Ack",
        "Discover",
        "Discover Ack"
    };
    return connectTypeDescriptions [type];
}

extern const char *
ethNodeStateDescribe (const BREthereumNodeState *state,
                   char description[128]) {
    switch (state->type) {
        case NODE_AVAILABLE:  return strcpy (description, "Available");
        case NODE_CONNECTING: return strcat (strcpy (description, "Connecting: "),
                                             ethNodeConnectTypeDescription(state->u.connecting.type));
        case NODE_CONNECTED:  return strcpy (description, "Connected");
        case NODE_ERROR:
            switch (state->u.error.type) {
                case NODE_ERROR_UNIX:
                    return strcat (strcpy (description, "Unix: "),
                                   strerror (state->u.error.u.unx));
                case NODE_ERROR_DISCONNECT:
                    return strcat (strcpy (description, "Disconnect : "),
                                   messageP2PDisconnectDescription(state->u.error.u.disconnect));
                case NODE_ERROR_PROTOCOL:
                    return strcat (strcpy (description, "Protocol  : "),
                                   ethNodeProtocolReasonDescription(state->u.error.u.protocol));
                    break;
            }
    }
}

extern BRRlpItem
ethNodeStateEncode (const BREthereumNodeState *state,
                 BRRlpCoder coder) {
    BRRlpItem typeItem = rlpEncodeUInt64 (coder, state->type, 0);

    switch (state->type) {
        case NODE_AVAILABLE:
        case NODE_CONNECTING:
        case NODE_CONNECTED:
            return rlpEncodeList1 (coder, typeItem);
        case NODE_ERROR: {
            BRRlpItem reasonItem;
            switch (state->u.error.type) {
                case NODE_ERROR_UNIX:
                    reasonItem = rlpEncodeUInt64(coder, (uint64_t) state->u.error.u.unx, 1);
                    break;
                case NODE_ERROR_DISCONNECT:
                    reasonItem = rlpEncodeUInt64(coder, state->u.error.u.disconnect, 1);
                    break;
                case NODE_ERROR_PROTOCOL:
                    reasonItem = rlpEncodeUInt64(coder, state->u.error.u.protocol, 1);
                    break;
            }

            return rlpEncodeList (coder, 3,
                                  typeItem,
                                  rlpEncodeUInt64(coder, state->u.error.type, 1),
                                  reasonItem);
            }
    }
}

extern BREthereumNodeState
ethNodeStateDecode (BRRlpItem item,
                 BRRlpCoder coder) {
    size_t itemsCount = 0;
    const BRRlpItem *items = rlpDecodeList (coder, item, &itemsCount);
    assert (1 == itemsCount || 3 == itemsCount);

    BREthereumNodeStateType type = (BREthereumNodeStateType) rlpDecodeUInt64(coder, items[0], 0);
    switch (type) {
        case NODE_AVAILABLE:
        case NODE_CONNECTING:
        case NODE_CONNECTED:
            return ethNodeStateCreate(type);
        case NODE_ERROR: {
            BREthereumNodeErrorType errorType = (BREthereumNodeErrorType) rlpDecodeUInt64 (coder, items[1], 0);
            switch (errorType) {
                case NODE_ERROR_UNIX:
                    return ethNodeStateCreateErrorUnix((int) rlpDecodeUInt64 (coder, items[1], 0));
                case NODE_ERROR_DISCONNECT:
                    return ethNodeStateCreateErrorDisconnect((BREthereumP2PDisconnectReason) rlpDecodeUInt64 (coder, items[1], 0));
                case NODE_ERROR_PROTOCOL:
                    return ethNodeStateCreateErrorProtocol((BREthereumNodeProtocolReason) rlpDecodeUInt64 (coder, items[1], 0));
            }
        }
    }
}

/// MARK: - Node Provisioner

/**
 * A Node Provisioner completes a Provision by dispatching messages, possibly multiple
 * messages, to fill the provision.  The number of messages dispatched depends on the type of the
 * message and the content requests.  For example, if 192 block bodies are requested but a block
 * bodies' LES message only accepts at most 64 hashes, then 3 messages will be created, each with
 * 64 hashes, to complete the provision of 192 headers.  Only when all 192 headers are received
 * will the provisioner be complete.
 */
typedef struct {
    /** The provision as a union of {reqeust, response} for each provision type. */
    BREthereumProvision provision;

    /** The node handling this provision.  How the provision is completed is determined by this
     * node; notably, different messages are sent based on if the node is for GETH or PARITY */
    BREthereumNode node;

    /** The base message identifier.  If the provision applies to multiple messages, then
     * the messages identifers will be sequential starting at this identifier */
    size_t messageIdentifier;

    /** The count of messages */
    size_t messagesCount;

    /** The limit for each message.  When constructing the 'response' from a set of messages
     * we'll expect eash message to have this many individual responses (except for the last
     * message which may have fewer). */
    size_t messageContentLimit;

    /** The count of messages remaining to be sent */
    size_t messagesRemainingCount;

    /** The count of messages received */
    size_t messagesReceivedCount;

    /** Time of creation */
    long timestamp;

    BREthereumProvisionStatus status;

    /** The messages needed to complete the provision.  These may be LES (for GETH) or PIP (for
     * Parity) messages. */
    BRArrayOf(BREthereumMessage) messages;

} BREthereumNodeProvisioner;

static int
provisionerSendMessagesPending (BREthereumNodeProvisioner *provisioner) {
    return provisioner->messagesRemainingCount > 0;
}

static int
provisionerRecvMessagesPending (BREthereumNodeProvisioner *provisioner) {
    return provisioner->messagesReceivedCount < provisioner->messagesCount;
}

static int
provisionerMessageOfInterest (BREthereumNodeProvisioner *provisioner,
                              uint64_t messageIdentifier) {
    return (provisioner->messageIdentifier <= messageIdentifier &&
            messageIdentifier < (provisioner->messageIdentifier + provisioner->messagesCount));
}

static BREthereumNodeStatus
provisionerMessageSend (BREthereumNodeProvisioner *provisioner) {
    BREthereumMessage message = provisioner->messages [provisioner->messagesCount -
                                                       provisioner->messagesRemainingCount];
    BREthereumNodeStatus status = ethNodeSend (provisioner->node, ETHEREUM_NODE_ROUTE_TCP, message);
    provisioner->messagesRemainingCount--;

    return status;
}

static size_t
provisionerGetCount (BREthereumNodeProvisioner *provisioner) {
    switch (provisioner->provision.type) {
        case PROVISION_BLOCK_HEADERS:
            return provisioner->provision.u.headers.limit;
        case PROVISION_BLOCK_PROOFS:
            return array_count (provisioner->provision.u.proofs.numbers);
        case PROVISION_BLOCK_BODIES:
            return array_count (provisioner->provision.u.bodies.hashes);
        case PROVISION_TRANSACTION_RECEIPTS:
            return array_count (provisioner->provision.u.receipts.hashes);
        case PROVISION_ACCOUNTS:
            return array_count (provisioner->provision.u.accounts.hashes);
        case PROVISION_TRANSACTION_STATUSES:
            return array_count (provisioner->provision.u.statuses.hashes);
        case PROVISION_SUBMIT_TRANSACTION:
            // We'll submit the transaction and then query it's status.  We'll only expect
            // one response.. which makes this different from all the other messages and thus
            // see how provisioner->messagesReceivedCount is handled in `provisionerEstablish()`.
            return 2;
    }
}

static size_t
provisionerGetMessageContentLimit (BREthereumNodeProvisioner *provisioner) {
    assert (NULL != provisioner->node);

    switch (ethNodeGetType(provisioner->node)) {
        case NODE_TYPE_UNKNOWN:
            assert (0);
        case NODE_TYPE_GETH: {
            BREthereumLESMessageIdentifier id = ethProvisionGetMessageLESIdentifier(provisioner->provision.type);
            return messageLESSpecs[id].limit;
        }
        case NODE_TYPE_PARITY:
            // The Parity code seems to have this implicit limit.
            return 256;
    }
}

static void
provisionerEstablish (BREthereumNodeProvisioner *provisioner,
                      BREthereumNode node) {
    // The `node` will handle the `provisioner`
    provisioner->node = node;

    // A message of `type` is limited to this number 'requests'
    provisioner->messageContentLimit = provisionerGetMessageContentLimit (provisioner);
    assert (0 != provisioner->messageContentLimit);

    // We'll need this many messages to handle all the 'requests'
    provisioner->messagesCount = (PROVISION_SUBMIT_TRANSACTION == provisioner->provision.type
                                  ? provisionerGetCount (provisioner)
                                  : (provisionerGetCount (provisioner) + provisioner->messageContentLimit - 1) / provisioner->messageContentLimit);

    // Set the `messageIdentifier` and the `messagesRemainingCount` given the `messagesCount`
    provisioner->messageIdentifier = ethNodeGetThenIncrementMessageIdentifier (node, provisioner->messagesCount);
    provisioner->messagesRemainingCount = provisioner->messagesCount;

    // For SUBMIT_TRANSACTION we send two messages but only expect one back; so, we increment
    // received count it make it look like one already arrived.
    provisioner->messagesReceivedCount  = (PROVISION_SUBMIT_TRANSACTION == provisioner->provision.type
                                           ? 1
                                           : 0);

    provisioner->status = PROVISION_SUCCESS;

    // Create the messages, or just one, needed to complete the provision
    array_new (provisioner->messages, provisioner->messagesCount);

    // Add each message, constructed from the provision
    for (size_t index = 0; index < provisioner->messagesCount; index++)
        array_add (provisioner->messages, ethProvisionCreateMessage (&provisioner->provision,
                                                                  (NODE_TYPE_GETH == ethNodeGetType(node)
                                                                   ? MESSAGE_LES
                                                                   : MESSAGE_PIP),
                                                                  provisioner->messageContentLimit,
                                                                  provisioner->messageIdentifier,
                                                                  index));
}

static void
provisionerHandleMessage (BREthereumNodeProvisioner *provisioner,
                          OwnershipGiven BREthereumMessage message) {
    BREthereumProvisionStatus status = ethProvisionHandleMessage (&provisioner->provision,
                                                               message,
                                                               provisioner->messageContentLimit,
                                                               provisioner->messageIdentifier);
    // Update the provision status on error
    if (PROVISION_ERROR == status)
        provisioner->status = PROVISION_ERROR;

    // We've processed another message;
    provisioner->messagesReceivedCount++;
}

static void
provisionerRelease (BREthereumNodeProvisioner *provisioner,
                    BREthereumBoolean releaseProvision,
                    BREthereumBoolean releaseProvisionResults) {
    messagesRelease(provisioner->messages);
    if (ETHEREUM_BOOLEAN_IS_TRUE(releaseProvision))
        ethProvisionRelease (&provisioner->provision, releaseProvisionResults);
}

/// MARK: - LES Node

struct BREthereumNodeRecord {
    // Must be first to support BRSet.
    /**
     * The identifer is the hash of the remote node endpoing.
     */
    BREthereumHash hash;

    /** The type as GETH or PARITY (only GETH supported) */
    BREthereumNodeType type;

    /** The priority as DIS, BRD or LCL */
    BREthereumNodePriority priority;

    /** The states by route; one for UDP and one for TCP */
    BREthereumNodeState states[ETHEREUM_NUMBER_OF_NODE_ROUTES];

    // The endpoints connected by this node.  The `local` endpoint is never owned by `Node`
    // whereas `remote` is.
    OwnershipKept  const BREthereumNodeEndpoint local;   // Not ours
    OwnershipGiven BREthereumNodeEndpoint remote;        // Ours

    // The DIS distance between local <==> remote.
    UInt256 distance;

    /** When connecting to a remote node, we'll have some additional requirements on the remote's
     * status if we will be syncing (rather than just submitting transactions). */
    BREthereumBoolean handleSync;

    /** The message specs by identifier.  Includes credit params and message count limits */
    // TODO: This should not be LES specific; applies to PIP too.
    BREthereumLESMessageSpec specs [NUMBER_OF_LES_MESSAGE_IDENTIFIERS];

    /** Credit remaining (if not zero) */
    uint64_t credits;

    /** Callbacks */
    BREthereumNodeContext callbackContext;
    BREthereumNodeCallbackStatus callbackStatus;
    BREthereumNodeCallbackAnnounce callbackAnnounce;
    BREthereumNodeCallbackProvide callbackProvide;
    BREthereumNodeCallbackNeighbor callbackNeighbor;

    /** Send/Recv Buffer */
    BRRlpData sendDataBuffer;
    BRRlpData recvDataBuffer;

    /** Message Coder - remember 'not thread safe'! */
    BREthereumMessageCoder coder;

    /** TRUE if we've discovered the neighbors of this node */
    BREthereumBoolean discovered;

    /** When waiting to receive a message, timeout when time exceeds `timeout`. */
    time_t timeout;

    /** But, in some cases try a ping on a timeout. */
    BREthereumBoolean timeoutPingAllowed;

    /** Frame Coder */
    BREthereumLESFrameCoder frameCoder;
    uint8_t authBuf[AUTH_BUF_LEN];
    uint8_t authBufCipher[AUTH_CIPHER_BUF_LEN];
    uint8_t ackBuf[ACK_BUF_LEN];
    uint8_t ackBufCipher[ACK_CIPHER_BUF_LEN];

    // Provision
    size_t messageIdentifier;

    BRArrayOf(BREthereumNodeProvisioner) provisioners;

    // A largely unneeded lock.
    pthread_mutex_t lock;
};

extern void
ethNodeShow (BREthereumNode node) {
    char descUDP[128], descTCP[128];

    BREthereumDISNeighborEnode enode = neighborDISAsEnode (ethNodeEndpointGetDISNeighbor(node->remote), 1);
    LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Node: %15s", ethNodeEndpointGetHostname(node->remote));
    LOG (LL_INFO, ETH_LES_LOG_TOPIC, "   NodeID    : %s", enode.chars);
    LOG (LL_INFO, ETH_LES_LOG_TOPIC, "   Type      : %s", ethNodeTypeGetName(node->type));
    LOG (LL_INFO, ETH_LES_LOG_TOPIC, "   UDP       : %s", ethNodeStateDescribe (&node->states[NODE_ROUTE_UDP], descUDP));
    LOG (LL_INFO, ETH_LES_LOG_TOPIC, "   TCP       : %s", ethNodeStateDescribe (&node->states[ETHEREUM_NODE_ROUTE_TCP], descTCP));
    LOG (LL_INFO, ETH_LES_LOG_TOPIC, "   Discovered: %s", (ETHEREUM_BOOLEAN_IS_TRUE(node->discovered) ? "Yes" : "No"));
    LOG (LL_INFO, ETH_LES_LOG_TOPIC, "   Credits   : %" PRIu64, node->credits);
}

extern const BREthereumNodeEndpoint
ethNodeGetRemoteEndpoint (BREthereumNode node) {
    return node->remote;
}

extern const BREthereumNodeEndpoint
ethNodeGetLocalEndpoint (BREthereumNode node) {
    return node->local;
}


static BREthereumComparison
ethNodeNeighborCompare (BREthereumNode n1,
                     BREthereumNode n2) {
    switch (uint256Compare (n1->distance, n2->distance)) {
        case -1: return ETHEREUM_COMPARISON_LT;
        case  0: return ETHEREUM_COMPARISON_EQ;
        case +1: return ETHEREUM_COMPARISON_GT;
        default: BRFail();
    }
}

extern BREthereumComparison
ethNodeCompare (BREthereumNode node1,
             BREthereumNode node2) {
    return (node1->priority < node2->priority
            ? ETHEREUM_COMPARISON_LT
            : (node1->priority > node2->priority
               ? ETHEREUM_COMPARISON_GT
               : ethNodeNeighborCompare(node1, node2)));
}

static size_t
ethNodeGetThenIncrementMessageIdentifier (BREthereumNode node,
                                       size_t byIncrement) {
    size_t identifier;
    pthread_mutex_lock(&node->lock);
    identifier = node->messageIdentifier;
    node->messageIdentifier += byIncrement;
    pthread_mutex_unlock(&node->lock);
    return identifier;
}

static BREthereumNodeState
ethNodeStateAnnounce (BREthereumNode node,
                   BREthereumNodeEndpointRoute route,
                   BREthereumNodeState state) {
    node->states [route] = state;
    return state;
}

extern int
ethNodeHasState (BREthereumNode node,
              BREthereumNodeEndpointRoute route,
              BREthereumNodeStateType type) {
    return type == node->states[route].type;
}

static int
ethNodeHasErrorState (BREthereumNode node,
                   BREthereumNodeEndpointRoute route) {
    switch (node->states[route].type) {
        case NODE_AVAILABLE:
        case NODE_CONNECTING:
        case NODE_CONNECTED:
            return 0;
        case NODE_ERROR:
            return 1;
    }
}

extern BREthereumNodeState
ethNodeGetState (BREthereumNode node,
              BREthereumNodeEndpointRoute route) {
    return node->states[route];
}

static void
ethNodeSetStateErrorProtocol (BREthereumNode node,
                           BREthereumNodeEndpointRoute route,
                           BREthereumNodeProtocolReason reason) {
    node->states[route] = ethNodeStateCreateErrorProtocol(reason);
}

// Support BRSet
extern size_t
ethNodeHashValue (const void *h) {
    return ethHashSetValue(&((BREthereumNode) h)->hash);
}

// Support BRSet
extern int
ethNodeHashEqual (const void *h1, const void *h2) {
    return h1 == h2 || ethHashSetEqual (&((BREthereumNode) h1)->hash,
                                     &((BREthereumNode) h2)->hash);
}

//
// Create
//
extern BREthereumNode
ethNodeCreate (BREthereumNodePriority priority,
            BREthereumNetwork network,
            OwnershipKept const BREthereumNodeEndpoint local,
            OwnershipGiven BREthereumNodeEndpoint remote,  // remote, local ??
            BREthereumNodeContext context,
            BREthereumNodeCallbackStatus callbackStatus,
            BREthereumNodeCallbackAnnounce callbackAnnounce,
            BREthereumNodeCallbackProvide callbackProvide,
            BREthereumNodeCallbackNeighbor callbackNeighbor,
            BREthereumBoolean handleSync) {
    BREthereumNode node = calloc (1, sizeof (struct BREthereumNodeRecord));

    // Identify this `node` with the remote hash.
    node->hash = ethNodeEndpointGetHash(remote);

    // Fix the type as UNKNOWN (for now, at least).
    node->type = NODE_TYPE_UNKNOWN;

    // Fix the priority.
    node->priority = priority;

    // Make all routes as 'available'
    for (int route = 0; route < ETHEREUM_NUMBER_OF_NODE_ROUTES; route++)
        node->states[route] = ethNodeStateCreate(NODE_AVAILABLE);

    // Save the local and remote nodes.
    *((BREthereumNodeEndpoint *) &node->local)  = local; // this allows assignement to 'const'
    node->remote = remote;

    // Compute the 'DIS Distance' between the two endpoints.  We'll favor nodes with a
    // remote endpoint that is closer to our local endpoint.
    node->distance = neighborDISDistance (ethNodeEndpointGetDISNeighbor(node->local),
                                          ethNodeEndpointGetDISNeighbor(node->remote));

    // Place additional constraints when connecting to a remote node.
    node->handleSync = handleSync;

    // Fill in the specs with default values (for GETH)
    for (int i = 0; i < NUMBER_OF_LES_MESSAGE_IDENTIFIERS; i++)
        node->specs[i] = messageLESSpecs[i];

    // No credits, yet.
    node->credits = 0;

    node->sendDataBuffer = (BRRlpData) { DEFAULT_SEND_DATA_BUFFER_SIZE, malloc (DEFAULT_SEND_DATA_BUFFER_SIZE) };
    node->recvDataBuffer = (BRRlpData) { DEFAULT_RECV_DATA_BUFFER_SIZE, malloc (DEFAULT_RECV_DATA_BUFFER_SIZE) };

    // Define the message coder
    node->coder.network = network;
    node->coder.rlp = rlpCoderCreate();
    node->coder.messageIdOffset = 0x00;  // Changed with 'hello' message exchange.

    node->discovered = ETHEREUM_BOOLEAN_FALSE;

    node->frameCoder = frameCoderCreate();

    node->callbackContext  = context;
    node->callbackStatus   = callbackStatus;
    node->callbackAnnounce = callbackAnnounce;
    node->callbackProvide  = callbackProvide;
    node->callbackNeighbor = callbackNeighbor;

    node->messageIdentifier = 0;
    array_new (node->provisioners, 10);

    // A remote port (TCP or UDP) of '0' marks this node in error.
    if (0 == ethNodeEndpointGetPort (remote, ETHEREUM_NODE_ROUTE_TCP))
        ethNodeSetStateErrorProtocol (node, ETHEREUM_NODE_ROUTE_TCP, NODE_PROTOCOL_NONSTANDARD_PORT);

    if (0 == ethNodeEndpointGetPort (remote, NODE_ROUTE_UDP))
        ethNodeSetStateErrorProtocol (node, NODE_ROUTE_UDP, NODE_PROTOCOL_NONSTANDARD_PORT);

    node->timeout = (time_t) -1;
    node->timeoutPingAllowed = ETHEREUM_BOOLEAN_TRUE;

    pthread_mutex_init_brd (&node->lock, PTHREAD_MUTEX_NORMAL);

    return node;
}

extern void
ethNodeRelease (BREthereumNode node) {
    ethNodeDisconnect (node, ETHEREUM_NODE_ROUTE_TCP, ethNodeStateCreate (NODE_AVAILABLE), ETHEREUM_BOOLEAN_FALSE);
    ethNodeDisconnect (node, NODE_ROUTE_UDP, ethNodeStateCreate (NODE_AVAILABLE), ETHEREUM_BOOLEAN_FALSE);

    ethNodeEndpointRelease(node->remote);

    for (size_t index = 0; index < array_count(node->provisioners); index++)
        ethProvisionRelease (&node->provisioners[index].provision , ETHEREUM_BOOLEAN_TRUE);
    array_free (node->provisioners);

    if (NULL != node->sendDataBuffer.bytes) free (node->sendDataBuffer.bytes);
    if (NULL != node->recvDataBuffer.bytes) free (node->recvDataBuffer.bytes);

    rlpCoderRelease(node->coder.rlp);
    frameCoderRelease(node->frameCoder);

    pthread_mutex_destroy(&node->lock);
    free (node);
}

extern void
ethNodeClean (BREthereumNode node) {
    rlpCoderReclaim(node->coder.rlp);
}

extern BREthereumBoolean
ethNodeUpdatedLocalStatus (BREthereumNode node,
                        BREthereumNodeEndpointRoute route) {
    // If any route has a error state of NODE_PROTOCOL_STATUS_MISMATCH, then return that
    // route's state to NODE_AVAILABLE.  That makes this node once again available for
    // the LES handshake - which might succeed with the new status.
    if (NODE_ERROR == node->states[route].type &&
        NODE_ERROR_PROTOCOL == node->states[route].u.error.type &&
        NODE_PROTOCOL_STATUS_MISMATCH == node->states[route].u.error.u.protocol) {
        node->states[route].type = NODE_AVAILABLE;
        return ETHEREUM_BOOLEAN_TRUE;
        }
    return ETHEREUM_BOOLEAN_FALSE;
}

extern BREthereumNodeType
ethNodeGetType (BREthereumNode node) {
    return node->type;
}

extern BREthereumNodePriority
ethNodeGetPriority (BREthereumNode node) {
    return node->priority;
}

static inline void
ethNodeUpdateTimeout (BREthereumNode node,
                   time_t now) {
    node->timeout = now + DEFAULT_NODE_TIMEOUT_IN_SECONDS;
}

static inline void
ethNodeUpdateTimeoutRecv (BREthereumNode node,
                       time_t now) {
    node->timeout = now + DEFAULT_NODE_TIMEOUT_IN_SECONDS_RECV;
}

static BREthereumNodeState
ethNodeProcessFailure (BREthereumNode node,
                    BREthereumNodeEndpointRoute route,
                    OwnershipGiven BREthereumMessage *message,
                    BREthereumNodeState state) {
    if (NULL != message) messageRelease (message);
    return ethNodeDisconnect (node, route, state, ETHEREUM_BOOLEAN_FALSE);
}

static BREthereumNodeState
ethNodeProcessSuccess (BREthereumNode node,
                    BREthereumNodeEndpointRoute route,
                    OwnershipGiven BREthereumMessage *message,
                    BREthereumNodeState state) {
    if (NULL != message) messageRelease (message);
    return ethNodeStateAnnounce(node, route, state);
}

extern BREthereumNodeState
ethNodeConnect (BREthereumNode node,
             BREthereumNodeEndpointRoute route,
             time_t now) {
    int error;

    // Nothing if not AVAILABLE
    if (!ethNodeHasState (node, route, NODE_AVAILABLE))
        return node->states[route];

#if defined (NODE_DEBUG_SOCKETS)
    // Increment here; on failure we'll decrement.
    LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Sockets: %d (Open)", ++socketOpenCount);
#endif


    // Actually open the endpoint connection/port
    error = ethNodeEndpointOpen (node->remote, route);
    if (error)
        return ethNodeProcessFailure (node, route, NULL, ethNodeStateCreateErrorUnix(error));

    // Move to the next state.
    ethNodeUpdateTimeout(node, now);
    return ethNodeStateAnnounce(node, route, ethNodeStateCreateConnecting (ETHEREUM_NODE_ROUTE_TCP == route
                                                                     ? NODE_CONNECT_AUTH
                                                                     : NODE_CONNECT_PING));
}

extern BREthereumNodeState
ethNodeDisconnect (BREthereumNode node,
                BREthereumNodeEndpointRoute route,
                BREthereumNodeState stateToAnnounce,
                BREthereumBoolean returnToAvailable) {

    ethNodeStateAnnounce (node, route, stateToAnnounce);

#if defined (NODE_DEBUG_SOCKETS)
    // Close the appropriate endpoint route
    int failed = ethNodeEndpointClose (&node->remote, route, !ethNodeHasErrorState (node, route));
    if (!failed) LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Sockets: %d (Closed)", --socketOpenCount);
#else
    ethNodeEndpointClose (node->remote, route, !ethNodeHasErrorState (node, route));
#endif

    // Clear any pending timeout.
    node->timeout = -1;

    // If we already announced `available`, then don't announce it again.
    if (ETHEREUM_BOOLEAN_IS_TRUE(returnToAvailable) && NODE_AVAILABLE != stateToAnnounce.type)
        ethNodeStateAnnounce(node, route, ethNodeStateCreate (NODE_AVAILABLE));

    return node->states[route];
}

/// MARK: - Node Process

extern BREthereumBoolean
ethNodeCanHandleProvision (BREthereumNode node,
                        BREthereumProvision provision) {
    switch (node->type) {
        case NODE_TYPE_UNKNOWN:
            return ETHEREUM_BOOLEAN_FALSE;
        case NODE_TYPE_GETH:
            return AS_ETHEREUM_BOOLEAN(((BREthereumLESMessageIdentifier) -1) != ethProvisionGetMessageLESIdentifier (provision.type));
        case NODE_TYPE_PARITY:
            return AS_ETHEREUM_BOOLEAN(((BREthereumPIPRequestType) -1) != ethProvisionGetMessagePIPIdentifier(provision.type));
    }
}

extern void
ethNodeHandleProvision (BREthereumNode node,
                     BREthereumProvision provision) {
    BREthereumNodeProvisioner provisioner = { provision };
    array_add (node->provisioners, provisioner);
    // Pass the proper provision reference - so we establish the actual provision
    provisionerEstablish (&node->provisioners[array_count(node->provisioners) - 1], node);
}

extern BRArrayOf(BREthereumProvision)
ethNodeUnhandleProvisions (BREthereumNode node) {
    BRArrayOf(BREthereumProvision) provisions;
    array_new (provisions, array_count(node->provisioners));
    for (size_t index = 0; index < array_count(node->provisioners); index++)
        array_add (provisions, node->provisioners[index].provision);
    array_clear(node->provisioners);
    return provisions;
}

static void
ethNodeHandleProvisionerMessage (BREthereumNode node,
                              BREthereumNodeProvisioner *provisioner,
                              OwnershipGiven BREthereumMessage message) {
    // Let the provisioner handle the message, gathering results as warranted.
    provisionerHandleMessage (provisioner, message); // `message` is OwnershipGiven

    // If all messages have been received...
    if (!provisionerRecvMessagesPending(provisioner)) {
        // ... callback the result,
        BREthereumProvisionResult result = {
            provisioner->provision.identifier,
            provisioner->provision.type,
            provisioner->status,
            provisioner->provision,
            { .success = {}}
        };

        if (PROVISION_ERROR == provisioner->status) {
            result.u.error.reason = PROVISION_ERROR_NODE_DATA;
            LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Recv: [ %3s, %15s ] => %15s (data error)",
                 messageGetIdentifierName(&message),
                 messageGetAnyIdentifierName(&message),
                 ethNodeEndpointGetHostname(node->remote));
        }

        node->callbackProvide (node->callbackContext, node, result);

        // ... and remove the provisioner
        for (size_t index = 0; index < array_count (node->provisioners); index++)
            if (provisioner == &node->provisioners[index]) {
                // Release the provision, including its messages.  But, do not release
                // the provision as it was passed on to `node->callbackProvide`.
                provisionerRelease(provisioner, ETHEREUM_BOOLEAN_FALSE, ETHEREUM_BOOLEAN_FALSE);

                // Remove but be sure to `break`.
                array_rm (node->provisioners, index);
                break;
            }
    }
}

static void
ethNodeProcessRecvP2P (BREthereumNode node,
                    BREthereumNodeEndpointRoute route,
                    OwnershipGiven BREthereumP2PMessage message) {
    assert (ETHEREUM_NODE_ROUTE_TCP == route);

    int mustReleaseMessage = 1;
    switch (message.identifier) {
        case P2P_MESSAGE_DISCONNECT:
            ethNodeDisconnect(node, ETHEREUM_NODE_ROUTE_TCP, ethNodeStateCreateErrorDisconnect(message.u.disconnect.reason), ETHEREUM_BOOLEAN_FALSE);
            break;

        case P2P_MESSAGE_PING: {
            // Immediately send a poing message
            BREthereumMessage pong = {
                MESSAGE_P2P,
                { .p2p = {
                    P2P_MESSAGE_PONG,
                    {}}}
            };
            if (NODE_STATUS_ERROR == ethNodeSend (node, ETHEREUM_NODE_ROUTE_TCP, pong))
                ethNodeStateAnnounce(node, ETHEREUM_NODE_ROUTE_TCP, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_PING_PONG_MISSED));
            break;
        }

        case P2P_MESSAGE_PONG:
            // On a PONG perhaps we 'reenable' PING?  (See ethNodeHandleTimeout()).  But, nothing to
            // do here as it is.
            break;

        case P2P_MESSAGE_HELLO:
            LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Recv: [ P2P, %15s ] Unexpected",
                 messageP2PGetIdentifierName (message.identifier));
            break;
    }
    if (mustReleaseMessage) messageP2PRelease (&message);
}

static void
ethNodeProcessRecvDIS (BREthereumNode node,
                    BREthereumNodeEndpointRoute route,
                    OwnershipGiven BREthereumDISMessage message) {
    assert (NODE_ROUTE_UDP == route);

    int mustReleaseMessage = 1;
    switch (message.identifier) {
        case DIS_MESSAGE_PING: {
            // Immediately send a pong message
            BREthereumMessage pong = {
                MESSAGE_DIS,
                { .dis = {
                    DIS_MESSAGE_PONG,
                    { .pong =
                        messageDISPongCreate (message.u.ping.to,
                                              message.u.ping.hash,
                                              (uint64_t) time(NULL) + 1000000) },
                    ethNodeEndpointGetDISNeighbor(node->local).key }}
            };
            if (NODE_STATUS_ERROR == ethNodeSend (node, NODE_ROUTE_UDP, pong))
                ethNodeStateAnnounce(node, NODE_ROUTE_UDP, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_PING_PONG_MISSED));

            break;
        }

        case DIS_MESSAGE_NEIGHBORS: {
            node->callbackNeighbor (node->callbackContext,
                                    node,
                                    message.u.neighbors.neighbors);
            break;
        }

        case DIS_MESSAGE_PONG:
        case DIS_MESSAGE_FIND_NEIGHBORS:
            LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Recv: [ DIS, %15s ] Unexpected",
                 messageDISGetIdentifierName (message.identifier));
            break;
    }
    if (mustReleaseMessage) messageDISRelease (&message);
}

static void
ethNodeProcessRecvLES (BREthereumNode node,
                    BREthereumNodeEndpointRoute route,
                    OwnershipGiven BREthereumLESMessage message) {
    assert (NODE_TYPE_GETH == node->type);

    int mustReleaseMessage = 1;
    switch (message.identifier) {
        case LES_MESSAGE_STATUS:
            node->callbackStatus (node->callbackContext,
                                  node,
                                  message.u.status.p2p.headHash,
                                  message.u.status.p2p.headNum);
            break;

        case LES_MESSAGE_ANNOUNCE:
            node->callbackAnnounce (node->callbackContext,
                                    node,
                                    message.u.announce.headHash,
                                    message.u.announce.headNumber,
                                    message.u.announce.headTotalDifficulty,
                                    message.u.announce.reorgDepth);
            break;

        case LES_MESSAGE_GET_BLOCK_HEADERS:
        case LES_MESSAGE_GET_BLOCK_BODIES:
        case LES_MESSAGE_GET_RECEIPTS:
        case LES_MESSAGE_GET_PROOFS:
        case LES_MESSAGE_GET_CONTRACT_CODES:
        case LES_MESSAGE_SEND_TX:
        case LES_MESSAGE_GET_HEADER_PROOFS:
        case LES_MESSAGE_GET_PROOFS_V2:
        case LES_MESSAGE_GET_HELPER_TRIE_PROOFS:
        case LES_MESSAGE_SEND_TX2:
        case LES_MESSAGE_GET_TX_STATUS:
            LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Recv: [ LES, %15s ] Unexpected Request",
                 messageLESGetIdentifierName (message.identifier));
            break;

        case LES_MESSAGE_CONTRACT_CODES:
        case LES_MESSAGE_HELPER_TRIE_PROOFS:;
            LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Recv: [ LES, %15s ] Unexpected Response",
                 messageLESGetIdentifierName (message.identifier));
            break;

        case LES_MESSAGE_BLOCK_HEADERS:
        case LES_MESSAGE_BLOCK_BODIES:
        case LES_MESSAGE_RECEIPTS:
        case LES_MESSAGE_PROOFS:
        case LES_MESSAGE_PROOFS_V2:
        case LES_MESSAGE_TX_STATUS:
        case LES_MESSAGE_HEADER_PROOFS:
            // Find the provisioner applicable to `message`...
            for (size_t index = 0; index < array_count (node->provisioners); index++) {
                BREthereumNodeProvisioner *provisioner = &node->provisioners[index];
                // ... using the message's requestId
                if (provisionerMessageOfInterest (provisioner, messageLESGetRequestId (&message))) {
                    mustReleaseMessage = 0;
                    // When found, handle it.
                    ethNodeHandleProvisionerMessage (node, provisioner,
                                                  (BREthereumMessage) {
                                                      MESSAGE_LES,
                                                      { .les = message }
                                                  });
                    break;
                }
            }
            break;
    }
    if (mustReleaseMessage) messageLESRelease (&message);
}

static void
ethNodeProcessRecvPIP (BREthereumNode node,
                    BREthereumNodeEndpointRoute route,
                    OwnershipGiven BREthereumPIPMessage message) {
    assert (NODE_TYPE_PARITY == node->type);

    // We'll release the message by default.  However, in the case of a RESPONSE we'll
    // transfer ownership and won't release here.
    int mustReleaseMessage = 1;
    switch (message.type) {
        case PIP_MESSAGE_STATUS:
            // Note: Nothing to consume
            node->callbackStatus (node->callbackContext,
                                  node,
                                  message.u.status.p2p.headHash,
                                  message.u.status.p2p.headNum);
            break;

        case PIP_MESSAGE_ANNOUNCE:
            // Note: Nothing to consume
            node->callbackAnnounce (node->callbackContext,
                                    node,
                                    message.u.announce.headHash,
                                    message.u.announce.headNumber,
                                    message.u.announce.headTotalDifficulty,
                                    message.u.announce.reorgDepth);
            break;

        case PIP_MESSAGE_REQUEST: {
            BRArrayOf(BREthereumPIPRequestInput) inputs = message.u.request.inputs;
            if (array_count (inputs) > 0)
                LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Recv: [ PIP, %15s ] Unexpected Request (%zu)",
                     messagePIPGetRequestName (inputs[0].identifier),
                     array_count(inputs));
            break;
        }

        case PIP_MESSAGE_RESPONSE:
            // Find the provisioner applicable to `message`... it might be (stress 'might be')
            // that node->provisioners is empty at this point.  The node has been 'deactivated' (by
            // LES), the provisions reassigned but still, somehow, we handle this message.
            for (size_t index = 0; index < array_count (node->provisioners); index++) {
                BREthereumNodeProvisioner *provisioner = &node->provisioners[index];
                // ... using the message's requestId
                if (provisionerMessageOfInterest (provisioner, messagePIPGetRequestId (&message))) {
                    mustReleaseMessage = 0;
                    // When found, handle it.
                    ethNodeHandleProvisionerMessage (node, provisioner,
                                                  (BREthereumMessage) {
                                                      MESSAGE_PIP,
                                                      { .pip = message }
                                                  });
                    break;
                }
            }
            break;

        case PIP_MESSAGE_UPDATE_CREDIT_PARAMETERS: {
            // TODO: Process the new credit parameters...

            // ... and then, immediately acknowledge the update.
            BREthereumMessage ack = {
                MESSAGE_PIP,
                { .pip = {
                    PIP_MESSAGE_ACKNOWLEDGE_UPDATE,
                    { .acknowledgeUpdate = {}}}}
            };
            ethNodeSend (node, ETHEREUM_NODE_ROUTE_TCP, ack);
            break;
        }

        case PIP_MESSAGE_ACKNOWLEDGE_UPDATE:
        case PIP_MESSAGE_RELAY_TRANSACTIONS:
            // Nobody sends these to us.
            LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Recv: [ PIP, %15s ] Unexpected Response",
                 messagePIPGetIdentifierName (message));
            break;
    }

    if (mustReleaseMessage) messagePIPRelease (&message);
}

static uint64_t
ethNodeGetLocalStatusProtocolVersion (BREthereumNode node) {
    switch (node->type) {
        case NODE_TYPE_UNKNOWN: assert (0);
        case NODE_TYPE_GETH:    return LES_SUPPORT_GETH_VERSION;
        case NODE_TYPE_PARITY:  return LES_SUPPORT_PARITY_VERSION;
    }
}

static BREthereumMessage
ethNodeCreateLocalHelloMessage (BREthereumNode node) {
    BREthereumP2PMessageHello locHello = ethNodeEndpointGetHello(node->local);
    BREthereumP2PMessageHello newHello = messageP2PHelloCopy (&locHello);

    return (BREthereumMessage) {
        MESSAGE_P2P,
        { .p2p = {
            P2P_MESSAGE_HELLO,
            { .hello = newHello }}}
    };
}

static BREthereumMessage
ethNodeCreateLocalStatusMessage (BREthereumNode node) {
    BREthereumP2PMessageStatus locStatus = ethNodeEndpointGetStatus(node->local);
    BREthereumP2PMessageStatus newStatus = messageP2PStatusCopy (&locStatus);

    switch (node->type) {
        case NODE_TYPE_UNKNOWN: assert (0);

        case NODE_TYPE_GETH:
            newStatus.protocolVersion = LES_SUPPORT_GETH_VERSION;
            return (BREthereumMessage) {
                MESSAGE_LES,
                { .les = {
                    LES_MESSAGE_STATUS,
                    { .status = { newStatus, {} }}}}
            };

        case NODE_TYPE_PARITY:
            newStatus.protocolVersion = LES_SUPPORT_PARITY_VERSION;
            return (BREthereumMessage) {
                MESSAGE_PIP,
                { .pip = {
                    PIP_MESSAGE_STATUS,
                    { .status = { newStatus }}}}
            };
    }
}

static void
ethNodeDefineRemoteStatus (BREthereumNode node,
                        OwnershipKept BREthereumMessage message) {
    BREthereumP2PMessageStatus status;

    switch (node->type) {
        case NODE_TYPE_UNKNOWN: assert (0);

        case NODE_TYPE_GETH:
            assert (MESSAGE_LES == message.identifier);
            assert (LES_MESSAGE_STATUS == message.u.les.identifier);
            status = message.u.les.u.status.p2p;
            break;

        case NODE_TYPE_PARITY:
            assert (MESSAGE_PIP == message.identifier);
            assert (PIP_MESSAGE_STATUS == message.u.pip.type);
            status = message.u.pip.u.status.p2p;
            break;
    }

    ethNodeEndpointSetStatus (node->remote, messageP2PStatusCopy (&status));
}

static int
ethNodeStatusIsSufficient (BREthereumNode node) {
    BREthereumP2PMessageStatusValue remValue;

    BREthereumP2PMessageStatus locStatus = ethNodeEndpointGetStatus (node->local);
    BREthereumP2PMessageStatus remStatus = ethNodeEndpointGetStatus (node->remote);

    // Must be our network
    if (remStatus.chainId != locStatus.chainId)
        return 0;

    // Must be our protocol - doesn't this cause problems?  PIPv1 vs LESV2.  Didn't we check
    // the protocol version when matching capabilities?  Our local node has a 'status' but we
    // can be PIPv1 or LESv2 - do we need two status messages?
    if (remStatus.protocolVersion != ethNodeGetLocalStatusProtocolVersion(node))
        return 0;

    // Must have blocks in the future
    if (remStatus.headNum <= locStatus.headNum)
        return 0;

    // If we are (potentially) syncing from this node, compare remStatus to locStatus
    if (ETHEREUM_BOOLEAN_IS_TRUE(node->handleSync)) {

        // Must serve headers
        if (!messageP2PStatusExtractValue (&remStatus, P2P_MESSAGE_STATUS_SERVE_HEADERS, &remValue) ||
            ETHEREUM_BOOLEAN_IS_FALSE(remValue.u.boolean))
            return 0;

        // Must serve state (archival node is '0') from no later than locStatus.headNum
        if (!messageP2PStatusExtractValue (&remStatus, P2P_MESSAGE_STATUS_SERVE_STATE_SINCE, &remValue) ||
            remValue.u.integer > locStatus.headNum)
            return 0;

        // Must serve chain (archival node is '1' Parity or '0' Geth) from no later then locStatus.headNum
        if (!messageP2PStatusExtractValue (&remStatus, P2P_MESSAGE_STATUS_SERVE_CHAIN_SINCE, &remValue) ||
            remValue.u.integer - (node->type == NODE_TYPE_PARITY ? 1 : 0) > locStatus.headNum)
            return 0;
    }

    // Must Relay Tranactions
    if (!messageP2PStatusExtractValue( &remStatus, P2P_MESSAGE_STATUS_TX_RELAY, &remValue) ||
        ETHEREUM_BOOLEAN_IS_FALSE(remValue.u.boolean))
        return 0;

    return 1;
}


extern BREthereumNodeState
ethNodeProcess (BREthereumNode node,
             BREthereumNodeEndpointRoute route,
             time_t now,
             fd_set *recv,    // read
             fd_set *send) {  // write
    BREthereumNodeMessageResult result;
    BREthereumMessage message;
    size_t ackCipherBufCount;
    int error;

    int socket = ethNodeEndpointGetSocket (node->remote, route);

    // Do nothing if there is no socket.
    if (-1 == socket) return node->states[route];

    switch (node->states[route].type) {
            //
            // When CONNECTED:
            //   a) we'll send PIP and LES messages based on provisioned requests.
            //   b) we'll recv any message and dispatch to the appropriate handler.
            // Note: both FD SETS (send and recv) apply when CONNECTED.
            //
            // If we are just waiting to receive (typically an 'ANNOUNCE' message with a new
            // block header), then we are willing to wait for 1DEFAULT_NODE_TIMEOUT_IN_SECONDS_RECV;
            // otherwise if we sent a request, then we are only willing to wait for
            // DEFAULT_NODE_TIMEOUT_IN_SECONDS seconds.
            //
        case NODE_CONNECTED:
            if (FD_ISSET (socket, recv)) {
                ethNodeUpdateTimeoutRecv(node, now);  // wait w/ a longer timeout.

                // Recv if we can.  Get a result for the provided route; on success dispatch to
                // a protocol-specific (P2P, DIS, ETH, LES and PIP) handler.
                BREthereumNodeMessageResult result = ethNodeRecv (node, route);
                if (NODE_STATUS_ERROR == result.status)
                    return ethNodeProcessFailure (node, route, NULL, ethNodeStateCreateErrorProtocol (NODE_PROTOCOL_RLP_PARSE));

                BREthereumMessage message = result.u.success.message;

                switch (message.identifier) {
                    case MESSAGE_P2P:
                        ethNodeProcessRecvP2P (node, route, message.u.p2p);
                        break;
                    case MESSAGE_DIS:
                        ethNodeProcessRecvDIS (node, route, message.u.dis);
                        break;
                    case MESSAGE_ETH:
                        assert (0);
                        break;
                    case MESSAGE_LES:
                        ethNodeProcessRecvLES (node, route, message.u.les);
                        break;
                    case MESSAGE_PIP:
                        ethNodeProcessRecvPIP (node, route, message.u.pip);
                        break;
                }
                // No release for `message` - it has be OwnershipGiven in the above
            }

            if (FD_ISSET (socket, send)) {
                ethNodeUpdateTimeoutRecv(node, now);   // override prior timeout; expect a response.

                // Send if we can.  Really only applies to provision messages, for PIP and LES, using
                // the TCP route.  We might have multiple provisiones to deal with but we'll send them
                // one at a time to avoid potential blocking.
                switch (route) {
                    case NODE_ROUTE_UDP:
                        break;

                    case ETHEREUM_NODE_ROUTE_TCP:
                        // Look for the pending message in some provisioner
                        for (size_t index = 0; index < array_count (node->provisioners); index++)
                            if (provisionerSendMessagesPending (&node->provisioners[index])) {
                                BREthereumNodeStatus status = provisionerMessageSend(&node->provisioners[index]);
                                switch (status) {
                                    case NODE_STATUS_SUCCESS:
                                        break;
                                    case NODE_STATUS_ERROR:
                                        break;
                                }
                                // Only send one at a time - socket might be blocked
                                break; // from for node->provisioners
                            }
                        break;
                }
            }

            return node->states[route];

            //
            // When CONNECTING - we'll deal with sub-states as part of a handshake process.
            //
        case NODE_CONNECTING:
            switch (node->states[route].u.connecting.type) {
                case NODE_CONNECT_OPEN:
                    return node->states[route];

                case NODE_CONNECT_AUTH:
                    assert (ETHEREUM_NODE_ROUTE_TCP == route);
                    if (!FD_ISSET (socket, send)) return node->states[route];
                    ethNodeUpdateTimeout(node, now);

                    if (0 != _sendAuthInitiator(node))
                        return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, NULL, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_TCP_AUTHENTICATION));

                    LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Send: [ WIP, %15s ] => %15s", "Auth", ethNodeEndpointGetHostname(node->remote));

                    error = ethNodeEndpointSendData (node->remote, ETHEREUM_NODE_ROUTE_TCP, node->authBufCipher, AUTH_CIPHER_BUF_LEN); //  "auth initiator");
                    if (error)
                        return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, NULL, ethNodeStateCreateErrorUnix(error));

                    return ethNodeProcessSuccess (node, route, NULL, ethNodeStateCreateConnecting(NODE_CONNECT_AUTH_ACK));

                case NODE_CONNECT_AUTH_ACK:
                    assert (ETHEREUM_NODE_ROUTE_TCP == route);
                    if (!FD_ISSET (socket, recv)) return node->states[route];
                    ethNodeUpdateTimeout(node, now);

                    ackCipherBufCount = ACK_CIPHER_BUF_LEN;
                    error = ethNodeEndpointRecvData (node->remote, ETHEREUM_NODE_ROUTE_TCP, node->ackBufCipher, &ackCipherBufCount, 1); // "auth ack from receivier"
                    if (error)
                        return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, NULL, ethNodeStateCreateErrorUnix(error));

                    LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Recv: [ WIP, %15s ] <= %15s", "Auth Ack", ethNodeEndpointGetHostname(node->remote));
                    if (ackCipherBufCount != ACK_CIPHER_BUF_LEN)
                        return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, NULL, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_TCP_AUTHENTICATION));

                    if (0 != _readAuthAckFromRecipient (node)) {
                        LOG (LL_INFO, ETH_LES_LOG_TOPIC, "%s", "Something went wrong with AUK");
                        return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, NULL, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_TCP_AUTHENTICATION));
                    }

                    // Initilize the frameCoder with the information from the auth
                    frameCoderInit(node->frameCoder,
                                   ethNodeEndpointGetEphemeralKey(node->remote), ethNodeEndpointGetNonce(node->remote),
                                   ethNodeEndpointGetEphemeralKey(node->local), ethNodeEndpointGetNonce(node->local),
                                   node->ackBufCipher, ACK_CIPHER_BUF_LEN,
                                   node->authBufCipher, AUTH_CIPHER_BUF_LEN,
                                   ETHEREUM_BOOLEAN_TRUE);

                    return ethNodeProcessSuccess (node, route, NULL, ethNodeStateCreateConnecting(NODE_CONNECT_HELLO));

                case NODE_CONNECT_HELLO:
                    assert (ETHEREUM_NODE_ROUTE_TCP == route);
                    if (!FD_ISSET (socket, send)) return node->states[route];
                    ethNodeUpdateTimeout(node, now);

                    message = ethNodeCreateLocalHelloMessage(node);

                    if (NODE_STATUS_ERROR == ethNodeSend (node, ETHEREUM_NODE_ROUTE_TCP, message))
                        return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, &message, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_TCP_HELLO_MISSED));

                    return ethNodeProcessSuccess (node, route, &message, ethNodeStateCreateConnecting(NODE_CONNECT_HELLO_ACK));

                case NODE_CONNECT_HELLO_ACK:
                    assert (ETHEREUM_NODE_ROUTE_TCP == route);
                    if (!FD_ISSET (socket, recv)) return node->states[route];
                    ethNodeUpdateTimeout(node, now);

                    result = ethNodeRecv (node, ETHEREUM_NODE_ROUTE_TCP);
                    if (NODE_STATUS_ERROR == result.status)
                        return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, NULL, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_TCP_HELLO_MISSED));

                    message = result.u.success.message;

                    // Handle a disconnect request
                    if (MESSAGE_P2P == message.identifier && P2P_MESSAGE_DISCONNECT == message.u.p2p.identifier)
                        return ethNodeProcessFailure(node, ETHEREUM_NODE_ROUTE_TCP, &message, ethNodeStateCreateErrorDisconnect(message.u.p2p.u.disconnect.reason));

                    // Require a P2P Hello message.
                    if (MESSAGE_P2P != message.identifier || P2P_MESSAGE_HELLO != message.u.p2p.identifier)
                        return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, &message, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_TCP_HELLO_MISSED));

                    // Save the 'hello' message received and then move on
                    ethNodeEndpointSetHello  (node->remote, messageP2PHelloCopy (&message.u.p2p.u.hello));
                    ethNodeEndpointShowHello (node->remote);

                    // Assign the node type even before checking capabilities.
                    if (ETHEREUM_BOOLEAN_IS_TRUE (ethNodeEndpointHasHelloCapability (node->remote, "pip", LES_SUPPORT_PARITY_VERSION)))
                        node->type = NODE_TYPE_PARITY;
                    else if (ETHEREUM_BOOLEAN_IS_TRUE (ethNodeEndpointHasHelloCapability (node->remote, "les", LES_SUPPORT_GETH_VERSION)))
                        node->type = NODE_TYPE_GETH;

                    // Confirm that the remote supports ETH.  We've seen a node announce support for
                    // PIPv1 and being 200,000 blocks into the future.  Perhaps we avoid connecting
                    // to such a node - but will still have to handle rogue nodes.
                    if (ETHEREUM_BOOLEAN_IS_FALSE (ethNodeEndpointHasHelloCapability (node->remote, "eth", 62)) &&
                        ETHEREUM_BOOLEAN_IS_FALSE (ethNodeEndpointHasHelloCapability (node->remote, "eth", 63)))
                        return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, &message, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_CAPABILITIES_MISMATCH));

                    // Confirm that the remote has one and only one of the local capabilities.  It is unlikely,
                    // but possible, that a remote offers both LESv2 and PIPv1 capabilities - we aren't interested.
                    const BREthereumP2PCapability *capability =
                    ethNodeEndpointHasHelloMatchingCapability (node->local,
                                                            node->remote);

                    if (NULL == capability)
                        return ethNodeProcessFailure(node, ETHEREUM_NODE_ROUTE_TCP, &message, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_CAPABILITIES_MISMATCH));

                    // https://github.com/ethereum/wiki/wiki/ÐΞVp2p-Wire-Protocol
                    // ÐΞVp2p is designed to support arbitrary sub-protocols (aka capabilities) over the basic wire
                    // protocol. Each sub-protocol is given as much of the message-ID space as it needs (all such
                    // protocols must statically specify how many message IDs they require). On connection and
                    // reception of the Hello message, both peers have equivalent information about what
                    // subprotocols they share (including versions) and are able to form consensus over the
                    // composition of message ID space.
                    //
                    // Message IDs are assumed to be compact from ID 0x10 onwards (0x00-0x10 is reserved for
                    // ÐΞVp2p messages) and given to each shared (equal-version, equal name) sub-protocol in
                    // alphabetic order. Sub-protocols that are not shared are ignored. If multiple versions are
                    // shared of the same (equal name) sub-protocol, the numerically highest wins, others are
                    // ignored

                    // We'll trusted (but verified) above that we have one and only one (LES, PIP) subprotocol.
                    node->coder.messageIdOffset = 0x10;

                    // We handle a Parity Race Condition - We cannot send a STATUS message at this point...
                    // At this point in time Parity is constructing/sending a PING message and will be waiting for
                    // a PONG message.  If we send STATUS, Parity will see it but expected a PONG and then will
                    // instantly dump us.
                    //
                    // ... Except, apparently this is not struct as we get dumped no matter what.
                    //

                    // ETH: LES: Send: [ WIP,            Auth ] => 193.70.55.37
                    // ETH: LES: Open: UDP @ 30303 =>   139.99.51.203 Success
                    // ETH: LES: Send: [ DIS,            Ping ] => 139.99.51.203
                    // ETH: LES: Recv: [ WIP,        Auth Ack ] <= 193.70.55.37
                    // ETH: LES: Send: [ P2P,           Hello ] => 193.70.55.37
                    // ETH: LES: Recv: [ P2P,           Hello ] <= 193.70.55.37
                    // ETH: LES: Hello
                    // ETH: LES:     Version     : 5
                    // ETH: LES:     ClientId    : Parity/v1.11.8-stable-c754a02-20180725/x86_64-linux-gnu/rustc1.27.2
                    // ETH: LES:     ListenPort  : 30303
                    // ETH: LES:     NodeId      : 0x81863f47e9bd652585d3f78b4b2ee07b93dad603fd9bc3c293e1244250725998adc88da0cef48f1de89b15ab92b15db8f43dc2b6fb8fbd86a6f217a1dd886701
                    // ETH: LES:     Capabilities:
                    // ETH: LES:         eth = 62
                    // ETH: LES:         eth = 63
                    // ETH: LES:         par = 1
                    // ETH: LES:         par = 2
                    // ETH: LES:         par = 3
                    // ETH: LES:         pip = 1
                    // ETH: LES: StatusMessage:
                    // ETH: LES:     ProtocolVersion: 1
                    // ETH: LES:     NetworkId      : 1
                    // ETH: LES:     HeadNum        : 0
                    // ETH: LES:     HeadHash       : 0xd4e56740f876aef8c010b86a40d5f56745a118d0906a34e69aec8c0db1cb8fa3
                    // ETH: LES:     HeadTd         : 0
                    // ETH: LES:     GenesisHash    : 0xd4e56740f876aef8c010b86a40d5f56745a118d0906a34e69aec8c0db1cb8fa3
                    // ETH: LES: Send: [ PIP,          Status ] => 193.70.55.37
                    // ETH: LES: Recv: [ P2P,            Ping ] <= 193.70.55.37
                    // ETH: LES: Send: [ P2P,            Pong ] => 193.70.55.37
                    // ETH: LES: Recv: TCP @ 30303 =>    193.70.55.37 Error: Connection reset by peer

                    return ethNodeProcessSuccess (node, route, &message,
                                               ethNodeStateCreateConnecting (NODE_TYPE_PARITY == node->type
                                                                          ? NODE_CONNECT_PRE_STATUS_PING_RECV
                                                                          : NODE_CONNECT_STATUS));

                case NODE_CONNECT_PRE_STATUS_PING_RECV:
                    assert (NODE_TYPE_PARITY == node->type);
                    assert (ETHEREUM_NODE_ROUTE_TCP == route);
                    if (!FD_ISSET (socket, recv))  return node->states[route];
                    ethNodeUpdateTimeout(node, now);

                    result = ethNodeRecv (node, ETHEREUM_NODE_ROUTE_TCP);
                    if (NODE_STATUS_ERROR == result.status)
                        return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, NULL, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_TCP_STATUS_MISSED));  // PARITY_PING

                    message = result.u.success.message;

                    if (MESSAGE_P2P != message.identifier || P2P_MESSAGE_PING != message.u.p2p.identifier)
                        return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, &message, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_TCP_STATUS_MISSED));

                    return ethNodeProcessSuccess (node, route, &message, ethNodeStateCreateConnecting (NODE_CONNECT_PRE_STATUS_PONG_SEND));

                case NODE_CONNECT_PRE_STATUS_PONG_SEND:
                    assert (NODE_TYPE_PARITY == node->type);
                    assert (ETHEREUM_NODE_ROUTE_TCP == route);
                    if (!FD_ISSET (socket, send)) return node->states[route];
                    ethNodeUpdateTimeout(node, now);

                    BREthereumMessage pong = {
                        MESSAGE_P2P,
                        { .p2p = {
                            P2P_MESSAGE_PONG,
                            {}}}
                    };
                    if (NODE_STATUS_ERROR == ethNodeSend (node, ETHEREUM_NODE_ROUTE_TCP, pong))
                        return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, &pong, node->states[ETHEREUM_NODE_ROUTE_TCP]); // ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_TCP_STATUS_MISSED));  // PARITY_PING

                    return ethNodeProcessSuccess (node, route, &pong, ethNodeStateCreateConnecting (NODE_CONNECT_STATUS));

                case NODE_CONNECT_STATUS:
                    assert (ETHEREUM_NODE_ROUTE_TCP == route);
                    if (!FD_ISSET (socket, send)) return node->states[route];
                    ethNodeUpdateTimeout(node, now);

                    message = ethNodeCreateLocalStatusMessage (node);

                    if (NODE_STATUS_ERROR == ethNodeSend (node, ETHEREUM_NODE_ROUTE_TCP, message))
                        return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, &message, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_TCP_STATUS_MISSED));  // PARITY_PING

                    return ethNodeProcessSuccess (node, route, &message, ethNodeStateCreateConnecting (NODE_CONNECT_STATUS_ACK));

                case NODE_CONNECT_STATUS_ACK:
                    assert (ETHEREUM_NODE_ROUTE_TCP == route);
                    if (!FD_ISSET (socket, recv)) return node->states[route];
                    ethNodeUpdateTimeout(node, now);

                    result = ethNodeRecv (node, ETHEREUM_NODE_ROUTE_TCP);
                    if (NODE_STATUS_ERROR == result.status)
                        return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, NULL, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_TCP_STATUS_MISSED));  // PARITY_PING

                    message = result.u.success.message;

                    // Handle a disconnect request
                    if (MESSAGE_P2P == message.identifier && P2P_MESSAGE_DISCONNECT == message.u.p2p.identifier)
                        return ethNodeProcessFailure(node, ETHEREUM_NODE_ROUTE_TCP, &message, ethNodeStateCreateErrorDisconnect(message.u.p2p.u.disconnect.reason));

                    // Handle a ping - send a PONG and then wait again for a status
                    if (MESSAGE_P2P == message.identifier && P2P_MESSAGE_PING == message.u.p2p.identifier) {
                        BREthereumMessage pong = {
                            MESSAGE_P2P,
                            { .p2p = {
                                P2P_MESSAGE_PONG,
                                {}}}
                        };
                        if (NODE_STATUS_ERROR == ethNodeSend (node, ETHEREUM_NODE_ROUTE_TCP, pong))  /// release 'message'; no reason to w/ 'pong'
                            return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, &message, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_TCP_STATUS_MISSED));  // PARITY_PING

                        // Return an unchanged state - come right back expecting a STATUS ACK
                        messageRelease(&message);
                        return node->states[route];
                    }

                    if (MESSAGE_P2P == message.identifier && P2P_MESSAGE_DISCONNECT == message.u.p2p.identifier)
                        return ethNodeProcessFailure(node, ETHEREUM_NODE_ROUTE_TCP, &message, ethNodeStateCreateErrorDisconnect(message.u.p2p.u.disconnect.reason));

                    // Require a Status message.
                    if ((MESSAGE_LES != message.identifier || LES_MESSAGE_STATUS != message.u.les.identifier) &&
                        (MESSAGE_PIP != message.identifier || PIP_MESSAGE_STATUS != message.u.pip.type))
                        return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, &message, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_TCP_STATUS_MISSED));

                    // Save the 'status' message
                    ethNodeDefineRemoteStatus (node, message);
                    ethNodeEndpointShowStatus (node->remote);

                    // NOTE: Don't 'release' message; we took it's 'status'
                    // Require a sufficient remote status.
                    if (!ethNodeStatusIsSufficient(node))
                        return ethNodeProcessFailure (node, ETHEREUM_NODE_ROUTE_TCP, &message, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_STATUS_MISMATCH));

                    // Finally, CONNECTED
                    ethNodeProcessSuccess (node, ETHEREUM_NODE_ROUTE_TCP, NULL, ethNodeStateCreateConnected());

                    // 'Announce' the STATUS message.  Pass owership of 'message'
                    switch (node->type) {
                        case NODE_TYPE_UNKNOWN:
                            assert (0);
                        case NODE_TYPE_GETH:
                            ethNodeProcessRecvLES (node, ETHEREUM_NODE_ROUTE_TCP, message.u.les);
                            break;
                        case NODE_TYPE_PARITY:
                            ethNodeProcessRecvPIP (node, ETHEREUM_NODE_ROUTE_TCP, message.u.pip);
                            break;
                    }

                    // Once connected, use a *much* longer timeout.
                    ethNodeUpdateTimeoutRecv(node, now);
                    return node->states[route];

                case NODE_CONNECT_PING:
                    assert (NODE_ROUTE_UDP == route);
                    if (!FD_ISSET (socket, send)) return node->states[route];
                    ethNodeUpdateTimeout(node, now);

                    message = (BREthereumMessage) {
                        MESSAGE_DIS,
                        { .dis = {
                            DIS_MESSAGE_PING,
                            { .ping = messageDISPingCreate (ethNodeEndpointGetDISNeighbor(node->local).node, // endpointDISCreate(&node->local),
                                                            ethNodeEndpointGetDISNeighbor(node->remote).node, // endpointDISCreate(&node->remote),
                                                            (uint64_t) time(NULL) + 1000000) },
                            ethNodeEndpointGetDISNeighbor(node->local).key }}
                    };
                    if (NODE_STATUS_ERROR == ethNodeSend (node, NODE_ROUTE_UDP, message))
                        return ethNodeProcessFailure (node, NODE_ROUTE_UDP, &message, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_PING_PONG_MISSED));

                    return ethNodeProcessSuccess (node, route, &message, ethNodeStateCreateConnecting (NODE_CONNECT_PING_ACK));

                case NODE_CONNECT_PING_ACK:
                    assert (NODE_ROUTE_UDP == route);
                    if (!FD_ISSET (socket, recv)) return node->states[route];
                    ethNodeUpdateTimeout(node, now);

                    result = ethNodeRecv (node, NODE_ROUTE_UDP);
                    if (NODE_STATUS_ERROR == result.status)
                        return ethNodeProcessFailure (node, NODE_ROUTE_UDP, NULL, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_PING_PONG_MISSED));

                    // The PING_ACK must be a PONG message
                    message = result.u.success.message;
                    if (MESSAGE_DIS != message.identifier || DIS_MESSAGE_PONG != message.u.dis.identifier)
                        return ethNodeProcessFailure (node, NODE_ROUTE_UDP, &message, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_PING_PONG_MISSED));

                    return ethNodeProcessSuccess (node, route, &message, ethNodeStateCreateConnecting (NODE_CONNECT_PING_ACK_DISCOVER));

                case NODE_CONNECT_PING_ACK_DISCOVER:
                    // GETH and PARITY differ - at this point, we do not know which node type we have.  The GETH
                    // node will send a PING and require a PONG response before answering a FIND_NEIGHBORS.  By
                    // contrast, a PARITY node will not send a PING but will respond to a FIND_NEIGHBORS.
                    //
                    // Thus, if here we wait for a PING then, for a Parity node, we'll timeout as a PING is not
                    // coming.
                    //
                    // But if we send a FIND_NEIGHBORS message, a Geth node will ignore it and a Parity node will
                    // respond.  So, we'll send it and wait for a response.

                    assert (NODE_ROUTE_UDP == route);
                    if (!FD_ISSET (socket, send)) return node->states[route];
                    ethNodeUpdateTimeout(node, now);

                    // Send a FIND_NEIGHBORS.
                    if (NODE_STATUS_ERROR == ethNodeDiscover (node, node->local))
                        return ethNodeProcessFailure (node, NODE_ROUTE_UDP, NULL, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_PING_PONG_MISSED)); // discover

                    return ethNodeProcessSuccess (node, route, NULL, ethNodeStateCreateConnecting (NODE_CONNECT_PING_ACK_DISCOVER_ACK));

                case NODE_CONNECT_PING_ACK_DISCOVER_ACK:
                        // We are waiting for a PING message or a NEIGHBORS message.
                    assert (NODE_ROUTE_UDP == route);
                    if (!FD_ISSET (socket, recv)) return node->states[route];
                    ethNodeUpdateTimeout(node, now);

                    result = ethNodeRecv (node, NODE_ROUTE_UDP);
                    if (NODE_STATUS_ERROR == result.status)
                        return ethNodeProcessFailure (node, NODE_ROUTE_UDP, NULL, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_PING_PONG_MISSED));

                    // Require a PING message or a NEIGHBORS message
                    message = result.u.success.message;
                    if (MESSAGE_DIS != message.identifier ||
                        (DIS_MESSAGE_PING != message.u.dis.identifier && DIS_MESSAGE_NEIGHBORS != message.u.dis.identifier))
                        return ethNodeProcessFailure (node, NODE_ROUTE_UDP, &message, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_PING_PONG_MISSED));

                    // Connected?

                    // If we got a PING message, then respond with the required PONG
                    if (DIS_MESSAGE_PING == message.u.dis.identifier) {
                        message = (BREthereumMessage) {
                            MESSAGE_DIS,
                            { .dis = {
                                DIS_MESSAGE_PONG,
                                { .pong =
                                    messageDISPongCreate (message.u.dis.u.ping.to,
                                                          message.u.dis.u.ping.hash,
                                                          (uint64_t) time(NULL) + 1000000) },
                                ethNodeEndpointGetDISNeighbor (ethNodeGetLocalEndpoint(node)).key }}
                        };
                        // TODO: We could block here - need another state...
                        if (NODE_STATUS_ERROR == ethNodeSend (node, NODE_ROUTE_UDP, message))
                            return ethNodeProcessFailure (node, NODE_ROUTE_UDP, &message, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_PING_PONG_MISSED));
                    }

                    // Finally, CONNECTED
                    ethNodeProcessSuccess (node, NODE_ROUTE_UDP, NULL, ethNodeStateCreateConnected());

                    if (DIS_MESSAGE_NEIGHBORS == message.u.dis.identifier) {
                        // We got a NEIGHBORS response - this node is discovered and is Parity
                        ethNodeSetDiscovered (node, ETHEREUM_BOOLEAN_TRUE);
                        ethNodeProcessSuccess (node, NODE_ROUTE_UDP, NULL, ethNodeStateCreateConnected());
                        ethNodeProcessRecvDIS (node, NODE_ROUTE_UDP, message.u.dis);

                        // This is success...
                        return ethNodeDisconnect(node, NODE_ROUTE_UDP, ethNodeStateCreate(NODE_AVAILABLE), ETHEREUM_BOOLEAN_FALSE);
                    }

                    return ethNodeProcessSuccess (node, route, &message, ethNodeStateCreateConnecting (NODE_CONNECT_DISCOVER));

                case NODE_CONNECT_DISCOVER:
                    assert (NODE_ROUTE_UDP == route);
                    if (!FD_ISSET (socket, send)) return node->states[route];
                    ethNodeUpdateTimeout(node, now);

                    // Send a FIND_NEIGHBORS.
                    if (NODE_STATUS_ERROR == ethNodeDiscover (node, node->local))
                        return ethNodeProcessFailure (node, NODE_ROUTE_UDP, NULL, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_PING_PONG_MISSED)); // discover

                    return ethNodeProcessSuccess (node, route, NULL, ethNodeStateCreateConnecting (NODE_CONNECT_DISCOVER_ACK));

                case NODE_CONNECT_DISCOVER_ACK:
                case NODE_CONNECT_DISCOVER_ACK_TOO:
                    assert (NODE_ROUTE_UDP == route);
                    if (!FD_ISSET (socket, recv)) return node->states[route];
                    ethNodeUpdateTimeout(node, now);

                    result = ethNodeRecv (node, NODE_ROUTE_UDP);
                    if (NODE_STATUS_ERROR == result.status)
                        return ethNodeProcessFailure (node, NODE_ROUTE_UDP, NULL, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_PING_PONG_MISSED));

                    // Require a NEIGHBORS message
                    message = result.u.success.message;
                    if (MESSAGE_DIS != message.identifier || DIS_MESSAGE_NEIGHBORS != message.u.dis.identifier)
                        return ethNodeProcessFailure (node, NODE_ROUTE_UDP, &message, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_PING_PONG_MISSED));

                    ethNodeSetDiscovered (node, ETHEREUM_BOOLEAN_TRUE);
                    ethNodeProcessRecvDIS (node, NODE_ROUTE_UDP, message.u.dis); // pass ownership

                    if (NODE_CONNECT_DISCOVER_ACK == node->states[route].u.connecting.type)
                        return ethNodeProcessSuccess (node, route, NULL, ethNodeStateCreateConnecting (NODE_CONNECT_DISCOVER_ACK_TOO));
                    else {
                        ethNodeProcessSuccess (node, NODE_ROUTE_UDP, NULL, ethNodeStateCreateConnected());
                        // This is success...
                        return ethNodeDisconnect(node, NODE_ROUTE_UDP, ethNodeStateCreate(NODE_AVAILABLE), ETHEREUM_BOOLEAN_FALSE);
                    }

            }
            break;

        case NODE_AVAILABLE:
        case NODE_ERROR:
            node->timeout = (time_t) -1;
            return node->states[route];
    }
}

extern int
ethNodeUpdateDescriptors (BREthereumNode node,
                       BREthereumNodeEndpointRoute route,
                       fd_set *recv,   // read
                       fd_set *send) {  // write
    int socket = ethNodeEndpointGetSocket(node->remote, route);

    // Do nothing - if there is no socket.
    if (-1 == socket) return -1;

    switch (node->states[route].type) {
        case NODE_AVAILABLE:
        case NODE_ERROR:
            break;

        case NODE_CONNECTED:
            if (NULL != recv)
                FD_SET (socket, recv);

            // If we have any provisioner with a pending message, we are willing to send
            for (size_t index = 0; index < array_count (node->provisioners); index++)
                if (provisionerSendMessagesPending (&node->provisioners[index])) {
                    if (NULL != send) FD_SET (socket, send);
                    break;
                }

            break;

        case NODE_CONNECTING:
            switch (node->states[route].u.connecting.type) {
                case NODE_CONNECT_OPEN:
                    assert (0);

                case NODE_CONNECT_AUTH:
                case NODE_CONNECT_HELLO:
                case NODE_CONNECT_PRE_STATUS_PONG_SEND:
                case NODE_CONNECT_STATUS:
                case NODE_CONNECT_PING:
                case NODE_CONNECT_PING_ACK_DISCOVER:
                case NODE_CONNECT_DISCOVER:
                    if (NULL != send) FD_SET (socket, send);
                    break;

                case NODE_CONNECT_AUTH_ACK:
                case NODE_CONNECT_HELLO_ACK:
                case NODE_CONNECT_PRE_STATUS_PING_RECV:
                case NODE_CONNECT_STATUS_ACK:
                case NODE_CONNECT_PING_ACK:
                case NODE_CONNECT_PING_ACK_DISCOVER_ACK:
                case NODE_CONNECT_DISCOVER_ACK:
                case NODE_CONNECT_DISCOVER_ACK_TOO:
                    if (NULL != recv) FD_SET (socket, recv);
                    break;
            }
            break;
    }
    // When connected, we are always willing to recv

    return socket;
}

/// MARK: - LES Node Support

/**
 * Extract the `type` and `subtype` of a message from the RLP-encoded `value`.  The `value` has
 * any applicable messagerIdOffset applied; thus we need to undo that offset.
 *
 * We've already assumed that we have one subprotocol (LES, PIP) and thus one and only one
 * offset to deal with.
 */
static void
extractIdentifier (BREthereumNode node,
                   uint8_t value,
                   BREthereumMessageIdentifier *type,
                   BREthereumANYMessageIdentifier *subtype) {
    if (value < node->coder.messageIdOffset || 0 == node->coder.messageIdOffset) {
        *type = MESSAGE_P2P;
        *subtype = value - 0x00;
    }
    else {
        switch (node->type) {
            case NODE_TYPE_UNKNOWN:
                assert (0);
            case NODE_TYPE_GETH:
                *type = MESSAGE_LES;
                break;
            case NODE_TYPE_PARITY:
                *type = MESSAGE_PIP;
                break;
        }
        *subtype = value - node->coder.messageIdOffset;
    }
}

/// MARK: - LES Node State

extern void
ethNodeSetStateInitial (BREthereumNode node,
                     BREthereumNodeEndpointRoute route,
                     BREthereumNodeState state) {
    // Assume that the route is AVAILABLE.
    node->states[route] = ethNodeStateCreate (NODE_AVAILABLE);

    switch (state.type) {
        case NODE_AVAILABLE:
        case NODE_CONNECTING:
        case NODE_CONNECTED:
            break;

        case NODE_ERROR:
            switch (state.u.error.type) {
                case NODE_ERROR_UNIX:
                case NODE_ERROR_DISCONNECT:
                    break;
                case NODE_ERROR_PROTOCOL:
                    switch (state.u.error.u.protocol) {
                        case NODE_PROTOCOL_NONSTANDARD_PORT:
                        case NODE_PROTOCOL_CAPABILITIES_MISMATCH:
                        case NODE_PROTOCOL_STATUS_MISMATCH:
                        case NODE_PROTOCOL_UDP_EXCESSIVE_BYTE_COUNT:
                        case NODE_PROTOCOL_RLP_PARSE:
                            node->states[route] = state; // no recover; adopt the PROTOCOL error.
                            break;

                        case NODE_PROTOCOL_EXHAUSTED:
                        case NODE_PROTOCOL_PING_PONG_MISSED:
                        case NODE_PROTOCOL_TCP_AUTHENTICATION:
                        case NODE_PROTOCOL_TCP_HELLO_MISSED:
                        case NODE_PROTOCOL_TCP_STATUS_MISSED:
                            break;
                        }
                    break;
            }
            break;
    }
}

/// MARK: - Send / Recv

static BREthereumNodeStatus
ethNodeSendFailed (BREthereumNode node,
                BREthereumNodeEndpointRoute route,
                BREthereumNodeState state) {
    ethNodeStateAnnounce (node, route, state);
    return NODE_STATUS_ERROR;
}

/**
 * Send `message` on `route` to `node`.  There is a consistency constraint whereby the message
 * identifier must be MESSAGE_DIS if and only if route is UDP.
 *
 * @param node
 * @param route
 * @param message
 */
static BREthereumNodeStatus
ethNodeSend (BREthereumNode node,
          BREthereumNodeEndpointRoute route,
          BREthereumMessage message) {

    int error = 0;

    assert ((NODE_ROUTE_UDP == route && MESSAGE_DIS == message.identifier) ||
            (NODE_ROUTE_UDP != route && MESSAGE_DIS != message.identifier));

    BRRlpItem item = messageEncode (message, node->coder);

#if defined (NEED_TO_AVOID_PROOFS_LOGGING)
    if (MESSAGE_LES != message.identifier || LES_MESSAGE_GET_PROOFS_V2 != message.u.les.identifier)
#endif
    LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Send: [ %s, %15s ] => %15s",
         messageGetIdentifierName (&message),
         messageGetAnyIdentifierName (&message),
         ethNodeEndpointGetHostname(node->remote));

    // Handle DIS messages specially.
    switch (message.identifier) {
        case MESSAGE_DIS: {
            // Extract the `item` bytes w/o the RLP length prefix.  This ends up being
            // simply the raw bytes.  We *know* the `item` is an RLP encoding of bytes; thus we
            // use `rlpDecodeBytes` (rather than `rlpDecodeList`.  Then simply send them.
            BRRlpData data = rlpDecodeBytesSharedDontRelease (node->coder.rlp, item);

            pthread_mutex_lock (&node->lock);
            error = ethNodeEndpointSendData (node->remote, route, data.bytes, data.bytesCount);
            pthread_mutex_unlock (&node->lock);
            break;
        }

        default: {
#if defined (NODE_SHOW_SEND_RLP_ITEMS)
            if ((MESSAGE_PIP == message.identifier && PIP_MESSAGE_STATUS != message.u.pip.type) ||
                (MESSAGE_LES == message.identifier && LES_MESSAGE_STATUS != message.u.les.identifier))
                rlpShowItem (node->coder.rlp, item, "SEND");
#elif defined (NODE_SHOW_SEND_TX_ALWAYS)
            if ((MESSAGE_PIP == message.identifier && PIP_MESSAGE_RELAY_TRANSACTIONS == message.u.pip.type) ||
                (MESSAGE_LES == message.identifier && LES_MESSAGE_SEND_TX2 == message.u.les.identifier) ||
                (MESSAGE_LES == message.identifier && LES_MESSAGE_SEND_TX  == message.u.les.identifier))
                rlpItemShow (node->coder.rlp, item, "SEND");
#endif
            
            // Extract the `items` bytes w/o the RLP length prefix.  We *know* the `item` is an
            // RLP encoding of a list; thus we use `rlpDecodeList`.
            BRRlpData data = rlpDecodeListSharedDontRelease(node->coder.rlp, item);

            // Encrypt the length-less data
            BRRlpData encryptedData;
            pthread_mutex_lock (&node->lock);
            frameCoderEncrypt(node->frameCoder,
                              data.bytes, data.bytesCount,
                              &encryptedData.bytes, &encryptedData.bytesCount);

            error = ethNodeEndpointSendData (node->remote, route, encryptedData.bytes, encryptedData.bytesCount);
            pthread_mutex_unlock (&node->lock);
            rlpDataRelease(encryptedData);
            break;
        }
    }
    rlpItemRelease (node->coder.rlp, item);

#if defined (NEED_TO_PRINT_SEND_RECV_DATA)
    if (!error)
        LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Size: Send: %s: PayLoad: %zu",
             ethNodeEndpointRouteGetName(route),
             data.bytesCount);
#endif

    return (0 == error
            ? NODE_STATUS_SUCCESS
            : ethNodeSendFailed (node, route, ethNodeStateCreateErrorUnix (error)));
}

static BREthereumNodeMessageResult
ethNodeRecvFailed (BREthereumNode node,
                BREthereumNodeEndpointRoute route,
                BREthereumNodeState state) {
    ethNodeStateAnnounce (node, route, state);
    return (BREthereumNodeMessageResult) { NODE_STATUS_ERROR };
}

static BREthereumNodeMessageResult
ethNodeRecv (BREthereumNode node,
          BREthereumNodeEndpointRoute route) {
    uint8_t *bytes = node->recvDataBuffer.bytes;
    size_t   bytesLimit = node->recvDataBuffer.bytesCount;
    size_t   bytesCount = 0;
    int error;

    BREthereumMessage message;

    rlpCoderClrFailed (node->coder.rlp);

    switch (route) {
        case NODE_ROUTE_UDP: {
            bytesCount = 1500;

            error = ethNodeEndpointRecvData (node->remote, route, bytes, &bytesCount, 0);
            if (error) return ethNodeRecvFailed (node, NODE_ROUTE_UDP, ethNodeStateCreateErrorUnix (error));
            if (bytesCount > 1500)
                return ethNodeRecvFailed(node, NODE_ROUTE_UDP,
                                      ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_UDP_EXCESSIVE_BYTE_COUNT));

            // Wrap at RLP Byte
            BRRlpItem item = rlpEncodeBytes (node->coder.rlp, bytes, bytesCount);

            message = messageDecode (item, node->coder,
                                     MESSAGE_DIS,
                                     MESSAGE_DIS_IDENTIFIER_ANY);
            rlpItemRelease (node->coder.rlp, item);
            break;
        }

        case ETHEREUM_NODE_ROUTE_TCP: {
            size_t headerCount = 32;

            {
                // get header, decrypt it, validate it and then determine the bytesCount
                uint8_t header[32];
                memset(header, -1, 32);

                error = ethNodeEndpointRecvData (node->remote, route, header, &headerCount, 1);
                if (error) return ethNodeRecvFailed (node, ETHEREUM_NODE_ROUTE_TCP, ethNodeStateCreateErrorUnix (error));

                pthread_mutex_lock (&node->lock);
                if (ETHEREUM_BOOLEAN_IS_FALSE(frameCoderDecryptHeader(node->frameCoder, header, 32))) {
                    pthread_mutex_unlock (&node->lock);
                    return ethNodeRecvFailed (node, ETHEREUM_NODE_ROUTE_TCP, ethNodeStateCreateErrorProtocol(NODE_PROTOCOL_TCP_AUTHENTICATION));
                }
                pthread_mutex_unlock (&node->lock);
                headerCount = ((uint32_t)(header[2]) <<  0 |
                               (uint32_t)(header[1]) <<  8 |
                               (uint32_t)(header[0]) << 16);

                // ??round to 16 ?? 32 ??
                bytesCount = headerCount + ((16 - (headerCount % 16)) % 16) + 16;
                // bytesCount = (headerCount + 15) & ~15;

                // ?? node->bodySize = headerCount; ??
            }

            // Given bytesCount, update recvDataBuffer if too small
            pthread_mutex_lock (&node->lock);
            if (bytesCount > bytesLimit) {
                // Expand recvDataBuffer, with some margin
                node->recvDataBuffer = (BRRlpData) {
                    2 * bytesCount,
                    realloc(node->recvDataBuffer.bytes, 2 * bytesCount)
                };
                bytes = node->recvDataBuffer.bytes;
                // bytesLimit = node->recvDataBuffer.bytesCount;
            }
            pthread_mutex_unlock (&node->lock);

#if defined (NEED_TO_PRINT_SEND_RECV_DATA)
            LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Size: Recv: TCP: PayLoad: %u, Frame: %zu", headerCount, bytesCount);
#endif
            
            // get body/frame
            error = ethNodeEndpointRecvData (node->remote, route, bytes, &bytesCount, 1);
            if (error) return ethNodeRecvFailed (node, ETHEREUM_NODE_ROUTE_TCP, ethNodeStateCreateErrorUnix (error));

            pthread_mutex_lock (&node->lock);
            frameCoderDecryptFrame(node->frameCoder, bytes, bytesCount);
            pthread_mutex_unlock (&node->lock);

            // ?? node->bodySize = headerCount; ??

            // Identifier is at byte[0]
            BRRlpData identifierData = { 1, &bytes[0] };
            BRRlpItem identifierItem = rlpDataGetItem (node->coder.rlp, identifierData);
            uint8_t value = (uint8_t) rlpDecodeUInt64 (node->coder.rlp, identifierItem, 1);

            BREthereumMessageIdentifier type;
            BREthereumANYMessageIdentifier subtype;

            extractIdentifier(node, value, &type, &subtype);

            // Actual body
            BRRlpData data = { headerCount - 1, &bytes[1] };
            BRRlpItem item = rlpDataGetItem (node->coder.rlp, data);

#if defined (NEED_TO_PRINT_SEND_RECV_DATA)
            LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Size: Recv: TCP: Type: %u, Subtype: %d", type, subtype);
#endif

            // Finally, decode the message
            message = messageDecode (item, node->coder, type, subtype);
#if defined (NODE_SHOW_RECV_RLP_ITEMS)
            if (!rlpCoderHasFailed(node->coder.rlp) &&
                ((MESSAGE_PIP == message.identifier && PIP_MESSAGE_STATUS != message.u.pip.type) ||
                 (MESSAGE_LES == message.identifier && LES_MESSAGE_STATUS != message.u.les.identifier)))
                rlpShowItem(node->coder.rlp, item, "RECV");
#endif

            // If this is a LES response message, then it has credit information.
            if (!rlpCoderHasFailed(node->coder.rlp) &&
                MESSAGE_LES == message.identifier &&
                messageLESHasUse (&message.u.les, LES_MESSAGE_USE_RESPONSE))
                node->credits = messageLESGetCredits (&message.u.les);
            
            rlpItemRelease (node->coder.rlp, item);
            rlpItemRelease (node->coder.rlp, identifierItem);

            break;
        }
    }

    if (!rlpCoderHasFailed(node->coder.rlp) ) {
        char disconnect[64] = { '\0' };

        if (MESSAGE_P2P == message.identifier && P2P_MESSAGE_DISCONNECT == message.u.p2p.identifier)
            sprintf (disconnect, " (%s)", messageP2PDisconnectDescription (message.u.p2p.u.disconnect.reason));

        LOG (LL_INFO, ETH_LES_LOG_TOPIC, "Recv: [ %s, %15s ] <= %15s%s",
             messageGetIdentifierName (&message),
             messageGetAnyIdentifierName (&message),
             ethNodeEndpointGetHostname(node->remote),
             disconnect);
    }

    if (rlpCoderHasFailed(node->coder.rlp))
        messageRelease(&message);

    return (rlpCoderHasFailed(node->coder.rlp)
            ? (BREthereumNodeMessageResult) {
                NODE_STATUS_ERROR,
                { .error = {}}}
            : (BREthereumNodeMessageResult) {
                NODE_STATUS_SUCCESS,
                { .success = { message }}});
}


#pragma clang diagnostic push
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-function"
static uint64_t
ethNodeEstimateCredits (BREthereumNode node,
                     BREthereumMessage message) {
    switch (message.identifier) {
        case MESSAGE_P2P: return 0;
        case MESSAGE_DIS: return 0;
        case MESSAGE_ETH: return 0;
        case MESSAGE_LES:
            return (node->specs[message.u.les.identifier].baseCost +
                    messageLESGetCreditsCount (&message.u.les) * node->specs[message.u.les.identifier].reqCost);
        case MESSAGE_PIP: return 0;
    }
}

static uint64_t
ethNodeGetCredits (BREthereumNode node) {
    return node->credits;
}
#pragma clang diagnostic pop
#pragma GCC diagnostic pop

/// MARK: - Discovered

extern BREthereumBoolean
ethNodeGetDiscovered (BREthereumNode node) {
    return node->discovered;
}

extern void
ethNodeSetDiscovered (BREthereumNode node,
                   BREthereumBoolean discovered) {
    node->discovered = discovered;
}

extern BREthereumNodeStatus
ethNodeDiscover (BREthereumNode node,
              const BREthereumNodeEndpoint endpoint) {
    BREthereumMessage findNodes = {
        MESSAGE_DIS,
        { .dis = {
            DIS_MESSAGE_FIND_NEIGHBORS,
            { .findNeighbors =
                messageDISFindNeighborsCreate (ethNodeEndpointGetDISNeighbor(endpoint).key,
                                               (uint64_t) time(NULL) + 1000000) },
            ethNodeEndpointGetDISNeighbor(ethNodeGetLocalEndpoint(node)).key }}
    };

    BREthereumNodeStatus status = ethNodeSend (node, NODE_ROUTE_UDP, findNodes);
    messageRelease(&findNodes);
    return status;
}

extern BREthereumBoolean
ethNodeHandleTime (BREthereumNode node,
                BREthereumNodeEndpointRoute route,
                time_t now,
                BREthereumBoolean tryPing) {
    if (now != (time_t) -1 &&
        node->timeout != (time_t) -1 &&
        now >= node->timeout) {
        // The node has timed out.

        if (ETHEREUM_BOOLEAN_IS_FALSE (tryPing) ||
            ETHEREUM_BOOLEAN_IS_FALSE (node->timeoutPingAllowed)) {
            ethNodeDisconnect (node, route, ethNodeStateCreateErrorDisconnect (P2P_MESSAGE_DISCONNECT_TIMEOUT), ETHEREUM_BOOLEAN_FALSE);
            return ETHEREUM_BOOLEAN_TRUE;
        }

        // Try a ping
        BREthereumMessage ping = {
            MESSAGE_P2P,
            { .p2p = {
                P2P_MESSAGE_PING, {}}}
        };

        if (NODE_STATUS_SUCCESS != ethNodeSend (node, route, ping)) {
            ethNodeDisconnect (node, route, ethNodeStateCreateErrorDisconnect (P2P_MESSAGE_DISCONNECT_TIMEOUT), ETHEREUM_BOOLEAN_FALSE);
            return ETHEREUM_BOOLEAN_TRUE;
        }

        // On another timeout, don't allow a ping.
        node->timeoutPingAllowed = ETHEREUM_BOOLEAN_FALSE;

        // Must respond w/i a shorter 'recv' time.
        ethNodeUpdateTimeoutRecv (node, now);

    }

    // When not a timeout, we'll enable ping.
    else node->timeoutPingAllowed = ETHEREUM_BOOLEAN_TRUE;

    //
    return ETHEREUM_BOOLEAN_FALSE;
}

/// MARK: - Auth Support

static void
bytesXOR(uint8_t * op1, uint8_t* op2, uint8_t* result, size_t len) {
    for (unsigned int i = 0; i < len;  ++i) {
        result[i] = op1[i] ^ op2[i];
    }
}

static void
_BRECDH(void *out32, const BRKey *privKey, BRKey *pubKey)
{
    uint8_t p[65];
    size_t pLen = BRKeyPubKey(pubKey, p, sizeof(p));

    if (pLen == 65) p[0] = (p[64] % 2) ? 0x03 : 0x02; // convert to compressed pubkey format
    BRSecp256k1PointMul((BRECPoint *)p, &privKey->secret); // calculate shared secret ec-point
    memcpy(out32, &p[1], 32); // unpack the x coordinate

    mem_clean(p, sizeof(p));
}


static int // 0 on success
_sendAuthInitiator(BREthereumNode node) {

    // LOG (LL_INFO, ETH_LES_LOG_TOPIC, "%s", "generating auth initiator");

    // authInitiator -> E(remote-pubk, S(ephemeral-privk, static-shared-secret ^ nonce) || H(ephemeral-pubk) || pubk || nonce || 0x0)
    uint8_t * authBuf = node->authBuf;
    uint8_t * authBufCipher = node->authBufCipher;

    uint8_t* signature = &authBuf[0];
    uint8_t* hPubKey = &authBuf[SIG_SIZE_BYTES];
    uint8_t* pubKey = &authBuf[SIG_SIZE_BYTES + HEPUBLIC_BYTES];
    uint8_t* nonce =  &authBuf[SIG_SIZE_BYTES + HEPUBLIC_BYTES + PUBLIC_SIZE_BYTES];
    BRKey* localKey  = ethNodeEndpointGetKey (node->local);
    BRKey* remoteKey = ethNodeEndpointGetKey (node->remote);

    //static-shared-secret = ecdh.agree(privkey, remote-pubk)
    UInt256 staticSharedSecret;
    _BRECDH(staticSharedSecret.u8, localKey, remoteKey);

    //static-shared-secret ^ nonce
    UInt256 xorStaticNonce;
    UInt256* localNonce =  ethNodeEndpointGetNonce(node->local);
    BRKey* localEphemeral = ethNodeEndpointGetEphemeralKey(node->local);
    memset(xorStaticNonce.u8, 0, 32);
    bytesXOR(staticSharedSecret.u8, localNonce->u8, xorStaticNonce.u8, sizeof(localNonce->u8));

    // S(ephemeral-privk, static-shared-secret ^ nonce)
    // Determine the signature length
    size_t signatureLen = BRKeyCompactSignEthereum (localEphemeral,
                                                    NULL, 0,
                                                    xorStaticNonce);
    assert (65 == signatureLen);
    
    // Fill the signature
    BRKeyCompactSignEthereum (localEphemeral,
                              signature, signatureLen,
                              xorStaticNonce);

    // || H(ephemeral-pubk)||
    memset(&hPubKey[32], 0, 32);
    uint8_t ephPublicKey[65];
    BRKeyPubKey(localEphemeral, ephPublicKey, 65);
    BRKeccak256(hPubKey, &ephPublicKey[1], PUBLIC_SIZE_BYTES);
    // || pubK ||
    uint8_t nodePublicKey[65] = {0};
    BRKeyPubKey(localKey, nodePublicKey, 65);
    memcpy(pubKey, &nodePublicKey[1], PUBLIC_SIZE_BYTES);
    // || nonce ||
    memcpy(nonce, localNonce->u8, sizeof(localNonce->u8));
    // || 0x0   ||
    authBuf[AUTH_BUF_LEN - 1] = 0x0;

    // E(remote-pubk, S(ephemeral-privk, static-shared-secret ^ nonce) || H(ephemeral-pubk) || pubk || nonce || 0x0)
    BRKeyECIESAES128SHA256Encrypt(remoteKey, authBufCipher, AUTH_CIPHER_BUF_LEN, localEphemeral, authBuf, AUTH_BUF_LEN);
    return 0;
}

//static void
//_readAuthFromInitiator(BREthereumNode node) {
//    BRKey* nodeKey = &node->local.key; // ethNodeGetKey(node);
//    LOG (LL_INFO, ETH_LES_LOG_TOPIC, "%s", "received auth from initiator");
//
//    size_t len = BRKeyECIESAES128SHA256Decrypt(nodeKey, node->authBuf, AUTH_BUF_LEN, node->authBufCipher, AUTH_CIPHER_BUF_LEN);
//
//    if (len != AUTH_BUF_LEN) {
//        //TODO: call _readAuthFromInitiatorEIP8...
//    }
//    else {
//        //copy remote nonce
//        UInt256* remoteNonce = &node->remote.nonce; // ethNodeGetPeerNonce(node);
//        memcpy(remoteNonce->u8, &node->authBuf[SIG_SIZE_BYTES + HEPUBLIC_BYTES + PUBLIC_SIZE_BYTES], sizeof(remoteNonce->u8));
//
//        //copy remote public key
//        uint8_t remotePubKey[65];
//        remotePubKey[0] = 0x04;
//        BRKey* remoteKey = &node->remote.key; // ethNodeGetPeerKey(node);
//        remoteKey->compressed = 0;
//        memcpy(&remotePubKey[1], &node->authBuf[SIG_SIZE_BYTES + HEPUBLIC_BYTES], PUBLIC_SIZE_BYTES);
//        BRKeySetPubKey(remoteKey, remotePubKey, 65);
//
//        UInt256 sharedSecret;
//        _BRECDH(sharedSecret.u8, nodeKey, remoteKey);
//
//        UInt256 xOrSharedSecret;
//        bytesXOR(sharedSecret.u8, remoteNonce->u8, xOrSharedSecret.u8, sizeof(xOrSharedSecret.u8));
//
//        // The ephemeral public key of the remote peer
//        BRKey* remoteEphemeral = &node->remote.ephemeralKey; // ethNodeGetPeerEphemeral(node);
//        BRKeyRecoverPubKeyEthereum(remoteEphemeral, xOrSharedSecret, node->authBuf, SIG_SIZE_BYTES);
//    }
//}
//
//static void
//_sendAuthAckToInitiator(BREthereumNode node) {
//    LOG (LL_INFO, ETH_LES_LOG_TOPIC, "%s", "generating auth ack for initiator");
//
//    // authRecipient -> E(remote-pubk, epubK|| nonce || 0x0)
//    uint8_t* ackBuf = node->ackBuf;
//    uint8_t* ackBufCipher = node->ackBufCipher;
//    BRKey* remoteKey = &node->remote.key; // ethNodeGetPeerKey(node);
//
//    uint8_t* pubKey = &ackBuf[0];
//    uint8_t* nonce =  &ackBuf[PUBLIC_SIZE_BYTES];
//
//    // || epubK ||
//    uint8_t localEphPublicKey[65];
//    BRKey* localEphemeral = &node->local.ephemeralKey; // ethNodeGetEphemeral(node);
//    size_t ephPubKeyLength = BRKeyPubKey(localEphemeral, localEphPublicKey, 65);
//    assert(ephPubKeyLength == 65);
//    memcpy(pubKey, &localEphPublicKey[1], 64);
//
//    // || nonce ||
//    UInt256* localNonce = &node->local.nonce; // ethNodeGetNonce(node);
//    memcpy(nonce, localNonce->u8, sizeof(localNonce->u8));
//    // || 0x0   ||
//    ackBuf[ACK_BUF_LEN- 1] = 0x0;
//
//    //E(remote-pubk, epubK || nonce || 0x0)
//    BRKeyECIESAES128SHA256Encrypt(remoteKey, ackBufCipher, ACK_CIPHER_BUF_LEN, localEphemeral, ackBuf, ACK_BUF_LEN);
//
//}

static int // 0 on success
_readAuthAckFromRecipient(BREthereumNode node) {

    BRKey* nodeKey = ethNodeEndpointGetKey (node->local);

    // LOG (LL_INFO, ETH_LES_LOG_TOPIC,"%s", "received auth ack from recipient");

    size_t len = BRKeyECIESAES128SHA256Decrypt(nodeKey, node->ackBuf, ACK_BUF_LEN, node->ackBufCipher, ACK_CIPHER_BUF_LEN);

    if (len != ACK_BUF_LEN) {
        //TODO: call _readAckAuthFromRecipientEIP8...
        return 1;
    }
    else {
        //copy remote nonce key
        UInt256* nonce = ethNodeEndpointGetNonce(node->remote);
        memcpy(nonce->u8, &node->ackBuf[PUBLIC_SIZE_BYTES], sizeof(nonce->u8));

        //copy ephemeral public key of the remote peer
        uint8_t remoteEPubKey[65];
        remoteEPubKey[0] = 0x04;
        BRKey* remoteEphemeral = ethNodeEndpointGetEphemeralKey(node->remote);
        memcpy(&remoteEPubKey[1], node->ackBuf, PUBLIC_SIZE_BYTES);
        BRKeySetPubKey(remoteEphemeral, remoteEPubKey, 65);
        return 0;
    }
}
