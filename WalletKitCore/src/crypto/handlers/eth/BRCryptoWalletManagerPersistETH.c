//
//  BREthereumEWMPersist.c
//  BRCore
//
//  Created by Ed Gamble on 9/24/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoETH.h"

#include "ethereum/blockchain/BREthereumBlock.h"
#include "ethereum/blockchain/BREthereumTransaction.h"
#include "ethereum/blockchain/BREthereumLog.h"
#include "ethereum/les/BREthereumLES.h"

/// MARK: - Transaction File Service
#define fileServiceTypeTransactions "transactions"

enum {
    EWM_TRANSACTION_VERSION_1
};

static UInt256
fileServiceTypeTransactionV1Identifier (BRFileServiceContext context,
                                        BRFileService fs,
                                        const void *entity) {
    BREthereumTransaction transaction = (BREthereumTransaction) entity;
    BREthereumHash hash = transactionGetHash(transaction);

    UInt256 result;
    memcpy (result.u8, hash.bytes, ETHEREUM_HASH_BYTES);
    return result;
}

static uint8_t *
fileServiceTypeTransactionV1Writer (BRFileServiceContext context,
                                    BRFileService fs,
                                    const void* entity,
                                    uint32_t *bytesCount) {
    BRCryptoWalletManagerETH manager = context;
    BREthereumTransaction transaction = (BREthereumTransaction) entity;

    BRRlpItem item = transactionRlpEncode(transaction, manager->network, RLP_TYPE_ARCHIVE, manager->coder);
    BRRlpData data = rlpItemGetData (manager->coder, item);
    rlpItemRelease (manager->coder, item);

    *bytesCount = (uint32_t) data.bytesCount;
    return data.bytes;
}

static void *
fileServiceTypeTransactionV1Reader (BRFileServiceContext context,
                                    BRFileService fs,
                                    uint8_t *bytes,
                                    uint32_t bytesCount) {
    BRCryptoWalletManagerETH manager = context;

    BRRlpData data = { bytesCount, bytes };
    BRRlpItem item = rlpDataGetItem (manager->coder, data);

    BREthereumTransaction transaction = transactionRlpDecode(item, manager->network, RLP_TYPE_ARCHIVE, manager->coder);
    rlpItemRelease (manager->coder, item);

    return transaction;
}

/// MARK: - Log File Service

#define fileServiceTypeLogs "logs"

enum {
    EWM_LOG_VERSION_1
};

static UInt256
fileServiceTypeLogV1Identifier (BRFileServiceContext context,
                                BRFileService fs,
                                const void *entity) {
    const BREthereumLog log = (BREthereumLog) entity;
    BREthereumHash hash = logGetHash( log);

    UInt256 result;
    memcpy (result.u8, hash.bytes, ETHEREUM_HASH_BYTES);
    return result;
}

static uint8_t *
fileServiceTypeLogV1Writer (BRFileServiceContext context,
                            BRFileService fs,
                            const void* entity,
                            uint32_t *bytesCount) {
    BRCryptoWalletManagerETH manager = context;
    BREthereumLog log = (BREthereumLog) entity;

    BRRlpItem item = logRlpEncode (log, RLP_TYPE_ARCHIVE, manager->coder);
    BRRlpData data = rlpItemGetData (manager->coder, item);
    rlpItemRelease (manager->coder, item);

    *bytesCount = (uint32_t) data.bytesCount;
    return data.bytes;
}

static void *
fileServiceTypeLogV1Reader (BRFileServiceContext context,
                            BRFileService fs,
                            uint8_t *bytes,
                            uint32_t bytesCount) {
    BRCryptoWalletManagerETH manager = context;

    BRRlpData data = { bytesCount, bytes };
    BRRlpItem item = rlpDataGetItem (manager->coder, data);

    BREthereumLog log = logRlpDecode(item, RLP_TYPE_ARCHIVE, manager->coder);
    rlpItemRelease (manager->coder, item);

    return log;
}

/// MARK: - Block File Service

#define fileServiceTypeBlocks "blocks"
enum {
    EWM_BLOCK_VERSION_1
};

static UInt256
fileServiceTypeBlockV1Identifier (BRFileServiceContext context,
                                  BRFileService fs,
                                  const void *entity) {
    const BREthereumBlock block = (BREthereumBlock) entity;
    BREthereumHash hash = blockGetHash(block);

    UInt256 result;
    memcpy (result.u8, hash.bytes, ETHEREUM_HASH_BYTES);
    return result;
}

static uint8_t *
fileServiceTypeBlockV1Writer (BRFileServiceContext context,
                              BRFileService fs,
                              const void* entity,
                              uint32_t *bytesCount) {
    BRCryptoWalletManagerETH manager = context;
    BREthereumBlock block = (BREthereumBlock) entity;

    BRRlpItem item = blockRlpEncode(block, manager->network, RLP_TYPE_ARCHIVE, manager->coder);
    BRRlpData data = rlpItemGetData (manager->coder, item);
    rlpItemRelease (manager->coder, item);

    *bytesCount = (uint32_t) data.bytesCount;
    return data.bytes;
}

static void *
fileServiceTypeBlockV1Reader (BRFileServiceContext context,
                              BRFileService fs,
                              uint8_t *bytes,
                              uint32_t bytesCount) {
    BRCryptoWalletManagerETH manager = context;

    BRRlpData data = { bytesCount, bytes };
    BRRlpItem item = rlpDataGetItem (manager->coder, data);

    BREthereumBlock block = blockRlpDecode (item, manager->network, RLP_TYPE_ARCHIVE, manager->coder);
    rlpItemRelease (manager->coder, item);

    return block;
}

// MARK: - Node File Service

#define fileServiceTypeNodes "nodes"
enum {
    EWM_NODE_VERSION_1
};

static UInt256
fileServiceTypeNodeV1Identifier (BRFileServiceContext context,
                                 BRFileService fs,
                                 const void *entity) {
    const BREthereumNodeConfig node = (BREthereumNodeConfig) entity;

    BREthereumHash hash = nodeConfigGetHash(node);

    UInt256 result;
    memcpy (result.u8, hash.bytes, ETHEREUM_HASH_BYTES);
    return result;
}

static uint8_t *
fileServiceTypeNodeV1Writer (BRFileServiceContext context,
                             BRFileService fs,
                             const void* entity,
                             uint32_t *bytesCount) {
    BRCryptoWalletManagerETH manager = context;
    const BREthereumNodeConfig node = (BREthereumNodeConfig) entity;

    BRRlpItem item = nodeConfigEncode (node, manager->coder);
    BRRlpData data = rlpItemGetData (manager->coder, item);
    rlpItemRelease (manager->coder, item);

    *bytesCount = (uint32_t) data.bytesCount;
    return data.bytes;
}

static void *
fileServiceTypeNodeV1Reader (BRFileServiceContext context,
                             BRFileService fs,
                             uint8_t *bytes,
                             uint32_t bytesCount) {
    BRCryptoWalletManagerETH manager = context;

    BRRlpData data = { bytesCount, bytes };
    BRRlpItem item = rlpDataGetItem (manager->coder, data);

    BREthereumNodeConfig node = nodeConfigDecode (item, manager->coder);
    rlpItemRelease (manager->coder, item);

    return node;
}

/// MARK: - Token File Service

#define fileServiceTypeTokens "tokens"

enum {
    EWM_TOKEN_VERSION_1
};

static UInt256
fileServiceTypeTokenV1Identifier (BRFileServiceContext context,
                                        BRFileService fs,
                                        const void *entity) {
    BREthereumToken token = (BREthereumToken) entity;
    BREthereumHash hash = ethTokenGetHash(token);

    UInt256 result;
    memcpy (result.u8, hash.bytes, ETHEREUM_HASH_BYTES);
    return result;
}

static uint8_t *
fileServiceTypeTokenV1Writer (BRFileServiceContext context,
                                    BRFileService fs,
                                    const void* entity,
                                    uint32_t *bytesCount) {
    BRCryptoWalletManagerETH manager = context;
    BREthereumToken token = (BREthereumToken) entity;

    BRRlpItem item = ethTokenRlpEncode(token, manager->coder);
    BRRlpData data = rlpItemGetData (manager->coder, item);
    rlpItemRelease (manager->coder, item);

    *bytesCount = (uint32_t) data.bytesCount;
    return data.bytes;
}

static void *
fileServiceTypeTokenV1Reader (BRFileServiceContext context,
                                    BRFileService fs,
                                    uint8_t *bytes,
                                    uint32_t bytesCount) {
    BRCryptoWalletManagerETH manager = context;

    BRRlpData data = { bytesCount, bytes };
    BRRlpItem item = rlpDataGetItem (manager->coder, data);

    BREthereumToken token = ethTokenRlpDecode(item, manager->coder);
    rlpItemRelease (manager->coder, item);

    return token;
}

/// MARK: - Wallet File Service
#if 0
#define fileServiceTypeWallets "wallets"

enum {
    EWM_WALLET_VERSION_1
};

static UInt256
fileServiceTypeWalletV1Identifier (BRFileServiceContext context,
                                   BRFileService fs,
                                   const void *entity) {
    BREthereumWalletState state = (BREthereumWalletState) entity;
    BREthereumHash hash = walletStateGetHash (state);

    UInt256 result;
    memcpy (result.u8, hash.bytes, ETHEREUM_HASH_BYTES);
    return result;
}

static uint8_t *
fileServiceTypeWalletV1Writer (BRFileServiceContext context,
                               BRFileService fs,
                               const void* entity,
                               uint32_t *bytesCount) {
    BRCryptoWalletManagerETH manager = context;
    BREthereumWalletState state = (BREthereumWalletState) entity;

    BRRlpItem item = walletStateEncode (state, manager->coder);
    BRRlpData data = rlpItemGetData (manager->coder, item);
    rlpItemRelease (manager->coder, item);

    *bytesCount = (uint32_t) data.bytesCount;
    return data.bytes;
}

static void *
fileServiceTypeWalletV1Reader (BRFileServiceContext context,
                               BRFileService fs,
                               uint8_t *bytes,
                               uint32_t bytesCount) {
    BRCryptoWalletManagerETH manager = context;

    BRRlpData data = { bytesCount, bytes };
    BRRlpItem item = rlpDataGetItem (manager->coder, data);

    BREthereumWalletState state = walletStateDecode(item, manager->coder);
    rlpItemRelease (manager->coder, item);

    return state;
}
#endif

static BRFileServiceTypeSpecification fileServiceSpecifications[] = {
    {
        fileServiceTypeTransactions,
        EWM_TRANSACTION_VERSION_1,
        1,
        {
            {
                EWM_TRANSACTION_VERSION_1,
                fileServiceTypeTransactionV1Identifier,
                fileServiceTypeTransactionV1Reader,
                fileServiceTypeTransactionV1Writer
            }
        }
    },

    {
        fileServiceTypeLogs,
        EWM_LOG_VERSION_1,
        1,
        {
            {
                EWM_LOG_VERSION_1,
                fileServiceTypeLogV1Identifier,
                fileServiceTypeLogV1Reader,
                fileServiceTypeLogV1Writer
            }
        }
    },

    {
        fileServiceTypeBlocks,
        EWM_BLOCK_VERSION_1,
        1,
        {
            {
                EWM_BLOCK_VERSION_1,
                fileServiceTypeBlockV1Identifier,
                fileServiceTypeBlockV1Reader,
                fileServiceTypeBlockV1Writer
            }
        }
    },

    {
        fileServiceTypeNodes,
        EWM_NODE_VERSION_1,
        1,
        {
            {
                EWM_NODE_VERSION_1,
                fileServiceTypeNodeV1Identifier,
                fileServiceTypeNodeV1Reader,
                fileServiceTypeNodeV1Writer
            }
        }
    },

    {
        fileServiceTypeTokens,
        EWM_TOKEN_VERSION_1,
        1,
        {
            {
                EWM_TOKEN_VERSION_1,
                fileServiceTypeTokenV1Identifier,
                fileServiceTypeTokenV1Reader,
                fileServiceTypeTokenV1Writer
            }
        }
    },
#if 0
    {
        fileServiceTypeWallets,
        EWM_WALLET_VERSION_1,
        1,
        {
            {
                EWM_TOKEN_VERSION_1,
                fileServiceTypeWalletV1Identifier,
                fileServiceTypeWalletV1Reader,
                fileServiceTypeWalletV1Writer
            }
        }
    }
#endif
};

const char *fileServiceTypeTransactionsETH = fileServiceTypeTransactions;
const char *fileServiceTypeLogsETH         = fileServiceTypeLogs;
const char *fileServiceTypeBlocksETH       = fileServiceTypeBlocks;
const char *fileServiceTypeNodesETH        = fileServiceTypeNodes;
const char *fileServiceTypeTokensETH       = fileServiceTypeTokens;
//const char *ewmFileServiceTypeWallets      = fileServiceTypeWallets;

size_t fileServiceSpecificationsCountETH = sizeof(fileServiceSpecifications)/sizeof(BRFileServiceTypeSpecification);
BRFileServiceTypeSpecification *fileServiceSpecificationsETH = fileServiceSpecifications;

