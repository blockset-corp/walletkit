//
//  WKWalletManagerXRP.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXRP.h"

#include "walletkit/WKAccountP.h"
#include "walletkit/WKNetworkP.h"
#include "walletkit/WKKeyP.h"
#include "walletkit/WKClientP.h"
#include "walletkit/WKWalletP.h"
#include "walletkit/WKAmountP.h"
#include "walletkit/WKWalletManagerP.h"
#include "walletkit/WKFileService.h"

#include "ripple/BRRippleAccount.h"


// MARK: - Events

//TODO:XRP make common
static const BREventType *xrpEventTypes[] = {
    WK_CLIENT_EVENT_TYPES
};

static const unsigned int
xrpEventTypesCount = (sizeof (xrpEventTypes) / sizeof (BREventType*));


//static WKWalletManagerXRP
//wkWalletManagerCoerce (WKWalletManager wm) {
//    assert (WK_NETWORK_TYPE_XRP == wm->type);
//    return (WKWalletManagerXRP) wm;
//}

// MARK: - Handlers

static WKWalletManager
wkWalletManagerCreateXRP (WKWalletManagerListener listener,
                              WKClient client,
                              WKAccount account,
                              WKNetwork network,
                              WKSyncMode mode,
                              WKAddressScheme scheme,
                              const char *path) {
    return wkWalletManagerAllocAndInit (sizeof (struct WKWalletManagerXRPRecord),
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
wkWalletManagerReleaseXRP (WKWalletManager manager) {
}

static BRFileService
crytpWalletManagerCreateFileServiceXRP (WKWalletManager manager,
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
wkWalletManagerGetEventTypesXRP (WKWalletManager manager,
                                     size_t *eventTypesCount) {
    assert (NULL != eventTypesCount);
    *eventTypesCount = xrpEventTypesCount;
    return xrpEventTypes;
}

static WKClientP2PManager
crytpWalletManagerCreateP2PManagerXRP (WKWalletManager manager) {
    // not supported
    return NULL;
}

static WKBoolean
wkWalletManagerSignTransactionWithSeedXRP (WKWalletManager manager,
                                               WKWallet wallet,
                                               WKTransfer transfer,
                                               UInt512 seed) {
    BRRippleAccount account = wkAccountAsXRP (manager->account);
    BRRippleTransaction tid = wkTransferCoerceXRP(transfer)->xrpTransaction;
    if (tid) {
        size_t tx_size = rippleAccountSignTransaction (account, tid, seed);
        return AS_WK_BOOLEAN(tx_size > 0);
    } else {
        return WK_FALSE;
    }
}

static WKBoolean
wkWalletManagerSignTransactionWithKeyXRP (WKWalletManager manager,
                                              WKWallet wallet,
                                              WKTransfer transfer,
                                              WKKey key) {
    assert(0);
    return WK_FALSE;
}

static WKAmount
wkWalletManagerEstimateLimitXRP (WKWalletManager manager,
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
        
        // Ripple has fixed network fee (costFactor = 1.0)
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
wkWalletManagerEstimateFeeBasisXRP (WKWalletManager manager,
                                        WKWallet wallet,
                                        WKCookie cookie,
                                        WKAddress target,
                                        WKAmount amount,
                                        WKNetworkFee networkFee,
                                        size_t attributesCount,
                                        OwnershipKept WKTransferAttribute *attributes) {
    UInt256 value = wkAmountGetValue (wkNetworkFeeGetPricePerCostFactor (networkFee));
    BRRippleUnitDrops fee = value.u64[0];
    return wkFeeBasisCreateAsXRP (wallet->unitForFee, fee);
}

static void
wkWalletManagerRecoverTransfersFromTransactionBundleXRP (WKWalletManager manager,
                                                             OwnershipKept WKClientTransactionBundle bundle) {
    // Not XRP functionality
    assert (0);
}

static void
wkWalletManagerRecoverTransferFromTransferBundleXRP (WKWalletManager manager,
                                                         OwnershipKept WKClientTransferBundle bundle) {
    // create BRRippleTransfer

    BRRippleAccount xrpAccount = wkAccountAsXRP(manager->account);
    
    BRRippleUnitDrops amountDrops = 0;
    sscanf(bundle->amount, "%" PRIu64, &amountDrops);

    BRRippleUnitDrops feeDrops = 0;
    if (NULL != bundle->fee) sscanf(bundle->fee, "%" PRIu64, &feeDrops);
    BRRippleFeeBasis xrpFeeBasis = { feeDrops, 1};

    BRRippleAddress toAddress   = rippleAddressCreateFromString (bundle->to,   false);
    BRRippleAddress fromAddress = rippleAddressCreateFromString (bundle->from, false);
    // Convert the hash string to bytes
    BRRippleTransactionHash txId;
    hexDecode(txId.bytes, sizeof(txId.bytes), bundle->hash, strlen(bundle->hash));
    int error = (WK_TRANSFER_STATE_ERRORED == bundle->status);

    bool xrpTransactionNeedFree = true;
    BRRippleTransaction xrpTransaction = rippleTransactionCreateFull (fromAddress,
                                                                      toAddress,
                                                                      amountDrops,
                                                                      xrpFeeBasis,
                                                                      txId,
                                                                      bundle->blockTimestamp,
                                                                      bundle->blockNumber,
                                                                      error);

    rippleAddressFree (toAddress);
    rippleAddressFree (fromAddress);

    // create WKTransfer
    
    WKWallet wallet = wkWalletManagerGetWallet (manager);
    WKHash hash = wkHashCreateAsXRP (txId);
    
    WKTransfer baseTransfer = wkWalletGetTransferByHash (wallet, hash);
    wkHashGive (hash);

    WKFeeBasis      feeBasis = wkFeeBasisCreateAsXRP (wallet->unitForFee, feeDrops);
    WKTransferState state    = wkClientTransferBundleGetTransferState (bundle, feeBasis);

    if (NULL == baseTransfer) {
        baseTransfer = wkTransferCreateAsXRP (wallet->listenerTransfer,
                                                  wallet->unit,
                                                  wallet->unitForFee,
                                                  state,
                                                  xrpAccount,
                                                  xrpTransaction);
        xrpTransactionNeedFree = false;

        wkWalletAddTransfer (wallet, baseTransfer);
    }
    else {
        wkTransferSetState (baseTransfer, state);
    }
    
    wkWalletManagerRecoverTransferAttributesFromTransferBundle (wallet, baseTransfer, bundle);

    wkTransferGive(baseTransfer);
    wkFeeBasisGive (feeBasis);
    wkTransferStateGive (state);

    if (xrpTransactionNeedFree)
        rippleTransactionFree (xrpTransaction);
}

extern WKWalletSweeperStatus
wkWalletManagerWalletSweeperValidateSupportedXRP (WKWalletManager manager,
                                                      WKWallet wallet,
                                                      WKKey key) {
    return WK_WALLET_SWEEPER_UNSUPPORTED_CURRENCY;
}

extern WKWalletSweeper
wkWalletManagerCreateWalletSweeperXRP (WKWalletManager manager,
                                           WKWallet wallet,
                                           WKKey key) {
    // not supported
    return NULL;
}

static WKWallet
wkWalletManagerCreateWalletXRP (WKWalletManager manager,
                                    WKCurrency currency,
                                    Nullable OwnershipKept BRArrayOf(WKClientTransactionBundle) transactions,
                                    Nullable OwnershipKept BRArrayOf(WKClientTransferBundle) transfers) {
    BRRippleAccount xrpAccount = wkAccountAsXRP(manager->account);

    // Create the primary WKWallet
    WKNetwork  network       = manager->network;
    WKUnit     unitAsBase    = wkNetworkGetUnitAsBase    (network, currency);
    WKUnit     unitAsDefault = wkNetworkGetUnitAsDefault (network, currency);
    
    WKWallet wallet = wkWalletCreateAsXRP (manager->listenerWallet,
                                                     unitAsDefault,
                                                     unitAsDefault,
                                                     xrpAccount);
    wkWalletManagerAddWallet (manager, wallet);
    
    // TODO:XRP load transfers from fileService
    
    wkUnitGive (unitAsDefault);
    wkUnitGive (unitAsBase);
    
    return wallet;
}

WKWalletManagerHandlers wkWalletManagerHandlersXRP = {
    wkWalletManagerCreateXRP,
    wkWalletManagerReleaseXRP,
    crytpWalletManagerCreateFileServiceXRP,
    wkWalletManagerGetEventTypesXRP,
    crytpWalletManagerCreateP2PManagerXRP,
    wkWalletManagerCreateWalletXRP,
    wkWalletManagerSignTransactionWithSeedXRP,
    wkWalletManagerSignTransactionWithKeyXRP,
    wkWalletManagerEstimateLimitXRP,
    wkWalletManagerEstimateFeeBasisXRP,
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    wkWalletManagerRecoverTransfersFromTransactionBundleXRP,
    wkWalletManagerRecoverTransferFromTransferBundleXRP,
    NULL,//WKWalletManagerRecoverFeeBasisFromFeeEstimateHandler not supported
    wkWalletManagerWalletSweeperValidateSupportedXRP,
    wkWalletManagerCreateWalletSweeperXRP
};
