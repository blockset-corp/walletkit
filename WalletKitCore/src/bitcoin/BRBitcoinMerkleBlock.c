//
//  BRMerkleBlock.c
//
//  Created by Aaron Voisine on 8/6/15.
//  Copyright (c) 2015 breadwallet LLC
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

#include "BRBitcoinMerkleBlock.h"
#include "BRBitcoinTransaction.h"
#include "support/BRBase.h"
#include "support/BRCrypto.h"
#include "support/BRAddress.h"
#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>
#include <string.h>
#include <assert.h>

#define MAX_PROOF_OF_WORK 0x1d00ffff    // highest value for difficulty target (higher values are less difficult)
#define TARGET_TIMESPAN   (14*24*60*60) // the targeted timespan between difficulty target adjustments

inline static uint32_t _ceil_log2(uint32_t x)
{
    uint32_t r = (x & (x - 1)) ? 1 : 0;
    
    while ((x >>= 1) != 0) r++;
    return r;
}

// from https://en.bitcoin.it/wiki/Protocol_specification#Merkle_Trees
// Merkle trees are binary trees of hashes. Merkle trees in bitcoin use a double SHA-256, the SHA-256 hash of the
// SHA-256 hash of something. If, when forming a row in the tree (other than the root of the tree), it would have an odd
// number of elements, the final double-hash is duplicated to ensure that the row has an even number of hashes. First
// form the bottom row of the tree with the ordered double-SHA-256 hashes of the byte streams of the transactions in the
// block. Then the row above it consists of half that number of hashes. Each entry is the double-SHA-256 of the 64-byte
// concatenation of the corresponding two hashes below it in the tree. This procedure repeats recursively until we reach
// a row consisting of just a single double-hash. This is the merkle root of the tree.
//
// from https://github.com/bitcoin/bips/blob/master/bip-0037.mediawiki#Partial_Merkle_branch_format
// The encoding works as follows: we traverse the tree in depth-first order, storing a bit for each traversed node,
// signifying whether the node is the parent of at least one matched leaf txid (or a matched txid itself). In case we
// are at the leaf level, or this bit is 0, its merkle node hash is stored, and its children are not explored further.
// Otherwise, no hash is stored, but we recurse into both (or the only) child branch. During decoding, the same
// depth-first traversal is performed, consuming bits and hashes as they were written during encoding.
//
// example tree with three transactions, where only tx2 is matched by the bloom filter:
//
//     merkleRoot
//      /     \
//    m1       m2
//   /  \     /  \
// tx1  tx2 tx3  tx3
//
// flag bits (little endian): 00001011 [merkleRoot = 1, m1 = 1, tx1 = 0, tx2 = 1, m2 = 0, byte padding = 000]
// hashes: [tx1, tx2, m2]
//
// NOTE: this merkle tree design has a security vulnerability (CVE-2012-2459), which can be defended against by
// considering the merkle root invalid if there are duplicate hashes in any rows with an even number of elements

// AuxPow merge-mined block: https://en.bitcoin.it/wiki/Merged_mining_specification
typedef struct {
    BRBitcoinTransaction *coinbaseTx;
    UInt256 parentHash;
    UInt256 *cbHashes;
    size_t cbHashesCount;
    uint32_t cbMask;
    UInt256 *chainHashes;
    size_t chainHashesCount;
    uint32_t chainMask;
    BRBitcoinMerkleBlock *parent;
} _BRAuxPow;

typedef struct {
    BRBitcoinMerkleBlock block;
    _BRAuxPow *ap;
} _BRAuxPowBlock;

static void _BRAuxPowFree(_BRAuxPow *ap)
{
    if (ap) {
        if (ap->coinbaseTx) btcTransactionFree(ap->coinbaseTx);
        if (ap->cbHashes) free(ap->cbHashes);
        if (ap->chainHashes) free(ap->chainHashes);
        if (ap->parent) btcMerkleBlockFree(ap->parent);
        free(ap);
    }
}

static int _BRAuxPowIsValid(const _BRAuxPow *ap, UInt256 blockHash)
{
    UInt256 pair[2], root;
    int i = 0;
    
    if (! ap || ! ap->parent || ! ap->coinbaseTx) return 0;
    
    for (i = 0, root = ap->coinbaseTx->txHash; i < ap->cbHashesCount; i++) {
        pair[(ap->cbMask >> i) & 0x01] = root;
        pair[(~ap->cbMask >> i) & 0x01] = ap->cbHashes[i];
        BRSHA256_2(&root, pair, sizeof(pair));
    }

    // check if coinbaseTx is included in parent block
    if (! UInt256Eq(root, ap->parent->merkleRoot)) return 0;
        
    for (i = 0, root = blockHash; i < ap->chainHashesCount; i++) {
        pair[(ap->chainMask >> i) & 0x01] = root;
        pair[(~ap->chainMask >> i) & 0x01] = ap->chainHashes[i];
        BRSHA256_2(&root, pair, sizeof(pair));
    }

    // scan for chain merkle root in coinbase input
    for (i = 0, root = UInt256Reverse(root); i + 32 <= ap->coinbaseTx->inputs->sigLen; i++) {
        if (memcmp(&ap->coinbaseTx->inputs->signature[i], &root, 32) == 0) break;
    }
        
    if (i + 32 > ap->coinbaseTx->inputs->sigLen) return 0;
    
    // check if parent block is valid
    return (btcMerkleBlockIsValid(ap->parent, ap->parent->timestamp + BLOCK_MAX_TIME_DRIFT));
}

static _BRAuxPow *_BRAuxPowParse(const uint8_t *buf, size_t bufLen, UInt256 blockHash, size_t *off)
{
    size_t len, o = (off) ? *off : 0;
    _BRAuxPow *ap = (o + 41 < bufLen) ? calloc(1, sizeof(*ap)) : NULL;

    assert(buf != NULL || bufLen == 0);
    
    if (ap) {
        if (buf[o + 4] == 1 && UInt256IsZero(UInt256Get(&buf[o + 5])) && UInt32GetLE(&buf[o + 37]) == 0xffffffff &&
            buf[o + 41] >= 2 && buf[o + 41] <= 100 && o + 42 + buf[o + 41] <= bufLen) {
            ap->coinbaseTx = btcTransactionParse(&buf[o], bufLen - o);
        }
        
        o = (ap->coinbaseTx) ? o + btcTransactionSerialize(ap->coinbaseTx, NULL, 0) : bufLen;
        if (o + sizeof(UInt256) <= bufLen) ap->parentHash = UInt256Get(&buf[o]);
        o += sizeof(UInt256);
        ap->cbHashesCount = BRVarInt(&buf[o], (o <= bufLen ? bufLen - o : 0), &len);
        o += len;
        len = ap->cbHashesCount*sizeof(UInt256);
        ap->cbHashes = (o + len <= bufLen) ? malloc(len) : NULL;
        if (ap->cbHashes) memcpy(ap->cbHashes, &buf[o], len);
        o += len;
        if (o + sizeof(uint32_t) <= bufLen) ap->cbMask = UInt32GetLE(&buf[o]);
        o += sizeof(uint32_t);
        ap->chainHashesCount = BRVarInt(&buf[o], (o <= bufLen ? bufLen - o : 0), &len);
        o += len;
        len = ap->chainHashesCount*sizeof(UInt256);
        ap->chainHashes = (o + len <= bufLen) ? malloc(len) : NULL;
        if (ap->chainHashes) memcpy(ap->chainHashes, &buf[o], len);
        o += len;
        if (o + sizeof(uint32_t) <= bufLen) ap->chainMask = UInt32GetLE(&buf[o]);
        o += sizeof(uint32_t);
        if (o + 80 <= bufLen) ap->parent = btcMerkleBlockParse(&buf[o], 80);
        o += 80;
        if (o > bufLen || ! _BRAuxPowIsValid(ap, blockHash)) { _BRAuxPowFree(ap); ap = NULL; }
    }
    
    if (ap && off) *off = o;
    return ap;
}

static size_t _BRAuxPowSerialize(const _BRAuxPow *ap, uint8_t *buf, size_t bufLen, size_t *off)
{
    size_t len, o = (off) ? *off : 0;

    if (! ap || ! ap->coinbaseTx || ! ap->parent) return 0;
    o += btcTransactionSerialize(ap->coinbaseTx, (buf ? &buf[o] : NULL), (o <= bufLen ? bufLen - o : 0));
    if (buf && o + sizeof(UInt256) <= bufLen) UInt256Set(&buf[o], ap->parentHash);
    o += sizeof(UInt256);
    o += BRVarIntSet((buf ? &buf[o] : NULL), (o <= bufLen ? bufLen - o : 0), ap->cbHashesCount);
    len = ap->cbHashesCount*sizeof(UInt256);
    if (buf && o + len <= bufLen) memcpy(&buf[o], ap->cbHashes, len);
    o += len;
    if (buf && o + sizeof(uint32_t) <= bufLen) UInt32SetLE(&buf[o], ap->cbMask);
    o += sizeof(uint32_t);
    o += BRVarIntSet((buf ? &buf[o] : NULL), (o <= bufLen ? bufLen - o : 0), ap->chainHashesCount);
    len = ap->chainHashesCount*sizeof(UInt256);
    if (buf && o + len <= bufLen) memcpy(&buf[o], ap->chainHashes, len);
    o += len;
    if (buf && o + sizeof(uint32_t) <= bufLen) UInt32SetLE(&buf[o], ap->chainMask);
    o += sizeof(uint32_t);
    o += btcMerkleBlockSerialize(ap->parent, (buf ? &buf[o] : NULL), (o <= bufLen ? bufLen - o : 0));
    len = (off) ? o - *off : o;
    if (off && (! buf || o <= bufLen)) *off = o;
    return (! buf || o <= bufLen) ? len : 0;
}

// returns a newly allocated merkle block struct that must be freed by calling btcMerkleBlockFree()
BRBitcoinMerkleBlock *btcMerkleBlockNew(void)
{
    BRBitcoinMerkleBlock *block = calloc(1, sizeof(_BRAuxPowBlock));

    assert(block != NULL);
    
    block->height = BLOCK_UNKNOWN_HEIGHT;
    return block;
}

// returns a deep copy of block and that must be freed by calling btcMerkleBlockFree()
BRBitcoinMerkleBlock *btcMerkleBlockCopy(const BRBitcoinMerkleBlock *block)
{
    assert(block != NULL);
    
    BRBitcoinMerkleBlock *cpy = btcMerkleBlockNew();
    size_t len = _BRAuxPowSerialize(((_BRAuxPowBlock *)block)->ap, NULL, 0, NULL);
    uint8_t _buf[0x1000], *buf = (len <= 0x1000) ? _buf : malloc(len);

    *cpy = *block;
    cpy->hashes = NULL;
    cpy->flags = NULL;
    btcMerkleBlockSetTxHashes(cpy, block->hashes, block->hashesCount, block->flags, block->flagsLen);
    len = _BRAuxPowSerialize(((_BRAuxPowBlock *)block)->ap, buf, len, NULL);
    ((_BRAuxPowBlock *)cpy)->ap = _BRAuxPowParse(buf, len, block->blockHash, NULL);
    if (buf != _buf) free(buf);
    return cpy;
}

// buf must contain either a serialized merkleblock or header
// returns a merkle block struct that must be freed by calling btcMerkleBlockFree()
BRBitcoinMerkleBlock *btcMerkleBlockParse(const uint8_t *buf, size_t bufLen)
{
    BRBitcoinMerkleBlock *block = (buf && 80 <= bufLen) ? btcMerkleBlockNew() : NULL;
    _BRAuxPowBlock *apBlock = (_BRAuxPowBlock *)block;
    size_t off = 0, len = 0, l = 0;
    
    assert(buf != NULL || bufLen == 0);
    
    if (block) {
        block->version = UInt32GetLE(&buf[off]);
        off += sizeof(uint32_t);
        block->prevBlock = UInt256Get(&buf[off]);
        off += sizeof(UInt256);
        block->merkleRoot = UInt256Get(&buf[off]);
        off += sizeof(UInt256);
        block->timestamp = UInt32GetLE(&buf[off]);
        off += sizeof(uint32_t);
        block->target = UInt32GetLE(&buf[off]);
        off += sizeof(uint32_t);
        block->nonce = UInt32GetLE(&buf[off]);
        off += sizeof(uint32_t);
        BRSHA256_2(&block->blockHash, buf, 80);

        if (block->version >> 16 != 0) apBlock->ap = _BRAuxPowParse(buf, bufLen, block->blockHash, &off);

        
        block->totalTx = (off + sizeof(uint32_t) <= bufLen) ? UInt32GetLE(&buf[off]) : 0;
        block->hashesCount = (size_t)BRVarInt(&buf[off + 4], (off + 4 <= bufLen ? bufLen - (off + 4) : 0), &l);
        len = 4 + l + block->hashesCount*sizeof(UInt256);
        block->flagsLen = (size_t)BRVarInt(&buf[off + len], (off + len <= bufLen ? bufLen - (off + len) : 0), &l);
        
        if (block->totalTx > 0 && off + len + l + block->flagsLen == bufLen) {
            block->hashes = malloc(block->hashesCount*sizeof(UInt256));
            memcpy(block->hashes, &buf[off + 4 + BRVarIntSize(block->hashesCount)], block->hashesCount*sizeof(UInt256));
            block->flags = malloc(block->flagsLen);
            memcpy(block->flags, &buf[off + len + l], block->flagsLen);

            if (! btcMerkleBlockIsValid(block, block->timestamp + BLOCK_MAX_TIME_DRIFT)) { // parse as header
                btcMerkleBlockFree(block);
                block = btcMerkleBlockParse(buf, off);
            }
            else off = bufLen;
        }
        else block->totalTx = block->hashesCount = block->flagsLen = 0;
        
        if (off > bufLen) {
            btcMerkleBlockFree(block);
            block = NULL;
        }
    }
    
    return block;
}

// returns number of bytes written to buf, or total bufLen needed if buf is NULL (block->height is not serialized)
size_t btcMerkleBlockSerialize(const BRBitcoinMerkleBlock *block, uint8_t *buf, size_t bufLen)
{
    size_t len, off = 0;
    
    assert(block != NULL);
    
    if (buf && 80 <= bufLen) {
        UInt32SetLE(&buf[off], block->version);
        off += sizeof(uint32_t);
        UInt256Set(&buf[off], block->prevBlock);
        off += sizeof(UInt256);
        UInt256Set(&buf[off], block->merkleRoot);
        off += sizeof(UInt256);
        UInt32SetLE(&buf[off], block->timestamp);
        off += sizeof(uint32_t);
        UInt32SetLE(&buf[off], block->target);
        off += sizeof(uint32_t);
        UInt32SetLE(&buf[off], block->nonce);
        off += sizeof(uint32_t);
    }
    else off = 80;
    
    _BRAuxPowSerialize(((_BRAuxPowBlock *)block)->ap, buf, bufLen, &off);
    
    if (block->totalTx > 0) {
        if (buf && off + sizeof(uint32_t) <= bufLen) UInt32SetLE(&buf[off], block->totalTx);
        off += sizeof(uint32_t);
        off += BRVarIntSet((buf ? &buf[off] : NULL), (off <= bufLen ? bufLen - off : 0), block->hashesCount);
        len = block->hashesCount*sizeof(UInt256);
        if (buf && off + len <= bufLen) memcpy(&buf[off], block->hashes, len);
        off += len;
        off += BRVarIntSet((buf ? &buf[off] : NULL), (off <= bufLen ? bufLen - off : 0), block->flagsLen);
        len = block->flagsLen;
        if (buf && off + len <= bufLen) memcpy(&buf[off], block->flags, len);
        off += len;
    }

    return (! buf || off <= bufLen) ? off : 0;
}

static size_t _btcMerkleBlockTxHashesR(const BRBitcoinMerkleBlock *block, UInt256 *txHashes, size_t hashesCount,
                                       size_t *idx, size_t *hashIdx, size_t *flagIdx, uint32_t depth)
{
    uint8_t flag;
    
    if (*flagIdx/8 < block->flagsLen && *hashIdx < block->hashesCount) {
        flag = (block->flags[*flagIdx/8] & (1 << (*flagIdx % 8)));
        (*flagIdx)++;
    
        if (! flag || depth == _ceil_log2(block->totalTx)) {
            if (flag && *idx < hashesCount) {
                if (txHashes) txHashes[*idx] = block->hashes[*hashIdx]; // leaf
                (*idx)++;
            }
        
            (*hashIdx)++;
        }
        else {
            _btcMerkleBlockTxHashesR(block, txHashes, hashesCount, idx, hashIdx, flagIdx, depth + 1); // left branch
            _btcMerkleBlockTxHashesR(block, txHashes, hashesCount, idx, hashIdx, flagIdx, depth + 1); // right branch
        }
    }

    return *idx;
}

// populates txHashes with the matched tx hashes in the block
// returns number of hashes written, or the total hashesCount needed if txHashes is NULL
size_t btcMerkleBlockTxHashes(const BRBitcoinMerkleBlock *block, UInt256 *txHashes, size_t hashesCount)
{
    size_t idx = 0, hashIdx = 0, flagIdx = 0;

    assert(block != NULL);
    
    return _btcMerkleBlockTxHashesR(block, txHashes, (txHashes) ? hashesCount : SIZE_MAX, &idx, &hashIdx, &flagIdx, 0);
}

// sets the hashes and flags fields for a block created with btcMerkleBlockNew()
void btcMerkleBlockSetTxHashes(BRBitcoinMerkleBlock *block, const UInt256 hashes[], size_t hashesCount,
                               const uint8_t *flags, size_t flagsLen)
{
    assert(block != NULL);
    assert(hashes != NULL || hashesCount == 0);
    assert(flags != NULL || flagsLen == 0);
    
    if (block->hashes) free(block->hashes);
    block->hashes = (hashesCount > 0) ? malloc(hashesCount*sizeof(UInt256)) : NULL;
    if (block->hashes) memcpy(block->hashes, hashes, hashesCount*sizeof(UInt256));
    if (block->flags) free(block->flags);
    block->flags = (flagsLen > 0) ? malloc(flagsLen) : NULL;
    if (block->flags) memcpy(block->flags, flags, flagsLen);
}

// recursively walks the merkle tree to calculate the merkle root
// NOTE: this merkle tree design has a security vulnerability (CVE-2012-2459), which can be defended against by
// considering the merkle root invalid if there are duplicate hashes in any rows with an even number of elements
static UInt256 _btcMerkleBlockRootR(const BRBitcoinMerkleBlock *block, size_t *hashIdx, size_t *flagIdx, uint32_t depth)
{
    uint8_t flag;
    UInt256 hashes[2], md = UINT256_ZERO;

    if (*flagIdx/8 < block->flagsLen && *hashIdx < block->hashesCount) {
        flag = (block->flags[*flagIdx/8] & (1 << (*flagIdx % 8)));
        (*flagIdx)++;

        if (flag && depth != _ceil_log2(block->totalTx)) {
            hashes[0] = _btcMerkleBlockRootR(block, hashIdx, flagIdx, depth + 1); // left branch
            hashes[1] = _btcMerkleBlockRootR(block, hashIdx, flagIdx, depth + 1); // right branch

            if (! UInt256IsZero(hashes[0]) && ! UInt256Eq(hashes[0], hashes[1])) {
                if (UInt256IsZero(hashes[1])) hashes[1] = hashes[0]; // if right branch is missing, dup left branch
                BRSHA256_2(&md, hashes, sizeof(hashes));
            }
            else *hashIdx = SIZE_MAX; // defend against (CVE-2012-2459)
        }
        else md = block->hashes[(*hashIdx)++]; // leaf
    }
    
    return md;
}

// true if the given tx hash is known to be included in the block
int btcMerkleBlockContainsTxHash(const BRBitcoinMerkleBlock *block, UInt256 txHash)
{
    int r = 0;
    
    assert(block != NULL);
    assert(! UInt256IsZero(txHash));
    
    for (size_t i = 0; ! r && i < block->hashesCount; i++) {
        if (UInt256Eq(block->hashes[i], txHash)) r = 1;
    }
    
    return r;
}

// true if merkle tree, timestamp and difficulty target are valid
// NOTE: this does not check proof-of-work, or if the target is correct for the block's height in the chain
// - use BRMerkleBlockVerifyDifficulty() for that
int btcMerkleBlockIsValid(const BRBitcoinMerkleBlock *block, uint32_t currentTime)
{
    assert(block != NULL);
    
    // target is in "compact" format, where the most significant byte is the size of the value in bytes, next
    // bit is the sign, and the last 23 bits is the value after having been right shifted by (size - 3)*8 bits
    const uint32_t size = block->target >> 24, target = block->target & 0x007fffff;
    size_t hashIdx = 0, flagIdx = 0;
    UInt256 merkleRoot = _btcMerkleBlockRootR(block, &hashIdx, &flagIdx, 0);
    _BRAuxPow *ap = ((_BRAuxPowBlock *)block)->ap;
    int r = 1;
    
    // check if merkle root is correct
    if (block->totalTx > 0 && ! UInt256Eq(merkleRoot, block->merkleRoot)) r = 0;
    
    // check if timestamp is too far in future
    if (block->timestamp > currentTime + BLOCK_MAX_TIME_DRIFT) r = 0;
    
    // check if proof-of-work target is valid
    if (size == 0 || target == 0 || (block->target & 0x00800000)) r = 0;
            
    // check if auxPow parent block is valid
    if (ap && (! ap->parent || ! btcMerkleBlockIsValid(ap->parent, currentTime))) r = 0;

    return r;
}

// true if proof-of-work meets the stated difficulty target in the header
// NOTE: this does not check if the target is correct for the block's height in the chain
// - use BRMerkleBlockVerifyDifficulty() for that
int btcMerkleBlockVerifyProofOfWork(const BRBitcoinMerkleBlock *block)
{
    assert(block != NULL);

    // target is in "compact" format, where the most significant byte is the size of the value in bytes, next
    // bit is the sign, and the last 23 bits is the value after having been right shifted by (size - 3)*8 bits
    const uint32_t size = block->target >> 24, target = block->target & 0x007fffff;
    int i;
    UInt256 t = UINT256_ZERO;
    
    if (size - 3 >= sizeof(t) - sizeof(uint32_t)) return 0;

    if (size > 3) UInt32SetLE(&t.u8[size - 3], target);
    else UInt32SetLE(t.u8, target >> (3 - size)*8);
        
    for (i = sizeof(t) - 1; i >= 0; i--) { // check proof-of-work
        if (block->blockHash.u8[i] < t.u8[i]) break;
        if (block->blockHash.u8[i] > t.u8[i]) return 0;
    }
    
    return 1;
}

// verifies proof-of-work, and that the difficulty target is correct for the block's position in the chain
// transitionTime is the timestamp of the block at the previous difficulty transition
// transitionTime may be 0 if block->height is not a multiple of BLOCK_DIFFICULTY_INTERVAL
//
// The difficulty target algorithm works as follows:
// The target must be the same as in the previous block unless the block's height is a multiple of 2016. Every 2016
// blocks there is a difficulty transition where a new difficulty is calculated. The new target is the previous target
// multiplied by the time between the last transition block's timestamp and this one (in seconds), divided by the
// targeted time between transitions (14*24*60*60 seconds). If the new difficulty is more than 4x or less than 1/4 of
// the previous difficulty, the change is limited to either 4x or 1/4. There is also a minimum difficulty value
// intuitively named MAX_PROOF_OF_WORK... since larger values are less difficult.
int btcMerkleBlockVerifyDifficulty(const BRBitcoinMerkleBlock *block, const BRBitcoinMerkleBlock *previous,
                                   uint32_t transitionTime)
{
    int r = 1, size = 0;
    uint64_t target = 0;
    int64_t timespan;
    
    assert(block != NULL);
    assert(previous != NULL);

    if (! previous && !UInt256Eq(block->prevBlock, previous->blockHash) && block->height != previous->height + 1) r = 0;
        
    if (r && (block->height % BLOCK_DIFFICULTY_INTERVAL) == 0) {
        // target is in "compact" format, where the most significant byte is the size of the value in bytes, next
        // bit is the sign, and the last 23 bits is the value after having been right shifted by (size - 3)*8 bits
        size = previous->target >> 24;
        target = previous->target & 0x007fffff;
        timespan = (int64_t)previous->timestamp - transitionTime;
        
        // limit difficulty transition to -75% or +400%
        if (timespan < TARGET_TIMESPAN/4) timespan = TARGET_TIMESPAN/4;
        if (timespan > TARGET_TIMESPAN*4) timespan = TARGET_TIMESPAN*4;
    
        // TARGET_TIMESPAN happens to be a multiple of 256, and since timespan is at least TARGET_TIMESPAN/4, we don't
        // lose precision when target is multiplied by timespan and then divided by TARGET_TIMESPAN/256
        target *= (uint64_t)timespan;
        target /= TARGET_TIMESPAN >> 8;
        size--; // decrement size since we only divided by TARGET_TIMESPAN/256
    
        while (size < 1 || target > 0x007fffff) { target >>= 8; size++; } // normalize target for "compact" format
        target |= (uint64_t)size << 24;
    
        if (target > MAX_PROOF_OF_WORK) target = MAX_PROOF_OF_WORK; // limit to MAX_PROOF_OF_WORK
        if (block->target != target || transitionTime == 0) r = 0;
    }
    else if (r && block->target != previous->target) r = 0;
        
    return r && btcMerkleBlockVerifyProofOfWork(block);
}

// returns parent block if block uses AuxPow merge-mining: https://en.bitcoin.it/wiki/Merged_mining_specification
const BRBitcoinMerkleBlock *btcMerkleBlockAuxPowParent(const BRBitcoinMerkleBlock *block)
{
    return (((_BRAuxPowBlock *)block)->ap) ? ((_BRAuxPowBlock *)block)->ap->parent : NULL;
}

// frees memory allocated by BRMerkleBlockParse
void btcMerkleBlockFree(BRBitcoinMerkleBlock *block)
{
    assert(block != NULL);
    if (block->hashes) free(block->hashes);
    if (block->flags) free(block->flags);
    _BRAuxPowFree(((_BRAuxPowBlock *)block)->ap);
    free(block);
}
