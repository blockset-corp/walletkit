//
//  WKPeer.h
//  WalletKitCore
//
//  Created by Ed Gamble on 10/17/2019.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <arpa/inet.h>
#include "support/BRArray.h"
#include "support/BRInt.h"
#include "WKPeer.h"

struct WKPeerRecord {
    WKNetwork network;
    char *address;
    uint16_t port;
    char *publicKey;

    // Address parsed as AF_INET or AF_INET6
    UInt128 addressAsInt;

    WKRef ref;
};

IMPLEMENT_WK_GIVE_TAKE (WKPeer, wkPeer);

extern WKPeer
wkPeerCreate (WKNetwork network,
                  const char *address,
                  uint16_t port,
                  const char *publicKey) {
    // Require `address`
    if (NULL == address || 0 == strlen (address)) return NULL;

    // Require `address` to parse as a INET address
    UInt128 addressAsInt = UINT128_ZERO;

    struct in_addr addr;
    struct in6_addr addr6;

    if (1 == inet_pton (AF_INET6, address, &addr6)) {
        addressAsInt = *((UInt128*) &addr6.s6_addr);
    }
    else if (1 == inet_pton (AF_INET, address, &addr)) {
        addressAsInt.u16[5] = 0xffff;
        addressAsInt.u32[3] = addr.s_addr;
    }
    else return NULL;

    WKPeer peer = calloc (1, sizeof (struct WKPeerRecord));

    peer->network   = wkNetworkTake (network);
    peer->address   = strdup (address);
    peer->port      = port;
    peer->publicKey = (NULL == publicKey ? NULL : strdup(publicKey));
    peer->addressAsInt = addressAsInt;
    peer->ref       = WK_REF_ASSIGN (wkPeerRelease);

    return peer;
}

static void
wkPeerRelease (WKPeer peer) {
    wkNetworkGive (peer->network);
    free (peer->address);
    if (NULL != peer->publicKey) free (peer->publicKey);

    memset (peer, 0, sizeof(*peer));
    free (peer);
}

extern WKData16
wkPeerGetAddrAsInt (WKPeer peer) {
    WKData16 addrAsInt;
    memcpy (addrAsInt.data, peer->addressAsInt.u8, sizeof (addrAsInt.data));
    return addrAsInt;
}

extern WKNetwork
wkPeerGetNetwork (WKPeer peer) {
    return wkNetworkTake (peer->network);
}

extern const char *
wkPeerGetAddress (WKPeer peer) {
    return peer->address;
}

extern uint16_t
wkPeerGetPort (WKPeer peer) {
    return peer->port;
}

extern const char *
wkPeerGetPublicKey (WKPeer peer) {
    return peer->publicKey;
}

extern uint8_t *
wkPeerSerialize (WKPeer peer, size_t *bytesCount) {
    assert (NULL != bytesCount);
    *bytesCount = 0;
    return NULL;
}

extern WKBoolean
wkPeerIsIdentical (WKPeer p1, WKPeer p2) {
    return AS_WK_BOOLEAN (p1 == p2);
}
