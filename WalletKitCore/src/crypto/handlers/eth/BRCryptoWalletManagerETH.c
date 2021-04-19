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

#include "crypto/BRCryptoAmountP.h"
#include "crypto/BRCryptoAccountP.h"
#include "crypto/BRCryptoNetworkP.h"
#include "crypto/BRCryptoKeyP.h"
#include "crypto/BRCryptoClientP.h"
#include "crypto/BRCryptoFileService.h"
#include "crypto/BRCryptoWalletManagerP.h"

#include <ctype.h>

// MARK: - Forward Declarations

static void
cryptoWalletManagerCreateTokenForCurrency (BRCryptoWalletManagerETH manager,
                                           BRCryptoCurrency currency);

static void
cryptoWalletManagerCreateCurrencyForToken (BRCryptoWalletManagerETH managerETH,
                                           BREthereumToken token);

static void
cryptoWalletManagerCreateTokensForNetwork (BRCryptoWalletManagerETH manager,
                                           BRCryptoNetwork network);

static BREthereumToken
cryptoWalletManagerGetTokenETH (BRCryptoWalletManagerETH managerETH,
                                const BREthereumAddress *ethAddress) {
    BREthereumToken token;

    pthread_mutex_lock (&managerETH->base.lock);
    token = BRSetGet (managerETH->tokens, ethAddress);
    pthread_mutex_unlock (&managerETH->base.lock);

    return token;
}

static void
cryptoWalletManagerAddTokenETH (BRCryptoWalletManagerETH managerETH,
                                BREthereumToken ethToken) {
    pthread_mutex_lock (&managerETH->base.lock);
    BRSetAdd (managerETH->tokens, ethToken);
    pthread_mutex_unlock (&managerETH->base.lock);
}

// MARK: - Event Types

const BREventType *eventTypesETH[] = {
    CRYPTO_CLIENT_EVENT_TYPES
};
const unsigned int eventTypesCountETH = (sizeof (eventTypesETH) / sizeof (BREventType*));

// MARK: - Wallet Manager

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

    // Save the recovered tokens
    managerETH->tokens = ethTokenSetCreate (EWM_INITIAL_SET_SIZE_DEFAULT);

    // Ensure a token (but not a wallet) for each currency
    cryptoWalletManagerCreateTokensForNetwork (managerETH, network);

    // Ensure a currency for each token
    FOR_SET (BREthereumToken, token, managerETH->tokens) {
        cryptoWalletManagerCreateCurrencyForToken (managerETH, token);
    }

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
cryptoWalletManagerCreateFileServiceETH (BRCryptoWalletManager manager,
                                         const char *basePath,
                                         const char *currency,
                                         const char *network,
                                         BRFileServiceContext context,
                                         BRFileServiceErrorHandler handler) {
    return fileServiceCreateFromTypeSpecifications (basePath, currency, network,
                                                    context, handler,
                                                    cryptoFileServiceSpecificationsCount,
                                                    cryptoFileServiceSpecifications);
}

static const BREventType **
cryptoWalletManagerGetEventTypesETH (BRCryptoWalletManager manager,
                                     size_t *eventTypesCount) {
    assert (NULL != eventTypesCount);
    *eventTypesCount = eventTypesCountETH;
    return eventTypesETH;
}

static BRCryptoBoolean
cryptoWalletManagerSignTransactionETH (BRCryptoWalletManager manager,
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

    pthread_mutex_lock (&transfer->lock);
    transferETH->hash = cryptoHashCreateAsETH(transactionGetHash(ethTransaction));
    pthread_mutex_unlock (&transfer->lock);

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
    
    return cryptoWalletManagerSignTransactionETH (manager,
                                                  wallet,
                                                  transfer,
                                                  &key);
}

static BRCryptoBoolean
cryptoWalletManagerSignTransactionWithKeyETH (BRCryptoWalletManager manager,
                                              BRCryptoWallet wallet,
                                              BRCryptoTransfer transfer,
                                              BRCryptoKey key) {
    return cryptoWalletManagerSignTransactionETH (manager,
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
    BRCryptoFeeBasis feeBasis = cryptoFeeBasisCreateAsETH (wallet->unitForFee, ethFeeBasis);

    BRCryptoTransfer transfer = cryptoWalletCreateTransferETH (wallet,
                                                               target,
                                                               amount,
                                                               feeBasis,
                                                               attributesCount,
                                                               attributes,
                                                               currency,
                                                               wallet->unit,
                                                               wallet->unitForFee);

    cryptoCurrencyGive(currency);

    cryptoClientQRYEstimateTransferFee (manager->qryManager,
                                        cookie,
                                        transfer,
                                        networkFee,
                                        feeBasis);

    cryptoTransferGive (transfer);
    cryptoFeeBasisGive (feeBasis);

    // Require QRY with cookie - made above
    return NULL;
}

static BRCryptoFeeBasis
cryptoWalletManagerRecoverFeeBasisFromFeeEstimateETH (BRCryptoWalletManager cwm,
                                                      BRCryptoNetworkFee networkFee,
                                                      BRCryptoFeeBasis initialFeeBasis,
                                                      double costUnits,
                                                      size_t attributesCount,
                                                      OwnershipKept const char **attributeKeys,
                                                      OwnershipKept const char **attributeVals) {
    BREthereumFeeBasis feeBasis = ethFeeBasisCreate (ethGasCreate ((uint64_t) costUnits),
                                                     cryptoNetworkFeeAsETH (networkFee));
    return cryptoFeeBasisCreateAsETH (networkFee->pricePerCostFactorUnit, feeBasis);
}

static void
cryptoWalletManagerCreateTokenForCurrencyInternal (BRCryptoWalletManagerETH managerETH,
                                                   BRCryptoCurrency currency,
                                                   bool updateIfNeeded) {
    const char *address = cryptoCurrencyGetIssuer(currency);

    if (NULL == address || 0 == strlen(address)) return;
    if (ETHEREUM_BOOLEAN_FALSE == ethAddressValidateString(address)) return;

    BREthereumAddress addr = ethAddressCreate(address);

    // Check for an existing token
    BREthereumToken token = cryptoWalletManagerGetTokenETH(managerETH, &addr);

    if (NULL != token && !updateIfNeeded) return;

    const char *code = cryptoCurrencyGetCode (currency);
    const char *name = cryptoCurrencyGetName (currency);
    const char *desc = cryptoCurrencyGetUids (currency);

    BRCryptoUnit unitDefault = cryptoNetworkGetUnitAsDefault (managerETH->base.network, currency);
    unsigned int decimals = cryptoUnitGetBaseDecimalOffset(unitDefault);
    cryptoUnitGive(unitDefault);

    BREthereumGas      defaultGasLimit = ethGasCreate(TOKEN_BRD_DEFAULT_GAS_LIMIT);
    BREthereumGasPrice defaultGasPrice = ethGasPriceCreate(ethEtherCreate(uint256Create(TOKEN_BRD_DEFAULT_GAS_PRICE_IN_WEI_UINT64)));

    if (NULL == token) {
        token = ethTokenCreate (address,
                                code,
                                name,
                                desc,
                                decimals,
                                defaultGasLimit,
                                defaultGasPrice);
        cryptoWalletManagerAddTokenETH (managerETH, token);
    }
    else {
        pthread_mutex_lock (&managerETH->base.lock);
        ethTokenUpdate (token,
                        code,
                        name,
                        desc,
                        decimals,
                        defaultGasLimit,
                        defaultGasPrice);
        pthread_mutex_unlock (&managerETH->base.lock);
    }
}

static void
cryptoWalletManagerEnsureTokenForCurrency (BRCryptoWalletManagerETH managerETH,
                                           BRCryptoCurrency currency) {
    cryptoWalletManagerCreateTokenForCurrencyInternal (managerETH, currency, false);
}

static void
cryptoWalletManagerCreateTokenForCurrency (BRCryptoWalletManagerETH managerETH,
                                           BRCryptoCurrency currency) {
    cryptoWalletManagerCreateTokenForCurrencyInternal (managerETH, currency, true);
}

static void
cryptoWalletManagerCreateTokensForNetwork (BRCryptoWalletManagerETH managerETH,
                                           BRCryptoNetwork network) {
    size_t currencyCount = cryptoNetworkGetCurrencyCount (network);
    for (size_t index = 0; index < currencyCount; index++) {
        BRCryptoCurrency c = cryptoNetworkGetCurrencyAt (network, index);
        if (c != network->currency)
            cryptoWalletManagerCreateTokenForCurrency (managerETH, c);
        cryptoCurrencyGive (c);
    }
}

static void
cryptoWalletManagerCreateCurrencyForToken (BRCryptoWalletManagerETH managerETH,
                                           BREthereumToken token) {
    BRCryptoNetwork network = managerETH->base.network;

    const char *issuer = ethTokenGetAddress (token);

    BRCryptoCurrency currency = cryptoNetworkGetCurrencyForIssuer(network, issuer);
    if (NULL == currency) {
        const char *tokenName = ethTokenGetName (token);
        const char *tokenSymb = ethTokenGetSymbol (token);
        const char *tokenAddr = ethTokenGetAddress (token);

        const char *networkUids = cryptoNetworkGetUids(network);

        size_t uidsCount = strlen(networkUids) + 1 + strlen(tokenAddr) + 1;
        char   uids [uidsCount];
        sprintf (uids, "%s:%s", networkUids, tokenAddr);

        currency = cryptoCurrencyCreate (uids,
                                         tokenName,
                                         tokenSymb,
                                         "erc20",
                                         issuer);

        const char *code = tokenSymb;
        char codeBase[strlen(code) + strlen("i") + 1];
        sprintf (codeBase, "%si", code);

        const char *name = tokenName;
        char nameBase[strlen(tokenName) + strlen(" INT") + 1];
        sprintf (nameBase, "%s INT", tokenName);

        size_t symbolCount = strlen(tokenSymb);
        char symbol[symbolCount + 1];
        sprintf (symbol, "%s", tokenSymb);
        // Uppercase
        for (size_t index = 0; index < symbolCount; index++) symbol[index] = toupper (symbol[index]);

        char symbolBase[strlen(symbol) + strlen("I") + 1];
        sprintf (symbolBase, "%sI", symbol);

        BRCryptoUnit baseUnit = cryptoUnitCreateAsBase (currency,
                                                        codeBase,
                                                        nameBase,
                                                        symbolBase);

        BRCryptoUnit defaultUnit = cryptoUnitCreate (currency,
                                                     code,
                                                     name,
                                                     symbol,
                                                     baseUnit,
                                                     ethTokenGetDecimals(token));

        cryptoNetworkAddCurrency (network, currency, baseUnit, defaultUnit);

        cryptoUnitGive(defaultUnit);
        cryptoUnitGive(baseUnit);
    }

    cryptoCurrencyGive (currency);
}

static BRCryptoClientP2PManager
cryptoWalletManagerCreateP2PManagerETH (BRCryptoWalletManager manager) {
    // unsupported
    return NULL;
}


static BRCryptoWallet
cryptoWalletManagerCreateWalletETH (BRCryptoWalletManager manager,
                                    BRCryptoCurrency currency,
                                    Nullable OwnershipKept BRArrayOf(BRCryptoClientTransactionBundle) transactions,
                                    Nullable OwnershipKept BRArrayOf(BRCryptoClientTransferBundle) transfers) {
    BRCryptoWalletManagerETH managerETH = cryptoWalletManagerCoerceETH (manager);

    BREthereumToken ethToken = NULL;

    // If there is an issuer, then `currency` is for an ERC20 token; otherwise for 'Ether'
    const char *issuer = cryptoCurrencyGetIssuer (currency);
    if (NULL != issuer) {
        BREthereumAddress ethAddress = ethAddressCreate (issuer);
        ethToken = cryptoWalletManagerGetTokenETH(managerETH, &ethAddress);

        // If there isn't an existing token, then create one.  A token is expected to exist (there
        // used to be an assert here); however, we've seen a crash on a non-existent token so we'll
        // create a token as 'belt-and-suspenders'.  Still, the currency is required to be in
        // `manager->network`, which implies that there should be a token.

        if (NULL == ethToken) {
            printf ("SYS: ETH: No token for: %s\n", issuer);
            if (CRYPTO_TRUE != cryptoNetworkHasCurrency (manager->network, currency))
                printf ("SYS: ETH: No currency in network: %s\n", issuer);
            assert (CRYPTO_TRUE == cryptoNetworkHasCurrency (manager->network, currency));

            // Create the token
            cryptoWalletManagerCreateTokenForCurrency (managerETH, currency);

            // Must find one now, surely.
            ethToken = cryptoWalletManagerGetTokenETH (managerETH, &ethAddress);
        }
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
    if (!string || 0 == strlen(string)) { *error = true; return 0; }

    char *endPtr;
    unsigned long long result = strtoull (string, &endPtr, 0);
    if ('\0' != *endPtr) { *error = true; return 0; } // must parse the entire string

    return result;
}

static UInt256
cwmParseUInt256 (const char *string, bool *error) {
    if (!string || 0 == strlen (string)) { *error = true; return UINT256_ZERO; }

    BRCoreParseStatus status;
    UInt256 result = uint256CreateParse (string, 0, &status);
    if (CORE_PARSE_OK != status) { *error = true; return UINT256_ZERO; }

    return result;
}

// Assigns `true` into `*error` if there is a parse error; otherwise leaves `*error` unchanged.
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
    *gasUsed  = cwmParseUInt64 (cwmLookupAttributeValueForKey ("gasUsed",  attributesCount, attributeKeys, attributeVals), error);
    *gasPrice = cwmParseUInt256(cwmLookupAttributeValueForKey ("gasPrice", attributesCount, attributeKeys, attributeVals), error);
    *nonce    = cwmParseUInt64 (cwmLookupAttributeValueForKey ("nonce",    attributesCount, attributeKeys, attributeVals), error);

    if (*gasLimit == 21000 && *gasUsed == 0x21000) *gasUsed = 21000;
}

#if defined (INCLUDE_UNUSED_RecoverTransaction)
static bool // true if error
cryptoWalletManagerRecoverTransaction (BRCryptoWalletManager manager,
                                       OwnershipKept BRCryptoClientTransferBundle bundle) {

    BRCryptoWalletManagerETH managerETH = cryptoWalletManagerCoerceETH (manager);

    UInt256  amount;
    uint64_t gasLimit;
    uint64_t gasUsed;
    UInt256  gasPrice;
    uint64_t nonce;

    bool error = false;
    cwmExtractAttributes (bundle, &amount, &gasLimit, &gasUsed, &gasPrice, &nonce, &error);
    if (error) return true;

    bool statusError = (CRYPTO_TRANSFER_STATE_ERRORED == bundle->status);

    BREthereumAddress sourceAddress = ethAddressCreate (bundle->from);
    BREthereumAddress targetAddress = ethAddressCreate (bundle->to);

    // If account contains the source address, then update the nonce
    if (ETHEREUM_BOOLEAN_IS_TRUE (ethAccountHasAddress (managerETH->account, sourceAddress))) {
        assert (ETHEREUM_BOOLEAN_IS_TRUE (ethAddressEqual (sourceAddress, ethAccountGetPrimaryAddress (managerETH->account))));
        // Update the ETH account's nonce if we originated this.
        ethAccountSetAddressNonce (managerETH->account,
                                   ethAccountGetPrimaryAddress (managerETH->account),
                                   nonce + 1, // The NEXT nonce; one more than this transaction's
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

    uint64_t success = statusError ? 0 : 1;
    BREthereumTransactionStatus status = transactionStatusCreateIncluded (ethHashCreate (bundle->blockHash),
                                                                          bundle->blockNumber,
                                                                          bundle->blockTransactionIndex,
                                                                          bundle->blockTimestamp,
                                                                          ethGasCreate(gasUsed),
                                                                          success);
    transactionSetStatus (tid, status);

    // If we had a `bcs` we might think about `bcsSignalTransaction(ewm->bcs, transaction);`
#if defined (NEED_ETH_LES_SUPPORT)
    ewmHandleTransaction (managerETH, BCS_CALLBACK_TRANSACTION_UPDATED, tid);
#endif

    return false;
}
#endif // defined (INCLUDE_UNUSED_RecoverTransaction)

#if defined (INCLUDE_UNUSED_RecoverLog)
static bool // true if error
cryptoWalletManagerRecoverLog (BRCryptoWalletManager manager,
                               const char *contract,
                               OwnershipKept BRCryptoClientTransferBundle bundle) {
    BRCryptoWalletManagerETH managerETH = cryptoWalletManagerCoerceETH (manager);

    UInt256  amount;
    uint64_t gasLimit;
    uint64_t gasUsed;
    UInt256  gasPrice;
    uint64_t nonce;
    size_t   logIndex = 0;

    bool error = false;
    cwmExtractAttributes(bundle, &amount, &gasLimit, &gasUsed, &gasPrice, &nonce, &error);
    if (error) return true;

    bool statusError = (CRYPTO_TRANSFER_STATE_ERRORED == bundle->status);

    // On a `statusError`, until we understand the meaning, assume that the log can't be recovered.
    // Not that cryptoWalletManagerRecoverTransaction DOES NOT avoid the transaction; and thus
    // we'll get `nonce` and `fee` updates.

    // TODO: Handle `errorStatus`; which means what?
    if (statusError) return true;

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

        free (topicsStr[1]);
        free (topicsStr[2]);
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
                                     ethGasCreate(gasUsed),
                                     1); // failed transactions (statusError) are skipped above
    logSetStatus (log, status);

    // If we had a `bcs` we might think about `bcsSignalLog(ewm->bcs, log);`
    //            ewmSignalLog(ewm, BCS_CALLBACK_LOG_UPDATED, log);
#if defined (NEED_ETH_LES_SUPPORT)
    ewmHandleLog (managerETH, BCS_CALLBACK_LOG_UPDATED, log);
#endif

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
#endif //defined (INCLUDE_UNUSED_RecoverLog)

#if defined (INCLUDE_UNUSED_RecoverExchange)
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
                                                     (NULL != contract
                                                      ? ethAddressCreate(contract)
                                                      : EMPTY_ADDRESS_INIT),
                                                     0, // contractdAssetIndex,
                                                     amount);

    ethExchangeInitializeIdentifier (exchange,
                                     ethHashCreate(bundle->hash),
                                     exchangeIndex);

    uint64_t success = (CRYPTO_TRANSFER_STATE_ERRORED == bundle->status) ? 0 : 1;
    BREthereumTransactionStatus status =
    transactionStatusCreateIncluded (ethHashCreate(bundle->blockHash),
                                     bundle->blockNumber,
                                     bundle->blockTransactionIndex,
                                     bundle->blockTimestamp,
                                     ethGasCreate(0),
                                     success);
    ethExchangeSetStatus (exchange, status);

#if defined (NEED_ETH_LES_SUPPORT)
    ewmHandleExchange (managerETH, BCS_CALLBACK_EXCHANGE_UPDATED, exchange);
#endif

    return false; // no error
}
#endif //defined (INCLUDE_UNUSED_RecoverExchange)

static void
cryptoWalletManagerRecoverTransfersFromTransactionBundleETH (BRCryptoWalletManager manager,
                                                             OwnershipKept BRCryptoClientTransactionBundle bundle) {
    BRCryptoWalletManagerETH managerETH = cryptoWalletManagerCoerceETH (manager);
    (void) managerETH;
    assert (0);
}

#if defined (INCLUDE_UNUSED)
static const char *
cryptoWalletManagerParseIssuer (const char *currency) {
    // Parse: "<blockchain-id>:<issuer>"
    return 1 + strrchr (currency, ':');
}
#endif

bool strPrefix(const char *pre, const char *str)
{
    return strncmp (pre, str, strlen(pre)) == 0;
}

static void
cryptoWalletManagerRecoverTransferFromTransferBundleETH (BRCryptoWalletManager manager,
                                                         OwnershipKept BRCryptoClientTransferBundle bundle) {
    BRCryptoWalletManagerETH managerETH = cryptoWalletManagerCoerceETH (manager);
    (void) managerETH;

    BRCryptoNetwork network = cryptoWalletManagerGetNetwork (manager);

    // We'll only have a `walletCurrency` if the bundle->currency is for ETH or from an ERC20 token
    // that is known by `network`.  If `bundle` indicates a `transfer` that we sent and we do not
    // know about the ERC20 token we STILL MUST process the fee and the nonce.
    BRCryptoCurrency currency = cryptoNetworkGetCurrencyForUids (network, bundle->currency);

    // If we have a currency, ensure that we also have an ERC20 token
    if (NULL != currency)
        cryptoWalletManagerEnsureTokenForCurrency (managerETH, currency);

    UInt256  amountETH;
    uint64_t gasLimit;
    uint64_t gasUsed;
    UInt256  gasPrice;
    uint64_t nonce;

    bool error = false;
    cwmExtractAttributes (bundle, &amountETH, &gasLimit, &gasUsed, &gasPrice, &nonce, &error);

    if (error) {
        printf ("SYS: ETH: Bundle Attribute Error - Want to FATAL: %s\n", bundle->uids);
        cryptoCurrencyGive(currency);
        cryptoNetworkGive(network);
        return;
    }

    // The Ethereum Account.
    BREthereumAccount accountETH = managerETH->account;

    // The primary wallet always holds transfers for fees paid.
    BRCryptoWallet primaryWallet = manager->wallet;
    assert (NULL != primaryWallet);

    // The wallet holds currency transfers.
    BRCryptoWallet wallet = (NULL == currency ? NULL : cryptoWalletManagerCreateWallet (manager, currency));

    // Get the confirmed feeBasis which we'll use even if the transfer is already known.
    BREthereumFeeBasis feeBasisConfirmedETH = ethFeeBasisCreate (ethGasCreate(gasUsed), ethGasPriceCreate(ethEtherCreate(gasPrice)));
    BRCryptoFeeBasis   feeBasisConfirmed    = cryptoFeeBasisCreateAsETH (primaryWallet->unitForFee, feeBasisConfirmedETH);

    // Derive the transfer's state
    BRCryptoTransferState state = cryptoClientTransferBundleGetTransferState (bundle, feeBasisConfirmed);

    // Get the hash; we'll use it to find a pre-existing transfer in wallet or primaryWallet
    BRCryptoHash hash = cryptoNetworkCreateHashFromString (network, bundle->hash);

    // We'll create or find a transfer for the bundle
    BRCryptoTransfer transfer = NULL;

    // Look for a transfer in the wallet for currency
    if (NULL != wallet)
        transfer = cryptoWalletGetTransferByHash (wallet, hash);

    // If there isn't a wallet, the currency is unknown, look in the primaryWallet (for a fee)
    else
        transfer = cryptoWalletGetTransferByHash (primaryWallet, hash);

    // If we have a transfer, simply update its state
    if (NULL != transfer) {
        cryptoTransferSetState (transfer, state);
    }

    else {
        BRCryptoAddress source = cryptoNetworkCreateAddress (network, bundle->from);
        BRCryptoAddress target = cryptoNetworkCreateAddress (network, bundle->to);

        BREthereumFeeBasis feeBasisEstimatedETH = ethFeeBasisCreate (ethGasCreate(gasLimit), ethGasPriceCreate(ethEtherCreate(gasPrice)));
        BRCryptoFeeBasis   feeBasisEstimated = cryptoFeeBasisCreateAsETH (primaryWallet->unitForFee, feeBasisEstimatedETH);

        BRCryptoAmount amount = NULL;

        // If we have a currency, then create an amount
        if (NULL != currency) {
            BRCryptoUnit   amountUnit = cryptoNetworkGetUnitAsDefault (network, currency);
            amount = cryptoAmountCreate (amountUnit, CRYPTO_FALSE, amountETH);
            cryptoUnitGive(amountUnit);
        }

        // We pay the fee
        bool paysFee = (ETHEREUM_BOOLEAN_TRUE == ethAccountHasAddress (accountETH, ethAddressCreate(bundle->from)));

        // If we pay the fee but don't have a currency, then we'll need a transfer with a zero amount.
        if (NULL == amount && paysFee)
            amount = cryptoAmountCreateInteger(0, primaryWallet->unit);

        // If we have a currency or pay the fee, we'll need a transfer
        if (NULL != currency || paysFee) {
            BRCryptoWallet transfersPrimaryWallet = (NULL != wallet ? wallet : primaryWallet);

            // Finally create a transfer
            transfer = cryptoTransferCreateAsETH (transfersPrimaryWallet->listenerTransfer,
                                                  hash,
                                                  transfersPrimaryWallet->unit,
                                                  transfersPrimaryWallet->unitForFee,
                                                  feeBasisEstimated,
                                                  amount,
                                                  source,
                                                  target,
                                                  state,
                                                  accountETH,
                                                  nonce,
                                                  NULL);

            // The transfer's primaryWallet holds the transfer
            cryptoWalletAddTransfer (transfersPrimaryWallet, transfer);

            // If we pay the fee, then the manager's primaryWallet holds the transfer too.
            if (paysFee && transfersPrimaryWallet != primaryWallet)
                cryptoWalletAddTransfer (primaryWallet, transfer);

#if defined (NEVER_DEFINED)
            // if we pay the fee, then we send the transfer, update the Ethereum Account's nonce
            if (paysFee) {
                ethAccountSetAddressNonce (accountETH,
                                           ethAccountGetPrimaryAddress(accountETH),
                                           nonce + 1, // next Nonce
                                           ETHEREUM_BOOLEAN_FALSE);
                printf ("DBG: Nonce: try: %llu, now: %llu\n", nonce + 1, ethAccountGetAddressNonce(accountETH, ethAccountGetPrimaryAddress(accountETH)));
            }
#endif
        }

        cryptoAmountGive (amount);
        cryptoFeeBasisGive (feeBasisEstimated);
        cryptoAddressGive (target);
        cryptoAddressGive (source);
    }

    cryptoTransferGive (transfer);
    cryptoHashGive (hash);
    cryptoTransferStateGive (state);
    cryptoFeeBasisGive (feeBasisConfirmed);
    cryptoCurrencyGive (currency);
    cryptoNetworkGive (network);
}

BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersETH = {
    cryptoWalletManagerCreateETH,
    cryptoWalletManagerReleaseETH,
    cryptoWalletManagerCreateFileServiceETH,
    cryptoWalletManagerGetEventTypesETH,
    cryptoWalletManagerCreateP2PManagerETH,
    cryptoWalletManagerCreateWalletETH,
    cryptoWalletManagerSignTransactionWithSeedETH,
    cryptoWalletManagerSignTransactionWithKeyETH,
    cryptoWalletManagerEstimateLimitETH,
    cryptoWalletManagerEstimateFeeBasisETH,
    NULL, // BRCryptoWalletManagerSaveTransactionBundleHandler
    NULL, // BRCryptoWalletManagerSaveTransactionBundleHandler
    cryptoWalletManagerRecoverTransfersFromTransactionBundleETH,
    cryptoWalletManagerRecoverTransferFromTransferBundleETH,
    cryptoWalletManagerRecoverFeeBasisFromFeeEstimateETH,
    NULL,//BRCryptoWalletManagerWalletSweeperValidateSupportedHandler not supported
    NULL,//BRCryptoWalletManagerCreateWalletSweeperHandler not supported
};
