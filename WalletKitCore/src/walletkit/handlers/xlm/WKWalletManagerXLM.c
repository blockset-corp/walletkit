//
//  WKWalletManagerXLM.c
//  WalletKitCore
//
//  Created by Carl Cherry on 2021-05-19.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXLM.h"

#include "walletkit/WKAccountP.h"
#include "walletkit/WKNetworkP.h"
#include "walletkit/WKKeyP.h"
#include "walletkit/WKClientP.h"
#include "walletkit/WKWalletP.h"
#include "walletkit/WKAmountP.h"
#include "walletkit/WKWalletManagerP.h"
#include "walletkit/WKFileService.h"

#include "stellar/BRStellarAccount.h"


// MARK: - Events

static const BREventType *xlmEventTypes[] = {
    WK_CLIENT_EVENT_TYPES
};

static const unsigned int
xlmEventTypesCount = (sizeof (xlmEventTypes) / sizeof (BREventType*));

// MARK: - Handlers

static WKWalletManager
wkWalletManagerCreateXLM (WKWalletManagerListener listener,
                               WKClient client,
                               WKAccount account,
                               WKNetwork network,
                               WKSyncMode mode,
                               WKAddressScheme scheme,
                               const char *path) {
    return wkWalletManagerAllocAndInit (sizeof (struct WKWalletManagerXLMRecord),
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
wkWalletManagerReleaseXLM (WKWalletManager manager) {
    
}

static BRFileService
crytpWalletManagerCreateFileServiceXLM (WKWalletManager manager,
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
wkWalletManagerGetEventTypesXLM (WKWalletManager manager,
                                      size_t *eventTypesCount) {
    assert (NULL != eventTypesCount);
    *eventTypesCount = xlmEventTypesCount;
    return xlmEventTypes;
}

static WKClientP2PManager
crytpWalletManagerCreateP2PManagerXLM (WKWalletManager manager) {
    // not supported
    return NULL;
}

static WKBoolean
wkWalletManagerSignTransactionWithSeedXLM (WKWalletManager manager,
                                                WKWallet wallet,
                                                WKTransfer transfer,
                                                UInt512 seed) {
    BRStellarAccount account = (BRStellarAccount) wkAccountAs (manager->account,
                                                               WK_NETWORK_TYPE_XLM);
    BRKey publicKey = stellarAccountGetPublicKey (account);
    BRStellarTransaction transaction = wkTransferCoerceXLM(transfer)->xlmTransaction;

    // TODO - carl
    size_t tx_size = 0; //stellarTransactionSignTransaction (transaction, publicKey, seed, nodeAddress);

    return AS_WK_BOOLEAN(tx_size > 0);
}

static WKBoolean
wkWalletManagerSignTransactionWithKeyXLM (WKWalletManager manager,
                                               WKWallet wallet,
                                               WKTransfer transfer,
                                               WKKey key) {
    assert(0);
    return WK_FALSE;
}

//TODO:XLM make common?
static WKAmount
wkWalletManagerEstimateLimitXLM (WKWalletManager manager,
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
        
        // Stellar has fixed network fee (costFactor = 1.0)
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
wkWalletManagerEstimateFeeBasisXLM (WKWalletManager manager,
                                         WKWallet  wallet,
                                         WKCookie cookie,
                                         WKAddress target,
                                         WKAmount amount,
                                         WKNetworkFee networkFee,
                                         size_t attributesCount,
                                         OwnershipKept WKTransferAttribute *attributes) {
    UInt256 value = wkAmountGetValue (wkNetworkFeeGetPricePerCostFactor (networkFee));
    BRStellarFeeBasis xlmFeeBasis;

    // No margin needed.
    xlmFeeBasis.pricePerCostFactor = (BRStellarFee) value.u32[0];
    xlmFeeBasis.costFactor = 1;  // 'cost factor' is 'transaction'

    // TODO - Carl
    return NULL; //wkFeeBasisCreateAsXLM (wallet->unitForFee, xlmFeeBasis);
}

static void
wkWalletManagerRecoverTransfersFromTransactionBundleXLM (WKWalletManager manager,
                                                              OwnershipKept WKClientTransactionBundle bundle) {
    // Not Stellar functionality
    assert (0);
}

static void
wkWalletManagerRecoverTransferFromTransferBundleXLM (WKWalletManager manager,
                                                          OwnershipKept WKClientTransferBundle bundle) {
    // create BRStellarTransaction
    
    BRStellarAccount xlmAccount = (BRStellarAccount) wkAccountAs (manager->account,
                                                                  WK_NETWORK_TYPE_XLM);
    
    BRStellarAmount amount = 0;
    sscanf(bundle->amount, "%" PRIu64, &amount);
    BRStellarFee fee = 0;
    if (NULL != bundle->fee) sscanf(bundle->fee, "%" PRIi32, &fee);
    BRStellarFeeBasis stellarFeeBasis = { fee, 1};
    BRStellarAddress toAddress   = stellarAddressCreateFromString(bundle->to,   false);
    BRStellarAddress fromAddress = stellarAddressCreateFromString(bundle->from, false);
    // Convert the hash string to bytes
    BRStellarTransactionHash txHash;
    memset(txHash.bytes, 0x00, sizeof(txHash.bytes));
    if (bundle->hash != NULL) {
        hexDecode(txHash.bytes, sizeof(txHash.bytes), bundle->hash, strlen(bundle->hash));
    }

    int error = (WK_TRANSFER_STATE_ERRORED == bundle->status);

    bool xlmTransactionNeedFree = true;
    BRStellarTransaction xlmTransaction = stellarTransactionCreateFull(fromAddress,
                                                                  toAddress,
                                                                  amount,
                                                                  stellarFeeBasis,
                                                                  txHash,
                                                                  bundle->blockTimestamp,
                                                                  bundle->blockNumber,
                                                                  error);

    stellarAddressFree (toAddress);
    stellarAddressFree (fromAddress);

    // create WKTransfer
    
    WKWallet wallet = wkWalletManagerGetWallet (manager);
    WKHash hash = wkHashCreateAsXLM (txHash);

    WKTransfer baseTransfer = wkWalletGetTransferByHashOrUIDS (wallet, hash, bundle->uids);
    wkHashGive(hash);

    WKFeeBasis      feeBasis = wkFeeBasisCreateAsXLM (wallet->unit, stellarTransactionGetFee(xlmTransaction));
    WKTransferState state    = wkClientTransferBundleGetTransferState (bundle, feeBasis);

    if (NULL == baseTransfer) {
        baseTransfer = wkTransferCreateAsXLM (wallet->listenerTransfer,
                                                   bundle->uids,
                                                   wallet->unit,
                                                   wallet->unitForFee,
                                                   state,
                                                   xlmAccount,
                                                   xlmTransaction);
        xlmTransactionNeedFree = false;

        wkWalletAddTransfer (wallet, baseTransfer);
    }
    else {
        wkTransferSetUids  (baseTransfer, bundle->uids);
        wkTransferSetState (baseTransfer, state);
    }
    
    wkWalletManagerRecoverTransferAttributesFromTransferBundle (wallet, baseTransfer, bundle);
    
    wkTransferGive(baseTransfer);
    wkFeeBasisGive (feeBasis);
    wkTransferStateGive (state);

    if (xlmTransactionNeedFree)
        stellarTransactionFree (xlmTransaction);
}

extern WKWalletSweeperStatus
wkWalletManagerWalletSweeperValidateSupportedXLM (WKWalletManager manager,
                                                       WKWallet wallet,
                                                       WKKey key) {
    return WK_WALLET_SWEEPER_UNSUPPORTED_CURRENCY;
}

extern WKWalletSweeper
wkWalletManagerCreateWalletSweeperXLM (WKWalletManager manager,
                                            WKWallet wallet,
                                            WKKey key) {
    // not supported
    return NULL;
}

static WKWallet
wkWalletManagerCreateWalletXLM (WKWalletManager manager,
                                     WKCurrency currency,
                                     Nullable OwnershipKept BRArrayOf(WKClientTransactionBundle) transactions,
                                     Nullable OwnershipKept BRArrayOf(WKClientTransferBundle) transfers) {
    BRStellarAccount xlmAccount = (BRStellarAccount) wkAccountAs (manager->account,
                                                                  WK_NETWORK_TYPE_XLM);

    // Create the primary WKWallet
    WKNetwork  network       = manager->network;
    WKUnit     unitAsBase    = wkNetworkGetUnitAsBase    (network, currency);
    WKUnit     unitAsDefault = wkNetworkGetUnitAsDefault (network, currency);

    WKWallet wallet = wkWalletCreateAsXLM (manager->listenerWallet,
                                                      unitAsDefault,
                                                      unitAsDefault,
                                                      xlmAccount);
    wkWalletManagerAddWallet (manager, wallet);

    //TODO:XLM load transfers from fileService

    wkUnitGive (unitAsDefault);
    wkUnitGive (unitAsBase);

    return wallet;
}

WKWalletManagerHandlers wkWalletManagerHandlersXLM = {
    wkWalletManagerCreateXLM,
    wkWalletManagerReleaseXLM,
    crytpWalletManagerCreateFileServiceXLM,
    wkWalletManagerGetEventTypesXLM,
    crytpWalletManagerCreateP2PManagerXLM,
    wkWalletManagerCreateWalletXLM,
    wkWalletManagerSignTransactionWithSeedXLM,
    wkWalletManagerSignTransactionWithKeyXLM,
    wkWalletManagerEstimateLimitXLM,
    wkWalletManagerEstimateFeeBasisXLM,
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    wkWalletManagerRecoverTransfersFromTransactionBundleXLM,
    wkWalletManagerRecoverTransferFromTransferBundleXLM,
    NULL,//WKWalletManagerRecoverFeeBasisFromFeeEstimateHandler not supported
    wkWalletManagerWalletSweeperValidateSupportedXLM,
    wkWalletManagerCreateWalletSweeperXLM
};
