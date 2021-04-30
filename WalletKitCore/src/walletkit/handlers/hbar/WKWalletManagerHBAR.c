//
//  WKWalletManagerHBAR.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKHBAR.h"

#include "walletkit/WKAccountP.h"
#include "walletkit/WKNetworkP.h"
#include "walletkit/WKKeyP.h"
#include "walletkit/WKClientP.h"
#include "walletkit/WKWalletP.h"
#include "walletkit/WKAmountP.h"
#include "walletkit/WKWalletManagerP.h"
#include "walletkit/WKFileService.h"

#include "hedera/BRHederaAccount.h"


// MARK: - Events

static const BREventType *hbarEventTypes[] = {
    WK_CLIENT_EVENT_TYPES
};

static const unsigned int
hbarEventTypesCount = (sizeof (hbarEventTypes) / sizeof (BREventType*));

// MARK: - Handlers

static WKWalletManager
wkWalletManagerCreateHBAR (WKWalletManagerListener listener,
                               WKClient client,
                               WKAccount account,
                               WKNetwork network,
                               WKSyncMode mode,
                               WKAddressScheme scheme,
                               const char *path) {
    return wkWalletManagerAllocAndInit (sizeof (struct WKWalletManagerHBARRecord),
                                            wkNetworkGetType(network),
                                            listener,
                                            client,
                                            account,
                                            network,
                                            scheme,
                                            path,
                                            WK_CLIENT_REQUEST_USE_TRANSFERS,
                                            NULL,
                                            NULL);
}

static void
wkWalletManagerReleaseHBAR (WKWalletManager manager) {
    
}

static BRFileService
crytpWalletManagerCreateFileServiceHBAR (WKWalletManager manager,
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
wkWalletManagerGetEventTypesHBAR (WKWalletManager manager,
                                      size_t *eventTypesCount) {
    assert (NULL != eventTypesCount);
    *eventTypesCount = hbarEventTypesCount;
    return hbarEventTypes;
}

static WKClientP2PManager
crytpWalletManagerCreateP2PManagerHBAR (WKWalletManager manager) {
    // not supported
    return NULL;
}

static WKBoolean
wkWalletManagerSignTransactionWithSeedHBAR (WKWalletManager manager,
                                                WKWallet wallet,
                                                WKTransfer transfer,
                                                UInt512 seed) {
    BRHederaAccount account = wkAccountAsHBAR (manager->account);
    BRKey publicKey = hederaAccountGetPublicKey (account);
    BRHederaTransaction transaction = wkTransferCoerceHBAR(transfer)->hbarTransaction;
    // BRHederaAddress nodeAddress = hederaAccountGetNodeAddress(account);
    BRHederaAddress nodeAddress = NULL;

    size_t tx_size = hederaTransactionSignTransaction (transaction, publicKey, seed, nodeAddress);

    if (nodeAddress) hederaAddressFree(nodeAddress);
    return AS_WK_BOOLEAN(tx_size > 0);
}

static WKBoolean
wkWalletManagerSignTransactionWithKeyHBAR (WKWalletManager manager,
                                               WKWallet wallet,
                                               WKTransfer transfer,
                                               WKKey key) {
    assert(0);
    return WK_FALSE;
}

//TODO:HBAR make common?
static WKAmount
wkWalletManagerEstimateLimitHBAR (WKWalletManager manager,
                                      WKWallet  wallet,
                                      WKBoolean asMaximum,
                                      WKAddress target,
                                      WKNetworkFee networkFee,
                                      WKBoolean *needEstimate,
                                      WKBoolean *isZeroIfInsuffientFunds,
                                      WKUnit unit) {
    UInt256 amount = UINT256_ZERO;
    
    *needEstimate = WK_FALSE;
    *isZeroIfInsuffientFunds = WK_FALSE;
    
    if (WK_TRUE == asMaximum) {
        WKAmount minBalance = wallet->balanceMinimum;
        assert(minBalance);
        
        // Available balance based on minimum wallet balance
        WKAmount balance = wkAmountSub(wallet->balance, minBalance);
        
        // Hedera has fixed network fee (costFactor = 1.0)
        WKAmount fee = wkNetworkFeeGetPricePerCostFactor (networkFee);
        WKAmount newBalance = wkAmountSub(balance, fee);
        
        if (WK_TRUE == wkAmountIsNegative(newBalance)) {
            amount = UINT256_ZERO;
        } else {
            amount = wkAmountGetValue(newBalance);
        }
        
        wkAmountGive (balance);
        wkAmountGive (fee);
        wkAmountGive (newBalance);
    }
    
    return wkAmountCreate (unit, WK_FALSE, amount);
}

static WKFeeBasis
wkWalletManagerEstimateFeeBasisHBAR (WKWalletManager manager,
                                         WKWallet  wallet,
                                         WKCookie cookie,
                                         WKAddress target,
                                         WKAmount amount,
                                         WKNetworkFee networkFee,
                                         size_t attributesCount,
                                         OwnershipKept WKTransferAttribute *attributes) {
    UInt256 value = wkAmountGetValue (wkNetworkFeeGetPricePerCostFactor (networkFee));
    BRHederaFeeBasis hbarFeeBasis;
    hbarFeeBasis.pricePerCostFactor = (BRHederaUnitTinyBar) value.u64[0];
    hbarFeeBasis.costFactor = 1;  // 'cost factor' is 'transaction'
    
    return wkFeeBasisCreateAsHBAR (wallet->unitForFee, hbarFeeBasis);
}

static void
wkWalletManagerRecoverTransfersFromTransactionBundleHBAR (WKWalletManager manager,
                                                              OwnershipKept WKClientTransactionBundle bundle) {
    // Not Hedera functionality
    assert (0);
}

static void
wkWalletManagerRecoverTransferFromTransferBundleHBAR (WKWalletManager manager,
                                                          OwnershipKept WKClientTransferBundle bundle) {
    // create BRHederaTransaction
    
    BRHederaAccount hbarAccount = wkAccountAsHBAR (manager->account);
    
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

    int error = (WK_TRANSFER_STATE_ERRORED == bundle->status);

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

    // create WKTransfer
    
    WKWallet wallet = wkWalletManagerGetWallet (manager);
    WKHash hash = wkHashCreateAsHBAR (txHash);

    WKTransfer baseTransfer = wkWalletGetTransferByHash (wallet, hash);
    wkHashGive(hash);

    WKFeeBasis      feeBasis = wkFeeBasisCreateAsHBAR (wallet->unit, hederaTransactionGetFeeBasis(hbarTransaction));
    WKTransferState state    = wkClientTransferBundleGetTransferState (bundle, feeBasis);

    if (NULL == baseTransfer) {
        baseTransfer = wkTransferCreateAsHBAR (wallet->listenerTransfer,
                                                   wallet->unit,
                                                   wallet->unitForFee,
                                                   state,
                                                   hbarAccount,
                                                   hbarTransaction);
        hbarTransactionNeedFree = false;

        wkWalletAddTransfer (wallet, baseTransfer);
    }
    else {
        wkTransferSetState (baseTransfer, state);
    }
    
    wkWalletManagerRecoverTransferAttributesFromTransferBundle (wallet, baseTransfer, bundle);
    
    wkTransferGive(baseTransfer);
    wkFeeBasisGive (feeBasis);
    wkTransferStateGive (state);

    if (hbarTransactionNeedFree)
        hederaTransactionFree (hbarTransaction);
}

extern WKWalletSweeperStatus
wkWalletManagerWalletSweeperValidateSupportedHBAR (WKWalletManager manager,
                                                       WKWallet wallet,
                                                       WKKey key) {
    return WK_WALLET_SWEEPER_UNSUPPORTED_CURRENCY;
}

extern WKWalletSweeper
wkWalletManagerCreateWalletSweeperHBAR (WKWalletManager manager,
                                            WKWallet wallet,
                                            WKKey key) {
    // not supported
    return NULL;
}

static WKWallet
wkWalletManagerCreateWalletHBAR (WKWalletManager manager,
                                     WKCurrency currency,
                                     Nullable OwnershipKept BRArrayOf(WKClientTransactionBundle) transactions,
                                     Nullable OwnershipKept BRArrayOf(WKClientTransferBundle) transfers) {
    BRHederaAccount hbarAccount = wkAccountAsHBAR(manager->account);

    // Create the primary WKWallet
    WKNetwork  network       = manager->network;
    WKUnit     unitAsBase    = wkNetworkGetUnitAsBase    (network, currency);
    WKUnit     unitAsDefault = wkNetworkGetUnitAsDefault (network, currency);

    WKWallet wallet = wkWalletCreateAsHBAR (manager->listenerWallet,
                                                      unitAsDefault,
                                                      unitAsDefault,
                                                      hbarAccount);
    wkWalletManagerAddWallet (manager, wallet);

    //TODO:HBAR load transfers from fileService

    wkUnitGive (unitAsDefault);
    wkUnitGive (unitAsBase);

    return wallet;
}

WKWalletManagerHandlers wkWalletManagerHandlersHBAR = {
    wkWalletManagerCreateHBAR,
    wkWalletManagerReleaseHBAR,
    crytpWalletManagerCreateFileServiceHBAR,
    wkWalletManagerGetEventTypesHBAR,
    crytpWalletManagerCreateP2PManagerHBAR,
    wkWalletManagerCreateWalletHBAR,
    wkWalletManagerSignTransactionWithSeedHBAR,
    wkWalletManagerSignTransactionWithKeyHBAR,
    wkWalletManagerEstimateLimitHBAR,
    wkWalletManagerEstimateFeeBasisHBAR,
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    wkWalletManagerRecoverTransfersFromTransactionBundleHBAR,
    wkWalletManagerRecoverTransferFromTransferBundleHBAR,
    NULL,//WKWalletManagerRecoverFeeBasisFromFeeEstimateHandler not supported
    wkWalletManagerWalletSweeperValidateSupportedHBAR,
    wkWalletManagerCreateWalletSweeperHBAR
};
