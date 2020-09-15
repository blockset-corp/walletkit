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

// MARK: - Forward Declarations

static void
cryptoWalletManagerCreateTokenForCurrency (BRCryptoWalletManagerETH manager,
                                           BRCryptoCurrency currency,
                                           BRCryptoUnit     unitDefault);

extern BRCryptoWalletManagerETH
cryptoWalletManagerCoerceETH (BRCryptoWalletManager manager) {
    assert (CRYPTO_NETWORK_TYPE_ETH == manager->type);
    return (BRCryptoWalletManagerETH) manager;
}

typedef struct {
    BREthereumNetwork network;
    BREthereumAccount account;
    BRRlpCoder coder;
} BRCryptoWalletManagerCreateContextETH;

static void
cryptoWaleltMangerCreateCallbackETH (BRCryptoWalletManagerCreateContext context,
                                     BRCryptoWalletManager manager) {
    BRCryptoWalletManagerCreateContextETH *contextETH = (BRCryptoWalletManagerCreateContextETH*) context;
    BRCryptoWalletManagerETH managerETH = cryptoWalletManagerCoerceETH (manager);

    managerETH->network = contextETH->network;
    managerETH->account = contextETH->account;
    managerETH->coder   = contextETH->coder;
}

static BRCryptoWalletManager
cryptoWalletManagerCreateETH (BRCryptoWalletManagerListener listener,
                              BRCryptoClient client,
                              BRCryptoAccount account,
                              BRCryptoNetwork network,
                              BRCryptoSyncMode mode,
                              BRCryptoAddressScheme scheme,
                              const char *path) {
    BRCryptoWalletManagerCreateContextETH contextETH = {
        cryptoNetworkAsETH (network),
        cryptoAccountAsETH (account),
        rlpCoderCreate()
    };

    BRCryptoWalletManager manager = cryptoWalletManagerAllocAndInit (sizeof (struct BRCryptoWalletManagerETHRecord),
                                                                     cryptoNetworkGetType(network),
                                                                     listener,
                                                                     client,
                                                                     account,
                                                                     network,
                                                                     scheme,
                                                                     path,
                                                                     CRYPTO_CLIENT_REQUEST_USE_TRANSFERS,
                                                                     &contextETH,
                                                                     cryptoWaleltMangerCreateCallbackETH);
    BRCryptoWalletManagerETH managerETH = cryptoWalletManagerCoerceETH (manager);

    // Load the persistently stored data.
    BRSetOf(BREthereumTransaction) transactions = initialTransactionsLoadETH (manager);
    BRSetOf(BREthereumLog)         logs         = initialLogsLoadETH         (manager);
    BRSetOf(BREthereumExchanges)   exchanges    = initialExchangesLoadETH    (manager);

    // Save the recovered tokens
    managerETH->tokens = initialTokensLoadETH (manager);

    // Ensure a token (but not a wallet) for each currency
    size_t currencyCount = cryptoNetworkGetCurrencyCount (network);
    for (size_t index = 0; index < currencyCount; index++) {
        BRCryptoCurrency c = cryptoNetworkGetCurrencyAt (network, index);
        if (c != network->currency) {
            BRCryptoUnit unitDefault = cryptoNetworkGetUnitAsDefault (network, c);
            cryptoWalletManagerCreateTokenForCurrency (managerETH, c, unitDefault);
            cryptoUnitGive (unitDefault);
        }
        cryptoCurrencyGive (c);
    }

    // Announce all the provided transactions...
    FOR_SET (BREthereumTransaction, transaction, transactions)
        ewmHandleTransaction (managerETH, BCS_CALLBACK_TRANSACTION_ADDED, transaction);
    BRSetFree(transactions);

    // ... and all the provided logs
    FOR_SET (BREthereumLog, log, logs)
        ewmHandleLog (managerETH, BCS_CALLBACK_LOG_ADDED, log);
    BRSetFree (logs);

    // ... and all the provided exhanges
    FOR_SET (BREthereumExchange, exchange, exchanges)
        ewmHandleExchange (managerETH, BCS_CALLBACK_EXCHANGE_ADDED,  exchange);
    BRSetFree(exchanges);

    pthread_mutex_unlock (&manager->lock);
    return manager;
}

static void
cryptoWalletManagerReleaseETH (BRCryptoWalletManager manager) {
    BRCryptoWalletManagerETH managerETH = cryptoWalletManagerCoerceETH (manager);

    rlpCoderRelease (managerETH->coder);
    if (NULL != managerETH->tokens)
        BRSetFreeAll(managerETH->tokens, (void (*) (void*)) ethTokenRelease);
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
cryptoWalletManagerSignTransaction (BRCryptoWalletManager manager,
                                    BRCryptoWallet wallet,
                                    BRCryptoTransfer transfer,
                                    BRKey *key) {
    BRCryptoWalletManagerETH managerETH  = cryptoWalletManagerCoerceETH (manager);
    BRCryptoTransferETH      transferETH = cryptoTransferCoerceETH      (transfer);


    BREthereumNetwork     ethNetwork     = managerETH->network;
    BREthereumAccount     ethAccount     = managerETH->account;
    BREthereumAddress     ethAddress     = ethAccountGetPrimaryAddress (ethAccount);
    BREthereumTransaction ethTransaction = transferETH->originatingTransaction;

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
cryptoWalletManagerSignTransactionWithSeedETH (BRCryptoWalletManager manager,
                                                      BRCryptoWallet wallet,
                                                      BRCryptoTransfer transfer,
                                                      UInt512 seed) {
    BRCryptoWalletManagerETH managerETH  = cryptoWalletManagerCoerceETH (manager);

    BREthereumAccount     ethAccount     = managerETH->account;
    BREthereumAddress     ethAddress     = ethAccountGetPrimaryAddress (ethAccount);

    BRKey key = derivePrivateKeyFromSeed (seed, ethAccountGetAddressIndex (ethAccount, ethAddress));
    
    return cryptoWalletManagerSignTransaction (manager,
                                               wallet,
                                               transfer,
                                               &key);
}

static BRCryptoBoolean
cryptoWalletManagerSignTransactionWithKeyETH (BRCryptoWalletManager manager,
                                              BRCryptoWallet wallet,
                                              BRCryptoTransfer transfer,
                                              BRCryptoKey key) {
    return cryptoWalletManagerSignTransaction (manager,
                                               wallet,
                                               transfer,
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
    // We only need a fee estimate if this IS the manager's primary wallet.  Otherwise, we are
    // transfering some ERC20 token (typically) and the limits are not impacted by the fees, which
    // are paid in ETH
    *needEstimate = AS_CRYPTO_BOOLEAN (wallet == cwm->wallet);

    return (CRYPTO_TRUE == asMaximum
            ? cryptoWalletGetBalance (wallet)        // Maximum is balance - fees 'needEstimate'
            : cryptoAmountCreateInteger (0, unit));  // No minimum
}

static BRCryptoFeeBasis
cryptoWalletManagerEstimateFeeBasisETH (BRCryptoWalletManager manager,
                                        BRCryptoWallet  wallet,
                                        BRCryptoCookie cookie,
                                        BRCryptoAddress target,
                                        BRCryptoAmount amount,
                                        BRCryptoNetworkFee networkFee,
                                        size_t attributesCount,
                                        OwnershipKept BRCryptoTransferAttribute *attributes) {
    BRCryptoWalletETH walletETH = cryptoWalletCoerce (wallet);

    BREthereumFeeBasis ethFeeBasis = {
        FEE_BASIS_GAS,
        { .gas = { walletETH->ethGasLimit, cryptoNetworkFeeAsETH (networkFee) }}
    };

    BRCryptoCurrency currency = cryptoAmountGetCurrency (amount);
    BRCryptoTransfer transfer = cryptoWalletCreateTransferETH (wallet,
                                                               target,
                                                               amount,
                                                               cryptoFeeBasisCreateAsETH (wallet->unitForFee, ethFeeBasis),
                                                               0, NULL,
                                                               currency,
                                                               wallet->unit,
                                                               wallet->unitForFee);

    cryptoCurrencyGive(currency);

    cryptoClientQRYEstimateTransferFee (manager->qryManager,
                                        cookie,
                                        transfer,
                                        networkFee);

    cryptoTransferGive (transfer);

    // Require QRY with cookie - made above
    return NULL;
}

static BRCryptoFeeBasis
cryptoWalletManagerRecoverFeeBasisFromEstimateETH (BRCryptoWalletManager cwm,
                                                   BRCryptoNetworkFee networkFee,
                                                   double costUnits,
                                                   size_t attributesCount,
                                                   OwnershipKept const char **attributeKeys,
                                                   OwnershipKept const char **attributeVals) {
    BREthereumFeeBasis feeBasis = ethFeeBasisCreate (ethGasCreate ((uint64_t) costUnits),
                                                     cryptoNetworkFeeAsETH (networkFee));
    return cryptoFeeBasisCreateAsETH (networkFee->pricePerCostFactorUnit, feeBasis);
}

static void
cryptoWalletManagerCreateTokenForCurrency (BRCryptoWalletManagerETH managerETH,
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
    BREthereumToken token = BRSetGet (managerETH->tokens, &addr);

    if (NULL == token) {
        token = ethTokenCreate (address,
                             code,
                             name,
                             desc,
                             decimals,
                             defaultGasLimit,
                             defaultGasPrice);
        BRSetAdd (managerETH->tokens, token);
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

    fileServiceSave (managerETH->base.fileService, fileServiceTypeTokensETH, token);
}

static BRCryptoWallet
cryptoWalletManagerCreateWalletETH (BRCryptoWalletManager manager,
                                    BRCryptoCurrency currency) {
    BRCryptoWalletManagerETH managerETH  = cryptoWalletManagerCoerceETH (manager);

    BREthereumToken ethToken = NULL;

    const char *issuer = cryptoCurrencyGetIssuer (currency);
    if (NULL != issuer) {
        BREthereumAddress ethAddress = ethAddressCreate (issuer);
        ethToken = BRSetGet (managerETH->tokens, &ethAddress);
        assert (NULL != ethToken);
    }

    // The `currency` unit: e.g. ETH, BRD, DNT, etc for ETH or the ERC20 token
    BRCryptoUnit unit = cryptoNetworkGetUnitAsDefault (manager->network, currency);

    // The `unitForFee` is always the default/base unit for the network.
    BRCryptoUnit unitForFee = cryptoNetworkGetUnitAsDefault (manager->network, NULL);

    BRCryptoWallet wallet = cryptoWalletCreateAsETH (manager->listenerWallet,
                                                     unit,
                                                     unitForFee,
                                                     ethToken,
                                                     managerETH->account);
    cryptoWalletManagerAddWallet (manager, wallet);

    cryptoUnitGive (unitForFee);
    cryptoUnitGive (unit);

    return wallet;
}

static BRCryptoWalletETH
cryptoWalletManagerLookupWalletForToken (BRCryptoWalletManagerETH managerETH,
                                         BREthereumToken token) {
    if (NULL == token) return (BRCryptoWalletETH) cryptoWalletTake (managerETH->base.wallet);

    for (size_t index = 0; index < array_count(managerETH->base.wallets); index++) {
        BRCryptoWalletETH wallet = cryptoWalletCoerce (managerETH->base.wallets[index]);
        if (token == wallet->ethToken)
            return wallet;
    }
    return NULL;
}

private_extern BRCryptoWalletETH
cryptoWalletManagerEnsureWalletForToken (BRCryptoWalletManagerETH managerETH,
                                         BREthereumToken token) {
    if (NULL == token) return (BRCryptoWalletETH) managerETH->base.wallet;

    BRCryptoWallet wallet = (BRCryptoWallet) cryptoWalletManagerLookupWalletForToken (managerETH, token);

    if (NULL == wallet) {
        BRCryptoCurrency currency   = cryptoNetworkGetCurrencyForIssuer (managerETH->base.network, ethTokenGetAddress(token));
        BRCryptoUnit     unit       = cryptoNetworkGetUnitAsDefault     (managerETH->base.network, currency);
        BRCryptoUnit     unitForFee = cryptoNetworkGetUnitAsDefault     (managerETH->base.network, NULL);

        wallet = cryptoWalletCreateAsETH (managerETH->base.listenerWallet,
                                          unit,
                                          unitForFee,
                                          token,
                                          managerETH->account);
        assert (NULL != wallet);

        cryptoWalletManagerAddWallet (&managerETH->base, wallet);

        cryptoUnitGive (unitForFee);
        cryptoUnitGive (unit);
        cryptoCurrencyGive (currency);
    }

    return (BRCryptoWalletETH) wallet;
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
    *error  = false;
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

//static bool
//cryptoWalletManagerHasAddressETH (BRCryptoWalletManagerETH manager,
//                                  const char *address) {
//    return 0 == strcasecmp (address, ethAccountGetPrimaryAddressString (manager->account));
//}

static bool // true if error
cryptoWalletManagerRecoverTransaction (BRCryptoWalletManager manager,
                                       OwnershipKept BRCryptoClientTransferBundle bundle) {

    BRCryptoWalletManagerETH managerETH = cryptoWalletManagerCoerceETH (manager);

    UInt256 amount;
    uint64_t gasLimit;
    uint64_t gasUsed;
    UInt256  gasPrice;
    uint64_t nonce;
    bool     error;

    cwmExtractAttributes (bundle, &amount, &gasLimit, &gasUsed, &gasPrice, &nonce, &error);
    // TODO: Handle `error` - bundle->status of 'ERROR' means??? Pay the fee
    // if (error) return true;

    BREthereumAddress sourceAddress = ethAddressCreate (bundle->from);
    BREthereumAddress targetAddress = ethAddressCreate (bundle->to);

    // If account contains the source address, then update the nonce
    if (ETHEREUM_BOOLEAN_IS_TRUE (ethAccountHasAddress (managerETH->account, sourceAddress))) {
        assert (ETHEREUM_BOOLEAN_IS_TRUE (ethAddressEqual (sourceAddress, ethAccountGetPrimaryAddress (managerETH->account))));
        // Update the ETH account's nonce if we originzed this
        ethAccountSetAddressNonce (managerETH->account,
                                   ethAccountGetPrimaryAddress (managerETH->account),
                                   nonce,
                                   ETHEREUM_BOOLEAN_FALSE);
    }

    //
    // This 'announce' call is coming from the guaranteed BRD endpoint; thus we don't need to
    // worry about the validity of the transaction - it is surely confirmed.  Is that true
    // if newly submitted?

    // TODO: Confirm we are not repeatedly creating transactions
    BREthereumTransaction tid = transactionCreate (sourceAddress,
                                                   targetAddress,
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
    transactionSetHash (tid, ethHashCreate (bundle->hash));

    BREthereumTransactionStatus status = transactionStatusCreateIncluded (ethHashCreate (bundle->blockHash),
                                                                          bundle->blockNumber,
                                                                          bundle->blockTransactionIndex,
                                                                          bundle->blockTimestamp,
                                                                          ethGasCreate(gasUsed));
    transactionSetStatus (tid, status);

    // If we had a `bcs` we might think about `bcsSignalTransaction(ewm->bcs, transaction);`
    ewmHandleTransaction (managerETH, BCS_CALLBACK_TRANSACTION_UPDATED, tid);

    return false;
}

static bool // true if error
cryptoWalletManagerRecoverLog (BRCryptoWalletManager manager,
                               const char *contract,
                               OwnershipKept BRCryptoClientTransferBundle bundle) {
    BRCryptoWalletManagerETH managerETH = cryptoWalletManagerCoerceETH (manager);

    UInt256 amount;
    uint64_t gasLimit;
    uint64_t gasUsed;
    UInt256  gasPrice;
    uint64_t nonce;
    bool     error;

    size_t logIndex = 0;

    cwmExtractAttributes(bundle, &amount, &gasLimit, &gasUsed, &gasPrice, &nonce, &error);
    if (error) return true;

    // TODO: Is `nonce` relevent here?  Or only in cryptoWalletManagerRecoverTransaction

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

    BRRlpItem  item  = rlpEncodeUInt256 (managerETH->coder, amount, 1);

    BREthereumLog log = logCreate (ethAddressCreate (contract),
                                   topicsCount,
                                   topics,
                                   rlpItemGetDataSharedDontRelease (managerETH->coder, item));
    rlpItemRelease (managerETH->coder, item);

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
    ewmHandleLog (managerETH, BCS_CALLBACK_LOG_UPDATED, log);

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

    return false; // no error
}

static bool // true if error
cryptoWalletManagerRecoverExchange (BRCryptoWalletManager manager,
                                    const char *contract,
                                    OwnershipKept BRCryptoClientTransferBundle bundle) {
    BRCryptoWalletManagerETH managerETH = cryptoWalletManagerCoerceETH (manager);

    bool error;
    UInt256 amount = cwmParseUInt256 (bundle->amount, &error);
    if (error) return true;

    size_t exchangeIndex = 0;

    BREthereumExchange exchange = ethExchangeCreate (ethAddressCreate(bundle->from),
                                                     ethAddressCreate(bundle->to),
                                                     ethAddressCreate(contract),
                                                     0, // contractdAssetIndex,
                                                     amount);

    ethExchangeInitializeIdentifier (exchange,
                                     ethHashCreate(bundle->hash),
                                     exchangeIndex);

    BREthereumTransactionStatus status =
    transactionStatusCreateIncluded (ethHashCreate(bundle->blockHash),
                                     bundle->blockNumber,
                                     bundle->blockTransactionIndex,
                                     bundle->blockTimestamp,
                                     ethGasCreate(0));
    ethExchangeSetStatus (exchange, status);

    ewmHandleExchange (managerETH, BCS_CALLBACK_EXCHANGE_UPDATED, exchange);

    return false; // no error
}

static void
cryptoWalletManagerRecoverTransfersFromTransactionBundleETH (BRCryptoWalletManager manager,
                                                             OwnershipKept BRCryptoClientTransactionBundle bundle) {
    BRCryptoWalletManagerETH managerETH = cryptoWalletManagerCoerceETH (manager);
    (void) managerETH;
assert (0);
}

static const char *
cryptoWalletManagerParseIssuer (const char *currency) {
    // Parse: "<blockchain-id>:<issuer>"
    return 1 + strrchr (currency, ':');
}

bool strPrefix(const char *pre, const char *str)
{
    return strncmp (pre, str, strlen(pre)) == 0;
}

static void
cryptoWalletManagerRecoverTransferFromTransferBundleETH (BRCryptoWalletManager manager,
                                                         OwnershipKept BRCryptoClientTransferBundle bundle) {
    BRCryptoWalletManagerETH managerETH = cryptoWalletManagerCoerceETH (manager);
    (void) managerETH;

    BRCryptoNetwork  network = cryptoWalletManagerGetNetwork (manager);

    // We'll only have a `walletCurrency` if the bundle->currency is for ETH or fro an ERC20 token
    // that is known.  If `bundle` indicates a `transfer` that we sent and we do not know about
    // the ERC20 token we STILL MUST process the fee and the nonce.
    BRCryptoCurrency walletCurrency = cryptoNetworkGetCurrencyForUids (network, bundle->currency);

    // The contract is NULL or an ERC20 Smart Contract address.
    const char *contract = (NULL == walletCurrency
                            ? cryptoWalletManagerParseIssuer (bundle->currency)
                            : cryptoCurrencyGetIssuer(walletCurrency));

    switch (manager->syncMode) {
        case CRYPTO_SYNC_MODE_API_ONLY:
        case CRYPTO_SYNC_MODE_API_WITH_P2P_SEND: {
            bool error = CRYPTO_TRANSFER_STATE_ERRORED == bundle->status;

            bool needTransaction = false;
            bool needLog         = false;
            bool needExchange    = false;

            // Use `data` to determine the need for {Transaction,Log,Exchange)
            const char *data = cwmLookupAttributeValueForKey ("input",
                                                              bundle->attributesCount,
                                                              (const char **) bundle->attributeKeys,
                                                              (const char **) bundle->attributeVals);
            assert (NULL != data);

            // A Primary Transaction of ETH.  The Transaction produces one and only one Transfer
            if (strcasecmp (data, "0x") == 0) {
                needTransaction = true;
            }

            // A Primary Transaction of ERC20 Transfer.  The Transaction produces at most two Transfers -
            // one Log for the ERC20 token (with a 'contract') and one Transaction for the ETH fee
            else if (strPrefix ("0xa9059cbb", data)) {
                needLog         = (NULL != contract);
                needTransaction = (NULL == contract);
            }

            // A Primary Transaction of some arbitrary asset.  The Transaction produces one or more
            // Transfers - one Transaction for the ETH fee and any number of other transfers, including
            // others for ETH
            else {
                needExchange    = (NULL == bundle->fee);        // NULL contract -> ETH exchange
                needTransaction = (NULL != bundle->fee && NULL == contract);
            }

            // On errors, skip Log and Exchange; but keep Transaction if needed.
            needLog      &= !error;
            needExchange &= !error;

            if (needLog) {
                // This could produce an error - generally a parse error.  We'll soldier on.
                cryptoWalletManagerRecoverLog (manager, contract, bundle);

            }

            if (needExchange) {
                cryptoWalletManagerRecoverExchange (manager, contract, bundle);
            }

            if (needTransaction) {
                if (needLog || needExchange) {
#ifdef REFACTOR
                    // If needLog or needExchange w/ needTransaction then the transaction
                     //     a) holds the fee; and
                     //     b) increases the nonce.
                     value = UINT256_ZERO;
                     to    = contract;
#endif
                }
                // bundle->data?
                cryptoWalletManagerRecoverTransaction (manager, bundle);
            }
            break;
        }

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

const BREventType *eventTypesETH[] = {};
const unsigned int eventTypesCountETH = (sizeof (eventTypesETH) / sizeof (BREventType*));

BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersETH = {
    cryptoWalletManagerCreateETH,
    cryptoWalletManagerReleaseETH,
    crytpWalletManagerCreateFileServiceETH,
    cryptoWalletManagerGetEventTypesETH,
    cryptoWalletManagerCreateP2PManagerETH,
    cryptoWalletManagerCreateWalletETH,
    cryptoWalletManagerSignTransactionWithSeedETH,
    cryptoWalletManagerSignTransactionWithKeyETH,
    cryptoWalletManagerEstimateLimitETH,
    cryptoWalletManagerEstimateFeeBasisETH,
    cryptoWalletManagerRecoverTransfersFromTransactionBundleETH,
    cryptoWalletManagerRecoverTransferFromTransferBundleETH,
    cryptoWalletManagerRecoverFeeBasisFromEstimateETH,
    NULL,//BRCryptoWalletManagerWalletSweeperValidateSupportedHandler not supported
    NULL,//BRCryptoWalletManagerCreateWalletSweeperHandler not supported
};

#if 0
extern void
ewmHandleLog (BREthereumEWM ewm,
              BREthereumBCSCallbackLogType type,
              OwnershipGiven BREthereumLog log) {
    BREthereumHash logHash = logGetHash(log);

    BREthereumHash transactionHash;
    size_t logIndex;

    // Assert that we always have an identifier for `log`.
    BREthereumBoolean extractedIdentifier = logExtractIdentifier (log, &transactionHash, &logIndex);
    assert (ETHEREUM_BOOLEAN_IS_TRUE (extractedIdentifier));

    BREthereumToken token = ewmLookupToken (ewm, logGetAddress(log));
    if (NULL == token) { logRelease(log); return;}

    // TODO: Confirm LogTopic[0] is 'transfer'
    if (3 != logGetTopicsCount(log)) { logRelease(log); return; }

    BREthereumWallet wallet = ewmGetWalletHoldingToken (ewm, token);
    assert (NULL != wallet);

    BREthereumTransfer transfer = walletGetTransferByIdentifier (wallet, logHash);
    if (NULL == transfer)
        transfer = walletGetTransferByOriginatingHash (wallet, transactionHash);

    int needStatusEvent = 0;

    // If we've no transfer, then create one and save `log` as the basis
    if (NULL == transfer) {
        transfer = transferCreateWithLog (log, token, ewm->coder); // log ownership given

        walletHandleTransfer (wallet, transfer);

        ewmSignalTransferEvent (ewm, wallet, transfer, (BREthereumTransferEvent) {
            TRANSFER_EVENT_CREATED,
            SUCCESS
        });

        // If this transfer references a transaction, fill out this log's fee basis
        ewmHandleLogFeeBasis (ewm, transactionHash, NULL, transfer);

        needStatusEvent = 1;
    }

    // We've got a transfer for log.  We'll update the transfer's basis and check if we need
    // to report a transfer status event.  We'll strive to only report events when the status has
    // actually changed.
    else {
        needStatusEvent = ewmReportTransferStatusAsEventIsNeeded (ewm, wallet, transfer,
                                                                  logGetStatus (log));

        // Log becomes the new basis for transfer
        transferSetBasisForLog (transfer, log);  // log ownership given
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

        ewmReportTransferStatusAsEvent (ewm, wallet, transfer);
    }

    // We've added a transfer and should update the wallet's balance.  Ethereum is 'account based';
    // but in API modes we don't have the account information - so we'll update the balance
    // explicitly.  In P2P mode, we get the 'account'.
    //
    if (CRYPTO_SYNC_MODE_API_ONLY          == ewm->mode ||
        CRYPTO_SYNC_MODE_API_WITH_P2P_SEND == ewm->mode)
        ewmUpdateAndReportWalletState (ewm, wallet);
}

extern void
ewmHandleExchange (BREthereumEWM ewm,
                   BREthereumBCSCallbackExchangeType type,
                   OwnershipGiven BREthereumExchange exchange) {
    BREthereumHash exchangeHash = ethExchangeGetHash(exchange);

    BREthereumHash transactionHash;
    size_t exchangeIndex;

    // Assert that we always have an identifier for `log`.
    BREthereumBoolean extractedIdentifier = ethExchangeExtractIdentifier (exchange, &transactionHash, &exchangeIndex);
    assert (ETHEREUM_BOOLEAN_IS_TRUE (extractedIdentifier));

    BREthereumAddress contract = ethExchangeGetContract(exchange);
    BREthereumToken   token    = ewmLookupToken (ewm, contract);
    BREthereumWallet  wallet = (ETHEREUM_BOOLEAN_IS_TRUE (ethAddressEqual (contract, EMPTY_ADDRESS_INIT))
                                ? ewmGetWallet (ewm)
                                : (NULL == token ? NULL : ewmGetWalletHoldingToken (ewm, token)));

    // TODO: Nothing to do?
    if (NULL == wallet) { ethExchangeRelease(exchange); return; }

    BREthereumTransfer transfer = walletGetTransferByIdentifier (wallet, exchangeHash);
    // We never have an originating transfers as we can't create internal transactions

    bool needStatusEvent = false;

    if (NULL == transfer) {
        transfer = transferCreateWithExchange(exchange, ewm->tokens);

        walletHandleTransfer (wallet, transfer);

        ewmSignalTransferEvent (ewm, wallet, transfer, (BREthereumTransferEvent) {
            TRANSFER_EVENT_CREATED,
            SUCCESS
        });

        // If this transfer references a transaction, fill out this exchanges's fee basis
        if (wallet != ewmGetWallet(ewm))
            ewmHandleExchangeFeeBasis (ewm, transactionHash, NULL, transfer);

        needStatusEvent = true;
    }
    else {
        needStatusEvent = ewmReportTransferStatusAsEventIsNeeded (ewm, wallet, transfer,
                                                                  ethExchangeGetStatus(exchange));

        // Exchange becomes the new basis for transfer
        transferSetBasisForExchange (transfer, exchange);
    }

    if (needStatusEvent) {
        BREthereumHashString exchnageHashString;
        ethHashFillString(exchangeHash, exchnageHashString);

        BREthereumHashString transactionHashString;
        ethHashFillString(transactionHash, transactionHashString);

        eth_log ("EWM", "Exchange: %s { %8s @ %zu }, Change: %s, Status: %d",
                 exchnageHashString, transactionHashString, exchangeIndex,
                 BCS_CALLBACK_TRANSACTION_TYPE_NAME(type),
                 ethExchangeGetStatus (exchange).type);

        ewmReportTransferStatusAsEvent (ewm, wallet, transfer);
    }

    // We've added a transfer and should update the wallet's balance.  Ethereum is 'account based';
    // but in API modes we don't have the account information - so we'll update the balance
    // explicitly.  In P2P mode, we get the 'account'.
    //
    if (CRYPTO_SYNC_MODE_API_ONLY          == ewm->mode ||
        CRYPTO_SYNC_MODE_API_WITH_P2P_SEND == ewm->mode)
        ewmUpdateAndReportWalletState (ewm, wallet);

}

#endif
