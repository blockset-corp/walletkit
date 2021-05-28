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
    {  100800, uint256("7fe46a0dfafd9529768a7c6958a207286bdfafcd5d77ae02d62e9411c57e393e"), 1392401107, 0x1b36f5e3 },
//  {  145000, uint256(""), 0, 0x1e0ffff0 }, // kimoto gravity well fork
    {  201600, uint256("4294ba1673480f1f6db42deeff7ac3cafdfd864e94715e876a5dd251ccc9bdd5"), 1398797573, 0x1b4ccbf2 },
    {  302400, uint256("02010996bfaca49b8f13fc8d2954589e1c0b6e97bc670838b9be031ad93cc2e8"), 1405604574, 0x1b5f2bd5 },
//  {  371337, uint256(""), 0, 0          }  // aux-pow fork
    {  403200, uint256("c165ef7f0600b1a53248f2397b50193ac5ca4da084c8d207298401f24aec4788"), 1412460609, 0x1b042f8d },
    {  504000, uint256("ab1a0a2fa1ba8fab2e9c05b5028a6268fe28c387b8246e332d6ac64b6976ca7f"), 1418824139, 0x1b02c97c },
    {  604800, uint256("12132684cb851dae412fff7451be27f5c1afefbd3bd67070b4780cbfbba42257"), 1425183802, 0x1b05a394 },
    {  705600, uint256("a1be125a64133c61f882f2415c6a2011d70a47a17a3c67b286d2e47d48d057f5"), 1431541894, 0x1b03e81c },
    {  806400, uint256("fc1c23df396e0bc958f9c0ad8ccdbd03818215a191ed04f3890fe473d6028515"), 1437875909, 0x1b039e1d },
    {  907200, uint256("b473e0a78d17a96763a45c3fc4bf313c6eb0d8853789346187bc30f7c81e1634"), 1444214733, 0x1b04ab15 },
    { 1008000, uint256("e217e486cd6ad7cafe994251eff68ac75f9471bb62d162878adff3d03cdc2b03"), 1450531716, 0x1b02cb15 },
    { 1108800, uint256("b55d440b9b932f5049987ff0bff54eaa5d7b516fcd1d5f095ad2e98c30f45e2a"), 1456840435, 0x1b02d088 },
    { 1209600, uint256("d1c9cfd537de4454319839e122e53956a7b863e449437bf12d3134d8ef15c5bb"), 1463157162, 0x1b02e3c0 },
    { 1310400, uint256("e70014e5de600a0dd6559052aec4df4d11ef4e1ff9b2f8bc887d7cda11e2b2b2"), 1469471927, 0x1b04e5ff },
    { 1411200, uint256("c4b582eb85f753b87333d1cf09d632e52750c2316e40496f9d6e8508b4ebbd0b"), 1475759173, 0x1b02ede7 },
    { 1512000, uint256("906fedeb9b043a74b7ff518801b34d1959756caf556011b35fdbe37b72ab1cd9"), 1482061224, 0x1b021591 },
    { 1612800, uint256("0c8d5694251f2cdcce7a31add9f980542f9fb6d5e9b1036caf7944528ae8ea7c"), 1488379808, 0x1b020af7 },
    { 1713600, uint256("165f3935dd0969cf3cc365b0a323756d612f6e4552ddab4a86bd33620823c94a"), 1494712515, 0x1b00b87f },
    { 1814400, uint256("fb7124bcb36b3bff337423c085383d83d1b490c1641d6fae1534c8fae6515168"), 1501011964, 0x1b0094c2 },
    { 1915200, uint256("89674ae391d044c965f126d346774cf79bb8bdbcb3e21f03670f5dfdd0c3b18d"), 1507308371, 0x1a24cd48 },
    { 2016000, uint256("c986dc6f7a1c53d7de584c523e591b443ed1a620dba674ed5460381bd85173ad"), 1513600711, 0x1a192809 },
    { 2116800, uint256("8618eb2a5e1cf5f0b847271596029dc5cfff1ee708da94bcf7c721c6b294ba1b"), 1519883651, 0x1a07f9eb },
    { 2217600, uint256("aa045d4b212bb3c647598b7fd5617098d6d67b930b8ae58b0106c00e09eacb62"), 1526189914, 0x1a052a4e },
    { 2318400, uint256("bd4c6a40fae4a0f32fa0696ab1158209fc56699b22b2b8686e2255ddf0812a81"), 1532497629, 0x1a05f2bb },
    { 2419200, uint256("f20601843abaacd7430c746a640d36c70df9e7985fde34637eb1f63170026b78"), 1538796648, 0x1a06aca7 },
    { 2520000, uint256("ee3cb5e288c139259f04557441e56b1eddf28a3b955b50051e4f250b6b8a0893"), 1545101124, 0x1a0883cc },
    { 2620800, uint256("951bfe28f67ff6ef770cff23cbbe250b164e99f187b9186ca09d95e4db338448"), 1551398184, 0x1a059f09 },
    { 2721600, uint256("6c5b97cf431854860e0fd236a882c1790ed6ca45bc7eed8deb9a1affcbd4e68d"), 1557685453, 0x1a03cc1e },  // 05-12-2019
    { 2822400, uint256("96c4b3268179c0e1673e8e7a0300aa950ae103b6a9b348b5b7669f06068731a4"), 1563985133, 0x1a0402a5 },
    { 2923200, uint256("b0ff8c7ce9b2dab6a2b5f835da086f21158ad6b77036dec816ad7bbe3ac47fb6"), 1570288282, 0x1a03d270 },
    { 3024000, uint256("c467b99bd68fbc1085bf59569d0155d3eee58e1411afc2d5466171546bd3067f"), 1576597961, 0x1a097be9 },
    { 3124800, uint256("09bada940041f421128e7eb91a5b80f6a158ea859f6a247bcf474c3f4ab2c79c"), 1582921998, 0x1a07c7d4 },
    { 3225600, uint256("e8fee08757314183d6da7f7781270215bc448f0013a30769e273881953ec49d2"), 1589267510, 0x1a06cf03 },
    { 3326400, uint256("9b3444ef569f02495c7d3b1bd0afa7b2753bc248e53d9abfd1d188fe18822169"), 1595599166, 0x1a08259f },
    { 3427200, uint256("a25c57588a42ab366f33fd562d40cae7b974d2594e8c400f273211a56e248835"), 1601949379, 0x1a053766 },
    { 3528000, uint256("edff655a14073e6aeb66594627d6d4f089a1752241b1a55983982ad769659744"), 1608312220, 0x1a0526b4 },
    { 3628800, uint256("1ae04a9862bb32849b28e1c0dbda5ba7cda5e9343599079432363199272be9c1"), 1614658299, 0x1a050b5e },
    { 3729600, uint256("865ef7942990a83421bd078f3306f130c67e3ae644e5fb928c014c34b9818dba"), 1621048398, 0x1a04bf82 }
    // 3830400
    // 3931200
    // 4032000
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
