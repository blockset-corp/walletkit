//
//  BRTezosBase.h
//  Core
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRTezosBase_h
#define BRTezosBase_h

#include <inttypes.h>
#include <stdbool.h>
#include <arpa/inet.h>          // htonl()
#include "BRCryptoBase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t bytes[48];
} BRTezosTransactionHash;

typedef int64_t BRTezosUnitMutez;
#define TEZOS_TEZ_SCALE_FACTOR       (1000000)  // 1 TEZ = 1e6 MUTEZ
#define TEZOS_TEZ_TO_MUTEZ(x)        ((x) * TEZOS_TEZ_SCALE_FACTOR)

#ifdef __cplusplus
}
#endif

#endif /* BRTezosBase_h */
