//
//  BREthereumProofOfWork.c
//  WalletKitCore
//
//  Created by Ed Gamble on 12/14/18.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <stdlib.h>
#include <stdarg.h>
#include "support/BRArray.h"
#include "support/BRSet.h"
#include "support/rlp/BRRlp.h"
#include "BREthereumBlock.h"

#define POW_WORD_BYTES            (4)
#define POW_DATA_SET_INIT         (1 << 30)
#define POW_DATA_SET_GROWTH       (1 << 23)
#define POW_CACHE_INIT            (1 << 24)
#define POW_CACHE_GROWTH          (1 << 17)
#define POW_EPOCH                 (30000)
#define POW_MIX_BYTES             (128)
#define POW_HASH_BYTES            (64)
#define POW_PARENTS               (256)
#define POW_CACHE_ROUNDS          (3)
#define POW_ACCESSES              (64)

//
// Proof Of Work
//
struct BREthereumProofOfWorkStruct {
    int foo;
};

extern BREthereumProofOfWork
ethProofOfWorkCreate (void) {
    BREthereumProofOfWork pow = calloc (1, sizeof (struct BREthereumProofOfWorkStruct));

    pow->foo = 0;

    return pow;
}

extern void
ethProofOfWorkRelease (BREthereumProofOfWork pow) {
    free (pow);
}

#if defined (INCLUDE_UNUSED_FUNCTIONS)
static uint64_t
ethPowEpoch (BREthereumBlockHeader header) {
    return ethBlockHeaderGetNonce(header) / POW_EPOCH;
}

static uint64_t
ethPowPrime (uint64_t x, uint64_t y) {
    return (0 == x % y
            ? x
            : ethPowPrime (x - 2 * y, y));
}

static uint64_t
ethPowDatasetSize (BREthereumBlockHeader header) {
    return ethPowPrime (POW_DATA_SET_INIT + POW_DATA_SET_GROWTH * ethPowEpoch(header) - POW_MIX_BYTES,
                     POW_MIX_BYTES);
}

static uint64_t
ethPowCacheSize (BREthereumBlockHeader header) {
    return ethPowPrime (POW_CACHE_INIT + POW_CACHE_GROWTH * ethPowEpoch(header) - POW_HASH_BYTES,
                     POW_HASH_BYTES);
}

static BREthereumHash
ethPowSeedHashInternal (uint64_t number) {
    BREthereumHash bytes;

    if (0 == number)
        memset (bytes.bytes, 0, 32);
    else
        bytes = ethPowSeedHashInternal(number - POW_EPOCH);

    return hashCreateFromData ((BRRlpData) { 32, bytes.bytes });
}

static BREthereumHash
ethPowSeedHash (BREthereumBlockHeader header) {
    return ethPowSeedHashInternal(ethBlockHeaderGetNumber(header));
}

static BREthereumData
ethPowLittleRMH (BREthereumData x, uint64_t i, uint64_t n) {
    return x;
}

static BREthereumData
ethPowBigRMH (BREthereumData x, uint64_t n) {
    for (uint64_t i = 0; i < n; i++)
        ethPowLittleRMH(x, i, n);
    return x;
}

static BREthereumData
ethPowCacheRounds (BREthereumData x, uint64_t y, uint64_t n) {
    return (0 == y
            ? x
            : (1 == y
               ? ethPowBigRMH (x, n)
               : powCacheRounds (ethPowBigRMH (x, n), y - 1, n)));
}

static void
ethPowBigFNV (BREthereumData x,
           BREthereumData y) {
}

static BREthereumData
ethPowMix (BREthereumData m,
        BREthereumData c,
        uint64_t i,
        uint64_t p) {
    return (0 == p
            ? m
            : m);
}

static BREthereumData
ethPowParents (BREthereumData c,
            uint64_t i,
            uint64_t p,
            BREthereumData m) {
    return (p < POW_PARENTS - 1
            ? ethPowParents(c, i, p + 1, ethPowMix (m, c, i, p + 1))
            : ethPowMix (m, c, i, p + 1));
}

static BREthereumData
ethPowDatasetItem (BREthereumData c,
                uint64_t i) {
    return ethPowParents(c, i, -i, (BREthereumData) { 0, NULL });
}
// On and On and On....
#endif

extern void
proofOfWorkGenerate(BREthereumProofOfWork pow,
                       BREthereumBlockHeader header) {
    //
}

extern void
proofOfWorkCompute (BREthereumProofOfWork pow,
                       BREthereumBlockHeader header,
                       UInt256 *n,
                       BREthereumHash *m) {
    assert (NULL != n && NULL != m);

    // On and on and on

    // TODO: Actually compute something.

    // For now, return values that allow subsequent use to succeed
    *n = UINT256_ZERO;
    *m = ethBlockHeaderGetMixHash (header);
}
