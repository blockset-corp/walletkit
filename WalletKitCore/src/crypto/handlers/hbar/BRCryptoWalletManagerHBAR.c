//
//  BRCryptoWalletManagerHBAR.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoHBAR.h"

#include "crypto/BRCryptoAccountP.h"
#include "crypto/BRCryptoNetworkP.h"
#include "crypto/BRCryptoKeyP.h"
#include "crypto/BRCryptoClientP.h"
#include "crypto/BRCryptoWalletP.h"
#include "crypto/BRCryptoAmountP.h"
#include "crypto/BRCryptoWalletManagerP.h"
#include "crypto/BRCryptoFileService.h"

#include "hedera/BRHederaAccount.h"


// MARK: - Events

static const BREventType *hbarEventTypes[] = {
    CRYPTO_CLIENT_EVENT_TYPES
};

static const unsigned int
hbarEventTypesCount = (sizeof (hbarEventTypes) / sizeof (BREventType*));

// MARK: - Handlers

static BRCryptoWalletManager
cryptoWalletManagerCreateHBAR (BRCryptoWalletManagerListener listener,
                               BRCryptoClient client,
                               BRCryptoAccount account,
                               BRCryptoNetwork network,
                               BRCryptoSyncMode mode,
                               BRCryptoAddressScheme scheme,
                               const char *path) {
    return cryptoWalletManagerAllocAndInit (sizeof (struct BRCryptoWalletManagerHBARRecord),
                                            cryptoNetworkGetType(network),
                                            listener,
                                            client,
                                            account,
                                            network,
                                            scheme,
                                            path,
                                            CRYPTO_CLIENT_REQUEST_USE_TRANSFERS,
                                            NULL,
                                            NULL);
}

static void
cryptoWalletManagerReleaseHBAR (BRCryptoWalletManager manager) {
    
}

static BRFileService
crytpWalletManagerCreateFileServiceHBAR (BRCryptoWalletManager manager,
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
cryptoWalletManagerGetEventTypesHBAR (BRCryptoWalletManager manager,
                                      size_t *eventTypesCount) {
    assert (NULL != eventTypesCount);
    *eventTypesCount = hbarEventTypesCount;
    return hbarEventTypes;
}

static BRCryptoClientP2PManager
crytpWalletManagerCreateP2PManagerHBAR (BRCryptoWalletManager manager) {
    // not supported
    return NULL;
}

static BRCryptoBoolean
cryptoWalletManagerSignTransactionWithSeedHBAR (BRCryptoWalletManager manager,
                                                BRCryptoWallet wallet,
                                                BRCryptoTransfer transfer,
                                                UInt512 seed) {
    BRHederaAccount account = cryptoAccountAsHBAR (manager->account);
    BRKey publicKey = hederaAccountGetPublicKey (account);
    BRHederaTransaction transaction = cryptoTransferCoerceHBAR(transfer)->hbarTransaction;
    // BRHederaAddress nodeAddress = hederaAccountGetNodeAddress(account);
    BRHederaAddress nodeAddress = NULL;

    size_t tx_size = hederaTransactionSignTransaction (transaction, publicKey, seed, nodeAddress);

    if (nodeAddress) hederaAddressFree(nodeAddress);
    return AS_CRYPTO_BOOLEAN(tx_size > 0);
}

static BRCryptoBoolean
cryptoWalletManagerSignTransactionWithKeyHBAR (BRCryptoWalletManager manager,
                                               BRCryptoWallet wallet,
                                               BRCryptoTransfer transfer,
                                               BRCryptoKey key) {
    assert(0);
    return CRYPTO_FALSE;
}

//TODO:HBAR make common?
static BRCryptoAmount
cryptoWalletManagerEstimateLimitHBAR (BRCryptoWalletManager manager,
                                      BRCryptoWallet  wallet,
                                      BRCryptoBoolean asMaximum,
                                      BRCryptoAddress target,
                                      BRCryptoNetworkFee networkFee,
                                      BRCryptoBoolean *needEstimate,
                                      BRCryptoBoolean *isZeroIfInsuffientFunds,
                                      BRCryptoUnit unit) {
    UInt256 amount = UINT256_ZERO;
    
    *needEstimate = CRYPTO_FALSE;
    *isZeroIfInsuffientFunds = CRYPTO_FALSE;
    
    if (CRYPTO_TRUE == asMaximum) {
        BRCryptoAmount minBalance = wallet->balanceMinimum;
        assert(minBalance);
        
        // Available balance based on minimum wallet balance
        BRCryptoAmount balance = cryptoAmountSub(wallet->balance, minBalance);
        
        // Hedera has fixed network fee (costFactor = 1.0)
        BRCryptoAmount fee = cryptoNetworkFeeGetPricePerCostFactor (networkFee);
        BRCryptoAmount newBalance = cryptoAmountSub(balance, fee);
        
        if (CRYPTO_TRUE == cryptoAmountIsNegative(newBalance)) {
            amount = UINT256_ZERO;
        } else {
            amount = cryptoAmountGetValue(newBalance);
        }
        
        cryptoAmountGive (balance);
        cryptoAmountGive (fee);
        cryptoAmountGive (newBalance);
    }
    
    return cryptoAmountCreate (unit, CRYPTO_FALSE, amount);
}

static BRCryptoFeeBasis
cryptoWalletManagerEstimateFeeBasisHBAR (BRCryptoWalletManager manager,
                                         BRCryptoWallet  wallet,
                                         BRCryptoCookie cookie,
                                         BRCryptoAddress target,
                                         BRCryptoAmount amount,
                                         BRCryptoNetworkFee networkFee,
                                         size_t attributesCount,
                                         OwnershipKept BRCryptoTransferAttribute *attributes) {
    UInt256 value = cryptoAmountGetValue (cryptoNetworkFeeGetPricePerCostFactor (networkFee));
    BRHederaFeeBasis hbarFeeBasis;
    hbarFeeBasis.pricePerCostFactor = (BRHederaUnitTinyBar) value.u64[0];
    hbarFeeBasis.costFactor = 1;  // 'cost factor' is 'transaction'
    
    return cryptoFeeBasisCreateAsHBAR (wallet->unitForFee, hbarFeeBasis);
}

static void
cryptoWalletManagerRecoverTransfersFromTransactionBundleHBAR (BRCryptoWalletManager manager,
                                                              OwnershipKept BRCryptoClientTransactionBundle bundle) {
    // Not Hedera functionality
    assert (0);
}

static void
cryptoWalletManagerRecoverTransferFromTransferBundleHBAR (BRCryptoWalletManager manager,
                                                          OwnershipKept BRCryptoClientTransferBundle bundle) {
    // create BRHederaTransaction
    
    BRHederaAccount hbarAccount = cryptoAccountAsHBAR (manager->account);
    
    BRHederaUnitTinyBar amountHbar, feeHbar = 0;
    sscanf(bundle->amount, "%" PRIi64, &amountHbar);
    if (NULL != bundle->fee) sscanf(bundle->fee, "%" PRIi64, &feeHbar);
    BRHederaAddress toAddress   = hederaAddressCreateFromString(bundle->to,   false);
    BRHederaAddress fromAddress = hederaAddressCreateFromString(bundle->from, false);
    // Convert the hash string to bytes
    BRHederaTransactionHash txHash;
    memset(txHash.bytes, 0x00, sizeof(txHash.bytes));
    if (bundle->hash != NULL) {
        assert(96 == strlen(bundle->hash));
        hexDecode(txHash.bytes, sizeof(txHash.bytes), bundle->hash, strlen(bundle->hash));
    }

    int error = (CRYPTO_TRANSFER_STATE_ERRORED == bundle->status);

    bool hbarTransactionNeedFree = true;
    BRHederaTransaction hbarTransaction = hederaTransactionCreate(fromAddress,
                                                                  toAddress,
                                                                  amountHbar,
                                                                  feeHbar,
                                                                  bundle->identifier,
                                                                  txHash,
                                                                  bundle->blockTimestamp,
                                                                  bundle->blockNumber,
                                                                  error);

    hederaAddressFree (toAddress);
    hederaAddressFree (fromAddress);

    // create BRCryptoTransfer
    
    BRCryptoWallet wallet = cryptoWalletManagerGetWallet (manager);
    BRCryptoHash hash = cryptoHashCreateAsHBAR (txHash);

    BRCryptoTransfer baseTransfer = cryptoWalletGetTransferByHashOrUIDS (wallet, hash, bundle->uids);
    cryptoHashGive(hash);

    BRCryptoFeeBasis      feeBasis = cryptoFeeBasisCreateAsHBAR (wallet->unit, hederaTransactionGetFeeBasis(hbarTransaction));
    BRCryptoTransferState state    = cryptoClientTransferBundleGetTransferState (bundle, feeBasis);

    if (NULL == baseTransfer) {
        baseTransfer = cryptoTransferCreateAsHBAR (wallet->listenerTransfer,
                                                   bundle->uids,
                                                   wallet->unit,
                                                   wallet->unitForFee,
                                                   state,
                                                   hbarAccount,
                                                   hbarTransaction);
        hbarTransactionNeedFree = false;

        cryptoWalletAddTransfer (wallet, baseTransfer);
    }
    else {
        cryptoTransferSetUids  (baseTransfer, bundle->uids);
        cryptoTransferSetState (baseTransfer, state);
    }
    
    cryptoWalletManagerRecoverTransferAttributesFromTransferBundle (wallet, baseTransfer, bundle);
    
    cryptoTransferGive(baseTransfer);
    cryptoFeeBasisGive (feeBasis);
    cryptoTransferStateGive (state);

    if (hbarTransactionNeedFree)
        hederaTransactionFree (hbarTransaction);
}

extern BRCryptoWalletSweeperStatus
cryptoWalletManagerWalletSweeperValidateSupportedHBAR (BRCryptoWalletManager manager,
                                                       BRCryptoWallet wallet,
                                                       BRCryptoKey key) {
    return CRYPTO_WALLET_SWEEPER_UNSUPPORTED_CURRENCY;
}

extern BRCryptoWalletSweeper
cryptoWalletManagerCreateWalletSweeperHBAR (BRCryptoWalletManager manager,
                                            BRCryptoWallet wallet,
                                            BRCryptoKey key) {
    // not supported
    return NULL;
}

static BRCryptoWallet
cryptoWalletManagerCreateWalletHBAR (BRCryptoWalletManager manager,
                                     BRCryptoCurrency currency,
                                     Nullable OwnershipKept BRArrayOf(BRCryptoClientTransactionBundle) transactions,
                                     Nullable OwnershipKept BRArrayOf(BRCryptoClientTransferBundle) transfers) {
    BRHederaAccount hbarAccount = cryptoAccountAsHBAR(manager->account);

    // Create the primary BRCryptoWallet
    BRCryptoNetwork  network       = manager->network;
    BRCryptoUnit     unitAsBase    = cryptoNetworkGetUnitAsBase    (network, currency);
    BRCryptoUnit     unitAsDefault = cryptoNetworkGetUnitAsDefault (network, currency);

    BRCryptoWallet wallet = cryptoWalletCreateAsHBAR (manager->listenerWallet,
                                                      unitAsDefault,
                                                      unitAsDefault,
                                                      hbarAccount);
    cryptoWalletManagerAddWallet (manager, wallet);

    //TODO:HBAR load transfers from fileService

    cryptoUnitGive (unitAsDefault);
    cryptoUnitGive (unitAsBase);

    return wallet;
}

BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersHBAR = {
    cryptoWalletManagerCreateHBAR,
    cryptoWalletManagerReleaseHBAR,
    crytpWalletManagerCreateFileServiceHBAR,
    cryptoWalletManagerGetEventTypesHBAR,
    crytpWalletManagerCreateP2PManagerHBAR,
    cryptoWalletManagerCreateWalletHBAR,
    cryptoWalletManagerSignTransactionWithSeedHBAR,
    cryptoWalletManagerSignTransactionWithKeyHBAR,
    cryptoWalletManagerEstimateLimitHBAR,
    cryptoWalletManagerEstimateFeeBasisHBAR,
    NULL, // BRCryptoWalletManagerSaveTransactionBundleHandler
    NULL, // BRCryptoWalletManagerSaveTransactionBundleHandler
    cryptoWalletManagerRecoverTransfersFromTransactionBundleHBAR,
    cryptoWalletManagerRecoverTransferFromTransferBundleHBAR,
    NULL,//BRCryptoWalletManagerRecoverFeeBasisFromFeeEstimateHandler not supported
    cryptoWalletManagerWalletSweeperValidateSupportedHBAR,
    cryptoWalletManagerCreateWalletSweeperHBAR
};
