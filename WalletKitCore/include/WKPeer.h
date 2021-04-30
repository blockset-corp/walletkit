//
//  WKPeer.h
//  WalletKitCore
//
//  Created by Ed Gamble on 10/17/2019.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKPeer_h
#define WKPeer_h

#include "WKBase.h"
#include "WKNetwork.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A Peer as a remote host that can be used for peer-to-peer sync modes
 *
 * @discussion Although not clear in the interface, relevent only to BTC-related networks
 */
typedef struct WKPeerRecord *WKPeer;

/**
 * Create a peer on network at `address` and `port` and optionally with `publicKey`.
 */
extern WKPeer
wkPeerCreate (WKNetwork network,
                  const char *address,
                  uint16_t port,
                  const char *publicKey);

/**
 * The peer's address as 16 bytes.
 */
extern WKData16
wkPeerGetAddrAsInt (WKPeer peer);

/**
 * Get the peer's network
 */
extern WKNetwork
wkPeerGetNetwork (WKPeer peer);

/**
 * Get the peer's address.
 */
extern const char *
wkPeerGetAddress (WKPeer peer);

/**
 * Get the peer's port.
 */
extern uint16_t
wkPeerGetPort (WKPeer peer);

extern const char *
wkPeerGetPublicKey (WKPeer peer);

extern uint8_t *
wkPeerSerialize (WKPeer peer, size_t *bytesCount);

extern WKBoolean
wkPeerIsIdentical (WKPeer p1, WKPeer p2);

DECLARE_WK_GIVE_TAKE (WKPeer, wkPeer);

#ifdef __cplusplus
}
#endif

#endif /* WKPeer_h */
