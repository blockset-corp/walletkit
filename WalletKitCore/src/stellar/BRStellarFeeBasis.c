//
//  BRStellarFeeBasis.c
//  WalletKitCore
//
//  Created by Carl Cherry on 2021-06-02.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRStellarFeeBasis.h"

extern BRStellarAmount stellarFeeBasisGetPricePerCostFactor(BRStellarFeeBasis *feeBasis)
{
    return feeBasis ? feeBasis->pricePerCostFactor : 100;
}

extern uint32_t stellarFeeBasisGetCostFactor(BRStellarFeeBasis *feeBasis)
{
    return feeBasis ? feeBasis->costFactor : 1;
}

extern BRStellarAmount stellarFeeBasisGetFee(BRStellarFeeBasis *feeBasis)
{
    return feeBasis ? feeBasis->costFactor * feeBasis->pricePerCostFactor : 100;
}

extern uint32_t stellarFeeBasisIsEqual(BRStellarFeeBasis *fb1, BRStellarFeeBasis *fb2)
{
    assert(fb1);
    assert(fb2);
    if (fb1->pricePerCostFactor == fb2->pricePerCostFactor &&
        fb1->costFactor == fb2->costFactor) {
        return 1;
    }
    return 0;
}

extern void stellarFeeBasisSet (uint32_t costFactor, BRStellarFee pricePerCostFactor,         BRStellarFeeBasis *feeBasis)
{
    assert (feeBasis);
    feeBasis->costFactor = costFactor;
    feeBasis->pricePerCostFactor = pricePerCostFactor;
}
