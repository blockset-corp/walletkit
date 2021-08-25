//
//  WKWalletManagerETH.c
//  WalletKitCore
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKETH.h"

#include "walletkit/WKAmountP.h"
#include "walletkit/WKAccountP.h"
#include "walletkit/WKNetworkP.h"
#include "walletkit/WKKeyP.h"
#include "walletkit/WKClientP.h"
#include "walletkit/WKFileService.h"
#include "walletkit/WKWalletManagerP.h"

#include <ctype.h>

// MARK: - Forward Declarations

static void
wkWalletManagerCreateTokenForCurrency (WKWalletManagerETH manager,
                                           WKCurrency currency);

static void
wkWalletManagerCreateCurrencyForToken (WKWalletManagerETH managerETH,
                                           BREthereumToken token);

static void
wkWalletManagerCreateTokensForNetwork (WKWalletManagerETH manager,
                                           WKNetwork network);

static BREthereumToken
cryptoWalletManagerGetTokenETH (WKWalletManagerETH managerETH,
                                const BREthereumAddress *ethAddress) {
    BREthereumToken token;

    pthread_mutex_lock (&managerETH->base.lock);
    token = BRSetGet (managerETH->tokens, ethAddress);
    pthread_mutex_unlock (&managerETH->base.lock);

    return token;
}

static void
cryptoWalletManagerAddTokenETH (WKWalletManagerETH managerETH,
                                BREthereumToken ethToken) {
    pthread_mutex_lock (&managerETH->base.lock);
    BRSetAdd (managerETH->tokens, ethToken);
    pthread_mutex_unlock (&managerETH->base.lock);
}

// MARK: - Event Types

const BREventType *eventTypesETH[] = {
    WK_CLIENT_EVENT_TYPES
};
const unsigned int eventTypesCountETH = (sizeof (eventTypesETH) / sizeof (BREventType*));

// MARK: - Wallet Manager

extern WKWalletManagerETH
wkWalletManagerCoerceETH (WKWalletManager manager) {
    assert (WK_NETWORK_TYPE_ETH == manager->type);
    return (WKWalletManagerETH) manager;
}

typedef struct {
    BREthereumNetwork network;
    BREthereumAccount account;
    BRRlpCoder coder;
} WKWalletManagerCreateContextETH;

static void
wkWaleltMangerCreateCallbackETH (WKWalletManagerCreateContext context,
                                     WKWalletManager manager) {
    WKWalletManagerCreateContextETH *contextETH = (WKWalletManagerCreateContextETH*) context;
    WKWalletManagerETH managerETH = wkWalletManagerCoerceETH (manager);

    managerETH->network = contextETH->network;
    managerETH->account = contextETH->account;
    managerETH->coder   = contextETH->coder;
}

static WKWalletManager
wkWalletManagerCreateETH (WKWalletManagerListener listener,
                              WKClient client,
                              WKAccount account,
                              WKNetwork network,
                              WKSyncMode mode,
                              WKAddressScheme scheme,
                              const char *path) {
    WKWalletManagerCreateContextETH contextETH = {
        wkNetworkAsETH (network),
        (BREthereumAccount) wkAccountAs (account,
                                         WK_NETWORK_TYPE_ETH),
        rlpCoderCreate()
    };

    WKWalletManager manager = wkWalletManagerAllocAndInit (sizeof (struct WKWalletManagerETHRecord),
                                                                     wkNetworkGetType(network),
                                                                     listener,
                                                                     client,
                                                                     account,
                                                                     network,
                                                                     scheme,
                                                                     path,
                                                                     WK_CLIENT_REQUEST_USE_TRANSFERS,
                                                                     &contextETH,
                                                                     wkWaleltMangerCreateCallbackETH);
    WKWalletManagerETH managerETH = wkWalletManagerCoerceETH (manager);

    // Save the recovered tokens
    managerETH->tokens = ethTokenSetCreate (EWM_INITIAL_SET_SIZE_DEFAULT);

    // Ensure a token (but not a wallet) for each currency
    wkWalletManagerCreateTokensForNetwork (managerETH, network);

    // Ensure a currency for each token
    FOR_SET (BREthereumToken, token, managerETH->tokens) {
        wkWalletManagerCreateCurrencyForToken (managerETH, token);
    }

    return manager;
}

static void
wkWalletManagerReleaseETH (WKWalletManager manager) {
    WKWalletManagerETH managerETH = wkWalletManagerCoerceETH (manager);

    rlpCoderRelease (managerETH->coder);
    if (NULL != managerETH->tokens)
        BRSetFreeAll(managerETH->tokens, (void (*) (void*)) ethTokenRelease);
}

static BRFileService
wkWalletManagerCreateFileServiceETH (WKWalletManager manager,
                                         const char *basePath,
                                         const char *currency,
                                         const char *network,
                                         BRFileServiceContext context,
                                         BRFileServiceErrorHandler handler) {
    return fileServiceCreateFromTypeSpecifications (basePath, currency, network,
                                                    context, handler,
                                                    wkFileServiceSpecificationsCount,
                                                    wkFileServiceSpecifications);
}

static const BREventType **
wkWalletManagerGetEventTypesETH (WKWalletManager manager,
                                     size_t *eventTypesCount) {
    assert (NULL != eventTypesCount);
    *eventTypesCount = eventTypesCountETH;
    return eventTypesETH;
}

static WKBoolean
wkWalletManagerSignTransactionETH (WKWalletManager manager,
                                       WKWallet wallet,
                                       WKTransfer transfer,
                                       BRKey *key) {
    WKWalletManagerETH managerETH  = wkWalletManagerCoerceETH (manager);
    WKTransferETH      transferETH = wkTransferCoerceETH      (transfer);


    BREthereumNetwork     ethNetwork     = managerETH->network;
    BREthereumAccount     ethAccount     = managerETH->account;
    BREthereumAddress     ethAddress     = ethAccountGetPrimaryAddress (ethAccount);
    BREthereumTransaction ethTransaction = transferETH->originatingTransaction;

    assert (NULL != ethTransaction);

    if (ETHEREUM_TRANSACTION_NONCE_IS_NOT_ASSIGNED == wkTransferGetNonceETH (transferETH))
        wkTransferSetNonceETH (transferETH, ethAccountGetThenIncrementAddressNonce (ethAccount, ethAddress));

    // RLP Encode the UNSIGNED transaction
    BRRlpCoder coder = rlpCoderCreate();
    BRRlpItem item = ethTransactionRlpEncode (ethTransaction,
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
    ethTransactionSign (ethTransaction, signature);

    // RLP Encode the SIGNED transaction and then assign the hash.
    item = ethTransactionRlpEncode (ethTransaction,
                                 ethNetwork,
                                 RLP_TYPE_TRANSACTION_SIGNED,
                                 coder);
    ethTransactionSetHash (ethTransaction,
                        ethHashCreateFromData (rlpItemGetDataSharedDontRelease (coder, item)));

    pthread_mutex_lock (&transfer->lock);
    transferETH->hash = wkHashCreateAsETH(ethTransactionGetHash(ethTransaction));
    pthread_mutex_unlock (&transfer->lock);

    rlpItemRelease(coder, item);
    rlpCoderRelease(coder);

    return WK_TRUE;
}

static WKBoolean
wkWalletManagerSignTransactionWithSeedETH (WKWalletManager manager,
                                               WKWallet wallet,
                                               WKTransfer transfer,
                                               UInt512 seed) {
    WKWalletManagerETH managerETH  = wkWalletManagerCoerceETH (manager);

    BREthereumAccount     ethAccount     = managerETH->account;
    BREthereumAddress     ethAddress     = ethAccountGetPrimaryAddress (ethAccount);

    BRKey key = ethAccountDerivePrivateKeyFromSeed (seed, ethAccountGetAddressIndex (ethAccount, ethAddress));
    
    return wkWalletManagerSignTransactionETH (manager,
                                                  wallet,
                                                  transfer,
                                                  &key);
}

static WKBoolean
wkWalletManagerSignTransactionWithKeyETH (WKWalletManager manager,
                                              WKWallet wallet,
                                              WKTransfer transfer,
                                              WKKey key) {
    return wkWalletManagerSignTransactionETH (manager,
                                                  wallet,
                                                  transfer,
                                                  wkKeyGetCore (key));
}

static WKAmount
wkWalletManagerEstimateLimitETH (WKWalletManager cwm,
                                     WKWallet  wallet,
                                     WKBoolean asMaximum,
                                     WKAddress target,
                                     WKNetworkFee networkFee,
                                     WKBoolean *needEstimate,
                                     WKBoolean *isZeroIfInsuffientFunds,
                                     WKUnit unit) {
    // We only need a fee estimate if this IS the manager's primary wallet.  Otherwise, we are
    // transfering some ERC20 token (typically) and the limits are not impacted by the fees, which
    // are paid in ETH
    *needEstimate = AS_WK_BOOLEAN (wallet == cwm->wallet);

    return (WK_TRUE == asMaximum
            ? wkWalletGetBalance (wallet)        // Maximum is balance - fees 'needEstimate'
            : wkAmountCreateInteger (0, unit));  // No minimum
}

static WKFeeBasis
wkWalletManagerEstimateFeeBasisETH (WKWalletManager manager,
                                        WKWallet  wallet,
                                        WKCookie cookie,
                                        WKAddress target,
                                        WKAmount amount,
                                        WKNetworkFee networkFee,
                                        size_t attributesCount,
                                        OwnershipKept WKTransferAttribute *attributes) {
    WKWalletETH walletETH = wkWalletCoerce (wallet);

    BREthereumFeeBasis ethFeeBasis = {
        FEE_BASIS_GAS,
        { .gas = { walletETH->ethGasLimit, wkNetworkFeeAsETH (networkFee) }}
    };

    WKCurrency currency = wkAmountGetCurrency (amount);
    WKFeeBasis feeBasis = wkFeeBasisCreateAsETH (wallet->unitForFee, ethFeeBasis);

    WKTransfer transfer = wkWalletCreateTransferETH (wallet,
                                                               target,
                                                               amount,
                                                               feeBasis,
                                                               attributesCount,
                                                               attributes,
                                                               currency,
                                                               wallet->unit,
                                                               wallet->unitForFee);

    wkCurrencyGive(currency);

    wkClientQRYEstimateTransferFee (manager->qryManager,
                                        cookie,
                                        transfer,
                                        networkFee);

    wkTransferGive (transfer);
    wkFeeBasisGive (feeBasis);

    // Require QRY with cookie - made above
    return NULL;
}

static WKFeeBasis
wkWalletManagerRecoverFeeBasisFromFeeEstimateETH (WKWalletManager cwm,
                                                  WKTransfer transfer,
                                                  WKNetworkFee networkFee,
                                                  double costUnits,
                                                  size_t attributesCount,
                                                  OwnershipKept const char **attributeKeys,
                                                  OwnershipKept const char **attributeVals) {
    WKTransferETH transferETH = wkTransferCoerceETH(transfer);
    assert (NULL != transferETH->originatingTransaction);
    
    BREthereumTransaction ethTransaction = transferETH->originatingTransaction;
    BREthereumGas         ethGasLimit    = ethGasCreate((uint64_t) costUnits);

    // Apply a margin
    BREthereumGas ethGasLimitWithMargin  = ethTransactionApplyGasLimitMargin (ethTransaction, ethGasLimit);

    return wkFeeBasisCreateAsETH (networkFee->pricePerCostFactorUnit,
                                  ethFeeBasisCreate (ethGasLimitWithMargin, wkNetworkFeeAsETH (networkFee)));
}

static void
wkWalletManagerCreateTokenForCurrencyInternal (WKWalletManagerETH managerETH,
                                                   WKCurrency currency,
                                                   bool updateIfNeeded) {
    const char *address = wkCurrencyGetIssuer(currency);

    if (NULL == address || 0 == strlen(address)) return;
    if (ETHEREUM_BOOLEAN_FALSE == ethAddressValidateString(address)) return;

    BREthereumAddress addr = ethAddressCreate(address);

    // Check for an existing token
    BREthereumToken token = cryptoWalletManagerGetTokenETH(managerETH, &addr);

    if (NULL != token && !updateIfNeeded) return;

    const char *code = wkCurrencyGetCode (currency);
    const char *name = wkCurrencyGetName (currency);
    const char *desc = wkCurrencyGetUids (currency);

    WKUnit unitDefault = wkNetworkGetUnitAsDefault (managerETH->base.network, currency);
    unsigned int decimals = wkUnitGetBaseDecimalOffset(unitDefault);
    wkUnitGive(unitDefault);

    BREthereumGas      defaultGasLimit = ethGasCreate(ETHEREUM_TOKEN_BRD_DEFAULT_GAS_LIMIT);
    BREthereumGasPrice defaultGasPrice = ethGasPriceCreate(ethEtherCreate(uint256Create(ETHEREUM_TOKEN_BRD_DEFAULT_GAS_PRICE_IN_WEI_UINT64)));

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
wkWalletManagerEnsureTokenForCurrency (WKWalletManagerETH managerETH,
                                           WKCurrency currency) {
    wkWalletManagerCreateTokenForCurrencyInternal (managerETH, currency, false);
}

static void
wkWalletManagerCreateTokenForCurrency (WKWalletManagerETH managerETH,
                                           WKCurrency currency) {
    wkWalletManagerCreateTokenForCurrencyInternal (managerETH, currency, true);
}

static void
wkWalletManagerCreateTokensForNetwork (WKWalletManagerETH managerETH,
                                           WKNetwork network) {
    size_t currencyCount = wkNetworkGetCurrencyCount (network);
    for (size_t index = 0; index < currencyCount; index++) {
        WKCurrency c = wkNetworkGetCurrencyAt (network, index);
        if (c != network->currency)
            wkWalletManagerCreateTokenForCurrency (managerETH, c);
        wkCurrencyGive (c);
    }
}

static void
wkWalletManagerCreateCurrencyForToken (WKWalletManagerETH managerETH,
                                           BREthereumToken token) {
    WKNetwork network = managerETH->base.network;

    const char *issuer = ethTokenGetAddress (token);

    WKCurrency currency = wkNetworkGetCurrencyForIssuer(network, issuer);
    if (NULL == currency) {
        const char *tokenName = ethTokenGetName (token);
        const char *tokenSymb = ethTokenGetSymbol (token);
        const char *tokenAddr = ethTokenGetAddress (token);

        const char *networkUids = wkNetworkGetUids(network);

        size_t uidsCount = strlen(networkUids) + 1 + strlen(tokenAddr) + 1;
        char   uids [uidsCount];
        sprintf (uids, "%s:%s", networkUids, tokenAddr);

        currency = wkCurrencyCreate (uids,
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

        WKUnit baseUnit = wkUnitCreateAsBase (currency,
                                                        codeBase,
                                                        nameBase,
                                                        symbolBase);

        WKUnit defaultUnit = wkUnitCreate (currency,
                                                     code,
                                                     name,
                                                     symbol,
                                                     baseUnit,
                                                     ethTokenGetDecimals(token));

        wkNetworkAddCurrency (network, currency, baseUnit, defaultUnit);

        wkUnitGive(defaultUnit);
        wkUnitGive(baseUnit);
    }

    wkCurrencyGive (currency);
}

static WKClientP2PManager
wkWalletManagerCreateP2PManagerETH (WKWalletManager manager) {
    // unsupported
    return NULL;
}

static WKWallet
wkWalletManagerCreateWalletETH (WKWalletManager manager,
                                    WKCurrency currency,
                                    Nullable OwnershipKept BRArrayOf(WKClientTransactionBundle) transactions,
                                    Nullable OwnershipKept BRArrayOf(WKClientTransferBundle) transfers) {
    WKWalletManagerETH managerETH  = wkWalletManagerCoerceETH (manager);

    BREthereumToken ethToken = NULL;

    // If there is an issuer, then `currency` is for an ERC20 token; otherwise for 'Ether'
    const char *issuer = wkCurrencyGetIssuer (currency);
    if (NULL != issuer) {
        BREthereumAddress ethAddress = ethAddressCreate (issuer);
        ethToken = cryptoWalletManagerGetTokenETH(managerETH, &ethAddress);

        // If there isn't an existing token, then create one.  A token is expected to exist (there
        // used to be an assert here); however, we've seen a crash on a non-existent token so we'll
        // create a token as 'belt-and-suspenders'.  Still, the currency is required to be in
        // `manager->network`, which implies that there should be a token.

        if (NULL == ethToken) {
            printf ("SYS: ETH: No token for: %s\n", issuer);
            if (WK_TRUE != wkNetworkHasCurrency (manager->network, currency))
                printf ("SYS: ETH: No currency in network: %s\n", issuer);
            assert (WK_TRUE == wkNetworkHasCurrency (manager->network, currency));

            // Create the token
            wkWalletManagerCreateTokenForCurrency (managerETH, currency);

            // Must find one now, surely.
            ethToken = cryptoWalletManagerGetTokenETH (managerETH, &ethAddress);
        }
        assert (NULL != ethToken);
    }

    // The `currency` unit: e.g. ETH, BRD, DNT, etc for ETH or the ERC20 token
    WKUnit unit = wkNetworkGetUnitAsDefault (manager->network, currency);

    // The `unitForFee` is always the default/base unit for the network.
    WKUnit unitForFee = wkNetworkGetUnitAsDefault (manager->network, NULL);

    WKWallet wallet = wkWalletCreateAsETH (manager->listenerWallet,
                                                     unit,
                                                     unitForFee,
                                                     ethToken,
                                                     managerETH->account);
    wkWalletManagerAddWallet (manager, wallet);

    wkUnitGive (unitForFee);
    wkUnitGive (unit);

    return wallet;
}

static WKWalletETH
wkWalletManagerLookupWalletForToken (WKWalletManagerETH managerETH,
                                         BREthereumToken token) {
    if (NULL == token) return (WKWalletETH) wkWalletTake (managerETH->base.wallet);

    for (size_t index = 0; index < array_count(managerETH->base.wallets); index++) {
        WKWalletETH wallet = wkWalletCoerce (managerETH->base.wallets[index]);
        if (token == wallet->ethToken)
            return wallet;
    }
    return NULL;
}

private_extern WKWalletETH
wkWalletManagerEnsureWalletForToken (WKWalletManagerETH managerETH,
                                         BREthereumToken token) {
    if (NULL == token) return (WKWalletETH) managerETH->base.wallet;

    WKWallet wallet = (WKWallet) wkWalletManagerLookupWalletForToken (managerETH, token);

    if (NULL == wallet) {
        WKCurrency currency   = wkNetworkGetCurrencyForIssuer (managerETH->base.network, ethTokenGetAddress(token));
        WKUnit     unit       = wkNetworkGetUnitAsDefault     (managerETH->base.network, currency);
        WKUnit     unitForFee = wkNetworkGetUnitAsDefault     (managerETH->base.network, NULL);

        wallet = wkWalletCreateAsETH (managerETH->base.listenerWallet,
                                          unit,
                                          unitForFee,
                                          token,
                                          managerETH->account);
        assert (NULL != wallet);

        wkWalletManagerAddWallet (&managerETH->base, wallet);

        wkUnitGive (unitForFee);
        wkUnitGive (unit);
        wkCurrencyGive (currency);
    }

    return (WKWalletETH) wallet;
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
cwmExtractAttributes (OwnershipKept WKClientTransferBundle bundle,
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
wkWalletManagerRecoverTransaction (WKWalletManager manager,
                                       OwnershipKept WKClientTransferBundle bundle) {

    WKWalletManagerETH managerETH = wkWalletManagerCoerceETH (manager);

    UInt256  amount;
    uint64_t gasLimit;
    uint64_t gasUsed;
    UInt256  gasPrice;
    uint64_t nonce;

    bool error = false;
    cwmExtractAttributes (bundle, &amount, &gasLimit, &gasUsed, &gasPrice, &nonce, &error);
    if (error) return true;

    bool statusError = (WK_TRANSFER_STATE_ERRORED == bundle->status);

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
    BREthereumTransaction tid = ethTransactionCreate (sourceAddress,
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
    ethTransactionSetHash (tid, ethHashCreate (bundle->hash));

    uint64_t success = statusError ? 0 : 1;
    BREthereumTransactionStatus status = ethTransactionStatusCreateIncluded (ethHashCreate (bundle->blockHash),
                                                                          bundle->blockNumber,
                                                                          bundle->blockTransactionIndex,
                                                                          bundle->blockTimestamp,
                                                                          ethGasCreate(gasUsed),
                                                                          success);
    ethTransactionSetStatus (tid, status);

    // If we had a `bcs` we might think about `bcsSignalTransaction(ewm->bcs, transaction);`
#if defined (NEED_ETH_LES_SUPPORT)
    ewmHandleTransaction (managerETH, BCS_CALLBACK_TRANSACTION_UPDATED, tid);
#endif

    return false;
}
#endif // defined (INCLUDE_UNUSED_RecoverTransaction)

#if defined (INCLUDE_UNUSED_RecoverLog)
static bool // true if error
wkWalletManagerRecoverLog (WKWalletManager manager,
                               const char *contract,
                               OwnershipKept WKClientTransferBundle bundle) {
    WKWalletManagerETH managerETH = wkWalletManagerCoerceETH (manager);

    UInt256  amount;
    uint64_t gasLimit;
    uint64_t gasUsed;
    UInt256  gasPrice;
    uint64_t nonce;
    size_t   logIndex = 0;

    bool error = false;
    cwmExtractAttributes(bundle, &amount, &gasLimit, &gasUsed, &gasPrice, &nonce, &error);
    if (error) return true;

    bool statusError = (WK_TRANSFER_STATE_ERRORED == bundle->status);

    // On a `statusError`, until we understand the meaning, assume that the log can't be recovered.
    // Not that wkWalletManagerRecoverTransaction DOES NOT avoid the transaction; and thus
    // we'll get `nonce` and `fee` updates.

    // TODO: Handle `errorStatus`; which means what?
    if (statusError) return true;

    // TODO: Is `nonce` relevent here?  Or only in wkWalletManagerRecoverTransaction

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
            topics[index] = ethLogTopicCreateFromString(topicsStr[index]);

        free (topicsStr[1]);
        free (topicsStr[2]);
    }

    // In general, log->data is arbitrary data.  In the case of an ERC20 token, log->data
    // is a numeric value - for the transfer amount.  When parsing in ethLogRlpDecode(),
    // log->data is assigned with rlpDecodeBytes(coder, items[2]); we'll need the same
    // thing, somehow

    BRRlpItem  item  = rlpEncodeUInt256 (managerETH->coder, amount, 1);

    BREthereumLog log = ethLogCreate (ethAddressCreate (contract),
                                   topicsCount,
                                   topics,
                                   rlpItemGetDataSharedDontRelease (managerETH->coder, item));
    rlpItemRelease (managerETH->coder, item);

    // Given {hash,logIndex}, initialize the log's identifier
    assert (logIndex <= (uint64_t) SIZE_MAX);
    ethLogInitializeIdentifier(log, ethHashCreate (bundle->hash), logIndex);

    BREthereumTransactionStatus status =
    ethTransactionStatusCreateIncluded (ethHashCreate(bundle->blockHash),
                                     bundle->blockNumber,
                                     bundle->blockTransactionIndex,
                                     bundle->blockTimestamp,
                                     ethGasCreate(gasUsed),
                                     1); // failed transactions (statusError) are skipped above
    ethLogSetStatus (log, status);

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
wkWalletManagerRecoverExchange (WKWalletManager manager,
                                    const char *contract,
                                    OwnershipKept WKClientTransferBundle bundle) {
    WKWalletManagerETH managerETH = wkWalletManagerCoerceETH (manager);

    bool error;
    UInt256 amount = cwmParseUInt256 (bundle->amount, &error);
    if (error) return true;

    size_t exchangeIndex = 0;

    BREthereumExchange exchange = ethExchangeCreate (ethAddressCreate(bundle->from),
                                                     ethAddressCreate(bundle->to),
                                                     (NULL != contract
                                                      ? ethAddressCreate(contract)
                                                      : ETHEREUM_EMPTY_ADDRESS_INIT),
                                                     0, // contractdAssetIndex,
                                                     amount);

    ethExchangeInitializeIdentifier (exchange,
                                     ethHashCreate(bundle->hash),
                                     exchangeIndex);

    uint64_t success = (WK_TRANSFER_STATE_ERRORED == bundle->status) ? 0 : 1;
    BREthereumTransactionStatus status =
    ethTransactionStatusCreateIncluded (ethHashCreate(bundle->blockHash),
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
wkWalletManagerRecoverTransfersFromTransactionBundlesETH (WKWalletManager manager,
                                                         OwnershipKept BRArrayOf (WKClientTransactionBundle) bundles) {
    WKWalletManagerETH managerETH = wkWalletManagerCoerceETH (manager);
    (void) managerETH;
    assert (0);
}

#if defined (INCLUDE_UNUSED)
static const char *
wkWalletManagerParseIssuer (const char *currency) {
    // Parse: "<blockchain-id>:<issuer>"
    return 1 + strrchr (currency, ':');
}
#endif

bool strPrefix(const char *pre, const char *str)
{
    return strncmp (pre, str, strlen(pre)) == 0;
}

static void
wkWalletManagerRecoverTransferFromTransferBundleETH (WKWalletManager manager,
                                                         OwnershipKept WKClientTransferBundle bundle) {
    WKWalletManagerETH managerETH = wkWalletManagerCoerceETH (manager);
    (void) managerETH;

    WKNetwork network = wkWalletManagerGetNetwork (manager);

    // We'll only have a `walletCurrency` if the bundle->currency is for ETH or from an ERC20 token
    // that is known by `network`.  If `bundle` indicates a `transfer` that we sent and we do not
    // know about the ERC20 token we STILL MUST process the fee and the nonce.
    WKCurrency currency = wkNetworkGetCurrencyForUids (network, bundle->currency);

    // If we have a currency, ensure that we also have an ERC20 token
    if (NULL != currency)
        wkWalletManagerEnsureTokenForCurrency (managerETH, currency);

    // If we have a fee, we'll check if we sent the transfer and, if so, need a transfer
    bool hasFee = (NULL != bundle->fee);

    UInt256  amountETH;
    uint64_t gasLimit;
    uint64_t gasUsed;
    UInt256  gasPrice;
    uint64_t nonce;

    bool error = false;
    cwmExtractAttributes (bundle, &amountETH, &gasLimit, &gasUsed, &gasPrice, &nonce, &error);

    if (error) {
        printf ("SYS: ETH: Bundle Attribute Error - Want to FATAL: %s\n", bundle->uids);
        wkCurrencyGive(currency);
        wkNetworkGive(network);
        return;
    }

    // The Ethereum Account.
    BREthereumAccount accountETH = managerETH->account;

    // The primary wallet always holds transfers for fees paid.
    WKWallet primaryWallet = manager->wallet;
    assert (NULL != primaryWallet);

    // The wallet holds currency transfers.
    WKWallet wallet = (NULL == currency ? NULL : wkWalletManagerCreateWallet (manager, currency));

    // Get the confirmed feeBasis which we'll use even if the transfer is already known.
    BREthereumFeeBasis feeBasisConfirmedETH = ethFeeBasisCreate (ethGasCreate(gasUsed), ethGasPriceCreate(ethEtherCreate(gasPrice)));
    WKFeeBasis   feeBasisConfirmed    = wkFeeBasisCreateAsETH (primaryWallet->unitForFee, feeBasisConfirmedETH);

    // Derive the transfer's state
    WKTransferState state = wkClientTransferBundleGetTransferState (bundle, feeBasisConfirmed);

    // Get the hash; we'll use it to find a pre-existing transfer in wallet or primaryWallet
    WKHash hash = wkNetworkCreateHashFromString (network, bundle->hash);

    // We'll create or find a transfer for the bundle uids
    WKTransfer transfer = wkWalletGetTransferByHashOrUIDS ((NULL != wallet ? wallet : primaryWallet), hash, bundle->uids);

    // If we have a transfer, simply update its state and the nonce
    if (NULL != transfer) {
        // Get the transferETH so as to check for a nonce update
        WKTransferETH transferETH = wkTransferCoerceETH(transfer);

        // Get the UIDS
        wkTransferSetUids (transfer, bundle->uids);

        // Compare the current nonce with the transfer's.
        bool nonceChanged = (nonce != wkTransferGetNonceETH(transferETH));

        // Update the nonce if it has chanaged
        if (nonceChanged) wkTransferSetNonceETH (transferETH, nonce);

        // On a state change the wallet will be updated.
        wkTransferSetStateForced (transfer, state, nonceChanged);
    }

    else {
        WKAmount amount = NULL;

        // If we have a currency, then create an amount
        if (NULL != currency) {
            WKUnit   amountUnit = wkNetworkGetUnitAsDefault (network, currency);
            amount = wkAmountCreate (amountUnit, WK_FALSE, amountETH);
            wkUnitGive(amountUnit);
        }

        // If we pay the fee, we'll need a transfer in the primaryWallet.
        bool paysFee = hasFee && (ETHEREUM_BOOLEAN_TRUE == ethAccountHasAddress (accountETH, ethAddressCreate(bundle->from)));

        // If we pay the fee but don't have a currency, then we'll need a transfer with a zero amount.
        if (NULL == amount && paysFee)
            amount = wkAmountCreateInteger(0, primaryWallet->unit);

        // If we have a currency or pay the fee, we'll need a transfer
        if (NULL != currency || paysFee) {
            WKAddress source = wkNetworkCreateAddress (network, bundle->from);
            WKAddress target = wkNetworkCreateAddress (network, bundle->to);

            BREthereumFeeBasis feeBasisEstimatedETH = ethFeeBasisCreate (ethGasCreate(gasLimit), ethGasPriceCreate(ethEtherCreate(gasPrice)));
            WKFeeBasis   feeBasisEstimated = wkFeeBasisCreateAsETH (primaryWallet->unitForFee, feeBasisEstimatedETH);

            WKWallet transfersPrimaryWallet = (NULL != wallet ? wallet : primaryWallet);

            // Finally create a transfer
            transfer = wkTransferCreateAsETH (transfersPrimaryWallet->listenerTransfer,
                                                  bundle->uids,
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
            wkWalletAddTransfer (transfersPrimaryWallet, transfer);

            // If we pay the fee, then the manager's primaryWallet holds the transfer too.
            if (paysFee && transfersPrimaryWallet != primaryWallet)
                wkWalletAddTransfer (primaryWallet, transfer);

            wkFeeBasisGive (feeBasisEstimated);
            wkAddressGive (target);
            wkAddressGive (source);

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

        wkAmountGive (amount);
    }

    wkTransferGive (transfer);
    wkHashGive (hash);
    wkTransferStateGive (state);
    wkFeeBasisGive (feeBasisConfirmed);
    wkCurrencyGive (currency);
    wkNetworkGive (network);
}

static void
wkWalletManagerRecoverTransfersFromTransferBundlesETH (WKWalletManager manager,
                                                       OwnershipKept BRArrayOf (WKClientTransferBundle) bundles) {
    for (size_t index = 0; index < array_count(bundles); index++)
        wkWalletManagerRecoverTransferFromTransferBundleETH (manager, bundles[index]);
}

WKWalletManagerHandlers wkWalletManagerHandlersETH = {
    wkWalletManagerCreateETH,
    wkWalletManagerReleaseETH,
    wkWalletManagerCreateFileServiceETH,
    wkWalletManagerGetEventTypesETH,
    wkWalletManagerCreateP2PManagerETH,
    wkWalletManagerCreateWalletETH,
    wkWalletManagerSignTransactionWithSeedETH,
    wkWalletManagerSignTransactionWithKeyETH,
    wkWalletManagerEstimateLimitETH,
    wkWalletManagerEstimateFeeBasisETH,
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    wkWalletManagerRecoverTransfersFromTransactionBundlesETH,
    wkWalletManagerRecoverTransfersFromTransferBundlesETH,
    wkWalletManagerRecoverFeeBasisFromFeeEstimateETH,
    NULL,//WKWalletManagerWalletSweeperValidateSupportedHandler not supported
    NULL,//WKWalletManagerCreateWalletSweeperHandler not supported
};
