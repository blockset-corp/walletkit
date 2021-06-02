//
//  BRStellarAddress.h
//  WalletKitCore
//
//  Created by Carl Cherry on 2021-06-02.
//  Copyright © 2021 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRStellarAddress_h
#define BRStellarAddress_h

#include <stdio.h>
#include "BRStellarBase.h"
#include "support/BRKey.h"

typedef struct BRStellarAddressRecord *BRStellarAddress;

extern BRStellarAddress
stellarAddressCreate(BRKey * publicKey);

extern BRStellarAddress stellarAddressClone (BRStellarAddress address);

extern BRStellarAddress
stellarAddressCreateFromString(const char * stellarAddressString, bool strict);

extern char *
stellarAddressAsString(BRStellarAddress address);

extern void
stellarAddressFree(BRStellarAddress address);

extern size_t
stellarAddressHashValue (BRStellarAddress address);

/**
 * Compare 2 stellar addresses
 *
 * @param a1  first address
 * @param a2  second address
 *
 * @return 1 - if addresses are equal
 *         0 - if not equal
 */
extern int
stellarAddressEqual (BRStellarAddress a1, BRStellarAddress a2);

extern size_t
stellarAddressGetRawSize (BRStellarAddress address);
extern void stellarAddressGetRawBytes (BRStellarAddress address, uint8_t *buffer, size_t bufferSize);

#endif /* BRStellarAddress_h */
