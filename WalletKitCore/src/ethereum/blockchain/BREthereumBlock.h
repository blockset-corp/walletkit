//
//  BBREthereumBlock.h
//  WalletKitCore Ethereum
//
//  Created by Ed Gamble on 3/23/2018.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BR_Ethereum_Block_H
#define BR_Ethereum_Block_H

#include <limits.h>
#include "ethereum/base/BREthereumBase.h"
#include "BREthereumTransaction.h"
#include "BREthereumLog.h"
#include "BREthereumAccountState.h"
#include "BREthereumBloomFilter.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * An Ethereum Block header.
 *
 * As per (2018-05-04 https://ethereum.github.io/yellowpaper/paper.pdf). [Documentation from
 * that reference].  The header consists of: {hash, paretnHash, ommersHash, beneficiary, stateRoot,
 * transactionRoot, receiptsRoot, logsBloom, difficulty, number, gasLimit, gasUsed, timestamp,
 * extraData, mixHash, nonce}.  Note: this nonce is not the account nonce but the 'mining nonce'.
 */
typedef struct BREthereumBlockHeaderRecord *BREthereumBlockHeader;

/**
 * An Ethereum Block
 *
 * As per the Etheruem specification: The block in Ethereum consists of: { header, transacctions,
 * and ommers/uncles}.  To that we add: a block status - which is a complicated structure used
 * during synchronization in order to gather all the needed, but intermediate, block state; the
 * totalDifficulty - which is computed from checkpoints (in the worst case the generis block) by
 * adding the difficulty of each individual block; and the next block - which provides the
 * block chain through parents.
 */
typedef struct BREthereumBlockRecord *BREthereumBlock;
    
/**
 * An Etheruem Proof of Work Struct holds intermediate state used in the Ethereum PoW algorithm
 * used to validate blocks.
 */
typedef struct BREthereumProofOfWorkStruct *BREthereumProofOfWork;

/// MARK: - Block Header

//
// Block Header
//
extern void
ethBlockHeaderRelease (BREthereumBlockHeader header);

/**
 * Check if the block header is internally consistent
 *
 * @param header
 * @return ETHEREUM_BOOLEAN_TRUE if valid
 */
extern BREthereumBoolean
ethBlockHeaderIsInternallyValid (BREthereumBlockHeader header);

/**
 * Check if the block header is valid.  If `parent` is NULL, then `header` is consisder
 * consistent (we'll check again at some point once we have the parent).  If `pow` is provided
 * then ProofOfWork is computed and used in validity.
 *
 * @note Section 4.3.3 'Block Header Validity in https://ethereum.github.io/yellowpaper/paper.pdf
 *
 * @param header
 * @param parent
 * @param parentOmmersCount
 * @parem genesis
 * @param pow
 *
 * @return ETHEREUM_BOOLEAN_TRUE if consistent
 */
extern BREthereumBoolean
ethBlockHeaderIsValid (BREthereumBlockHeader header,
                    BREthereumBlockHeader parent,
                    size_t parentOmmersCount,
                    BREthereumBlockHeader genesis,
                    BREthereumProofOfWork pow);

extern BREthereumBlockHeader
ethBlockHeaderRlpDecode (BRRlpItem item,
                      BREthereumRlpType type,
                      BRRlpCoder coder);

extern BRRlpItem
ethBlockHeaderRlpEncode (BREthereumBlockHeader header,
                      BREthereumBoolean withNonce,
                      BREthereumRlpType type,
                      BRRlpCoder coder);

extern BREthereumHash
ethBlockHeaderGetHash (BREthereumBlockHeader header);

extern uint64_t
ethBlockHeaderGetNumber (BREthereumBlockHeader header);

extern UInt256
ethBlockHeaderGetDifficulty (BREthereumBlockHeader header);

extern uint64_t
ethBlockHeaderGetGasUsed (BREthereumBlockHeader header);

extern uint64_t
ethBlockHeaderGetTimestamp (BREthereumBlockHeader header);

extern BREthereumHash
ethBlockHeaderGetParentHash (BREthereumBlockHeader header);

extern BREthereumHash
ethBlockHeaderGetMixHash (BREthereumBlockHeader header);

extern BREthereumBoolean
ethBlockHeaderIsCHTRoot (BREthereumBlockHeader header);

// ...

extern uint64_t
ethBlockHeaderGetNonce (BREthereumBlockHeader header);

extern BREthereumBoolean
ethBlockHeaderMatch (BREthereumBlockHeader header,
                  BREthereumBloomFilter filter);

extern BREthereumBoolean
ethBlockHeaderMatchAddress (BREthereumBlockHeader header,
                         BREthereumAddress address);

// Support BRSet
extern size_t
ethBlockHeaderHashValue (const void *h);

// Support BRSet
extern int
ethBlockHeaderHashEqual (const void *h1, const void *h2);    

extern void
ethBlockHeaderReleaseForSet (void *ignore, void *item);

// Support sorting
extern BREthereumComparison
ethBlockHeaderCompare (BREthereumBlockHeader h1,
                    BREthereumBlockHeader h2);

extern BREthereumBlockHeader
ethBlockHeaderCopy (BREthereumBlockHeader source);

extern void
ethBlockHeadersRelease (BRArrayOf(BREthereumBlockHeader) headers);

/// MARK: - CHT Root (Number)

/**
 * From Parity:

 * " Get the nth CHT root, if it's been computed.
 *
 *  CHT root 0 is from block `1..2048`.
 *  CHT root 1 is from block `2049..4096`
 *  and so on.
 *
 *  This is because it's assumed that the genesis hash is known,
 *  so including it within a CHT would be redundant."
 */
extern uint64_t
ethBlockHeaderGetCHTRootNumber (BREthereumBlockHeader header);

extern uint64_t
chtRootNumberGetFromNumber (uint64_t number);


#define BLOCK_HEADER_CHT_ROOT_INTERVAL          (2048)
#define BLOCK_HEADER_CHT_ROOT_INTERVAL_SHIFT    (11)          // 2048 == (1 << 11)

/// MARK: - Block

//
// Block
//
extern BREthereumBlock
ethBlockCreateMinimal(BREthereumHash hash,
                   uint64_t number,
                   uint64_t timestamp,
                   UInt256 difficulty);

extern BREthereumBlock
ethBlockCreateFull (BREthereumBlockHeader header,
                 BREthereumBlockHeader ommers[], size_t ommersCount,
                 BREthereumTransaction transactions[], size_t transactionCount);

extern BREthereumBlock
ethBlockCreate (BREthereumBlockHeader header);

extern void
ethBlockUpdateBody (BREthereumBlock block,
                 BRArrayOf(BREthereumBlockHeader) ommers,
                 BRArrayOf(BREthereumTransaction) transactions);

extern void
ethBlockRelease (BREthereumBlock block);

extern BREthereumBlockHeader
ethBlockGetHeader (BREthereumBlock block);

extern unsigned long
ethBlockGetTransactionsCount (BREthereumBlock block);

extern BREthereumTransaction
ethBlockGetTransaction (BREthereumBlock block, size_t index);

extern BREthereumBoolean
ethBlockTransactionsAreValid (BREthereumBlock block);

extern unsigned long
ethBlockGetOmmersCount (BREthereumBlock block);

extern BREthereumBlockHeader
ethBlockGetOmmer (BREthereumBlock block, unsigned int index);

extern BREthereumHash
ethBlockGetHash (BREthereumBlock block);

extern uint64_t
ethBlockGetNumber (BREthereumBlock block);

extern uint64_t
ethBlockGetConfirmations (BREthereumBlock block);

extern uint64_t
ethBlockGetTimestamp (BREthereumBlock block);

extern UInt256
ethBlockGetDifficulty (BREthereumBlock block);

/*!
 * The total difficulty is computed as an emergent property of the entire chain by summing up
 * the difficulty for each block.  That implies, as of 2018, >6,000,000 block to sum.  However,
 * if we can rely on checkpoints (blocks with a pre-computed totalDifficulty) then we only
 * need to sum back to the checkpoint - but still need all the blocks to the checkpoint.
 *
 * Additionally, both Parity and Geth support a 'header proof' which provides the total difficulty.
 */
extern UInt256
ethBlockGetTotalDifficulty (BREthereumBlock block);

extern  void
ethBlockSetTotalDifficulty (BREthereumBlock block,
                         UInt256 totalDifficulty);

extern void
ethBlockClrTotalDifficulty (BREthereumBlock block);

extern BREthereumBoolean
ethBlockHasTotalDifficulty (BREthereumBlock block);

/**
 * Update the totalDifficulty for `block` by recusively updating the totalDifficutly for
 * block->next until block->next is NULL or block->next already has a totalDifficulty.
 *
 * Note: this is not a tail recursive algorithm and may overflow stack memory.  In practice we
 * never have block chains more than several thousand (and would never be updating the entire
 * chain all at once anyways) and thus *will* be okay.
 */
extern UInt256
ethBlockRecursivelyPropagateTotalDifficulty (BREthereumBlock block);

/**
 * A block is valid if and only if it has a total difficulty.
 *
 * @param block the block
 *
 * @return true if valid; false otherwise.
 */
static inline BREthereumBoolean
ethBlockIsValid (BREthereumBlock block) {
    return ethBlockHasTotalDifficulty (block);
}

extern void
ethBlockLinkLogsWithTransactions (BREthereumBlock block);
    
extern BRRlpItem
ethBlockRlpEncode (BREthereumBlock block,
                BREthereumNetwork network,
                BREthereumRlpType type,
                BRRlpCoder coder);
    
extern BREthereumBlock
ethBlockRlpDecode (BRRlpItem item,
                BREthereumNetwork network,
                BREthereumRlpType type,
                BRRlpCoder coder);
    
// Support BRSet
extern size_t
ethBlockHashValue (const void *h);

// Support BRSet
extern int
ethBlockHashEqual (const void *h1, const void *h2);

extern void
ethBlockReleaseForSet (void *ignore, void *item);

extern void
blocksRelease (OwnershipGiven BRArrayOf(BREthereumBlock) blocks);

/// MARK: - Block Next (Chaining)

#define BLOCK_NEXT_NONE   ((BREthereumBlock) 0)

extern BREthereumBlock
ethBlockGetNext (BREthereumBlock block);

extern BREthereumBlock // old 'next'
ethBlockSetNext (BREthereumBlock block,
              BREthereumBlock next);

extern BREthereumBoolean
ethBlockHasNext (BREthereumBlock block);

static inline BREthereumBlock // old 'next'
ethBlockClrNext (BREthereumBlock block) {
    return ethBlockSetNext(block, BLOCK_NEXT_NONE);
}

/// MARK: - Block Body Pair

/**
 * A Block Body Pair ...
 */
typedef struct {
    BRArrayOf(BREthereumTransaction) transactions;
    BRArrayOf(BREthereumBlockHeader) uncles;
} BREthereumBlockBodyPair;

extern void
ethBlockBodyPairRelease (BREthereumBlockBodyPair *pair);

extern void
ethBlockBodyPairsRelease (BRArrayOf(BREthereumBlockBodyPair) pairs);

/// MARK: - Block Header Proof

typedef struct {
    BREthereumHash hash;
    UInt256 totalDifficulty;
} BREthereumBlockHeaderProof;

/// MARK: - Block Status

typedef enum {
    BLOCK_REQUEST_NOT_NEEDED,
    BLOCK_REQUEST_PENDING,
    BLOCK_REQUEST_COMPLETE,
    BLOCK_REQUEST_ERROR
} BREthereumBlockRequestState;

typedef struct {
    BREthereumHash hash;

    BREthereumBlockRequestState transactionRequest;
    BRArrayOf (BREthereumTransaction) transactions;
    BRArrayOf (BREthereumGas) gasUsed;

    BREthereumBlockRequestState logRequest;
    BRArrayOf(BREthereumLog) logs;

    BREthereumBlockRequestState accountStateRequest;
    BREthereumAccountState accountState;

    BREthereumBlockRequestState headerProofRequest;
    BREthereumBlockHeaderProof headerProof;

    BREthereumBoolean error;
} BREthereumBlockStatus;

/**
 * Get the block's status
 */
extern BREthereumBlockStatus
ethBlockGetStatus (BREthereumBlock block);

extern BREthereumBoolean
ethBlockHasStatusComplete (BREthereumBlock block);

extern BREthereumBoolean
ethBlockHasStatusError (BREthereumBlock block);

extern void
ethBlockReportStatusError (BREthereumBlock block,
                        BREthereumBoolean error);

//
// Transaction Request
//
extern BREthereumBoolean
ethBlockHasStatusTransactionsRequest (BREthereumBlock block,
                                   BREthereumBlockRequestState request);

extern void
ethBlockReportStatusTransactionsRequest (BREthereumBlock block,
                                      BREthereumBlockRequestState request);

/**
 * Set the transactions in the block's status.  The transactions are
 * BRArrayOf(BREthereumTransaction) and the transactions are stolen (the array is now owned
 * by `block`; the transactions are owned by another).
 */
extern void
ethBlockReportStatusTransactions (BREthereumBlock block,
                               OwnershipGiven BRArrayOf(BREthereumTransaction) transactions);

extern void
ethBlockReportStatusGasUsed (BREthereumBlock block,
                          BRArrayOf(BREthereumGas) gasUsed);

//
// Log Request
//
extern BREthereumBoolean
ethBlockHasStatusLogsRequest (BREthereumBlock block,
                           BREthereumBlockRequestState request);

extern void
ethBlockReportStatusLogsRequest (BREthereumBlock block,
                              BREthereumBlockRequestState request);

/**
 * Set the logs in the block's status.  Handling of `logs` is identical to handling of
 * `transactions` - see above
 */
extern void
ethBlockReportStatusLogs (BREthereumBlock block,
                       OwnershipGiven  BRArrayOf(BREthereumLog) log);


//
// Account State Reqeust
//
extern BREthereumBoolean
ethBlockHasStatusAccountStateRequest (BREthereumBlock block,
                                   BREthereumBlockRequestState request);

extern void
ethBlockReportStatusAccountStateRequest (BREthereumBlock block,
                                      BREthereumBlockRequestState request);

/**
 * Set the account state in the block's status.
 */
extern void
ethBlockReportStatusAccountState (BREthereumBlock block,
                               BREthereumAccountState accountState);

//
// Header Proof Request
//
extern BREthereumBoolean
ethBlockHasStatusHeaderProofRequest (BREthereumBlock block,
                                  BREthereumBlockRequestState request);

extern void
ethBlockReportStatusHeaderProofRequest (BREthereumBlock block,
                                     BREthereumBlockRequestState request);

extern void
ethBlockReportStatusHeaderProof (BREthereumBlock block,
                              BREthereumBlockHeaderProof proof);
    
    //
    //
    //
extern BREthereumBoolean
ethBlockHasStatusTransaction (BREthereumBlock block,
                           BREthereumTransaction transaction);

extern BREthereumBoolean
ethBlockHasStatusLog (BREthereumBlock block,
                   BREthereumLog log);

extern void
ethBlockReleaseStatus (BREthereumBlock block,
                    BREthereumBoolean releaseTransactions,
                    BREthereumBoolean releaseLogs);

/// MARK: - Block Decoding for LES

/**
 * Return BRArrayOf(BREthereumBlockHeader) w/ array owned by caller.
 */
extern BRArrayOf(BREthereumBlockHeader)
ethBlockOmmersRlpDecode (BRRlpItem item,
                      BREthereumNetwork network,
                      BREthereumRlpType type,
                      BRRlpCoder coder);

/**
 * Return BRArrayOf(BREthereumTransaction) w/ array owned by caller
 */
extern BRArrayOf(BREthereumTransaction)
ethBlockTransactionsRlpDecode (BRRlpItem item,
                            BREthereumNetwork network,
                            BREthereumRlpType type,
                            BRRlpCoder coder);

/// MARK: - Genesis Blocks

/**
 * Return a newly-allocaed block header duplicating the genesis block header for `network`.
 */
extern BREthereumBlockHeader
networkGetGenesisBlockHeader (BREthereumNetwork network);

/**
 * Returh a newly-allocated block duplicating the generic block's header for `network`.
 */
extern BREthereumBlock
networkGetGenesisBlock (BREthereumNetwork network);

/// MARK: - Block Checkpoint

/**
 * A Block Checkpoint ...
 */
typedef struct {
    uint64_t number;
    BREthereumHash hash;
    union {
        char *std;
        UInt256 td;
    } u;
    uint64_t timestamp;
} BREthereumBlockCheckpoint;

#define BLOCK_CHECKPOINT_LAST_NUMBER      (UINT64_MAX)
#define BLOCK_CHECKPOINT_LAST_TIMESTAMP   (UINT64_MAX)

extern const BREthereumBlockCheckpoint *
ethBlockCheckpointLookupLatest (BREthereumNetwork network);

extern const BREthereumBlockCheckpoint *
ethBlockCheckpointLookupByNumber (BREthereumNetwork network,
                               uint64_t number);

extern const BREthereumBlockCheckpoint *
ethBlockCheckpointLookupByTimestamp (BREthereumNetwork network,
                                  uint64_t timestamp);

extern BREthereumBlockHeader
ethBlockCheckpointCreatePartialBlockHeader (const BREthereumBlockCheckpoint *checkpoint);

//
// Private
//
private_extern void
ethBlockFree (BREthereumBlock block);

/// MARK: - Proof of Work

extern BREthereumProofOfWork
ethProofOfWorkCreate (void);

extern void
ethProofOfWorkRelease (BREthereumProofOfWork pow);

extern void
proofOfWorkGenerate (BREthereumProofOfWork pow,
                     BREthereumBlockHeader header);

extern void
proofOfWorkCompute (BREthereumProofOfWork pow,
                    BREthereumBlockHeader header,
                    UInt256 *n,
                    BREthereumHash *m);


#ifdef __cplusplus
}
#endif

#endif /* BR_Ethereum_Block_H */
