//
//  BRTezosFeeBasis.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include <stdint.h>
#include "BRTezosFeeBasis.h"
#include "ethereum/util/BRUtilMath.h"
#include <stdio.h>
#include <assert.h>

extern BRTezosFeeBasis
tezosDefaultFeeBasis() {
    BRTezosFeeBasis feeBasis;
    // https://github.com/TezTech/eztz/blob/master/PROTO_004_FEES.md
    feeBasis.gasLimit = 10600;
    feeBasis.storageLimit = 300;
    feeBasis.fee = 1420;
    return feeBasis;
}

extern BRTezosUnitMutez
tezosFeeBasisGetPricePerCostFactor (BRTezosFeeBasis *feeBasis)
{
    return 1; //TODO:TEZOS 1 mutez / opbyte
}

extern uint64_t
tezosFeeBasisGetCostFactor (BRTezosFeeBasis *feeBasis)
{
    return feeBasis ? feeBasis->gasLimit : 1; //TODO:TEZOS
}

extern BRTezosUnitMutez
tezosFeeBasisGetFee (BRTezosFeeBasis *feeBasis)
{
    // min + (gas_limit * 0.1) + (opbytes * 1)
    return feeBasis ? feeBasis->fee : 0; //TODO:TEZOS default/min value?
}


extern uint32_t
tezosFeeBasisIsEqual (BRTezosFeeBasis *fb1, BRTezosFeeBasis *fb2)
{
    assert(fb1);
    assert(fb2);
    if (fb1->fee == fb2->fee &&
        fb1->gasLimit == fb2->gasLimit &&
        fb1->storageLimit == fb2->storageLimit) {
        return 1;
    }
    return 0;
}
