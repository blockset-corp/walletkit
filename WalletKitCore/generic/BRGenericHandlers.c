//
//  BRGenericHandlers.c
//  Core
//
//  Created by Ed Gamble on 6/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "support/BRArray.h"
#include "BRGenericHandlers.h"
#include "BRCryptoBase.h"

BRGenericHandlers arrayOfHandlers[NUMBER_OF_NETWORK_TYPES];

extern void
genHandlersInstall (const BRGenericHandlers handlers) {
    arrayOfHandlers[handlers->type] = handlers;
}

extern const BRGenericHandlers
genHandlerLookup (BRCryptoNetworkCanonicalType type) {
    return arrayOfHandlers[type];
}

