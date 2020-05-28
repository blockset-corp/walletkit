//
//  BRCryptoWalletManagerETH.c
//  Core
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoETH.h"

#include "crypto/BRCryptoAccountP.h"
#include "crypto/BRCryptoNetworkP.h"
#include "crypto/BRCryptoKeyP.h"
#include "crypto/BRCryptoClientP.h"
#include "crypto/BRCryptoWalletManagerP.h"

#include "ethereum/blockchain/BREthereumBlock.h"
#include "ethereum/blockchain/BREthereumTransaction.h"
#include "ethereum/blockchain/BREthereumLog.h"
#include "ethereum/les/BREthereumLES.h"
#include "ethereum/bcs/BREthereumBCS.h"

// MARK: - Forward Declarations

static void
cryptoWalletManagerCreateTokenForCurrency (BRCryptoWalletManagerETH manager,
                                           BRCryptoCurrency currency,
                                           BRCryptoUnit     unitDefault);

// MARK: - BCS Listener Forward Declarations

static void
ewmHandleBlockChain (BREthereumBCSCallbackContext context,
                     BREthereumHash headBlockHash,
                     uint64_t headBlockNumber,
                     uint64_t headBlockTimestamp);

static void
ewmHandleAccountState (BREthereumBCSCallbackContext context,
                       BREthereumAccountState accountState);

static void
ewmHandleTransaction (BREthereumBCSCallbackContext context,
                      BREthereumBCSCallbackTransactionType type,
                      OwnershipGiven BREthereumTransaction transaction);

static void
ewmHandleLog (BREthereumBCSCallbackContext context,
              BREthereumBCSCallbackLogType type,
              OwnershipGiven BREthereumLog log);

static void
ewmHandleSaveBlocks (BREthereumBCSCallbackContext context,
                     OwnershipGiven BRArrayOf(BREthereumBlock) blocks);

static void
ewmHandleSaveNodes (BREthereumBCSCallbackContext context,
                    OwnershipGiven BRArrayOf(BREthereumNodeConfig) nodes);

static void
ewmHandleSync (BREthereumBCSCallbackContext context,
               BREthereumBCSCallbackSyncType type,
               uint64_t blockNumberStart,
               uint64_t blockNumberCurrent,
               uint64_t blockNumberStop);

static void
ewmHandleGetBlocks (BREthereumBCSCallbackContext context,
                    BREthereumAddress address,
                    BREthereumSyncInterestSet interests,
                    uint64_t blockStart,
                    uint64_t blockStop);

// BRFileService

/// MARK: - File Service, Initial Load

static void
cryptoWalletManagerCreateInitialSets (BRCryptoWalletManager manager,
                                      BREthereumNetwork network,
                                      BREthereumTimestamp accountTimestamp,
                                      BRSetOf(BREthereumTransaction) *transactions,
                                      BRSetOf(BREthereumLog) *logs,
                                      BRSetOf(BREthereumNodeConfig) *nodes,
                                      BRSetOf(BREthereumBlock) *blocks,
                                      BRSetOf(BREthereumToken) *tokens) {

    *transactions = initialTransactionsLoadETH (manager);
    *logs   = initialLogsLoadETH   (manager);
    *nodes  = initialNodesLoadETH  (manager);
    *blocks = initialBlocksLoadETH (manager);
    *tokens = initialTokensLoadETH  (manager);
#if 0
    *states = initialWalletsLoadETH (manager);
#endif

    // If any are NULL, then we have an error and a full sync is required.  The sync will be
    // started automatically, as part of the normal processing, of 'blocks' (we'll use a checkpoint,
    // before the `accountTimestamp, which will be well in the past and we'll sync up to the
    // head of the blockchain).
    if (NULL == *transactions || NULL == *logs || NULL == *nodes || NULL == *blocks || NULL == *tokens /* || NULL == *states */) {
        // If the set exists, clear it out completely and then create another one.  Note, since
        // we have `BRSetFreeAll()` we'll use that even though it frees the set and then we
        // create one again, minimally wastefully.
        if (NULL != *transactions) { BRSetFreeAll (*transactions, (void (*) (void*)) transactionRelease); }
        *transactions = BRSetNew (transactionHashValue, transactionHashEqual, EWM_INITIAL_SET_SIZE_DEFAULT);

        if (NULL != *logs) { BRSetFreeAll (*logs, (void (*) (void*)) logRelease); }
        *logs = BRSetNew (logHashValue, logHashEqual, EWM_INITIAL_SET_SIZE_DEFAULT);

        if (NULL != *blocks) { BRSetFreeAll (*blocks,  (void (*) (void*)) blockRelease); }
        *blocks = BRSetNew (blockHashValue, blockHashEqual, EWM_INITIAL_SET_SIZE_DEFAULT);

        if (NULL != *nodes) { BRSetFreeAll (*nodes, (void (*) (void*)) nodeConfigRelease); }
        *nodes = BRSetNew (nodeConfigHashValue, nodeConfigHashEqual, EWM_INITIAL_SET_SIZE_DEFAULT);

        if (NULL != *tokens) { BRSetFreeAll (*tokens, (void (*) (void*)) ethTokenRelease); }
        *tokens = ethTokenSetCreate (EWM_INITIAL_SET_SIZE_DEFAULT);
#if 0
        if (NULL != *states) { BRSetFreeAll (*states, (void (*) (void*)) walletStateRelease); }
        *states = walletStateSetCreate(EWM_INITIAL_SET_SIZE_DEFAULT);
#endif
    }

    // If we have no blocks; then add a checkpoint
    if (0 == BRSetCount(*blocks)) {
        const BREthereumBlockCheckpoint *checkpoint = blockCheckpointLookupByTimestamp (network, accountTimestamp);
        BREthereumBlock block = blockCreate (blockCheckpointCreatePartialBlockHeader (checkpoint));
        blockSetTotalDifficulty (block, checkpoint->u.td);
        BRSetAdd (*blocks, block);
    }
}

//static BRFileServiceTypeSpecification fileServiceSpecifications[];
//static size_t fileServiceSpecificationsCount;

static BRCryptoWalletManagerETH
cryptoWalletManagerCoerce (BRCryptoWalletManager manager) {
    assert (CRYPTO_NETWORK_TYPE_ETH == manager->type);
    return (BRCryptoWalletManagerETH) manager;
}

static BRCryptoWalletManager
cryptoWalletManagerCreateETH (BRCryptoListener listener,
                                     BRCryptoClient client,
                                     BRCryptoAccount account,
                                     BRCryptoNetwork network,
                                     BRCryptoSyncMode mode,
                                     BRCryptoAddressScheme scheme,
                                     const char *path) {
    BRCryptoWalletManager managerBase = cryptoWalletManagerAllocAndInit (sizeof (struct BRCryptoWalletManagerETHRecord),
                                                                         cryptoNetworkGetType(network),
                                                                         listener,
                                                                         client,
                                                                         account,
                                                                         network,
                                                                         scheme,
                                                                         path,
                                                                         CRYPTO_CLIENT_REQUEST_USE_TRANSFERS);
    BRCryptoWalletManagerETH manager = cryptoWalletManagerCoerce (managerBase);

    manager->network = cryptoNetworkAsETH (network);
    manager->account = cryptoAccountAsETH (account);
    manager->coder   = rlpCoderCreate();

    return managerBase;
}

static void
cryptoWalletManagerReleaseETH (BRCryptoWalletManager managerBase) {
    BRCryptoWalletManagerETH manager = cryptoWalletManagerCoerce (managerBase);

    rlpCoderRelease (manager->coder);
    if (NULL != manager->tokens)
        BRSetFreeAll(manager->tokens, (void (*) (void*)) ethTokenRelease);
}


static void
cryptoWalletManagerInitializeETH (BRCryptoWalletManager managerBase) {
    BRCryptoWalletManagerETH manager = cryptoWalletManagerCoerce (managerBase);

    BRCryptoNetwork  network       = managerBase->network;
    BRCryptoCurrency currency      = cryptoNetworkGetCurrency (network);
    BRCryptoUnit     unitAsBase    = cryptoNetworkGetUnitAsBase    (network, currency);
    BRCryptoUnit     unitAsDefault = cryptoNetworkGetUnitAsDefault (network, currency);

    // Create the primary BRCryptoWallet
    managerBase->wallet = cryptoWalletCreateAsETH (unitAsDefault, unitAsDefault, NULL, manager->account);
    array_add (managerBase->wallets, managerBase->wallet);

    // Load the persistently stored data.
    BRSetOf(BREthereumTransaction) transactions = initialTransactionsLoadETH (managerBase);
    BRSetOf(BREthereumLog)         logs         = initialLogsLoadETH         (managerBase);

    // Save the recovered tokens
    manager->tokens = initialTokensLoadETH (managerBase);

    // Ensure a token (but not a wallet) for each currency
    size_t currencyCount = cryptoNetworkGetCurrencyCount (network);
    for (size_t index = 0; index < currencyCount; index++) {
        BRCryptoCurrency c = cryptoNetworkGetCurrencyAt (network, index);
        if (c != currency) {
            BRCryptoUnit unitDefault = cryptoNetworkGetUnitAsDefault (network, c);
            cryptoWalletManagerCreateTokenForCurrency (manager, c, unitDefault);
            cryptoUnitGive (unitDefault);
        }
        cryptoCurrencyGive (c);
    }

    // Announce all the provided transactions...
    FOR_SET (BREthereumTransaction, transaction, transactions)
        ewmHandleTransaction (manager, BCS_CALLBACK_TRANSACTION_ADDED, transaction);
    BRSetFree(transactions);

    // ... and all the provided logs
    FOR_SET (BREthereumLog, log, logs)
        ewmHandleLog (manager, BCS_CALLBACK_LOG_ADDED, log);
    BRSetFree (logs);

    cryptoUnitGive (unitAsDefault);
    cryptoUnitGive (unitAsBase);
    cryptoCurrencyGive (currency);

    return;
}

static BRFileService
crytpWalletManagerCreateFileServiceETH (BRCryptoWalletManager manager,
                                        const char *basePath,
                                        const char *currency,
                                        const char *network,
                                        BRFileServiceContext context,
                                        BRFileServiceErrorHandler handler) {
    return fileServiceCreateFromTypeSpecfications (basePath, currency, network,
                                                   context, handler,
                                                   fileServiceSpecificationsCountETH,
                                                   fileServiceSpecificationsETH);
}

static const BREventType **
cryptoWalletManagerGetEventTypesETH (BRCryptoWalletManager manager,
                                     size_t *eventTypesCount) {
    assert (NULL != eventTypesCount);
    *eventTypesCount = eventTypesCountETH;
    return eventTypesETH;
}

static BRCryptoBoolean
cryptoWalletManagerSignTransaction (BRCryptoWalletManager managerBase,
                                    BRCryptoWallet walletBase,
                                    BRCryptoTransfer transferBase,
                                    BRKey *key) {
    BRCryptoWalletManagerETH manager  = cryptoWalletManagerCoerce (managerBase);
    BRCryptoTransferETH      transfer = cryptoTransferCoerce      (transferBase);


    BREthereumNetwork     ethNetwork     = manager->network;
    BREthereumAccount     ethAccount     = manager->account;
    BREthereumAddress     ethAddress     = ethAccountGetPrimaryAddress (ethAccount);
    BREthereumTransaction ethTransaction = transfer->originatingTransaction;

    assert (NULL != ethTransaction);

    if (TRANSACTION_NONCE_IS_NOT_ASSIGNED == transactionGetNonce (ethTransaction))
        transactionSetNonce (ethTransaction,
                             ethAccountGetThenIncrementAddressNonce (ethAccount, ethAddress));

    // RLP Encode the UNSIGNED transaction
    BRRlpCoder coder = rlpCoderCreate();
    BRRlpItem item = transactionRlpEncode (ethTransaction,
                                           ethNetwork,
                                           RLP_TYPE_TRANSACTION_UNSIGNED,
                                           coder);
    BRRlpData data = rlpItemGetDataSharedDontRelease (coder, item);

    // Sign the RLP Encoded UNSIGNED transaction bytes.
    BREthereumSignature signature = ethAccountSignBytesWithPrivateKey (ethAccount,
                                                                       ethAddress,
                                                                       SIGNATURE_TYPE_RECOVERABLE_VRS_EIP,
                                                                       data.bytes,
                                                                       data.bytesCount,
                                                                       *key);
    rlpItemRelease(coder, item);
    BRKeyClean(key);

    // Attach the signature
    transactionSign (ethTransaction, signature);

    // RLP Encode the SIGNED transaction and then assign the hash.
    item = transactionRlpEncode (ethTransaction,
                                 ethNetwork,
                                 RLP_TYPE_TRANSACTION_SIGNED,
                                 coder);
    transactionSetHash (ethTransaction,
                        ethHashCreateFromData (rlpItemGetDataSharedDontRelease (coder, item)));

    rlpItemRelease(coder, item);
    rlpCoderRelease(coder);

    return CRYPTO_TRUE;
}

static BRCryptoBoolean
cryptoWalletManagerSignTransactionWithSeedETH (BRCryptoWalletManager managerBase,
                                                      BRCryptoWallet walletBase,
                                                      BRCryptoTransfer transferBase,
                                                      UInt512 seed) {
    BRCryptoWalletManagerETH manager  = cryptoWalletManagerCoerce (managerBase);

    BREthereumAccount     ethAccount     = manager->account;
    BREthereumAddress     ethAddress     = ethAccountGetPrimaryAddress (ethAccount);

    BRKey key = derivePrivateKeyFromSeed (seed, ethAccountGetAddressIndex (ethAccount, ethAddress));
    
    return cryptoWalletManagerSignTransaction (managerBase,
                                               walletBase,
                                               transferBase,
                                               &key);
}

static BRCryptoBoolean
cryptoWalletManagerSignTransactionWithKeyETH (BRCryptoWalletManager managerBase,
                                              BRCryptoWallet walletBase,
                                              BRCryptoTransfer transferBase,
                                              BRCryptoKey key) {
    return cryptoWalletManagerSignTransaction (managerBase,
                                               walletBase,
                                               transferBase,
                                               cryptoKeyGetCore (key));
}

static BRCryptoAmount
cryptoWalletManagerEstimateLimitETH (BRCryptoWalletManager cwm,
                                     BRCryptoWallet  wallet,
                                     BRCryptoBoolean asMaximum,
                                     BRCryptoAddress target,
                                     BRCryptoNetworkFee networkFee,
                                     BRCryptoBoolean *needEstimate,
                                     BRCryptoBoolean *isZeroIfInsuffientFunds,
                                     BRCryptoUnit unit) {
    // We always need an estimate as we do not know the fees.
    *needEstimate = CRYPTO_TRUE;

    return (CRYPTO_TRUE == asMaximum
            ? cryptoWalletGetBalance (wallet)        // Maximum is balance - fees 'needEstimate'
            : cryptoAmountCreateInteger (0, unit));  // No minimum
}

static BRCryptoFeeBasis
cryptoWalletManagerEstimateFeeBasisETH (BRCryptoWalletManager managerBase,
                                               BRCryptoWallet  walletBase,
                                               BRCryptoCookie cookie,
                                               BRCryptoAddress target,
                                               BRCryptoAmount amount,
                                               BRCryptoNetworkFee networkFee) {
    BRCryptoWalletETH wallet = cryptoWalletCoerce (walletBase);

    BREthereumFeeBasis ethFeeBasis = {
        FEE_BASIS_GAS,
        { .gas = { wallet->ethGasLimit, cryptoNetworkFeeAsETH (networkFee) }}
    };

    BRCryptoCurrency currency = cryptoAmountGetCurrency (amount);
    BRCryptoTransfer transfer = cryptoWalletCreateTransferETH (walletBase,
                                                               target,
                                                               amount,
                                                               cryptoFeeBasisCreateAsETH (walletBase->unitForFee, ethFeeBasis),
                                                               0, NULL,
                                                               currency,
                                                               walletBase->unit,
                                                               walletBase->unitForFee);

    cryptoCurrencyGive(currency);

    cryptoClientQRYEstimateTransferFee (managerBase->qryManager,
                                        cookie,
                                        transfer,
                                        networkFee);

    cryptoTransferGive (transfer);

    // Require QRY with cookie - made above
    return NULL;
}

static void
cryptoWalletManagerCreateTokenForCurrency (BRCryptoWalletManagerETH manager,
                                           BRCryptoCurrency currency,
                                           BRCryptoUnit     unitDefault) {
    const char *address = cryptoCurrencyGetIssuer(currency);

    if (NULL == address || 0 == strlen(address)) return;
    if (ETHEREUM_BOOLEAN_FALSE == ethAddressValidateString(address)) return;

    BREthereumAddress addr = ethAddressCreate(address);

    const char *code = cryptoCurrencyGetCode (currency);
    const char *name = cryptoCurrencyGetName (currency);
    const char *desc = cryptoCurrencyGetUids (currency);
    unsigned int decimals = cryptoUnitGetBaseDecimalOffset(unitDefault);

    BREthereumGas      defaultGasLimit = ethGasCreate(TOKEN_BRD_DEFAULT_GAS_LIMIT);
    BREthereumGasPrice defaultGasPrice = ethGasPriceCreate(ethEtherCreate(uint256Create(TOKEN_BRD_DEFAULT_GAS_PRICE_IN_WEI_UINT64)));

    // Check for an existing token
    BREthereumToken token = BRSetGet (manager->tokens, &addr);

    if (NULL == token) {
        token = ethTokenCreate (address,
                             code,
                             name,
                             desc,
                             decimals,
                             defaultGasLimit,
                             defaultGasPrice);
        BRSetAdd (manager->tokens, token);
    }
    else {
        ethTokenUpdate (token,
                     code,
                     name,
                     desc,
                     decimals,
                     defaultGasLimit,
                     defaultGasPrice);
    }

    fileServiceSave (manager->base.fileService, fileServiceTypeTokensETH, token);
}

static BRCryptoWallet
cryptoWalletManagerRegisterWalletETH (BRCryptoWalletManager managerBase,
                                      BRCryptoCurrency currency) {
    BRCryptoWalletManagerETH manager  = cryptoWalletManagerCoerce (managerBase);

    const char *issuer = cryptoCurrencyGetIssuer (currency);
    BREthereumAddress ethAddress = ethAddressCreate (issuer);
    BREthereumToken ethToken = BRSetGet (manager->tokens, &ethAddress);
    assert (NULL != ethToken);

    BRCryptoUnit     unit       = cryptoNetworkGetUnitAsBase (managerBase->network, currency);
    BRCryptoUnit     unitForFee = cryptoNetworkGetUnitAsBase (managerBase->network, currency);

    BRCryptoWallet wallet = cryptoWalletCreateAsETH (unit,
                                                     unitForFee,
                                                     ethToken,
                                                     manager->account);

    cryptoWalletManagerAddWallet (managerBase, wallet);
//    cryptoWalletMangerSignalWalletCreated (managerBase, wallet);

    cryptoUnitGive (unitForFee);
    cryptoUnitGive (unit);

    return wallet;
}

static BRCryptoWalletETH
cryptoWalletManagerLookupWalletForToken (BRCryptoWalletManagerETH manager,
                                         BREthereumToken token) {
    if (NULL == token) return (BRCryptoWalletETH) cryptoWalletTake (manager->base.wallet);

    for (size_t index = 0; index < array_count(manager->base.wallets); index++) {
        BRCryptoWalletETH wallet = cryptoWalletCoerce (manager->base.wallets[index]);
        if (token == wallet->ethToken)
            return wallet;
    }
    return NULL;
}

static const char *
cwmLookupAttributeValueForKey (const char *key, size_t count, const char **keys, const char **vals) {
    for (size_t index = 0; index < count; index++)
        if (0 == strcasecmp (key, keys[index]))
            return vals[index];
    return NULL;
}

static uint64_t
cwmParseUInt64 (const char *string, bool *error) {
    if (!string) { *error = true; return 0; }
    return strtoull(string, NULL, 0);
}

static UInt256
cwmParseUInt256 (const char *string, bool *error) {
    if (!string) { *error = true; return UINT256_ZERO; }

    BRCoreParseStatus status;
    UInt256 result = uint256CreateParse (string, 0, &status);
    if (CORE_PARSE_OK != status) { *error = true; return UINT256_ZERO; }

    return result;
}

static void
cwmExtractAttributes (OwnershipKept BRCryptoClientTransferBundle bundle,
                      UInt256 *amount,
                      uint64_t *gasLimit,
                      uint64_t *gasUsed,
                      UInt256  *gasPrice,
                      uint64_t *nonce,
                      bool     *error) {
    *amount = cwmParseUInt256 (bundle->amount, error);

    size_t attributesCount = bundle->attributesCount;
    const char **attributeKeys   = (const char **) bundle->attributeKeys;
    const char **attributeVals   = (const char **) bundle->attributeVals;

    *gasLimit = cwmParseUInt64 (cwmLookupAttributeValueForKey ("gasLimit", attributesCount, attributeKeys, attributeVals), error);
    *gasUsed  = cwmParseUInt64 (cwmLookupAttributeValueForKey ("gasUsed",  attributesCount, attributeKeys, attributeVals), error); // strtoull(strGasUsed, NULL, 0);
    *gasPrice = cwmParseUInt256(cwmLookupAttributeValueForKey ("gasPrice", attributesCount, attributeKeys, attributeVals), error);
    *nonce    = cwmParseUInt64 (cwmLookupAttributeValueForKey ("nonce",    attributesCount, attributeKeys, attributeVals), error);

    *error |= (CRYPTO_TRANSFER_STATE_ERRORED == bundle->status);
}


static void
cryptoWalletManagerRecoverTransaction (BRCryptoWalletManager managerBase,
                                       OwnershipKept BRCryptoClientTransferBundle bundle) {

    BRCryptoWalletManagerETH manager = cryptoWalletManagerCoerce (managerBase);

    UInt256 amount;
    uint64_t gasLimit;
    uint64_t gasUsed;
    UInt256  gasPrice;
    uint64_t nonce;
    bool     error;

    cwmExtractAttributes (bundle, &amount, &gasLimit, &gasUsed, &gasPrice, &nonce, &error);
#if 0
    ewmAnnounceTransaction (cwm->u.eth,
                            callbackState->rid,
                            bundle->hash,
                            bundle->from,
                            bundle->to,
                            contract,
                            value,
                            gasLimit,
                            gasPrice,
                            "",
                            nonce,
                            gasUsed,
                            bundle->blockNumber,
                            bundle->blockHash,
                            bundle->blockConfirmations,
                            bundle->blockTransactionIndex,
                            bundle->blockTimestamp,
                            error);

#endif

    //
    // This 'announce' call is coming from the guaranteed BRD endpoint; thus we don't need to
    // worry about the validity of the transaction - it is surely confirmed.  Is that true
    // if newly submitted?

    // TODO: Confirm we are not repeatedly creating transactions
    BREthereumTransaction transaction = transactionCreate (ethAddressCreate (bundle->from),
                                                           ethAddressCreate (bundle->to),
                                                           ethEtherCreate(amount),
                                                           ethGasPriceCreate(ethEtherCreate(gasPrice)),
                                                           ethGasCreate(gasLimit),
                                                           NULL, // data
                                                           nonce);

    // We set the transaction's hash based on the value providedin the bundle.  However,
    // and importantly, if we attempted to compute the hash - as we normally do for a
    // signed transaction - the computed hash would be utterly wrong.  The transaction
    // just created does not have: network nor signature.
    //
    // TODO: Confirm that BRPersistData does not overwrite the transaction's hash
    transactionSetHash (transaction, ethHashCreate (bundle->hash));

    BREthereumTransactionStatus status = transactionStatusCreateIncluded (ethHashCreate (bundle->blockHash),
                                                                          bundle->blockNumber,
                                                                          bundle->blockTransactionIndex,
                                                                          bundle->blockTimestamp,
                                                                          ethGasCreate(gasUsed));
    transactionSetStatus (transaction, status);

    // If we had a `bcs` we might think about `bcsSignalTransaction(ewm->bcs, transaction);`
    ewmHandleTransaction (manager, BCS_CALLBACK_TRANSACTION_UPDATED, transaction);
}

static void
cryptoWalletManagerRecoverLog (BRCryptoWalletManager managerBase,
                               const char *contract,
                               OwnershipKept BRCryptoClientTransferBundle bundle) {
    BRCryptoWalletManagerETH manager = cryptoWalletManagerCoerce (managerBase);

    UInt256 amount;
    uint64_t gasLimit;
    uint64_t gasUsed;
    UInt256  gasPrice;
    uint64_t nonce;
    bool     error;

    size_t logIndex = 0;

    cwmExtractAttributes(bundle, &amount, &gasLimit, &gasUsed, &gasPrice, &nonce, &error);
#if 0
    size_t topicsCount = 3;
    char *topics[3] = {
        (char *) ethEventGetSelector(ethEventERC20Transfer),
        ethEventERC20TransferEncodeAddress (ethEventERC20Transfer, bundle->from),
        ethEventERC20TransferEncodeAddress (ethEventERC20Transfer, bundle->to)
    };

    size_t logIndex = 0;

    ewmAnnounceLog (cwm->u.eth,
                    callbackState->rid,
                    bundle->hash,
                    contract,
                    topicsCount,
                    (const char **) &topics[0],
                    amount,
                    gasPrice,
                    gasUsed,
                    logIndex,
                    bundle->blockNumber,
                    bundle->blockTransactionIndex,
                    bundle->blockTimestamp);

    free (topics[1]);
    free (topics[2]);
#endif

    // This 'announce' call is coming from the guaranteed BRD endpoint; thus we don't need to
    // worry about the validity of the transaction - it is surely confirmed.

    unsigned int topicsCount = 3;
    BREthereumLogTopic topics [topicsCount];

    {
        char *topicsStr[3] = {
            (char *) ethEventGetSelector(ethEventERC20Transfer),
            ethEventERC20TransferEncodeAddress (ethEventERC20Transfer, bundle->from),
            ethEventERC20TransferEncodeAddress (ethEventERC20Transfer, bundle->to)
        };

        for (size_t index = 0; index < topicsCount; index++)
            topics[index] = logTopicCreateFromString(topicsStr[index]);
    }

    // In general, log->data is arbitrary data.  In the case of an ERC20 token, log->data
    // is a numeric value - for the transfer amount.  When parsing in logRlpDecode(),
    // log->data is assigned with rlpDecodeBytes(coder, items[2]); we'll need the same
    // thing, somehow

    BRRlpItem  item  = rlpEncodeUInt256 (manager->coder, amount, 1);

    BREthereumLog log = logCreate (ethAddressCreate (contract),
                                   topicsCount,
                                   topics,
                                   rlpItemGetDataSharedDontRelease (manager->coder, item));
    rlpItemRelease (manager->coder, item);

    // Given {hash,logIndex}, initialize the log's identifier
    assert (logIndex <= (uint64_t) SIZE_MAX);
    logInitializeIdentifier(log, ethHashCreate (bundle->hash), logIndex);

    BREthereumTransactionStatus status =
    transactionStatusCreateIncluded (ethHashCreate(bundle->blockHash),
                                     bundle->blockNumber,
                                     bundle->blockTransactionIndex,
                                     bundle->blockTimestamp,
                                     ethGasCreate(gasUsed));
    logSetStatus (log, status);

    // If we had a `bcs` we might think about `bcsSignalLog(ewm->bcs, log);`
    //            ewmSignalLog(ewm, BCS_CALLBACK_LOG_UPDATED, log);
    ewmHandleLog (manager, BCS_CALLBACK_LOG_UPDATED, log);

    // The `bundle` has `gasPrice` and `gasUsed` values.  The above `ewmSignalLog()` is
    // going to create a `transfer` and that transfer needs a correct `feeBasis`.  We will
    // not use this bundle's feeBasis and put in place something that works for P2P modes
    // as well.  So instead will use the feeBasis derived from this log transfer's
    // transaction.  See ewmHandleLog, ewmHandleTransaction and their calls to
    // ewmHandleLogFeeBasis.

    // Do we need a transaction here?  No, if another originated this Log, then we can't ever
    // care and if we originated this Log, then we'll get the transaction (as part of the BRD
    // 'getTransactions' endpoint).
    //
    // Of course, see the comment in bcsHandleLog asking how to tell the client about a Log...

}


static void
cryptoWalletManagerRecoverTransfersFromTransactionBundleETH (BRCryptoWalletManager managerBase,
                                                             OwnershipKept BRCryptoClientTransactionBundle bundle) {
    BRCryptoWalletManagerETH manager = cryptoWalletManagerCoerce (managerBase);
    (void) manager;
assert (0);
}

static void
cryptoWalletManagerRecoverTransferFromTransferBundleETH (BRCryptoWalletManager managerBase,
                                                         OwnershipKept BRCryptoClientTransferBundle bundle) {
    BRCryptoWalletManagerETH    manager = cryptoWalletManagerCoerce (managerBase);
    (void) manager;
//    BRCryptoClientP2PManagerETH p2p     = cryptoClientP2PManagerCoerce (manager->base.p2pManager);

    BRCryptoNetwork  network        = cryptoWalletManagerGetNetwork (managerBase);
    BRCryptoCurrency walletCurrency = cryptoNetworkGetCurrencyForUids (network, bundle->currency);
    if (NULL == walletCurrency) return;
    const char *contract = cryptoCurrencyGetIssuer(walletCurrency);

    switch (managerBase->syncMode) {
        case CRYPTO_SYNC_MODE_API_ONLY:
        case CRYPTO_SYNC_MODE_API_WITH_P2P_SEND:
            if (NULL != contract)
                cryptoWalletManagerRecoverLog (managerBase, contract, bundle);
            else
                cryptoWalletManagerRecoverTransaction (managerBase, bundle);
            break;

        case CRYPTO_SYNC_MODE_P2P_WITH_API_SYNC:
        case CRYPTO_SYNC_MODE_P2P_ONLY:
//            if (NULL != cryptoCurrencyGetIssuer(walletCurrency))
//                bcsSendLogRequest (p2p->bcs,
//                                   bundle->hash,
//                                   bundle->blockNumber,
//                                   bundle->blockTransactionIndex);
//            else
//                bcsSendTransactionRequest (p2p->bcs,
//                                           bundle->hash,
//                                           bundle->blockNumber,
//                                           bundle->blockTransactionIndex);
            break;
    }
}

// MARK: - Client P2P

typedef struct BRCryptoClientP2PManagerRecordETH {
    struct BRCryptoClientP2PManagerRecord base;
    BRCryptoWalletManagerETH manager;
    BREthereumBCS bcs;
} *BRCryptoClientP2PManagerETH;

static BRCryptoClientP2PManagerETH
cryptoClientP2PManagerCoerce (BRCryptoClientP2PManager managerBase) {
    assert (CRYPTO_NETWORK_TYPE_ETH == managerBase->type);
    return (BRCryptoClientP2PManagerETH) managerBase;
}

static void
cryptoClientP2PManagerReleaseETH (BRCryptoClientP2PManager baseManager) {
    BRCryptoClientP2PManagerETH manager = cryptoClientP2PManagerCoerce (baseManager);
    bcsDestroy (manager->bcs);
}

static void
cryptoClientP2PManagerConnectETH (BRCryptoClientP2PManager baseManager,
                                         BRCryptoPeer peer) {
    BRCryptoClientP2PManagerETH manager = cryptoClientP2PManagerCoerce (baseManager);
    bcsStart (manager->bcs);
}

static void
cryptoClientP2PManagerDisconnectETH (BRCryptoClientP2PManager baseManager) {
    BRCryptoClientP2PManagerETH manager = cryptoClientP2PManagerCoerce (baseManager);
    bcsStop (manager->bcs);
}

// MARK: - Sync

static void
cryptoClientP2PManagerSyncETH (BRCryptoClientP2PManager baseManager,
                                      BRCryptoSyncDepth depth,
                                      BRCryptoBlockNumber height) {
    BRCryptoClientP2PManagerETH manager = cryptoClientP2PManagerCoerce (baseManager);
    bcsSync (manager->bcs, height);
}

typedef struct {
    BRCryptoWalletManager manager;
    BRCryptoTransfer transfer;
} BRCryptoClientP2PManagerPublishInfo;

static void
cryptoClientP2PManagerSendETH (BRCryptoClientP2PManager baseManager, BRCryptoTransfer baseTransfer) {
    BRCryptoClientP2PManagerETH manager = cryptoClientP2PManagerCoerce (baseManager);
    BRCryptoTransferETH transfer = cryptoTransferCoerce (baseTransfer);
    
    bcsSendTransaction (manager->bcs, transfer->originatingTransaction);
}

static BRCryptoClientP2PHandlers p2pHandlersETH = {
    cryptoClientP2PManagerReleaseETH,
    cryptoClientP2PManagerConnectETH,
    cryptoClientP2PManagerDisconnectETH,
    cryptoClientP2PManagerSyncETH,
    cryptoClientP2PManagerSendETH
};

static BRCryptoClientP2PManager
crytpWalletManagerCreateP2PManagerETH (BRCryptoWalletManager cwm) {
    BRCryptoClientP2PManager p2pBase = cryptoClientP2PManagerCreate (sizeof (struct BRCryptoClientP2PManagerRecordETH),
                                                                         cwm->type,
                                                                         &p2pHandlersETH);
    BRCryptoClientP2PManagerETH p2p = cryptoClientP2PManagerCoerce (p2pBase);
    p2p->manager = cryptoWalletManagerCoerce(cwm);

    BREthereumBCSListener listener = {
        p2p,
        ewmHandleBlockChain,
        ewmHandleAccountState,
        ewmHandleTransaction,
        ewmHandleLog,
        ewmHandleSaveBlocks,
        ewmHandleSaveNodes,
        ewmHandleSync,
        ewmHandleGetBlocks
    };

    BREthereumNetwork network = p2p->manager->network;
    BREthereumAddress address = ethAccountGetPrimaryAddress (p2p->manager->account);

    BRSetOf(BREthereumTransaction) transactions = initialTransactionsLoadETH (cwm);
    BRSetOf(BREthereumLog)         logs         = initialLogsLoadETH         (cwm);
    BRSetOf(BREthereumNodeConfig)  nodes        = initialNodesLoadETH        (cwm);
    BRSetOf(BREthereumBlock)       blocks       = initialBlocksLoadETH       (cwm);

    // If we have no blocks; then add a checkpoint
    if (0 == BRSetCount(blocks)) {
        const BREthereumBlockCheckpoint *checkpoint = blockCheckpointLookupByTimestamp (network, cryptoAccountGetTimestamp (cwm->account));
        BREthereumBlock block = blockCreate (blockCheckpointCreatePartialBlockHeader (checkpoint));
        blockSetTotalDifficulty (block, checkpoint->u.td);
        BRSetAdd (blocks, block);
    }

    p2p->bcs = bcsCreate (network,
                          address,
                          listener,
                          p2p->manager->base.syncMode,
                          nodes,
                          blocks,
                          transactions,
                          logs);


//    if (NULL != blocks) array_free (blocks);
//    if (NULL != peers ) array_free (peers);

    return p2pBase;
}

const BREventType *eventTypesETH[] = {};
const unsigned int eventTypesCountETH = (sizeof (eventTypesETH) / sizeof (BREventType*));

BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersETH = {
    cryptoWalletManagerCreateETH,
    cryptoWalletManagerReleaseETH,
    cryptoWalletManagerInitializeETH,
    crytpWalletManagerCreateFileServiceETH,
    cryptoWalletManagerGetEventTypesETH,
    cryptoWalletManagerSignTransactionWithSeedETH,
    cryptoWalletManagerSignTransactionWithKeyETH,
    cryptoWalletManagerEstimateLimitETH,
    cryptoWalletManagerEstimateFeeBasisETH,
    crytpWalletManagerCreateP2PManagerETH,
    cryptoWalletManagerRegisterWalletETH,
    cryptoWalletManagerRecoverTransfersFromTransactionBundleETH,
    cryptoWalletManagerRecoverTransferFromTransferBundleETH
};


#if 0
static void
cryptoWalletManagerEstimateFeeBasisHandlerETH (BRCryptoWalletManager cwm,
                                               BRCryptoWallet  wallet,
                                               BRCryptoCookie cookie,
                                               BRCryptoAddress target,
                                               BRCryptoAmount amount,
                                               BRCryptoNetworkFee networkFee) {
    BRCryptoTransfer transfer = ...;

    cryptoClientQRYEstimateTransferFee (cwm->qryManager, cookie, transfer, networkFee);
    return;
}
#endif


/**
 * Handle the BCS BlockChain callback.  This should result in a 'client block event' callback.
 * However, that callback accepts a `bid` and we don't have one (in the same sense as a tid or
 * wid); perhaps the blockNumber is the `bid`?
 *
 * Additionally, this handler has no indication of the type of BCS data.  E.g is this block chained
 * or orphaned.
 *
 * @param ewm
 * @param headBlockHash
 * @param headBlockNumber
 * @param headBlockTimestamp
 */
static void
ewmHandleBlockChain (BREthereumBCSCallbackContext context,
                     BREthereumHash headBlockHash,
                     uint64_t headBlockNumber,
                     uint64_t headBlockTimestamp) {
    BRCryptoWalletManagerETH manager = context;
    BRCryptoClientP2PManagerETH p2p = cryptoClientP2PManagerCoerce (manager->base.p2pManager);

    // Don't report during BCS sync.
    if (CRYPTO_SYNC_MODE_API_ONLY == manager->base.syncMode ||
        ETHEREUM_BOOLEAN_IS_FALSE(bcsSyncInProgress (p2p->bcs)))
        eth_log ("EWM", "BlockChain: %" PRIu64, headBlockNumber);

    // At least this - allows for: ewmGetBlockHeight
    cryptoNetworkSetHeight (p2p->manager->base.network, headBlockNumber);

    // TODO: Signal
    cryptoWalletManagerGenerateManagerEvent (&p2p->manager->base,
                                             (BRCryptoWalletManagerEvent) {
        CRYPTO_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED,
        { .blockHeight = { headBlockNumber }}
    });

#if defined (NEVER_DEFINED)
    // TODO: Need a 'block id' - or axe the need of 'block id'?
    ewmSignalBlockEvent (ewm,
                         (BREthereumBlockId) 0,
                         BLOCK_EVENT_CHAINED,
                         SUCCESS,
                         NULL);
#endif
}
/**
 * Handle the BCS AccountState callback.
 *
 * @param ewm
 * @param accountState
 */
static void
ewmHandleAccountState (BREthereumBCSCallbackContext context,
                       BREthereumAccountState accountState) {
    BRCryptoWalletManagerETH manager = context;
    (void) manager;
    eth_log("EWM", "AccountState: Nonce: %" PRIu64, accountState.nonce);
#if 0
    ewmHandleAnnounceNonce (ewm, ethAccountGetPrimaryAddress(ewm->account), accountState.nonce, 0);
    ewmSignalBalance(ewm, ethAmountCreateEther(accountState.balance));
#endif
}

#if 0
static void
ewmHandleLogFeeBasis (BREthereumEWM ewm,
                      BREthereumHash hash,
                      BREthereumTransfer transferTransaction,
                      BREthereumTransfer transferLog) {

    // Find the ETH transfer, if needed
    if (NULL == transferTransaction)
        transferTransaction = walletGetTransferByIdentifier (ewmGetWallet(ewm), hash);

    // If none exists, then the transaction hasn't been 'synced' yet.
    if (NULL == transferTransaction) return;

    // If we have a TOK transfer, set the fee basis.
    if (NULL != transferLog)
        transferSetFeeBasis(transferLog, transferGetFeeBasis(transferTransaction));

    // but if we don't have a TOK transfer, find every transfer referencing `hash` and set the basis.
    else
        for (size_t wid = 0; wid < array_count(ewm->wallets); wid++) {
            BREthereumWallet wallet = ewm->wallets[wid];

            // We are only looking for TOK transfers (non-ETH).
            if (wallet == ewm->walletHoldingEther) continue;

            size_t tidLimit = walletGetTransferCount (wallet);
            for (size_t tid = 0; tid < tidLimit; tid++) {
                transferLog = walletGetTransferByIndex (wallet, tid);

                // Look for a log that has a matching transaction hash
                BREthereumLog log = transferGetBasisLog(transferLog);
                if (NULL != log) {
                    BREthereumHash transactionHash;
                    if (ETHEREUM_BOOLEAN_TRUE == logExtractIdentifier (log, &transactionHash, NULL) &&
                        ETHEREUM_BOOLEAN_TRUE == ethHashEqual (transactionHash, hash))
                        ewmHandleLogFeeBasis (ewm, hash, transferTransaction, transferLog);
                }
            }
        }
}
#endif

static void
ewmHandleTransaction (BREthereumBCSCallbackContext context,
                      BREthereumBCSCallbackTransactionType type,
                      OwnershipGiven BREthereumTransaction transaction) {
    BRCryptoWalletManagerETH manager = context;
    (void) manager;

    BRCryptoHash hash = cryptoHashCreateAsETH (transactionGetHash(transaction));
    BRCryptoWallet wallet = manager->base.wallet;

    BRCryptoTransfer transfer = cryptoWalletGetTransferByHash (wallet, hash);

    bool needStatusEvent = false;

    if (NULL == transfer) {
        transfer = cryptoTransferCreateWithTransactionAsETH (wallet->unit,
                                                             wallet->unitForFee,
                                                             manager->account,
                                                             transaction);

        cryptoWalletAddTransfer (wallet, transfer);

        // TODO: Events CREATED

        cryptoWalletManagerGenerateTransferEvent (&manager->base, wallet, transfer, (BRCryptoTransferEvent) {
            CRYPTO_TRANSFER_EVENT_CREATED
        });

        cryptoWalletManagerGenerateWalletEvent (&manager->base, wallet, (BRCryptoWalletEvent) {
            CRYPTO_WALLET_EVENT_TRANSFER_ADDED,
            { .transfer = { cryptoTransferTake (transfer) }}
        });

         // If this transfer is referenced by a log, fill out the log's fee basis.
//        ewmHandleLogFeeBasis (ewm, hash, transfer, NULL);

    }

    else {

    }

    BRCryptoTransferState state = cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_CREATED);
    BREthereumTransactionStatus ethState = transactionGetStatus (transaction);
    switch (ethState.type) {
        case TRANSACTION_STATUS_UNKNOWN:
            state = cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_CREATED);
            break;
        case TRANSACTION_STATUS_QUEUED:
        case TRANSACTION_STATUS_PENDING:
            state = cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_SUBMITTED);
            break;

        case TRANSACTION_STATUS_INCLUDED:
            state = cryptoTransferStateIncludedInit (ethState.u.included.blockNumber,
                                                     ethState.u.included.transactionIndex,
                                                     ethState.u.included.blockTimestamp,
                                                     cryptoFeeBasisCreateAsETH (cryptoUnitTake(wallet->unitForFee),
                                                                                ethFeeBasisCreate (ethState.u.included.gasUsed,
                                                                                                   transactionGetGasPrice(transaction))),
                                                     CRYPTO_TRUE,
                                                     NULL);
            break;

        case TRANSACTION_STATUS_ERRORED:
            state = cryptoTransferStateErroredInit((BRCryptoTransferSubmitError) {
                CRYPTO_TRANSFER_SUBMIT_ERROR_UNKNOWN
            });
            break;
    }
    cryptoTransferSetState (transfer, state);

    if (needStatusEvent) {
        BREthereumHashString hashString;
        ethHashFillString (transactionGetHash(transaction), hashString);

        eth_log ("EWM", "Transaction: \"%s\", Change: %s, Status: %d", hashString,
                 BCS_CALLBACK_TRANSACTION_TYPE_NAME(type),
                 transactionGetStatus(transaction).type);

        //            ewmReportTransferStatusAsEvent(ewm, wallet, transfer);
    }

//  ewmHandleTransactionOriginatingLog (ewm, type, transaction);

#if 0
    BREthereumHash hash = transactionGetHash(transaction);

    // Find the wallet
    BREthereumWallet wallet = ewmGetWallet(ewm);
    assert (NULL != wallet);

    ///
    ///  What hash to use:
    ///     originating -> expecting a result
    ///     identifier  -> seen already.
    ///
    ///     originating should be good in one wallet?  [no, multiple logs?]
    ///        wallet will have transfers w/o a basis.
    ///        does a transfer with an ERC20 transfer fit in one wallet?
    ///        does a transfer with some smart contract fit in one wallet (no?)
    ///

    // Find a preexisting transfer
    BREthereumTransfer transfer = walletGetTransferByIdentifier (wallet, hash);
    if (NULL == transfer)
        transfer = walletGetTransferByOriginatingHash (wallet, hash);

    int needStatusEvent = 0;

    // If we've no transfer, then create one and save `transaction` as the basis
    if (NULL == transfer) {
        transfer = transferCreateWithTransaction (transaction); // transaction ownership given

        walletHandleTransfer (wallet, transfer);

        // We've added a transfer and arguably we should update the wallet's balance.  But don't.
        // Ethereum is 'account based'; we'll only update the balance based on a account state
        // change (based on a P2P or API callback).
        //
        // walletUpdateBalance (wallet);

        ewmSignalTransferEvent (ewm, wallet, transfer, (BREthereumTransferEvent) {
            TRANSFER_EVENT_CREATED,
            SUCCESS
        });

         // If this transfer is referenced by a log, fill out the log's fee basis.
        ewmHandleLogFeeBasis (ewm, hash, transfer, NULL);

        needStatusEvent = 1;
    }
    else {
        needStatusEvent = ewmReportTransferStatusAsEventIsNeeded (ewm, wallet, transfer,
                                                                  transactionGetStatus(transaction));


        // If this transaction is the transfer's originatingTransaction, then update the
        // originatingTransaction's status.
        BREthereumTransaction original = transferGetOriginatingTransaction (transfer);
        if (NULL != original && ETHEREUM_BOOLEAN_IS_TRUE(ethHashEqual (transactionGetHash(original),
                                                                    transactionGetHash(transaction))))
            transactionSetStatus (original, transactionGetStatus(transaction));

        transferSetBasisForTransaction (transfer, transaction); // transaction ownership given
    }

    if (needStatusEvent) {
        BREthereumHashString hashString;
        ethHashFillString(hash, hashString);
        eth_log ("EWM", "Transaction: \"%s\", Change: %s, Status: %d", hashString,
                 BCS_CALLBACK_TRANSACTION_TYPE_NAME(type),
                 transactionGetStatus(transaction).type);

        ewmReportTransferStatusAsEvent(ewm, wallet, transfer);
    }

    ewmHandleTransactionOriginatingLog (ewm, type, transaction);
#endif
}

static int
ewmReportTransferStatusAsEventIsNeeded (BRCryptoWalletManager manager,
                                        BRCryptoWallet wallet,
                                        BRCryptoTransfer transfer,
                                        BREthereumTransactionStatus status) {
    return 0;
#ifdef REFACTOR
    return (// If the status differs from the transfer's basis status...
            ETHEREUM_BOOLEAN_IS_FALSE (transactionStatusEqual (status, transferGetStatusForBasis(transfer))) ||
            // Otherwise, if the transfer's status differs.
            ETHEREUM_BOOLEAN_IS_FALSE (transferHasStatus (transfer, transferStatusCreate(status))));
#endif
}

static void
ewmReportTransferStatusAsEvent (BRCryptoWalletManager manager,
                                BRCryptoWallet wallet,
                                BRCryptoTransfer transfer,
                                BRCryptoTransferState oldState) {
    BRCryptoTransferState newState = cryptoTransferGetState (transfer);

    if (!cryptoTransferStateIsEqual (&oldState, &newState)) {
        cryptoWalletManagerGenerateTransferEvent (manager, wallet, transfer, (BRCryptoTransferEvent) {
            CRYPTO_TRANSFER_EVENT_CHANGED,
            { .state = { oldState, newState }}
        });
    }
#if 0
    if (ETHEREUM_BOOLEAN_IS_TRUE (transferHasStatus (transfer, TRANSFER_STATUS_SUBMITTED)))
        ewmSignalTransferEvent(ewm, wallet, transfer, (BREthereumTransferEvent) {
            TRANSFER_EVENT_SUBMITTED,
            SUCCESS
        });

    else if (ETHEREUM_BOOLEAN_IS_TRUE (transferHasStatus (transfer, TRANSFER_STATUS_INCLUDED)))
        ewmSignalTransferEvent(ewm, wallet, transfer, (BREthereumTransferEvent) {
            TRANSFER_EVENT_INCLUDED,
            SUCCESS
        });

    else if (ETHEREUM_BOOLEAN_IS_TRUE (transferHasStatus (transfer, TRANSFER_STATUS_ERRORED))) {
        char *reason = NULL;
        transferExtractStatusError (transfer, &reason);
        ewmSignalTransferEvent (ewm, wallet, transfer,
                                ethTransferEventCreateError (TRANSFER_EVENT_ERRORED,
                                                          ERROR_TRANSACTION_SUBMISSION,
                                                          reason));

        if (NULL != reason) free (reason);
    }
#endif
}

static void
ewmHandleLog (BREthereumBCSCallbackContext context,
              BREthereumBCSCallbackLogType type,
              OwnershipGiven BREthereumLog log) {
    BRCryptoWalletManagerETH manager = context;

    BREthereumHash logHash = logGetHash(log);

    BREthereumHash transactionHash;
    size_t logIndex;

    // Assert that we always have an identifier for `log`.
    BREthereumBoolean extractedIdentifier = logExtractIdentifier (log, &transactionHash, &logIndex);
    assert (ETHEREUM_BOOLEAN_IS_TRUE (extractedIdentifier));

    BREthereumAddress logAddress = logGetAddress(log);
    BREthereumToken   token      = BRSetGet (manager->tokens, &logAddress);
    if (NULL == token) { logRelease(log); return;}

    // TODO: Confirm LogTopic[0] is 'transfer'
    if (3 != logGetTopicsCount(log)) { logRelease(log); return; }

    BRCryptoWalletETH wallet = cryptoWalletManagerLookupWalletForToken (manager, token);
    assert (NULL != wallet);

    BRCryptoTransferETH transfer = cryptoWalletLookupTransferByIdentifier (wallet, logHash);
    if (NULL == transfer)
        transfer = cryptoWalletLookupTransferByOriginatingHash (wallet, transactionHash);

    int needStatusEvent = 0;

    BRCryptoTransferState oldState;

    // If we've no transfer, then create one and save `log` as the basis
    if (NULL == transfer) {

        // Do we know that log->data is a number?
        BRRlpItem  item  = rlpDataGetItem (manager->coder, logGetDataShared(log));
        UInt256 amount = rlpDecodeUInt256 (manager->coder, item, 1);
        rlpItemRelease (manager->coder, item);

        transfer = (BRCryptoTransferETH) cryptoTransferCreateWithLogAsETH (wallet->base.unit,
                                                                           wallet->base.unitForFee,
                                                                           manager->account,
                                                                           amount,
                                                                           log);

//        walletHandleTransfer (wallet, transfer);

        // We've added a transfer and arguably we should update the wallet's balance.  But don't.
        // Ethereum is 'account based'; we'll only update the balance based on a account state
        // change (based on a P2P or API callback).
        //
        // walletUpdateBalance (wallet);

        cryptoWalletManagerGenerateTransferEvent (&manager->base, &wallet->base, &transfer->base,
                                                  (BRCryptoTransferEvent) {
            CRYPTO_TRANSFER_EVENT_CREATED,
        });

        oldState = (BRCryptoTransferState) { CRYPTO_TRANSFER_STATE_CREATED };

        // If this transfer references a transaction, fill out this log's fee basis
#ifdef REFACTOR
        ewmHandleLogFeeBasis (ewm, transactionHash, NULL, transfer);
#endif
        needStatusEvent = 1;

    }

    // We've got a transfer for log.  We'll update the transfer's basis and check if we need
    // to report a transfer status event.  We'll strive to only report events when the status has
    // actually changed.
    else {
        needStatusEvent = ewmReportTransferStatusAsEventIsNeeded (&manager->base, &wallet->base, &transfer->base,
                                                                  logGetStatus (log));

        // Log becomes the new basis for transfer
#ifdef REFACTOR
        // release prior basis
#else
        assert (NULL == transfer->basis.log);
#endif
        transfer->type = TRANSFER_BASIS_LOG;
        transfer->basis.log = log;               // ownership give
    }

    if (needStatusEvent) {
        BREthereumHashString logHashString;
        ethHashFillString(logHash, logHashString);

        BREthereumHashString transactionHashString;
        ethHashFillString(transactionHash, transactionHashString);

        eth_log ("EWM", "Log: %s { %8s @ %zu }, Change: %s, Status: %d",
                 logHashString, transactionHashString, logIndex,
                 BCS_CALLBACK_TRANSACTION_TYPE_NAME(type),
                 logGetStatus(log).type);

        ewmReportTransferStatusAsEvent (&manager->base, &wallet->base, &transfer->base, oldState);
    }
}

static void
ewmHandleSaveBlocks (BREthereumBCSCallbackContext context,
                     OwnershipGiven BRArrayOf(BREthereumBlock) blocks) {
    BRCryptoWalletManagerETH manager = context;
    size_t count = array_count(blocks);

    eth_log("EWM", "Save Blocks (Storage): %zu", count);
    fileServiceReplace (manager->base.fileService,
                        fileServiceTypeBlocksETH,
                        (const void **) blocks,
                        count);

    array_free (blocks);
}

static void
ewmHandleSaveNodes (BREthereumBCSCallbackContext context,
                    OwnershipGiven BRArrayOf(BREthereumNodeConfig) nodes) {
    BRCryptoWalletManagerETH manager = context;
    size_t count = array_count(nodes);

    eth_log("EWM", "Save Nodes (Storage): %zu", count);
    fileServiceReplace (manager->base.fileService,
                        fileServiceTypeNodesETH,
                        (const void **) nodes,
                        count);

    array_free (nodes);
}

//extern void
//ewmHandleSaveTransaction (BREthereumBCSCallbackContext context,
//                          BREthereumTransaction transaction,
//                          BREthereumClientChangeType type) {
//    BREthereumHash hash = transactionGetHash(transaction);
//    BREthereumHashString fileName;
//    ethHashFillString(hash, fileName);
//
//    eth_log("EWM", "Transaction: Save: %s: %s",
//            CLIENT_CHANGE_TYPE_NAME (type),
//            fileName);
//
//    if (CLIENT_CHANGE_REM == type || CLIENT_CHANGE_UPD == type)
//        fileServiceRemove (ewm->fs, ewmFileServiceTypeTransactions,
//                           fileServiceGetIdentifier(ewm->fs, ewmFileServiceTypeTransactions, transaction));
//
//    if (CLIENT_CHANGE_ADD == type || CLIENT_CHANGE_UPD == type)
//        fileServiceSave (ewm->fs, ewmFileServiceTypeTransactions, transaction);
//}
//
//extern void
//ewmHandleSaveLog (BREthereumBCSCallbackContext context,
//                  BREthereumLog log,
//                  BREthereumClientChangeType type) {
//    BREthereumHash hash = logGetHash(log);
//    BREthereumHashString filename;
//    ethHashFillString(hash, filename);
//
//    eth_log("EWM", "Log: Save: %s: %s",
//            CLIENT_CHANGE_TYPE_NAME (type),
//            filename);
//
//    if (CLIENT_CHANGE_REM == type || CLIENT_CHANGE_UPD == type)
//        fileServiceRemove (ewm->fs, ewmFileServiceTypeLogs,
//                           fileServiceGetIdentifier (ewm->fs, ewmFileServiceTypeLogs, log));
//
//    if (CLIENT_CHANGE_ADD == type || CLIENT_CHANGE_UPD == type)
//        fileServiceSave (ewm->fs, ewmFileServiceTypeLogs, log);
//}
//
//extern void
//ewmHandleSaveWallet (BREthereumBCSCallbackContext context,
//                     BREthereumWallet wallet,
//                     BREthereumClientChangeType type) {
//    BREthereumWalletState state = walletStateCreate (wallet);
//
//    // If this is the primaryWallet, hack in the nonce
//    if (wallet == ewm->walletHoldingEther) {
//        walletStateSetNonce (state,
//                             ethAccountGetAddressNonce (ewm->account,
//                                                     ethAccountGetPrimaryAddress(ewm->account)));
//    }
//
//    BREthereumHash hash = walletStateGetHash(state);
//    BREthereumHashString filename;
//    ethHashFillString(hash, filename);
//
//    eth_log ("EWM", "Wallet: Save: %s: %s",
//             CLIENT_CHANGE_TYPE_NAME (type),
//             filename);
//
//    switch (type) {
//        case CLIENT_CHANGE_REM:
//            fileServiceRemove (ewm->fs, ewmFileServiceTypeWallets,
//                               fileServiceGetIdentifier (ewm->fs, ewmFileServiceTypeWallets, state));
//            break;
//
//        case CLIENT_CHANGE_ADD:
//        case CLIENT_CHANGE_UPD:
//            fileServiceSave (ewm->fs, ewmFileServiceTypeWallets, state);
//            break;
//    }
//
//    walletStateRelease (state);
//}

static void
ewmHandleSync (BREthereumBCSCallbackContext context,
               BREthereumBCSCallbackSyncType type,
               uint64_t blockNumberStart,
               uint64_t blockNumberCurrent,
               uint64_t blockNumberStop) {
    BRCryptoWalletManagerETH manager = context;

    //    assert (CRYPTO_SYNC_MODE_P2P_ONLY == ewm->mode || CRYPTO_SYNC_MODE_P2P_WITH_API_SYNC == ewm->mode);

    BRCryptoSyncPercentComplete syncCompletePercent = AS_CRYPTO_SYNC_PERCENT_COMPLETE (100.0 * (blockNumberCurrent - blockNumberStart) / (blockNumberStop - blockNumberStart));
    // We do not have blockTimestampCurrent

    if (blockNumberCurrent == blockNumberStart) {
        cryptoWalletManagerGenerateManagerEvent (&manager->base,
                                                 (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_SYNC_STARTED
        });

        // TODO: connected -> syncing
    }
    else if (blockNumberCurrent == blockNumberStop) {
        cryptoWalletManagerGenerateManagerEvent (&manager->base,
                                                 (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_SYNC_STOPPED
        });

        // TODO: syncing -> connected
    }
    else {
        cryptoWalletManagerGenerateManagerEvent (&manager->base,
                                                 (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_SYNC_CONTINUES,
            { .syncContinues = { NO_CRYPTO_TIMESTAMP, syncCompletePercent }}
        });
    }

    eth_log ("EWM", "Sync: %d, %.2f%%", type, syncCompletePercent);
}

static void
ewmHandleGetBlocks (BREthereumBCSCallbackContext context,
                    BREthereumAddress address,
                    BREthereumSyncInterestSet interests,
                    uint64_t blockStart,
                    uint64_t blockStop) {
    BRCryptoWalletManagerETH manager = context;
    (void) manager;
#if 0
    char *strAddress = ethAddressGetEncodedString(address, 0);

    ewm->client.funcGetBlocks (ewm->client.context,
                               ewm,
                               strAddress,
                               interests,
                               blockStart,
                               blockStop,
                               ++ewm->requestId);

    free (strAddress);
#endif
}
