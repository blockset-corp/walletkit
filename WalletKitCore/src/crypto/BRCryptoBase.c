//
//  BRCryptoBase.c
//  BRCore
//
//  Created by Ed Gamble on 3/3/21.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoBaseP.h"

extern void
cryptoMemoryFreeExtern (void *memory) {
    cryptoMemoryFree(memory);
}
