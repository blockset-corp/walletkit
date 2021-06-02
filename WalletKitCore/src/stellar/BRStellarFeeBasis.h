//
//  BRStellarFeeBasis.h
//  WalletKitCore
//
//  Created by Carl Cherry on 2021-06-02.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRStellarFeeBasis_h
#define BRStellarFeeBasis_h

#include <stdint.h>
#include <stdio.h>
#include "BRStellarBase.h"

typedef struct
{
    BRStellarFee     pricePerCostFactor;
    uint32_t         costFactor;
} BRStellarFeeBasis;

extern BRStellarAmount stellarFeeBasisGetPricePerCostFactor(BRStellarFeeBasis *feeBasis);
extern uint32_t stellarFeeBasisGetCostFactor(BRStellarFeeBasis *feeBasis);
extern uint32_t stellarFeeBasisIsEqual(BRStellarFeeBasis *fb1, BRStellarFeeBasis *fb2);

extern BRStellarAmount stellarFeeBasisGetFee(BRStellarFeeBasis *feeBasis);

extern void stellarFeeBasisSet (uint32_t costFactor, BRStellarAmount pricePerCostFactor, BRStellarFeeBasis *feeBasis);


#endif /* BRStellarFeeBasis_h */
