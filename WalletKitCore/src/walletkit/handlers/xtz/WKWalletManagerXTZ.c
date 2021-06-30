//
//  WKWalletManagerXTZ.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXTZ.h"

#include "walletkit/WKAccountP.h"
#include "walletkit/WKNetworkP.h"
#include "walletkit/WKKeyP.h"
#include "walletkit/WKClientP.h"
#include "walletkit/WKWalletP.h"
#include "walletkit/WKAmountP.h"
#include "walletkit/WKWalletManagerP.h"
#include "walletkit/WKFileService.h"
#include "walletkit/WKHashP.h"

#include "tezos/BRTezosAccount.h"


// MARK: - Events

static const BREventType *xtzEventTypes[] = {
    WK_CLIENT_EVENT_TYPES
};

static const unsigned int
xtzEventTypesCount = (sizeof (xtzEventTypes) / sizeof (BREventType*));

// MARK: - Handlers

static WKWalletManager
wkWalletManagerCreateXTZ (WKWalletManagerListener listener,
                              WKClient client,
                              WKAccount account,
                              WKNetwork network,
                              WKSyncMode mode,
                              WKAddressScheme scheme,
                              const char *path) {
    return wkWalletManagerAllocAndInit (sizeof (struct WKWalletManagerXTZRecord),
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
wkWalletManagerReleaseXTZ (WKWalletManager manager) {
}

static BRFileService
crytpWalletManagerCreateFileServiceXTZ (WKWalletManager manager,
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
wkWalletManagerGetEventTypesXTZ (WKWalletManager manager,
                                     size_t *eventTypesCount) {
    assert (NULL != eventTypesCount);
    *eventTypesCount = xtzEventTypesCount;
    return xtzEventTypes;
}

static WKClientP2PManager
crytpWalletManagerCreateP2PManagerXTZ (WKWalletManager manager) {
    // not supported
    return NULL;
}

static WKBoolean
wkWalletManagerSignTransactionWithSeedXTZ (WKWalletManager manager,
                                               WKWallet wallet,
                                               WKTransfer transfer,
                                               UInt512 seed) {
    BRTezosHash lastBlockHash = wkHashAsXTZ (wkNetworkGetVerifiedBlockHash (manager->network));
    BRTezosAccount account = (BRTezosAccount) wkAccountAs (manager->account,
                                                           WK_NETWORK_TYPE_XTZ);
    BRTezosTransaction tid = tezosTransferGetTransaction (wkTransferCoerceXTZ(transfer)->xtzTransfer);
    bool needsReveal = (TEZOS_OP_TRANSACTION == tezosTransactionGetOperationKind(tid)) && wkWalletNeedsRevealXTZ(wallet);
    
    if (tid) {
        size_t tx_size = tezosTransactionSerializeAndSign (tid, account, seed, lastBlockHash, needsReveal);
        return AS_WK_BOOLEAN(tx_size > 0);
    } else {
        return WK_FALSE;
    }
}

static WKBoolean
wkWalletManagerSignTransactionWithKeyXTZ (WKWalletManager manager,
                                              WKWallet wallet,
                                              WKTransfer transfer,
                                              WKKey key) {
    assert(0);
    return WK_FALSE;
}

static WKAmount
wkWalletManagerEstimateLimitXTZ (WKWalletManager manager,
                                     WKWallet  wallet,
                                     WKBoolean asMaximum,
                                     WKAddress target,
                                     WKNetworkFee networkFee,
                                     WKBoolean *needEstimate,
                                     WKBoolean *isZeroIfInsuffientFunds,
                                     WKUnit unit) {
    // We always need an estimate as we do not know the fees.
    *needEstimate = asMaximum;

    return (WK_TRUE == asMaximum
            ? wkWalletGetBalance (wallet)        // Maximum is balance - fees 'needEstimate'
            : wkAmountCreateInteger (0, unit));  // No minimum
}

static WKFeeBasis
wkWalletManagerEstimateFeeBasisXTZ (WKWalletManager manager,
                                        WKWallet wallet,
                                        WKCookie cookie,
                                        WKAddress target,
                                        WKAmount amount,
                                        WKNetworkFee networkFee,
                                        size_t attributesCount,
                                        OwnershipKept WKTransferAttribute *attributes) {
    BRTezosUnitMutez mutezPerByte = tezosMutezCreate (networkFee->pricePerCostFactor) / 1000; // given as nanotez/byte
    BRTezosFeeBasis xtzFeeBasis = tezosDefaultFeeBasis (mutezPerByte);
    WKFeeBasis feeBasis = wkFeeBasisCreateAsXTZ (networkFee->pricePerCostFactorUnit, xtzFeeBasis);

    WKCurrency currency = wkAmountGetCurrency (amount);
    WKTransfer transfer = wkWalletCreateTransferXTZ (wallet,
                                                               target,
                                                               amount,
                                                               feeBasis,
                                                               attributesCount,
                                                               attributes,
                                                               currency,
                                                               wallet->unit,
                                                               wallet->unitForFee);

    wkCurrencyGive(currency);
    
    // serialize the transaction for fee estimation payload
    BRTezosHash lastBlockHash = wkHashAsXTZ (wkNetworkGetVerifiedBlockHash (manager->network));
    BRTezosAccount account = (BRTezosAccount) wkAccountAs (manager->account,
                                                           WK_NETWORK_TYPE_XTZ);
    BRTezosTransaction tid = tezosTransferGetTransaction (wkTransferCoerceXTZ(transfer)->xtzTransfer);
    bool needsReveal = (TEZOS_OP_TRANSACTION == tezosTransactionGetOperationKind(tid)) && wkWalletNeedsRevealXTZ(wallet);
    
    tezosTransactionSerializeForFeeEstimation(tid,
                                              account,
                                              lastBlockHash,
                                              needsReveal);
    
    // serialized tx size is needed for fee estimation
    wkFeeBasisGive (feeBasis);
    feeBasis = wkFeeBasisCreateAsXTZ (networkFee->pricePerCostFactorUnit, tezosTransactionGetFeeBasis(tid));

    wkClientQRYEstimateTransferFee (manager->qryManager,
                                        cookie,
                                        transfer,
                                        networkFee,
                                        feeBasis);

    wkTransferGive (transfer);
    wkFeeBasisGive (feeBasis);

    // Require QRY with cookie - made above
    return NULL;
}

static void
wkWalletManagerRecoverTransfersFromTransactionBundleXTZ (WKWalletManager manager,
                                                             OwnershipKept WKClientTransactionBundle bundle) {
    // Not XTZ functionality
    assert (0);
}

//TODO:XTZ refactor (copied from WalletManagerETH)
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

static void
wkWalletManagerRecoverTransferFromTransferBundleXTZ (WKWalletManager manager,
                                                         OwnershipKept WKClientTransferBundle bundle) {
    // create BRTezosTransfer

    BRTezosAccount xtzAccount = (BRTezosAccount) wkAccountAs (manager->account,
                                                              WK_NETWORK_TYPE_XTZ);
    
    BRTezosUnitMutez amountMutez, feeMutez = 0;
    sscanf(bundle->amount, "%" PRIu64, &amountMutez);
    if (NULL != bundle->fee) sscanf(bundle->fee, "%" PRIu64, &feeMutez);
    BRTezosAddress toAddress   = tezosAddressCreateFromString (bundle->to,   false);
    BRTezosAddress fromAddress = tezosAddressCreateFromString (bundle->from, false);

    // Convert the hash string to bytes
    WKHash bundleHash = wkHashCreateFromStringAsXTZ (bundle->hash);
    BRTezosHash txId = wkHashAsXTZ (bundleHash);
    wkHashGive(bundleHash);

    int error = (WK_TRANSFER_STATE_ERRORED == bundle->status);

    bool xtzTransferNeedFree = true;
    BRTezosTransfer xtzTransfer = tezosTransferCreate(fromAddress, toAddress, amountMutez, feeMutez, txId, bundle->blockTimestamp, bundle->blockNumber, error);

    // create WKTransfer
    
    WKWallet wallet = wkWalletManagerGetWallet (manager);
    WKHash hash = wkHashCreateAsXTZ (txId);
    
    // A transaction may include a "burn" transfer to target address 'unknown' in addition to the
    // normal transfer, both sharing the same hash. Typically occurs when sending to an un-revealed
    // address.  It must be included since the burn amount is subtracted from wallet balance, but
    // is not considered a normal fee.
    WKAddress target = wkAddressCreateAsXTZ (toAddress);
    WKTransfer baseTransfer = wkWalletGetTransferByHashOrUIDSAndTargetXTZ (wallet, hash, bundle->uids, target);
    
    wkAddressGive (target);
    wkHashGive (hash);
    tezosAddressFree (fromAddress);

    BRTezosFeeBasis xtzFeeBasis = tezosFeeBasisCreateActual (feeMutez);

    WKFeeBasis      feeBasis = wkFeeBasisCreateAsXTZ (wallet->unitForFee, xtzFeeBasis);
    WKTransferState state    = wkClientTransferBundleGetTransferState (bundle, feeBasis);

    if (NULL == baseTransfer) {
        baseTransfer = wkTransferCreateAsXTZ (wallet->listenerTransfer,
                                                  bundle->uids,
                                                  wallet->unit,
                                                  wallet->unitForFee,
                                                  state,
                                                  xtzAccount,
                                                  xtzTransfer);
        xtzTransferNeedFree = false;
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
    
    if (xtzTransferNeedFree)
        tezosTransferFree (xtzTransfer);
}

static WKFeeBasis
wkWalletManagerRecoverFeeBasisFromFeeEstimateXTZ (WKWalletManager cwm,
                                                      WKNetworkFee networkFee,
                                                      WKFeeBasis initialFeeBasis,
                                                      double costUnits,
                                                      size_t attributesCount,
                                                      OwnershipKept const char **attributeKeys,
                                                      OwnershipKept const char **attributeVals) {
    bool parseError;
    
    int64_t gasUsed = (int64_t) cwmParseUInt64 (cwmLookupAttributeValueForKey ("consumed_gas", attributesCount, attributeKeys, attributeVals), &parseError);
    int64_t storageUsed = (int64_t) cwmParseUInt64 (cwmLookupAttributeValueForKey ("storage_size", attributesCount, attributeKeys, attributeVals), &parseError);
    int64_t counter = (int64_t) cwmParseUInt64 (cwmLookupAttributeValueForKey ("counter", attributesCount, attributeKeys, attributeVals), &parseError);
    // increment counter
    counter += 1;
    // add 10% padding to gas/storage limits
    gasUsed = (int64_t)(gasUsed * 1.1);
    storageUsed = (int64_t)(storageUsed * 1.1);
    BRTezosUnitMutez mutezPerKByte = tezosMutezCreate (networkFee->pricePerCostFactor); // given as nanotez/byte
    
    // get the serialized txn size from the estimation payload
    double sizeInKBytes = wkFeeBasisCoerceXTZ(initialFeeBasis)->xtzFeeBasis.u.initial.sizeInKBytes;

    BRTezosFeeBasis feeBasis = tezosFeeBasisCreateEstimate (mutezPerKByte,
                                                            sizeInKBytes,
                                                            gasUsed,
                                                            storageUsed,
                                                            counter);
    
    return wkFeeBasisCreateAsXTZ (networkFee->pricePerCostFactorUnit, feeBasis);
}

extern WKWalletSweeperStatus
wkWalletManagerWalletSweeperValidateSupportedXTZ (WKWalletManager manager,
                                                      WKWallet wallet,
                                                      WKKey key) {
    return WK_WALLET_SWEEPER_UNSUPPORTED_CURRENCY;
}

extern WKWalletSweeper
wkWalletManagerCreateWalletSweeperXTZ (WKWalletManager manager,
                                           WKWallet wallet,
                                           WKKey key) {
    // not supported
    return NULL;
}

static WKWallet
wkWalletManagerCreateWalletXTZ (WKWalletManager manager,
                                    WKCurrency currency,
                                    Nullable OwnershipKept BRArrayOf(WKClientTransactionBundle) transactions,
                                    Nullable OwnershipKept BRArrayOf(WKClientTransferBundle) transfers) {
    BRTezosAccount xtzAccount = (BRTezosAccount) wkAccountAs (manager->account,
                                                              WK_NETWORK_TYPE_XTZ);

    // Create the primary WKWallet
    WKNetwork  network       = manager->network;
    WKUnit     unitAsBase    = wkNetworkGetUnitAsBase    (network, currency);
    WKUnit     unitAsDefault = wkNetworkGetUnitAsDefault (network, currency);
    
    WKWallet wallet = wkWalletCreateAsXTZ (manager->listenerWallet,
                                                     unitAsDefault,
                                                     unitAsDefault,
                                                     xtzAccount);
    wkWalletManagerAddWallet (manager, wallet);
    
    // TODO:XTZ load transfers from fileService
    
    wkUnitGive (unitAsDefault);
    wkUnitGive (unitAsBase);
    
    return wallet;
}

WKWalletManagerHandlers wkWalletManagerHandlersXTZ = {
    wkWalletManagerCreateXTZ,
    wkWalletManagerReleaseXTZ,
    crytpWalletManagerCreateFileServiceXTZ,
    wkWalletManagerGetEventTypesXTZ,
    crytpWalletManagerCreateP2PManagerXTZ,
    wkWalletManagerCreateWalletXTZ,
    wkWalletManagerSignTransactionWithSeedXTZ,
    wkWalletManagerSignTransactionWithKeyXTZ,
    wkWalletManagerEstimateLimitXTZ,
    wkWalletManagerEstimateFeeBasisXTZ,
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    wkWalletManagerRecoverTransfersFromTransactionBundleXTZ,
    wkWalletManagerRecoverTransferFromTransferBundleXTZ,
    wkWalletManagerRecoverFeeBasisFromFeeEstimateXTZ,
    wkWalletManagerWalletSweeperValidateSupportedXTZ,
    wkWalletManagerCreateWalletSweeperXTZ
};
