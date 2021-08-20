//
//  WKWalletManagerAVAX.c
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKAVAX.h"

#include "walletkit/WKAccountP.h"
#include "walletkit/WKNetworkP.h"
#include "walletkit/WKKeyP.h"
#include "walletkit/WKClientP.h"
#include "walletkit/WKWalletP.h"
#include "walletkit/WKAmountP.h"
#include "walletkit/WKWalletManagerP.h"
#include "walletkit/WKFileService.h"
#include "walletkit/WKHashP.h"

#include "avalanche/BRAvalancheAccount.h"


// MARK: - Events

static const BREventType *avaxEventTypes[] = {
    WK_CLIENT_EVENT_TYPES
};

static const unsigned int
avaxEventTypesCount = (sizeof (avaxEventTypes) / sizeof (BREventType*));

// MARK: - Handlers

static WKWalletManager
wkWalletManagerCreateAVAX (WKWalletManagerListener listener,
                              WKClient client,
                              WKAccount account,
                              WKNetwork network,
                              WKSyncMode mode,
                              WKAddressScheme scheme,
                              const char *path) {
    return wkWalletManagerAllocAndInit (sizeof (struct WKWalletManagerAVAXRecord),
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
wkWalletManagerReleaseAVAX (WKWalletManager manager) {
}

static BRFileService
crytpWalletManagerCreateFileServiceAVAX (WKWalletManager manager,
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
wkWalletManagerGetEventTypesAVAX (WKWalletManager manager,
                                     size_t *eventTypesCount) {
    assert (NULL != eventTypesCount);
    *eventTypesCount = avaxEventTypesCount;
    return avaxEventTypes;
}

static WKClientP2PManager
crytpWalletManagerCreateP2PManagerAVAX (WKWalletManager manager) {
    // not supported
    return NULL;
}

static WKBoolean
wkWalletManagerSignTransactionWithSeedAVAX (WKWalletManager manager,
                                               WKWallet wallet,
                                               WKTransfer transfer,
                                               UInt512 seed) {
    BRAvalancheAccount avaxAccount = wkAccountGetAsAVAX (manager->account);
    BRAvalancheTransaction avaxTransaction = wkTransferCoerceAVAX(transfer)->avaxTransaction;

    size_t serializationSize = 0;

    if (avaxTransaction) {
        uint8_t *serialization = avalancheTransactionSerializeForSubmission (avaxTransaction, avaxAccount, seed, &serializationSize);
        free (serialization);
    }

    return AS_WK_BOOLEAN (serializationSize > 0);
}

static WKBoolean
wkWalletManagerSignTransactionWithKeyAVAX (WKWalletManager manager,
                                              WKWallet wallet,
                                              WKTransfer transfer,
                                              WKKey key) {
    assert(0);
    return WK_FALSE;
}

static WKAmount
wkWalletManagerEstimateLimitAVAX (WKWalletManager manager,
                                     WKWallet  wallet,
                                     WKBoolean asMaximum,
                                     WKAddress target,
                                     WKNetworkFee networkFee,
                                     WKBoolean *needEstimate,
                                     WKBoolean *isZeroIfInsuffientFunds,
                                     WKUnit unit) {
#if 0
    *needEstimate = asMaximum;
#endif

    return (WK_TRUE == asMaximum
            ? wkWalletGetBalance (wallet)        // Maximum is balance - fees 'needEstimate'
            : wkAmountCreateInteger (0, unit));  // No minimum
}

static WKFeeBasis
wkWalletManagerEstimateFeeBasisAVAX (WKWalletManager manager,
                                        WKWallet wallet,
                                        WKCookie cookie,
                                        WKAddress target,
                                        WKAmount amount,
                                        WKNetworkFee networkFee,
                                        size_t attributesCount,
                                        OwnershipKept WKTransferAttribute *attributes) {
#if 0
    BRAvalancheAmount mutezPerByte = avalancheMutezCreate (networkFee->pricePerCostFactor) / 1000; // given as nanotez/byte
    BRAvalancheFeeBasis avaxFeeBasis = avalancheDefaultFeeBasis (mutezPerByte);
    WKFeeBasis feeBasis = wkFeeBasisCreateAsAVAX (networkFee->pricePerCostFactorUnit, avaxFeeBasis);

    WKCurrency currency = wkAmountGetCurrency (amount);
    WKTransfer transfer = wkWalletCreateTransferAVAX (wallet,
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
    BRAvalancheHash lastBlockHash = wkHashAsAVAX (wkNetworkGetVerifiedBlockHash (manager->network));
    BRAvalancheAccount avaxAccount = wkAccountGetAsAVAX (manager->account);
    BRAvalancheTransaction tid = avalancheTransactionGetTransaction (wkTransferCoerceAVAX(transfer)->avaxTransaction);
    bool needsReveal = (AVALANCHE_OP_TRANSACTION == avalancheTransactionGetOperationKind(tid)) && wkWalletNeedsRevealAVAX(wallet);
    
    avalancheTransactionSerializeForFeeEstimation(tid,
                                              avaxAccount,
                                              lastBlockHash,
                                              needsReveal);
    
    // serialized tx size is needed for fee estimation
    wkFeeBasisGive (feeBasis);
    feeBasis = wkFeeBasisCreateAsAVAX (networkFee->pricePerCostFactorUnit, avalancheTransactionGetFeeBasis(tid));

    wkClientQRYEstimateTransferFee (manager->qryManager,
                                        cookie,
                                        transfer,
                                        networkFee,
                                        feeBasis);

    wkTransferGive (transfer);
    wkFeeBasisGive (feeBasis);

    // Require QRY with cookie - made above
    return NULL;
#endif
    BRAvalancheFeeBasis avaxFeeBasis = avalancheFeeBasisCreate (0);
    return wkFeeBasisCreateAsAVAX (wallet->unitForFee, avaxFeeBasis);
}

static void
wkWalletManagerRecoverTransfersFromTransactionBundleAVAX (WKWalletManager manager,
                                                             OwnershipKept WKClientTransactionBundle bundle) {
    // Not AVAX functionality
    assert (0);
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

static void
wkWalletManagerRecoverTransferFromTransferBundleAVAX (WKWalletManager manager,
                                                         OwnershipKept WKClientTransferBundle bundle) {
    // The wallet holds currency transfers
    WKWallet wallet = wkWalletManagerGetWallet (manager);

    BRAvalancheAccount avaxAccount = wkAccountGetAsAVAX (manager->account);

    BRAvalancheAmount avaxAmount;
    sscanf(bundle->amount, "%" PRIu64, &avaxAmount);

    BRAvalancheAmount avaxFee = 0;
    if (NULL != bundle->fee) sscanf(bundle->fee, "%" PRIu64, &avaxFee);

    // Get the `source` and `target` addresses.  We'll only use `source` if we need to create a
    // transfer; we'll use `target` both if a transfer is created and to identify a pre-existing
    // transfer held by wallet.
    BRAvalancheAddress avaxTarget = avalancheAddressCreateFromString (bundle->to,   false, AVALANCHE_CHAIN_TYPE_X);
    BRAvalancheAddress avaxSource = avalancheAddressCreateFromString (bundle->from, false, AVALANCHE_CHAIN_TYPE_X);

    WKAddress target = wkAddressCreateAsAVAX (avaxTarget);
    WKAddress source = wkAddressCreateAsAVAX (avaxSource);

    // A transaction may include a "burn" transfer to target address 'unknown' in addition to the
    // normal transfer, both sharing the same hash. Typically occurs when sending to an un-revealed
    // address.  It must be included since the burn amount is subtracted from wallet balance, but
    // is not considered a normal fee.
    WKHash         hash = wkHashCreateFromStringAsAVAX (bundle->hash);
    WKTransfer transfer = wkWalletGetTransferByHashOrUIDS (wallet, hash, bundle->uids);

    BRAvalancheFeeBasis avaxFeeBasis = avalancheFeeBasisCreate(0);
    WKFeeBasis  feeBasis = wkFeeBasisCreateAsAVAX (wallet->unitForFee, avaxFeeBasis);

    WKTransferState state = wkClientTransferBundleGetTransferState (bundle, feeBasis);

    if (NULL != transfer) {
        wkTransferSetUids  (transfer, bundle->uids);
        wkTransferSetState (transfer, state);
    }
    else {
        BRAvalancheTransaction avaxTransaction = avalancheTransactionCreate (avaxSource,
                                                                                 avaxTarget,
                                                                                 avaxAmount,
                                                                                 avaxFeeBasis);

        transfer = wkTransferCreateAsAVAX (wallet->listenerTransfer,
                                                  bundle->uids,
                                                  wallet->unit,
                                                  wallet->unitForFee,
                                                  state,
                                                  avaxAccount,
                                                  avaxTransaction);
        wkWalletAddTransfer (wallet, transfer);
    }

    wkWalletManagerRecoverTransferAttributesFromTransferBundle (wallet, transfer, bundle);
    
    wkTransferGive (transfer);
    wkHashGive (hash);

    wkAddressGive (source);
    wkAddressGive (target);

    wkFeeBasisGive (feeBasis);
    wkTransferStateGive (state);

    wkWalletGive (wallet);
}

static WKFeeBasis
wkWalletManagerRecoverFeeBasisFromFeeEstimateAVAX (WKWalletManager cwm,
                                                  WKTransfer transfer,
                                                  WKNetworkFee networkFee,
                                                  double costUnits,
                                                  size_t attributesCount,
                                                  OwnershipKept const char **attributeKeys,
                                                  OwnershipKept const char **attributeVals) {
#if 0
    bool parseError;
    
    int64_t gasUsed = (int64_t) cwmParseUInt64 (cwmLookupAttributeValueForKey ("consumed_gas", attributesCount, attributeKeys, attributeVals), &parseError);
    int64_t storageUsed = (int64_t) cwmParseUInt64 (cwmLookupAttributeValueForKey ("storage_size", attributesCount, attributeKeys, attributeVals), &parseError);
    int64_t counter = (int64_t) cwmParseUInt64 (cwmLookupAttributeValueForKey ("counter", attributesCount, attributeKeys, attributeVals), &parseError);
    // increment counter
    counter += 1;
    // add 10% padding to gas/storage limits
    gasUsed = (int64_t)(gasUsed * 1.1);
    storageUsed = (int64_t)(storageUsed * 1.1);
    BRAvalancheAmount mutezPerKByte = avalancheMutezCreate (networkFee->pricePerCostFactor); // given as nanotez/byte
    
    // get the serialized txn size from the estimation payload
    double sizeInKBytes = wkFeeBasisCoerceAVAX(initialFeeBasis)->avaxFeeBasis.u.initial.sizeInKBytes;

    BRAvalancheFeeBasis feeBasis = avalancheFeeBasisCreateEstimate (mutezPerKByte,
                                                            sizeInKBytes,
                                                            gasUsed,
                                                            storageUsed,
                                                            counter);
#endif

    BRAvalancheFeeBasis avaxFeeBasis = avalancheFeeBasisCreate (0);
    return wkFeeBasisCreateAsAVAX (networkFee->pricePerCostFactorUnit, avaxFeeBasis);
}

extern WKWalletSweeperStatus
wkWalletManagerWalletSweeperValidateSupportedAVAX (WKWalletManager manager,
                                                      WKWallet wallet,
                                                      WKKey key) {
    return WK_WALLET_SWEEPER_UNSUPPORTED_CURRENCY;
}

extern WKWalletSweeper
wkWalletManagerCreateWalletSweeperAVAX (WKWalletManager manager,
                                           WKWallet wallet,
                                           WKKey key) {
    // not supported
    return NULL;
}

static WKWallet
wkWalletManagerCreateWalletAVAX (WKWalletManager manager,
                                    WKCurrency currency,
                                    Nullable OwnershipKept BRArrayOf(WKClientTransactionBundle) transactions,
                                    Nullable OwnershipKept BRArrayOf(WKClientTransferBundle) transfers) {
    BRAvalancheAccount avaxAccount = wkAccountGetAsAVAX (manager->account);

    // Create the primary WKWallet
    WKNetwork  network       = manager->network;
    WKUnit     unitAsBase    = wkNetworkGetUnitAsBase    (network, currency);
    WKUnit     unitAsDefault = wkNetworkGetUnitAsDefault (network, currency);
    
    WKWallet wallet = wkWalletCreateAsAVAX (manager->listenerWallet,
                                                     unitAsDefault,
                                                     unitAsDefault,
                                                     avaxAccount);
    wkWalletManagerAddWallet (manager, wallet);
    
    // TODO:AVAX load transfers from fileService
    
    wkUnitGive (unitAsDefault);
    wkUnitGive (unitAsBase);
    
    return wallet;
}

WKWalletManagerHandlers wkWalletManagerHandlersAVAX = {
    wkWalletManagerCreateAVAX,
    wkWalletManagerReleaseAVAX,
    crytpWalletManagerCreateFileServiceAVAX,
    wkWalletManagerGetEventTypesAVAX,
    crytpWalletManagerCreateP2PManagerAVAX,
    wkWalletManagerCreateWalletAVAX,
    wkWalletManagerSignTransactionWithSeedAVAX,
    wkWalletManagerSignTransactionWithKeyAVAX,
    wkWalletManagerEstimateLimitAVAX,
    wkWalletManagerEstimateFeeBasisAVAX,
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    NULL, // WKWalletManagerSaveTransactionBundleHandler
    wkWalletManagerRecoverTransfersFromTransactionBundleAVAX,
    wkWalletManagerRecoverTransferFromTransferBundleAVAX,
    wkWalletManagerRecoverFeeBasisFromFeeEstimateAVAX,
    wkWalletManagerWalletSweeperValidateSupportedAVAX,
    wkWalletManagerCreateWalletSweeperAVAX
};
