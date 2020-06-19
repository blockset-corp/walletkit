//
//  BRTezosAddress.h
//  Core
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRTezosAddress_h
#define BRTezosAddress_h

#include "support/BRKey.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// implicit (tz) address bytes = tag (00) + curve (00|01|02) + pkh (20 bytes)
#define TEZOS_ADDRESS_SIZE 22
// address string is Base58(prefix + Blake2b(publicKey) (20 bytes) + checksum (4 bytes))

typedef struct BRTezosAddressRecord *BRTezosAddress;

#ifdef __cplusplus
}
#endif

#endif /* BRTezosAddress_h */

