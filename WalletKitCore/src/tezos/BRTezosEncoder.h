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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t * bytes;
    size_t length;
} BRCryptoData;

static inline BRCryptoData* cryptoDataNew (size_t length) {
    BRCryptoData *data = calloc (1, sizeof (*data));
    assert (data != NULL);
    data->length = length;
    if (data->length < 1) data->length = 1;
    data->bytes = calloc (data->length, sizeof(*(data->bytes)));
    assert (data->bytes != NULL);
    return data;
}

static inline void cryptoDataFree (BRCryptoData *data) {
    if (data->bytes) free(data->bytes);
    free(data);
}


#ifdef __cplusplus
}
#endif

#endif /* BRTezosEncoder_h */
