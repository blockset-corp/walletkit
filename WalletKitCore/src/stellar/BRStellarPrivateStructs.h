//
//  BRStellarPrivateStructs.h
//  WalletKitCore
//
//  Created by Carl Cherry on 5/21/2019.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRStellar_private_structs_h
#define BRStellar_private_structs_h

#include <stdbool.h>
#include "BRStellarBase.h"
#include "support/BRKey.h"

#define STELLAR_ADDRESS_BYTES   (32)

struct BRStellarAddressRecord {
    uint8_t bytes[STELLAR_ADDRESS_BYTES];
};

#endif
