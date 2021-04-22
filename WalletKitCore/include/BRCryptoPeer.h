//
//  BRCryptoPeer.h
//  WalletKitCore
//
//  Created by Ed Gamble on 10/17/2019.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoPeer_h
#define BRCryptoPeer_h

#include "BRCryptoBase.h"
#include "BRCryptoNetwork.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A Peer as a remote host that can be used for peer-to-peer sync modes
 *
 * @discussion Although not clear in the interface, relevent only to BTC-related networks
 */
typedef struct BRCryptoPeerRecord *BRCryptoPeer;

/**
 * Create a peer on network at `address` and `port` and optionally with `publicKey`.
 */
extern BRCryptoPeer
cryptoPeerCreate (BRCryptoNetwork network,
                  const char *address,
                  uint16_t port,
                  const char *publicKey);

/**
 * The peer's address as 16 bytes.
 */
extern BRCryptoData16
cryptoPeerGetAddrAsInt (BRCryptoPeer peer);

/**
 * Get the peer's network
 */
extern BRCryptoNetwork
cryptoPeerGetNetwork (BRCryptoPeer peer);

/**
 * Get the peer's address.
 */
extern const char *
cryptoPeerGetAddress (BRCryptoPeer peer);

/**
 * Get the peer's port.
 */
extern uint16_t
cryptoPeerGetPort (BRCryptoPeer peer);

extern const char *
cryptoPeerGetPublicKey (BRCryptoPeer peer);

extern uint8_t *
cryptoPeerSerialize (BRCryptoPeer peer, size_t *bytesCount);

extern BRCryptoBoolean
cryptoPeerIsIdentical (BRCryptoPeer p1, BRCryptoPeer p2);

DECLARE_CRYPTO_GIVE_TAKE (BRCryptoPeer, cryptoPeer);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoPeer_h */
