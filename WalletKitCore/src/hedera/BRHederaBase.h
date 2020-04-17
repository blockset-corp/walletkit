//
//  BRHederaBase.h
//  Core
//
//  Created by Carl Cherry on Oct. 15, 2019.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRHederaBase_h
#define BRHederaBase_h

#include <inttypes.h>
#include <stdbool.h>
#include <arpa/inet.h>          // htonl()
#include "BRCryptoBase.h"

#ifdef __cplusplus
extern "C" {
#endif

// Declare public/shared items
typedef int64_t BRHederaUnitTinyBar;
#define HEDERA_HBAR_SCALE_FACTOR       (100000000)  // 1 HBAR = 1e8 TINY_BAR
#define HEDERA_HBAR_TO_TINY_BAR(x)     ((x) * HEDERA_HBAR_SCALE_FACTOR)
#define HEDERA_TINY_BAR_TO_HBAR(x)     (((double) (x)) / HEDERA_HBAR_SCALE_FACTOR)

typedef struct __hedera_timestamp {
    int64_t seconds; // Number of complete seconds since the start of the epoch
    int32_t nano; // Number of nanoseconds since the start of the last second
} BRHederaTimeStamp;

typedef struct {
    uint8_t bytes[48];
} BRHederaTransactionHash;

// Needed for Android builds
#if !defined (ntohll)
#define ntohll(x) ((1==ntohl(1)) ? (x) : (((uint64_t)ntohl((x) & 0xFFFFFFFFUL)) << 32) | ntohl((uint32_t)((x) >> 32)))
#endif

#if !defined (htonll)
#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))
#endif

#ifdef __cplusplus
}
#endif

#endif /* BRHederaBase_h */
