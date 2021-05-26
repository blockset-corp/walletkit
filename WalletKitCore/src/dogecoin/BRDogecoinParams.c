//
//  BRDogecoinParams.h
//
//  Created by Aaron Voisine on 5/19/21.
//  Copyright (c) 2018 breadwallet LLC
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

#include "support/BRInt.h"
#include "support/BRSet.h"
#include "bitcoin/BRBitcoinPeer.h"
#include "BRDogecoinParams.h"

#define DOGE_PUBKEY_PREFIX       30
#define DOGE_SCRIPT_PREFIX       22
#define DOGE_PRIVKEY_PREFIX      158

#define DOGE_PUBKEY_PREFIX_TEST  113
#define DOGE_SCRIPT_PREFIX_TEST  196
#define DOGE_PRIVKEY_PREFIX_TEST 231

#define DOGE_TARGET_SPACING      60LL
#define DOGE_MAX_PROOF_OF_WORK   0x1e0ffff0 // highest value for difficulty target (higher values are less difficult)
#define DOGE_AUX_POW_CHAIN_ID    0x0062

static const char *dogeMainNetDNSSeeds[] = {
    "seed.dogecoin.com.", "seed.multidoge.org.", "seed2.multidoge.org.", "seed.doger.dogecoin.com", NULL
};

static const char *dogeTestNetDNSSeeds[] = {
    "testseed.jrn.me.uk.", NULL
};

static const BRBitcoinCheckPoint dogeTestNetCheckpoints[] = {
    {      0, uint256("bb0a78264637406b6360aad926284d544d7049f45189db5664f3c4d07350559e"), 1391503289, 0x1e0ffff0 },
//    {  157500, uint256(""), 0, 0 }, // kimoto gravity well fork
//    {  158100, uint256(""), 0, 0 }  // aux-pow fork
};

static const BRBitcoinCheckPoint dogeMainNetCheckpoints[] = {
    {      0, uint256("1a91e3dace36e2be3bf030a65679fe821aa1d6ef92e7c9902eb318182c355691"), 1386325540, 0x1e0ffff0 },
    {  20160, uint256("fedf3bebfe6fac06275b133a86fd80a5b2ab257f28867d894439e0df7d5ee41f"), 1387542614, 0x02018300 },
    {  40320, uint256("a3f5b50455ff15b149c790bf09a7d65e9d3711cd340e9710e40696fd40a8c368"), 1388763252, 0x02012700 },
    {  60480, uint256("7d2e4ee3371b030d87cea237e3dd950d8f02334c2f69d682cf403b9a8ad193ee"), 1389976419, 0x02023400 },
    {  80640, uint256("e7961b94f744997a5302d17ea9db0d745968ba39ba36388135c5176600adddc4"), 1391183316, 0x0204ea00 },
    { 100800, uint256("7fe46a0dfafd9529768a7c6958a207286bdfafcd5d77ae02d62e9411c57e393e"), 1392401107, 0x0204a800 },
    { 120960, uint256("bcff901618355372e3b1fc1ed654e7553b70edd552ddcaf966e28bdbb64b5a82"), 1393620670, 0x02049500 },
    { 141120, uint256("1f58bd2779f508ccd0f4127d4018ba968b8c6d9f1a80cda4b8a975583ef5b526"), 1394857300, 0x02040000 },
//    {  145000, uint256(""), 0, 0x1e0ffff0 }, // kimoto gravity well fork
    { 161280, uint256("1a0f06917f3c5879082c40e8e2c9181bbedb2555944cd278575546cd5a282ed8"), 1396151478, 0x0204e000 },
    { 181440, uint256("e6b21e580eecfff37ddbed66be367a07176d8096bddcb80fed9cc5d2905cd437"), 1397490952, 0x0204f400 },
    { 201600, uint256("4294ba1673480f1f6db42deeff7ac3cafdfd864e94715e876a5dd251ccc9bdd5"), 1398797573, 0x02035500 },
    { 221760, uint256("1bd7a053d632197280ec38243c1ff9843aca69b64dc3f1ddda7e19436ce3ad05"), 1400111033, 0x02029600 },
    { 241920, uint256("433a467734c61e29e6d764a60fd7ff3b5afecc160b6bf981adabc6405d122bb8"), 1401484237, 0x02037d00 },
    { 262080, uint256("7d0852f76ee97cf395166c7ba6fa841f05240f3e96e1ef8511aea030806a6c65"), 1402871023, 0x02038600 },
    { 282240, uint256("b9e1ce8466dbe907b3b75c630bed3c51cc2eba5ab0034ba720294e2beb1af60d"), 1404222140, 0x02047900 },
    { 302400, uint256("02010996bfaca49b8f13fc8d2954589e1c0b6e97bc670838b9be031ad93cc2e8"), 1405604574, 0x0202b000 },
    { 322560, uint256("e8ca47c71c2ffd7def6b07d114fc7e79bea377c895bcdbd958c88b2fde21b2ae"), 1407009627, 0x0202e500 },
    { 342720, uint256("49181ea0c279ecea67862aa812cc9f1717d1ea5f9ba2a8a008324917575e1d5c"), 1408444593, 0x02042a00 },
    { 362880, uint256("9a9c8d075a1c356eb43611ef2c24d42a10c3d1bd37ea7048d75ecfc6bdacf7a8"), 1409853429, 0x02043800 },
//    {  371337, uint256(""), 0, 0 }  // aux-pow fork
    { 383040, uint256("fda7054eb894665e5b038564c5e940f3ec11511eac8f77b4396d2a825b453e57"), 1411194748, 0x0235cc00 },
    { 403200, uint256("c165ef7f0600b1a53248f2397b50193ac5ca4da084c8d207298401f24aec4788"), 1412460609, 0x023d2800 },
    { 423360, uint256("95f14dd2ec3a75a36a47e7662ef08261e64762766caf7f3e5a747a9204159d63"), 1413730696, 0x023a0e00 },
    { 443520, uint256("ad563011733f7853b8d8714eebeb715eb67fbb2e5838189b1055a8605f8ecfda"), 1415004943, 0x023fdf00 },
    { 463680, uint256("e23b0b6bce00197ee9856edf1014b89e7578993c0678cb8cf16673f2614fb1d1"), 1416274432, 0x022dec00 },
    { 483840, uint256("ab4964d217f874874e5626b2030a2ac1a9febfb466568da7fcbd16016d4ed4db"), 1417548226, 0x0265ce00 },
    { 504000, uint256("ab1a0a2fa1ba8fab2e9c05b5028a6268fe28c387b8246e332d6ac64b6976ca7f"), 1418824139, 0x025bda00 },
    { 524160, uint256("0ecb6bd9129ba766190b6bba6ac9c416594866402a74eceb249827a0dd035968"), 1420098891, 0x02432400 }
};

static int dogeVerifyProofOfWork(const BRBitcoinMerkleBlock *block)
{
    assert(block != NULL);
    
    // target is in "compact" format, where the most significant byte is the size of the value in bytes, next
    // bit is the sign, and the last 23 bits is the value after having been right shifted by (size - 3)*8 bits
    uint32_t size = block->target >> 24, target = block->target & 0x007fffff;
    uint8_t buf[80];
    size_t off = 0;
    int i;
    UInt256 w, t = UINT256_ZERO;
    const BRBitcoinMerkleBlock *b = ((block->version >> 16) == 0) ? block : btcMerkleBlockAuxPowParent(block);

    if (! b) b = block;
    UInt32SetLE(&buf[off], b->version);
    off += sizeof(uint32_t);
    UInt256Set(&buf[off], b->prevBlock);
    off += sizeof(UInt256);
    UInt256Set(&buf[off], b->merkleRoot);
    off += sizeof(UInt256);
    UInt32SetLE(&buf[off], b->timestamp);
    off += sizeof(uint32_t);
    UInt32SetLE(&buf[off], b->target);
    off += sizeof(uint32_t);
    UInt32SetLE(&buf[off], b->nonce);
    off += sizeof(uint32_t);
    BRScrypt(w.u8, 32, buf, off, buf, off, 1024, 1, 1);

    if (size - 3 >= sizeof(t) - sizeof(uint32_t)) return 0;

    if (size > 3) UInt32SetLE(&t.u8[size - 3], target);
    else UInt32SetLE(t.u8, target >> (3 - size)*8);

    for (i = sizeof(t) - 1; i >= 0; i--) { // many proof-of-work
        if (w.u8[i] < t.u8[i]) break;
        if (w.u8[i] > t.u8[i]) return 0;
    }
    
    return 1;
}

static int dogeMainNetVerifyDifficulty(const BRBitcoinMerkleBlock *block, const BRSet *blockSet)
{
    const BRBitcoinMerkleBlock *previous, *b = NULL;
    int r = 1, size = 0;
    uint64_t target = 0;
    int64_t timespan;

    assert(block != NULL);
    assert(blockSet != NULL);
    previous = BRSetGet(blockSet, &block->prevBlock);

    if (! previous || !UInt256Eq(block->prevBlock, previous->blockHash) || block->height != previous->height + 1) r = 0;

    if (block->height >= 145000) { // very kimoto gravity well fork height
        // target is in "compact" format, where the most significant byte is the size of the value in bytes, next
        // bit is the sign, and the last 23 bits is the value after having been right shifted by (size - 3)*8 bits
        size = previous->target >> 24;
        target = previous->target & 0x007fffff;
        b = BRSetGet(blockSet, &previous->prevBlock);
        timespan = (b) ? (int64_t)previous->timestamp - b->timestamp : 0;

        timespan = DOGE_TARGET_SPACING - (DOGE_TARGET_SPACING - timespan)/8; // wow, such 0.125 low-pass filter

        // limit difficulty transition to -25% or +50%
        if (timespan < DOGE_TARGET_SPACING*3/4) timespan = DOGE_TARGET_SPACING*3/4;
        if (timespan > DOGE_TARGET_SPACING*3/2) timespan = DOGE_TARGET_SPACING*3/2;

        // DOGE_TARGET_SPACING happens to be a multiple of 4, and since timespan is at least DOGE_TARGET_SPACING*3/4,
        // we don't lose precision when target is multiplied by timespan*64 and then divided by DOGE_TARGET_SPACING/4
        target *= (uint64_t)timespan*64;
        target /= DOGE_TARGET_SPACING >> 2;
        size--; // decrement size since we multiplied target by timespan*64 and only divided by DOGE_TARGET_SPACING/4

        while (size < 1 || target > 0x007fffff) { target >>= 8; size++; } // normalize target for "compact" format
        target |= (uint64_t)size << 24;

        if (target > DOGE_MAX_PROOF_OF_WORK) target = DOGE_MAX_PROOF_OF_WORK; // limit to DOGE_MAX_PROOF_OF_WORK
        if (b && block->target != target) r = 0;
    }
    
    return r && dogeVerifyProofOfWork(block); // to the moon!
}

static int dogeTestNetVerifyDifficulty(const BRBitcoinMerkleBlock *block, const BRSet *blockSet)
{
    return 1; // skipping difficulty check
}

static const BRBitcoinChainParams dogeMainNetParamsRecord = {
    dogeMainNetDNSSeeds,
    22556,
    0xc0c0c0c0,
    0,
    dogeMainNetVerifyDifficulty,
    dogeMainNetCheckpoints,
    sizeof(dogeMainNetCheckpoints)/sizeof(*dogeMainNetCheckpoints),
    { DOGE_PUBKEY_PREFIX, DOGE_SCRIPT_PREFIX, DOGE_PRIVKEY_PREFIX, NULL },
    BITCOIN_FORKID,
    DOGE_BIP32_DEPTH,
    DOGE_BIP32_CHILD
};
const BRBitcoinChainParams *dogeMainNetParams = &dogeMainNetParamsRecord;

static const BRBitcoinChainParams dogeTestNetParamsRecord = {
    dogeTestNetDNSSeeds,
    44556,
    0xdcb7c1fc,
    0,
    dogeTestNetVerifyDifficulty,
    dogeTestNetCheckpoints,
    sizeof(dogeTestNetCheckpoints)/sizeof(*dogeTestNetCheckpoints),
    { DOGE_PUBKEY_PREFIX_TEST, DOGE_SCRIPT_PREFIX_TEST, DOGE_PRIVKEY_PREFIX_TEST, NULL },
    BITCOIN_FORKID,
    BITCOIN_BIP32_DEPTH_TEST,
    BITCOIN_BIP32_CHILD_TEST
};
const BRBitcoinChainParams *dogeTestNetParams = &dogeTestNetParamsRecord;
