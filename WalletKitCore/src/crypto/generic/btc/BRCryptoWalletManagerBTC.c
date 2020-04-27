#include "BRCryptoBTC.h"

#include "../../BRCryptoAccountP.h"
#include "../../BRCryptoKeyP.h"
#include "../../BRCryptoClientP.h"
#include "../../BRCryptoWalletManagerP.h"

#include "bitcoin/BRWallet.h"
#include "bitcoin/BRPeerManager.h"

// BRFileService

static BRArrayOf(BRTransaction*) initialTransactionsLoad (BRCryptoWalletManager manager);
static BRArrayOf(BRPeer)         initialPeersLoad (BRCryptoWalletManager manager);
static BRArrayOf(BRMerkleBlock*) initialBlocksLoad (BRCryptoWalletManager manager);
static BRFileServiceTypeSpecification fileServiceSpecifications[];
static size_t fileServiceSpecificationsCount;

// BRWallet Callbacks

// On: BRWalletRegisterTransaction, BRWalletRemoveTransaction
static void cryptoWalletManagerBTCBalanceChanged (void *info, uint64_t balanceInSatoshi);
// On: BRWalletRegisterTransaction
static void cryptoWalletManagerBTCTxAdded   (void *info, BRTransaction *tx);
// On: BRWalletUpdateTransactions, BRWalletSetTxUnconfirmedAfter (reorg)
static void cryptoWalletManagerBTCTxUpdated (void *info, const UInt256 *hashes, size_t count, uint32_t blockHeight, uint32_t timestamp);
// On: BRWalletRemoveTransaction
static void cryptoWalletManagerBTCTxDeleted (void *info, UInt256 hash, int notifyUser, int recommendRescan);

// BRPeerManager Callbacks
static void cryptoWalletManagerBTCSyncStarted (void *info);
static void cryptoWalletManagerBTCSyncStopped (void *info, int reason);
static void cryptoWalletManagerBTCTxStatusUpdate (void *info);
static void cryptoWalletManagerBTCSaveBlocks (void *info, int replace, BRMerkleBlock **blocks, size_t count);
static void cryptoWalletManagerBTCSavePeers  (void *info, int replace, const BRPeer *peers, size_t count);
static int  cryptoWalletManagerBTCNetworkIsReachable (void *info);
static void cryptoWalletManagerBTCThreadCleanup (void *info);
static void cryptoWalletManagerBTCTxPublished (void *info, int error);

struct BRCryptoWalletManagerBTCRecord {
    struct BRCryptoWalletManagerRecord base;

};

static BRCryptoWalletManagerBTC
cryptoWalletManagerCoerce (BRCryptoWalletManager wallet) {
    assert (CRYPTO_NETWORK_TYPE_BTC == wallet->type);
    return (BRCryptoWalletManagerBTC) wallet;
}

static void
cryptoWalletManagerReleaseHandlerBTC (BRCryptoWalletManager manager) {

}

static BRFileService
crytpWalletManagerCreateFileServiceBTC (BRCryptoWalletManager manager,
                                        const char *basePath,
                                        const char *currency,
                                        const char *network,
                                        BRFileServiceContext context,
                                        BRFileServiceErrorHandler handler) {
    return fileServiceCreateFromTypeSpecfications (basePath, currency, network,
                                                   context, handler,
                                                   fileServiceSpecificationsCount,
                                                   fileServiceSpecifications);
}

static BRArrayOf(BRCryptoWallet)
cryptoWalletManagerCreateWalletsHandlerBTC (BRCryptoWalletManager manager,
                                            BRArrayOf(BRCryptoTransfer) transfers,
                                            BRCryptoWallet *primaryWallet) {
    assert (NULL != primaryWallet);
    *primaryWallet = NULL;

    BRArrayOf(BRCryptoWallet) wallets;
    array_new (wallets, 1);

    // Get the btcMasterPublicKey
    BRCryptoAccount account = cryptoWalletManagerGetAccount(manager);
    BRMasterPubKey btcMPK = cryptoAccountAsBTC(account);
    
    // Get the btcChainParams
    BRCryptoNetwork network = cryptoWalletManagerGetNetwork(manager);
    const BRChainParams *btcChainParams = cryptoNetworkAsBTC(network);

    // Get the btcTransactions
    BRArrayOf(BRTransaction*) transactions;
    array_new (transactions, array_count(transfers));
    for (size_t index = 0; index < array_count(transfers); index++) {
        array_add (transactions, cryptoTransferAsBTC(transfers[index]));
    }

    // Since the BRWallet callbacks are not set, none of these transactions generate callbacks.
    // And, in fact, looking at BRWalletNew(), there is not even an attempt to generate callbacks
    // even if they could have been specified.
    BRWallet *btcWallet = BRWalletNew (btcChainParams->addrParams, transactions, array_count(transactions), btcMPK);

    // Free `transactions` before any non-local exists.
    array_free (transactions);

    if (NULL == btcWallet) {
        return wallets;  // bwmCreateErrorHandler (bwm, 0, "wallet");
    }

    // Set the callbacks if the wallet has been created successfully
    BRWalletSetCallbacks (btcWallet,
                          cryptoWalletManagerCoerce(manager),
                          cryptoWalletManagerBTCBalanceChanged,
                          cryptoWalletManagerBTCTxAdded,
                          cryptoWalletManagerBTCTxUpdated,
                          cryptoWalletManagerBTCTxDeleted);

    BRCryptoCurrency currency      = cryptoNetworkGetCurrency (network);
    BRCryptoUnit     unitAsBase    = cryptoNetworkGetUnitAsBase    (network, currency);
    BRCryptoUnit     unitAsDefault = cryptoNetworkGetUnitAsDefault (network, currency);

    *primaryWallet = cryptoWalletCreateAsBTC (unitAsDefault, unitAsBase, btcWallet);
    array_add (wallets, *primaryWallet);

    cryptoUnitGive (unitAsDefault);
    cryptoUnitGive (unitAsBase);
    cryptoCurrencyGive (currency);
    cryptoNetworkGive  (network);

    return wallets;
}

static BRCryptoBoolean
cryptoWalletManagerSignTransactionWithSeedHandlerBTC (BRCryptoWalletManager manager,
                                                      BRCryptoWallet wallet,
                                                      BRCryptoTransfer transfer,
                                                      UInt512 seed) {
    BRWallet      *btcWallet       = cryptoWalletAsBTC   (wallet);
    BRTransaction *btcTransaction  = cryptoTransferAsBTC (transfer);         // OWN/REF ?
    const BRChainParams *btcParams = cryptoNetworkAsBTC  (manager->network);

    return AS_CRYPTO_BOOLEAN (1 == BRWalletSignTransaction (btcWallet, btcTransaction, btcParams->forkId, seed.u8, sizeof(UInt512)));
}

static BRCryptoBoolean
cryptoWalletManagerSignTransactionWithKeyHandlerBTC (BRCryptoWalletManager manager,
                                                     BRCryptoWallet wallet,
                                                     BRCryptoTransfer transfer,
                                                     BRCryptoKey key) {
    BRTransaction *btcTransaction  = cryptoTransferAsBTC (transfer);         // OWN/REF ?
    BRKey         *btcKey          = cryptoKeyGetCore (key);
    const BRChainParams *btcParams = cryptoNetworkAsBTC  (manager->network);

    return AS_CRYPTO_BOOLEAN (1 == BRTransactionSign (btcTransaction, btcParams->forkId, btcKey, 1));
}

static BRCryptoAmount
cryptoWalletManagerEstimateLimitHandlerBTC (BRCryptoWalletManager cwm,
                                            BRCryptoWallet  wallet,
                                            BRCryptoBoolean asMaximum,
                                            BRCryptoAddress target,
                                            BRCryptoNetworkFee networkFee,
                                            BRCryptoBoolean *needEstimate,
                                            BRCryptoBoolean *isZeroIfInsuffientFunds,
                                            BRCryptoUnit unit) {
    BRWallet *btcWallet = cryptoWalletAsBTC (wallet);

    // Amount may be zero if insufficient fees
    *isZeroIfInsuffientFunds = CRYPTO_TRUE;

    // NOTE: We know BTC/BCH has a minimum balance of zero.

    uint64_t balance     = BRWalletBalance (btcWallet);
    uint64_t feePerKB    = 1000 * cryptoNetworkFeeAsBTC (networkFee);
    uint64_t amountInSAT = (CRYPTO_FALSE == asMaximum
                            ? BRWalletMinOutputAmountWithFeePerKb (btcWallet, feePerKB)
                            : BRWalletMaxOutputAmountWithFeePerKb (btcWallet, feePerKB));
    uint64_t fee         = (amountInSAT > 0
                            ? BRWalletFeeForTxAmountWithFeePerKb (btcWallet, feePerKB, amountInSAT)
                            : 0);

    //            if (CRYPTO_TRUE == asMaximum)
    //                assert (balance == amountInSAT + fee);

    if (amountInSAT + fee > balance)
        amountInSAT = 0;

    return cryptoAmountCreateInteger ((int64_t) amountInSAT, unit);
}

static void
cryptoWalletManagerEstimateFeeBasisHandlerBTC (BRCryptoWalletManager cwm,
                                               BRCryptoWallet  wallet,
                                               BRCryptoCookie cookie,
                                               BRCryptoAddress target,
                                               BRCryptoAmount amount,
                                               BRCryptoNetworkFee networkFee) {
    BRWallet *btcWallet = cryptoWalletAsBTC(wallet);

    BRCryptoBoolean overflow = CRYPTO_FALSE;
    uint64_t btcFeePerKB = 1000 * cryptoNetworkFeeAsBTC (networkFee);
    uint64_t btcAmount   = cryptoAmountGetIntegerRaw (amount, &overflow);
    assert(CRYPTO_FALSE == overflow);

    uint64_t btcFee = (0 == btcAmount ? 0 : BRWalletFeeForTxAmountWithFeePerKb (btcWallet, btcFeePerKB, btcAmount));
    uint32_t btcSizeInBytes = (uint32_t) ((1000 * btcFee) / btcFeePerKB);

    (void) btcSizeInBytes;
#ifdef REFACTOR
    bwmSignalWalletEvent(manager,
                         wallet,
                         (BRWalletEvent) {
        BITCOIN_WALLET_FEE_ESTIMATED,
        { .feeEstimated = { cookie, btcFeePerKB, btcSizeInBytes }}
    });
#endif
    return;
}

// MARK: - Client P2P

typedef struct BRCryptoClientP2PManagerRecordBTC {
    struct BRCryptoClientP2PManagerRecord base;
    BRCryptoWalletManagerBTC manager;
    BRPeerManager *btcPeerManager;
} *BRCryptoClientP2PManagerBTC;

static BRCryptoClientP2PManagerBTC
cryptoClientP2PManagerCoerce (BRCryptoClientP2PManager managerBase) {
    assert (CRYPTO_NETWORK_TYPE_BTC == managerBase->type);
    return (BRCryptoClientP2PManagerBTC) managerBase;
}

static void
cryptoClientP2PManagerReleaseHandlerBTC (BRCryptoClientP2PManager baseManager) {
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);
    BRPeerManagerFree (manager->btcPeerManager);
}

static void
cryptoClientP2PManagerSyncHandlerBTC (BRCryptoClientP2PManager baseManager, BRCryptoSyncDepth depth) {
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);
    (void) manager;
}

typedef struct {
    BRCryptoWalletManagerBTC manager;
} BRCryptoClientP2PManagerPublishInfo;

static void
cryptoClientP2PManagerSendHandlerBTC (BRCryptoClientP2PManager baseManager, BRCryptoTransfer transfer) {
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);

    BRTransaction *btcTransaction = cryptoTransferAsBTC (transfer);

    BRCryptoClientP2PManagerPublishInfo *btcInfo = calloc (1, sizeof (BRCryptoClientP2PManagerPublishInfo));
    btcInfo->manager = manager->manager;

    BRPeerManagerPublishTx (manager->btcPeerManager,
                            btcTransaction,
                            btcInfo,
                            cryptoWalletManagerBTCTxPublished);
}

static BRCryptoClientP2PManager
crytpWalletManagerCreateP2PManagerHandlerBTC (BRCryptoWalletManager cwm) {
    // load blocks, load peers

    BRCryptoClientP2PManager baseManager = cryptoClientP2PManagerCreate (sizeof (struct BRCryptoClientP2PManagerRecordBTC),
                                                                         CRYPTO_NETWORK_TYPE_BTC,
                                                                         cryptoClientP2PManagerReleaseHandlerBTC,
                                                                         cryptoClientP2PManagerSyncHandlerBTC,
                                                                         cryptoClientP2PManagerSendHandlerBTC);
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);
    manager->manager = cryptoWalletManagerCoerce(cwm);

    BRCryptoNetwork network = cryptoWalletManagerGetNetwork(cwm);
    const BRChainParams *btcChainParams = cryptoNetworkAsBTC(network);

    BRCryptoWallet wallet = cryptoWalletManagerGetWallet(cwm);
    BRWallet *btcWallet = cryptoWalletAsBTC(wallet);

    BRCryptoAccount account = cryptoWalletManagerGetAccount(cwm);
    uint32_t btcEarliestKeyTime = (uint32_t) cryptoAccountGetTimestamp(account);

    BRArrayOf(BRMerkleBlock*) blocks = initialBlocksLoad (cwm);
    BRArrayOf(BRPeer)         peers  = initialPeersLoad  (cwm);

    manager->btcPeerManager = BRPeerManagerNew (btcChainParams,
                                                btcWallet,
                                                btcEarliestKeyTime,
                                                blocks, (NULL == blocks ? 0 : array_count (blocks)),
                                                peers,  (NULL == peers  ? 0 : array_count (peers)));


    assert (NULL != manager->btcPeerManager);

    BRPeerManagerSetCallbacks (manager->btcPeerManager,
                               cryptoWalletManagerCoerce(cwm),
                               cryptoWalletManagerBTCSyncStarted,
                               cryptoWalletManagerBTCSyncStopped,
                               cryptoWalletManagerBTCTxStatusUpdate,
                               cryptoWalletManagerBTCSaveBlocks,
                               cryptoWalletManagerBTCSavePeers,
                               cryptoWalletManagerBTCNetworkIsReachable,
                               cryptoWalletManagerBTCThreadCleanup);

    if (NULL != blocks) array_free (blocks);
    if (NULL != peers ) array_free (peers);
    cryptoAccountGive (account);
    cryptoWalletGive  (wallet);
    cryptoNetworkGive (network);

    return baseManager;
}

/// MARK: - Transaction File Service

#define fileServiceTypeTransactions     "transactions"

enum {
    WALLET_MANAGER_TRANSACTION_VERSION_1
};

static UInt256
fileServiceTypeTransactionV1Identifier (BRFileServiceContext context,
                                        BRFileService fs,
                                        const void *entity) {
    const BRTransaction *transaction = entity;
    return transaction->txHash;
}

static uint8_t *
fileServiceTypeTransactionV1Writer (BRFileServiceContext context,
                                    BRFileService fs,
                                    const void* entity,
                                    uint32_t *bytesCount) {
    const BRTransaction *transaction = entity;

    size_t txTimestampSize  = sizeof (uint32_t);
    size_t txBlockHeightSize = sizeof (uint32_t);
    size_t txSize = BRTransactionSerialize (transaction, NULL, 0);

    assert (txTimestampSize   == sizeof(transaction->timestamp));
    assert (txBlockHeightSize == sizeof(transaction->blockHeight));

    *bytesCount = (uint32_t) (txSize + txBlockHeightSize + txTimestampSize);

    uint8_t *bytes = calloc (*bytesCount, 1);

    size_t bytesOffset = 0;

    BRTransactionSerialize (transaction, &bytes[bytesOffset], txSize);
    bytesOffset += txSize;

    UInt32SetLE (&bytes[bytesOffset], transaction->blockHeight);
    bytesOffset += txBlockHeightSize;

    UInt32SetLE(&bytes[bytesOffset], transaction->timestamp);

    return bytes;
}

static void *
fileServiceTypeTransactionV1Reader (BRFileServiceContext context,
                                    BRFileService fs,
                                    uint8_t *bytes,
                                    uint32_t bytesCount) {
    size_t txTimestampSize  = sizeof (uint32_t);
    size_t txBlockHeightSize = sizeof (uint32_t);
    if (bytesCount < (txTimestampSize + txBlockHeightSize)) return NULL;

    BRTransaction *transaction = BRTransactionParse (bytes, bytesCount - txTimestampSize - txBlockHeightSize);
    if (NULL == transaction) return NULL;

    transaction->blockHeight = UInt32GetLE (&bytes[bytesCount - txTimestampSize - txBlockHeightSize]);
    transaction->timestamp   = UInt32GetLE (&bytes[bytesCount - txTimestampSize]);

    return transaction;
}

static BRArrayOf(BRTransaction*)
initialTransactionsLoad (BRCryptoWalletManager manager) {
    BRSetOf(BRTransaction*) transactionSet = BRSetNew(BRTransactionHash, BRTransactionEq, 100);
    if (1 != fileServiceLoad (manager->fileService, transactionSet, fileServiceTypeTransactions, 1)) {
        BRSetFreeAll(transactionSet, (void (*) (void*)) BRTransactionFree);
        _peer_log ("BWM: failed to load transactions");
        return NULL;
    }

    size_t transactionsCount = BRSetCount(transactionSet);

    BRArrayOf(BRTransaction*) transactions;
    array_new (transactions, transactionsCount);
    array_set_count(transactions, transactionsCount);

    BRSetAll(transactionSet, (void**) transactions, transactionsCount);
    BRSetFree(transactionSet);

    _peer_log ("BWM: loaded %zu transactions", transactionsCount);
    return transactions;
}

/// MARK: - Block File Service

#define fileServiceTypeBlocks       "blocks"
enum {
    WALLET_MANAGER_BLOCK_VERSION_1
};

static UInt256
fileServiceTypeBlockV1Identifier (BRFileServiceContext context,
                                  BRFileService fs,
                                  const void *entity) {
    const BRMerkleBlock *block = (BRMerkleBlock*) entity;
    return block->blockHash;
}

static uint8_t *
fileServiceTypeBlockV1Writer (BRFileServiceContext context,
                              BRFileService fs,
                              const void* entity,
                              uint32_t *bytesCount) {
    const BRMerkleBlock *block = entity;

    // The serialization of a block does not include the block height.  Thus, we'll need to
    // append the height.

    // These are serialization sizes
    size_t blockHeightSize = sizeof (uint32_t);
    size_t blockSize = BRMerkleBlockSerialize(block, NULL, 0);

    // Confirm.
    assert (blockHeightSize == sizeof (block->height));

    // Update bytesCound with the total of what is written.
    *bytesCount = (uint32_t) (blockSize + blockHeightSize);

    // Get our bytes
    uint8_t *bytes = calloc (*bytesCount, 1);

    // We'll serialize the block itself first
    BRMerkleBlockSerialize(block, bytes, blockSize);

    // And then the height.
    UInt32SetLE(&bytes[blockSize], block->height);

    return bytes;
}

static void *
fileServiceTypeBlockV1Reader (BRFileServiceContext context,
                              BRFileService fs,
                              uint8_t *bytes,
                              uint32_t bytesCount) {
    size_t blockHeightSize = sizeof (uint32_t);
    if (bytesCount < blockHeightSize) return NULL;

    BRMerkleBlock *block = BRMerkleBlockParse (bytes, bytesCount - blockHeightSize);
    if (NULL == block) return NULL;

    block->height = UInt32GetLE(&bytes[bytesCount - blockHeightSize]);

    return block;
}

static BRArrayOf(BRMerkleBlock*)
initialBlocksLoad (BRCryptoWalletManager manager) {
    BRSetOf(BRMerkleBlock*) blockSet = BRSetNew(BRMerkleBlockHash, BRMerkleBlockEq, 100);
    if (1 != fileServiceLoad (manager->fileService, blockSet, fileServiceTypeBlocks, 1)) {
        BRSetFreeAll(blockSet, (void (*) (void*)) BRMerkleBlockFree);
        _peer_log ("BWM: failed to load blocks");
        return NULL;
    }

    size_t blocksCount = BRSetCount(blockSet);

    BRArrayOf(BRMerkleBlock*) blocks;
    array_new (blocks, blocksCount);
    array_set_count(blocks, blocksCount);

    BRSetAll(blockSet, (void**) blocks, blocksCount);
    BRSetFree(blockSet);

    _peer_log ("BWM: loaded %zu blocks", blocksCount);
    return blocks;
}

/// MARK: - Peer File Service

#define fileServiceTypePeers        "peers"
enum {
    WALLET_MANAGER_PEER_VERSION_1
};

static UInt256
fileServiceTypePeerV1Identifier (BRFileServiceContext context,
                                 BRFileService fs,
                                 const void *entity) {
    const BRPeer *peer = entity;

    UInt256 hash;
    BRSHA256 (&hash, peer, sizeof(BRPeer));

    return hash;
}

static uint8_t *
fileServiceTypePeerV1Writer (BRFileServiceContext context,
                             BRFileService fs,
                             const void* entity,
                             uint32_t *bytesCount) {
    const BRPeer *peer = entity;
    size_t offset = 0;

    *bytesCount = sizeof (BRPeer);
    uint8_t *bytes = malloc (*bytesCount);

    memcpy (&bytes[offset], peer->address.u8, sizeof (UInt128));
    offset += sizeof (UInt128);

    UInt16SetBE (&bytes[offset], peer->port);
    offset += sizeof (uint16_t);

    UInt64SetBE (&bytes[offset], peer->services);
    offset += sizeof (uint64_t);

    UInt64SetBE (&bytes[offset], peer->timestamp);
    offset += sizeof (uint64_t);

    bytes[offset] = peer->flags;
    offset += sizeof(uint8_t); (void) offset;

    return bytes;
}

static void *
fileServiceTypePeerV1Reader (BRFileServiceContext context,
                             BRFileService fs,
                             uint8_t *bytes,
                             uint32_t bytesCount) {
    assert (bytesCount == sizeof (BRPeer));

    size_t offset = 0;

    BRPeer *peer = malloc (bytesCount);

    memcpy (peer->address.u8, &bytes[offset], sizeof (UInt128));
    offset += sizeof (UInt128);

    peer->port = UInt16GetBE (&bytes[offset]);
    offset += sizeof (uint16_t);

    peer->services = UInt64GetBE(&bytes[offset]);
    offset += sizeof (uint64_t);

    peer->timestamp = UInt64GetBE(&bytes[offset]);
    offset += sizeof (uint64_t);

    peer->flags = bytes[offset];
    offset += sizeof(uint8_t); (void) offset;

    return peer;
}

static BRArrayOf(BRPeer)
initialPeersLoad (BRCryptoWalletManager manager) {
    /// Load peers for the wallet manager.
    BRSetOf(BRPeer*) peerSet = BRSetNew(BRPeerHash, BRPeerEq, 100);
    if (1 != fileServiceLoad (manager->fileService, peerSet, fileServiceTypePeers, 1)) {
        BRSetFreeAll(peerSet, free);
        _peer_log ("BWM: failed to load peers");
        return NULL;
    }

    size_t peersCount = BRSetCount(peerSet);

    BRArrayOf(BRPeer) peers;
    array_new (peers, peersCount);

    FOR_SET (BRPeer*, peer, peerSet) array_add (peers, *peer);
    BRSetFreeAll(peerSet, free);

    _peer_log ("BWM: loaded %zu peers", peersCount);
    return peers;
}

static BRFileServiceTypeSpecification fileServiceSpecifications[] = {
    {
        fileServiceTypeTransactions,
        WALLET_MANAGER_TRANSACTION_VERSION_1,
        1,
        {
            {
                WALLET_MANAGER_TRANSACTION_VERSION_1,
                fileServiceTypeTransactionV1Identifier,
                fileServiceTypeTransactionV1Reader,
                fileServiceTypeTransactionV1Writer
            }
        }
    },

    {
        fileServiceTypeBlocks,
        WALLET_MANAGER_BLOCK_VERSION_1,
        1,
        {
            {
                WALLET_MANAGER_BLOCK_VERSION_1,
                fileServiceTypeBlockV1Identifier,
                fileServiceTypeBlockV1Reader,
                fileServiceTypeBlockV1Writer
            }
        }
    },

    {
        fileServiceTypePeers,
        WALLET_MANAGER_PEER_VERSION_1,
        1,
        {
            {
                WALLET_MANAGER_PEER_VERSION_1,
                fileServiceTypePeerV1Identifier,
                fileServiceTypePeerV1Reader,
                fileServiceTypePeerV1Writer
            }
        }
    }
};
static size_t fileServiceSpecificationsCount = (sizeof (fileServiceSpecifications) /
                                                sizeof (BRFileServiceTypeSpecification));



// MARK: - BRWallet Callbacks

static void cryptoWalletManagerBTCBalanceChanged (void *info, uint64_t balanceInSatoshi) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
}

static void cryptoWalletManagerBTCTxAdded   (void *info, BRTransaction *tx) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
}

static void cryptoWalletManagerBTCTxUpdated (void *info, const UInt256 *hashes, size_t count, uint32_t blockHeight, uint32_t timestamp) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
}

static void cryptoWalletManagerBTCTxDeleted (void *info, UInt256 hash, int notifyUser, int recommendRescan) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
}

// MARK: BRPeerManager Callbacks

static void cryptoWalletManagerBTCSyncStarted (void *info) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
}

static void cryptoWalletManagerBTCSyncStopped (void *info, int reason) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
}

static void cryptoWalletManagerBTCTxStatusUpdate (void *info) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
}

static void cryptoWalletManagerBTCSaveBlocks (void *info, int replace, BRMerkleBlock **blocks, size_t count) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
}

static void cryptoWalletManagerBTCSavePeers  (void *info, int replace, const BRPeer *peers, size_t count) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
}

static int  cryptoWalletManagerBTCNetworkIsReachable (void *info) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
    return 1;
}

static void cryptoWalletManagerBTCThreadCleanup (void *info) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
}

static void cryptoWalletManagerBTCTxPublished (void *info, int error) {
    BRCryptoClientP2PManagerPublishInfo *btcInfo = info;
    BRCryptoWalletManagerBTC manager = btcInfo->manager;
    (void) manager;
    // Something.
    free (info);
}

BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersBTC = {
    cryptoWalletManagerReleaseHandlerBTC,
    crytpWalletManagerCreateFileServiceBTC,
    cryptoWalletManagerCreateWalletsHandlerBTC,
    cryptoWalletManagerSignTransactionWithSeedHandlerBTC,
    cryptoWalletManagerSignTransactionWithKeyHandlerBTC,
    cryptoWalletManagerEstimateLimitHandlerBTC,
    cryptoWalletManagerEstimateFeeBasisHandlerBTC,
    crytpWalletManagerCreateP2PManagerHandlerBTC
};
