//
//  BRBSVParams.h
//  Core
//
//  Created by Ehsan Rezaie on 2020-06-04.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRBSVParams_h
#define BRBSVParams_h

#include "bitcoin/BRChainParams.h"

#ifdef __cplusplus
extern "C" {
#endif
    
#define BSV_FORKID 0x40

extern const BRChainParams *BRBSVParams;
extern const BRChainParams *BRBSVTestNetParams;

static inline const BRChainParams *BRChainParamsGetBSV (int mainnet) {
    return mainnet ? BRBSVParams : BRBSVTestNetParams;
}

static inline int BRChainParamsIsBSV (const BRChainParams *params) {
    return BRBSVParams == params || BRBSVTestNetParams == params;
}

#ifdef __cplusplus
}
#endif

#endif // BRSVParams_h
