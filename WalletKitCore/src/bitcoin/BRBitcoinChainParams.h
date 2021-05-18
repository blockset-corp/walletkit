//
//  BRBitcoinChainParams.h
//
//  Created by Aaron Voisine on 1/10/18.
//  Copyright (c) 2018-2019 breadwallet LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#ifndef BRChainParams_h
#define BRChainParams_h

#include "BRBitcoinMerkleBlock.h"
#include "BRBitcoinPeer.h"
#include "support/BRSet.h"
#include "support/BRAddress.h"
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif
    
#define BITCOIN_FORKID 0x00

typedef struct {
    uint32_t height;
    UInt256 hash;
    uint32_t timestamp;
    uint32_t target;
} BRBitcoinCheckPoint;

typedef struct {
    const char * const *dnsSeeds; // NULL terminated array of dns seeds
    uint16_t standardPort;
    uint32_t magicNumber;
    uint64_t services;
    int (*verifyDifficulty)(const BRBitcoinMerkleBlock *block, const BRSet *blockSet); // blockSet must have last 2016 blocks
    const BRBitcoinCheckPoint *checkpoints;
    size_t checkpointsCount;
    BRAddressParams addrParams;
    uint8_t forkId;
} BRBitcoinChainParams;

extern const BRBitcoinChainParams *btcMainNetParams;
extern const BRBitcoinChainParams *btcTestNetParams;

static inline const BRBitcoinChainParams *btcChainParamsGetBitcoin (int mainnet) {
    return mainnet ? btcMainNetParams : btcTestNetParams;
}

static inline int btcChainParamsIsBitcoin (const BRBitcoinChainParams *params) {
    return btcMainNetParams == params || btcTestNetParams == params;
}

extern const BRBitcoinCheckPoint *btcChainParamsGetCheckpointBefore (const BRBitcoinChainParams *params, uint32_t timestamp);

extern const BRBitcoinCheckPoint *btcChainParamsGetCheckpointBeforeBlockNumber (const BRBitcoinChainParams *params, uint32_t blockNumber);

#ifdef __cplusplus
}
#endif

#endif // BRChainParams_h
