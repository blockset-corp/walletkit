//
//  BRTezosEncoder.h
//  Core
//
//  Created by Ehsan Rezaie on 2020-07-22.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRTezosEncoder_h
#define BRTezosEncoder_h

#include <stdint.h>
#include <assert.h>
#include "BRTezosBase.h"
#include "BRTezosTransaction.h"

#ifdef __cplusplus
extern "C" {
#endif

extern BRCryptoData
encodeZarith (int64_t value);

extern BRCryptoData
tezosSerializeTransaction (BRTezosTransaction tx);

extern BRCryptoData
tezosSerializeOperationList (BRTezosTransaction * tx, size_t txCount, BRTezosHash blockHash);


#ifdef __cplusplus
}
#endif

#endif /* BRTezosEncoder_h */
