//
//  BRCryptoFeeBasisP.h
//  BRCore
//
//  Created by Ed Gamble on 11/22/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoFeeBasisP_h
#define BRCryptoFeeBasisP_h

#include "BRCryptoFeeBasis.h"
#include "BRCryptoBaseP.h"

#ifdef __cplusplus
extern "C" {
#endif

struct BRCryptoFeeBasisRecord {
    BRCryptoAmount pricePerCostFactor;
    double costFactor;
    BRCryptoRef ref;
};

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoFeeBasisP_h */
