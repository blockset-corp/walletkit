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
#include "tezos/BRTezosFeeBasis.h"

#define TEZOS_FEE_PADDING_PERCENT           (10)

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
    BRTezosTransaction xtzTransaction = wkTransferCoerceXTZ(transfer)->originatingTransaction;

    if (NULL == xtzTransaction) return WK_FALSE;

    tezosTransactionSerializeAndSign (xtzTransaction, account, seed, lastBlockHash);

    if (tezosTransactionGetSignedBytesCount(xtzTransaction) > 0) {
        wkTransferCoerceXTZ(transfer)->hash = tezosTransactionGetHash(xtzTransaction);
        return WK_TRUE;
    }
    else return WK_FALSE;
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
    BRTezosUnitMutez mutezPerKByte = 1000 * tezosMutezCreate (networkFee->pricePerCostFactor /* mutez/byte */);

    BRTezosFeeBasis xtzFeeBasis = tezosFeeBasisCreateDefault (mutezPerKByte,
                                                              wkWalletHasTransferAttributeForDelegationXTZ (wallet, attributesCount, attributes),
                                                              wkWalletNeedsRevealXTZ(wallet));
    WKFeeBasis      feeBasis    = wkFeeBasisCreateAsXTZ (networkFee->pricePerCostFactorUnit, xtzFeeBasis);

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

    wkCurrencyGive (currency);
    
    // serialize the transaction for fee estimation payload
    BRTezosHash    xtzLastBlockHash = wkHashAsXTZ (wkNetworkGetVerifiedBlockHash (manager->network));
    BRTezosAccount xtzAccount       = (BRTezosAccount) wkAccountAs (manager->account, WK_NETWORK_TYPE_XTZ);

    BRTezosTransaction xtzTransaction = wkTransferCoerceXTZ(transfer)->originatingTransaction;
    assert (NULL != xtzTransaction);

    // Serialize the xtzTransaction; the serialization is saved (in `xtzTransaction`) and then used
    // in the call to `wkClientQRYEstimateTransferFee()` below.  Eventually,
    // `wkWalletManagerRecoverFeeBasisFromFeeEstimateXTZ()` is called and then the serialization
    // size will be used to complete the fee estimation.
    tezosTransactionSerializeForFeeEstimation (xtzTransaction,
                                               xtzAccount,
                                               xtzLastBlockHash);
    
    wkClientQRYEstimateTransferFee (manager->qryManager,
                                        cookie,
                                        transfer,
                                        networkFee);

    wkTransferGive (transfer);
    wkFeeBasisGive (feeBasis);

    // Require QRY with cookie - made above
    return NULL;
}

static void
wkWalletManagerRecoverTransfersFromTransactionBundlesXTZ (WKWalletManager manager,
                                                             OwnershipKept BRArrayOf (WKClientTransactionBundle) bundles) {
    // Not XTZ functionality
    assert (0);
}

static const char *
cwmLookupAttributeValueForKey (const char *key, size_t count, const char **keys, const char **vals) {
    for (size_t index = 0; index < count; index++)
        if (0 == strcasecmp (key, keys[index]))
            return vals[index];
    return NULL;
}

static BRTezosOperationKind
wkWalletManagerRecoverOperationKind (const char *key, size_t count, const char **keys, const char **vals, bool *hasKind) {

    // Assume success
    *hasKind = true;

    //

    const char *strKind = cwmLookupAttributeValueForKey (key, count, keys, vals);

    if (NULL == strKind) { *hasKind = false; return /* ignore */ TEZOS_OP_ENDORESEMENT; }
    else if (0 == strcasecmp (strKind, "transaction")) { return TEZOS_OP_TRANSACTION; }
    else if (0 == strcasecmp (strKind, "delegation" )) { return TEZOS_OP_DELEGATION;  }
    else if (0 == strcasecmp (strKind, "reveal"     )) { return TEZOS_OP_REVEAL;      }
    else { *hasKind = false; return /* ignore */ TEZOS_OP_ENDORESEMENT; }
}

static void
wkWalletManagerRecoverTransferFromTransferBundleXTZ (WKWalletManager manager,
                                                         OwnershipKept WKClientTransferBundle bundle) {
    BRTezosAccount xtzAccount = (BRTezosAccount) wkAccountAs (manager->account, WK_NETWORK_TYPE_XTZ);

    // The wallet holds currency transfers
    WKWallet wallet = wkWalletManagerGetWallet (manager);

    // Get the xtzAmount...
    BRTezosUnitMutez xtzAmount;
    sscanf(bundle->amount, "%" PRIu64, &xtzAmount);

    // and the xtzFee
    BRTezosUnitMutez xtzFee = 0;
    if (NULL != bundle->fee) sscanf (bundle->fee, "%" PRIu64, &xtzFee);

    //TODO: Get the kind
    bool hasKind = false;
    BRTezosOperationKind kind = wkWalletManagerRecoverOperationKind ("type",
                                                                     bundle->attributesCount,
                                                                     (const char **) bundle->attributeKeys,
                                                                     (const char **) bundle->attributeVals,
                                                                     &hasKind);
    if (!hasKind) kind = TEZOS_OP_TRANSACTION;
    
    // Create an amount from the xtZAmount with the default XTZ unit
    WKCurrency currency = wkNetworkGetCurrency (manager->network);
    WKUnit   amountUnit = wkNetworkGetUnitAsDefault (manager->network, currency);
    WKAmount amount     = wkAmountCreate (amountUnit, WK_FALSE, uint256Create((uint64_t) xtzAmount));
    wkUnitGive(amountUnit);
    wkCurrencyGive(currency);

    // Get the `source` and `target` addresses.  We'll only use `source` if we need to create a
    // transfer; we'll use `target` both if a transfer is created and to identify a pre-existing
    // transfer held by wallet.
    BRTezosAddress xtzSource = tezosAddressCreateFromString (bundle->from, false);
    BRTezosAddress xtzTarget = tezosAddressCreateFromString (bundle->to,   false);

    WKAddress target = wkAddressCreateAsXTZ (xtzTarget);
    WKAddress source = wkAddressCreateAsXTZ (xtzSource);

    // A transaction may include a "burn" transfer to target address 'unknown' in addition to the
    // normal transfer, both sharing the same hash. Typically occurs when sending to an un-revealed
    // address.  It must be included since the burn amount is subtracted from wallet balance, but
    // is not considered a normal fee.
    WKHash         hash = wkHashCreateFromStringAsXTZ (bundle->hash);
    WKTransfer transfer = wkWalletGetTransferByHashOrUIDSAndTargetXTZ (wallet, hash, bundle->uids, target);

    // We need the `WKState` which includes the 'confirmed fee basis'.  In general we don't have
    // enough information to derive a proper BZTezosFeeBasis.  Specifically, we don't have the
    // mutezPerKByte.  (We might have that if we originated the transaction but then on a restart
    // we will lose the transaction - since only Transfer/Transaction bundles are saved.
    //
    // We could approximate the mutezPerKByte if we invert `tezosOperationFeeBasisComputeMinimalFee()`
    // which uses `fee = f(mutezPerKByte, sizeInBytes, gasLimit) - be we don't have the size nor
    // the gasUsed.

    // Get the 'confirmed' `WKFeeBasis` for use in the `WKTransferState`
    BRTezosFeeBasis xtzFeeBasis = tezosFeeBasisCreateWithFee (kind, xtzFee);
    WKFeeBasis      feeBasis    = wkFeeBasisCreateAsXTZ (wallet->unitForFee, xtzFeeBasis);

    // The `state` with the confirmed `WKFeeBasis`.
    WKTransferState state = wkClientTransferBundleGetTransferState (bundle, feeBasis);

    if (NULL != transfer) {
        wkTransferSetUids  (transfer, bundle->uids);
        wkTransferSetState (transfer, state);
    }
    else {

        transfer = wkTransferCreateAsXTZ (wallet->listenerTransfer,
                                              bundle->uids,
                                              wallet->unit,
                                              wallet->unitForFee,
                                              feeBasis,
                                              amount,
                                              source,
                                              target,
                                              state,
                                              xtzAccount,
                                              wkHashAsXTZ (hash),
                                              NULL);
        wkWalletAddTransfer (wallet, transfer);
    }

    wkWalletManagerRecoverTransferAttributesFromTransferBundle (wallet, transfer, bundle);

    wkTransferGive(transfer);
    wkHashGive (hash);

    wkAddressGive (source);
    wkAddressGive (target);

    wkFeeBasisGive (feeBasis);
    wkTransferStateGive (state);

    wkWalletGive (wallet);
}

static void
wkWalletManagerRecoverTransfersFromTransferBundlesXTZ (WKWalletManager manager,
                                                       OwnershipKept BRArrayOf (WKClientTransferBundle) bundles) {
    for (size_t index = 0; index < array_count(bundles); index++)
        wkWalletManagerRecoverTransferFromTransferBundleXTZ (manager, bundles[index]);
}

static int64_t
cwmParseInt64 (const char *string, bool *error) {
    if (!string) { *error = true; return 0; }
    return (int64_t) strtoll(string, NULL, 0);
}


static const char *cwmAppendSuffix (char *buffer, const char *string, const char *suffix) {
    sprintf (buffer, "%s%s", string, suffix);
    return buffer;
}

static BRTezosOperationFeeBasis
wkWalletManagerRecoverOperationFeeBasis (const char *suffix, size_t count, const char **keys, const char **vals, bool *parseError) {
    assert (NULL == suffix || strlen(suffix) < 32);
    if (NULL == suffix) suffix = "";

    char key[128];

    bool hasKind = false;
    BRTezosOperationKind kind = wkWalletManagerRecoverOperationKind (cwmAppendSuffix (key, "kind", suffix), count, keys, vals, &hasKind);

    *parseError = false;
    int64_t fee         = cwmParseInt64 (cwmLookupAttributeValueForKey (cwmAppendSuffix (key, "fee",          suffix), count, keys, vals), parseError);
    int64_t burn        = cwmParseInt64 (cwmLookupAttributeValueForKey (cwmAppendSuffix (key, "burn",         suffix), count, keys, vals), parseError);
    int64_t gasUsed     = cwmParseInt64 (cwmLookupAttributeValueForKey (cwmAppendSuffix (key, "consumed_gas", suffix), count, keys, vals), parseError);
//    int64_t storageUsed = (int64_t) cwmParseUInt64 (cwmLookupAttributeValueForKey (cwmAppendSuffix (key, "storage_size", suffix), count, keys, vals), parseError);
    int64_t counter     = cwmParseInt64 (cwmLookupAttributeValueForKey (cwmAppendSuffix (key, "counter",      suffix), count, keys, vals), parseError);

    return tezosOperationFeeBasisCreate (kind,
                                         (BRTezosUnitMutez) fee,
                                         gasUsed,
                                         0,
                                         counter,
                                         (BRTezosUnitMutez) burn);
}

static WKFeeBasis
wkWalletManagerRecoverFeeBasisFromFeeEstimateXTZ (WKWalletManager cwm,
                                                  WKTransfer transfer,
                                                  WKNetworkFee networkFee,
                                                  double costUnits,
                                                  size_t attributesCount,
                                                  OwnershipKept const char **attributeKeys,
                                                  OwnershipKept const char **attributeVals) {
    BRTezosTransaction xtzTransaction = wkTransferCoerceXTZ(transfer)->originatingTransaction;
    assert (NULL != xtzTransaction);

    size_t           sizeInBytes   = tezosTransactionGetSignedBytesCount (xtzTransaction);
    BRTezosUnitMutez mutezPerKByte = 1000 * tezosMutezCreate (networkFee->pricePerCostFactor /* mutez/byte */);

    bool error_0, error_1;

    BRTezosOperationFeeBasis operationFeeBasis_0 = wkWalletManagerRecoverOperationFeeBasis ("_0", attributesCount, attributeKeys, attributeVals, &error_0);
    BRTezosOperationFeeBasis operationFeeBasis_1 = wkWalletManagerRecoverOperationFeeBasis ("_1", attributesCount, attributeKeys, attributeVals, &error_1);

    bool hasRevealOperationFeeBasis = !error_1;

    //
    // The result must include margin; we expect a) the User to confirm the upper-limit to the
    // fee and b) we need to ensure that the submission goes through.
    //
    operationFeeBasis_0 = tezosOperationFeeBasisApplyMargin (operationFeeBasis_0, mutezPerKByte, sizeInBytes, 10);
    operationFeeBasis_1 = tezosOperationFeeBasisApplyMargin (operationFeeBasis_1, mutezPerKByte, sizeInBytes, 10);

    BRTezosOperationFeeBasis primaryOperationFeeBasis = (hasRevealOperationFeeBasis
                                                         ? operationFeeBasis_1
                                                         : operationFeeBasis_0);

    BRTezosOperationFeeBasis revealOperationFeeBasis = (hasRevealOperationFeeBasis
                                                        ? operationFeeBasis_0
                                                        : tezosOperationFeeBasisCreateEmpty (TEZOS_OP_REVEAL));

    BRTezosFeeBasis xtzFeeBasis = (!hasRevealOperationFeeBasis
                                   ? tezosFeeBasisCreate (mutezPerKByte, primaryOperationFeeBasis)
                                   : tezosFeeBasisCreateWithReveal (mutezPerKByte, primaryOperationFeeBasis, revealOperationFeeBasis));

    return wkFeeBasisCreateAsXTZ (networkFee->pricePerCostFactorUnit, xtzFeeBasis);
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
    wkWalletManagerRecoverTransfersFromTransactionBundlesXTZ,
    wkWalletManagerRecoverTransfersFromTransferBundlesXTZ,
    wkWalletManagerRecoverFeeBasisFromFeeEstimateXTZ,
    wkWalletManagerWalletSweeperValidateSupportedXTZ,
    wkWalletManagerCreateWalletSweeperXTZ
};
