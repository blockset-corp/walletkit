//
//  BRCryptoWalletManagerXTZ.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoXTZ.h"

#include "crypto/BRCryptoAccountP.h"
#include "crypto/BRCryptoNetworkP.h"
#include "crypto/BRCryptoKeyP.h"
#include "crypto/BRCryptoClientP.h"
#include "crypto/BRCryptoWalletP.h"
#include "crypto/BRCryptoAmountP.h"
#include "crypto/BRCryptoWalletManagerP.h"
#include "crypto/BRCryptoFileService.h"
#include "crypto/BRCryptoHashP.h"

#include "tezos/BRTezosAccount.h"


// MARK: - Events

static const BREventType *xtzEventTypes[] = {
    CRYPTO_CLIENT_EVENT_TYPES
};

static const unsigned int
xtzEventTypesCount = (sizeof (xtzEventTypes) / sizeof (BREventType*));

// MARK: - Handlers

static BRCryptoWalletManager
cryptoWalletManagerCreateXTZ (BRCryptoWalletManagerListener listener,
                              BRCryptoClient client,
                              BRCryptoAccount account,
                              BRCryptoNetwork network,
                              BRCryptoSyncMode mode,
                              BRCryptoAddressScheme scheme,
                              const char *path) {
    return cryptoWalletManagerAllocAndInit (sizeof (struct BRCryptoWalletManagerXTZRecord),
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
cryptoWalletManagerReleaseXTZ (BRCryptoWalletManager manager) {
}

static BRFileService
crytpWalletManagerCreateFileServiceXTZ (BRCryptoWalletManager manager,
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
cryptoWalletManagerGetEventTypesXTZ (BRCryptoWalletManager manager,
                                     size_t *eventTypesCount) {
    assert (NULL != eventTypesCount);
    *eventTypesCount = xtzEventTypesCount;
    return xtzEventTypes;
}

static BRCryptoClientP2PManager
crytpWalletManagerCreateP2PManagerXTZ (BRCryptoWalletManager manager) {
    // not supported
    return NULL;
}

static BRCryptoBoolean
cryptoWalletManagerSignTransactionWithSeedXTZ (BRCryptoWalletManager manager,
                                               BRCryptoWallet wallet,
                                               BRCryptoTransfer transfer,
                                               UInt512 seed) {
    BRTezosHash lastBlockHash = cryptoHashAsXTZ (cryptoNetworkGetVerifiedBlockHash (manager->network));
    BRTezosAccount account = cryptoAccountAsXTZ (manager->account);
    BRTezosTransaction tid = tezosTransferGetTransaction (cryptoTransferCoerceXTZ(transfer)->xtzTransfer);
    bool needsReveal = (TEZOS_OP_TRANSACTION == tezosTransactionGetOperationKind(tid)) && cryptoWalletNeedsRevealXTZ(wallet);
    
    if (tid) {
        size_t tx_size = tezosTransactionSerializeAndSign (tid, account, seed, lastBlockHash, needsReveal);
        return AS_CRYPTO_BOOLEAN(tx_size > 0);
    } else {
        return CRYPTO_FALSE;
    }
}

static BRCryptoBoolean
cryptoWalletManagerSignTransactionWithKeyXTZ (BRCryptoWalletManager manager,
                                              BRCryptoWallet wallet,
                                              BRCryptoTransfer transfer,
                                              BRCryptoKey key) {
    assert(0);
    return CRYPTO_FALSE;
}

static BRCryptoAmount
cryptoWalletManagerEstimateLimitXTZ (BRCryptoWalletManager manager,
                                     BRCryptoWallet  wallet,
                                     BRCryptoBoolean asMaximum,
                                     BRCryptoAddress target,
                                     BRCryptoNetworkFee networkFee,
                                     BRCryptoBoolean *needEstimate,
                                     BRCryptoBoolean *isZeroIfInsuffientFunds,
                                     BRCryptoUnit unit) {
    // We always need an estimate as we do not know the fees.
    *needEstimate = asMaximum;

    return (CRYPTO_TRUE == asMaximum
            ? cryptoWalletGetBalance (wallet)        // Maximum is balance - fees 'needEstimate'
            : cryptoAmountCreateInteger (0, unit));  // No minimum
}

static BRCryptoFeeBasis
cryptoWalletManagerEstimateFeeBasisXTZ (BRCryptoWalletManager manager,
                                        BRCryptoWallet wallet,
                                        BRCryptoCookie cookie,
                                        BRCryptoAddress target,
                                        BRCryptoAmount amount,
                                        BRCryptoNetworkFee networkFee,
                                        size_t attributesCount,
                                        OwnershipKept BRCryptoTransferAttribute *attributes) {
    BRTezosUnitMutez mutezPerByte = tezosMutezCreate (networkFee->pricePerCostFactor) / 1000; // given as nanotez/byte
    BRTezosFeeBasis xtzFeeBasis = tezosDefaultFeeBasis (mutezPerByte);
    BRCryptoFeeBasis feeBasis = cryptoFeeBasisCreateAsXTZ (networkFee->pricePerCostFactorUnit, xtzFeeBasis);

    BRCryptoCurrency currency = cryptoAmountGetCurrency (amount);
    BRCryptoTransfer transfer = cryptoWalletCreateTransferXTZ (wallet,
                                                               target,
                                                               amount,
                                                               feeBasis,
                                                               attributesCount,
                                                               attributes,
                                                               currency,
                                                               wallet->unit,
                                                               wallet->unitForFee);

    cryptoCurrencyGive(currency);
    
    // serialize the transaction for fee estimation payload
    BRTezosHash lastBlockHash = cryptoHashAsXTZ (cryptoNetworkGetVerifiedBlockHash (manager->network));
    BRTezosAccount account = cryptoAccountAsXTZ (manager->account);
    BRTezosTransaction tid = tezosTransferGetTransaction (cryptoTransferCoerceXTZ(transfer)->xtzTransfer);
    bool needsReveal = (TEZOS_OP_TRANSACTION == tezosTransactionGetOperationKind(tid)) && cryptoWalletNeedsRevealXTZ(wallet);
    
    tezosTransactionSerializeForFeeEstimation(tid,
                                              account,
                                              lastBlockHash,
                                              needsReveal);
    
    // serialized tx size is needed for fee estimation
    cryptoFeeBasisGive (feeBasis);
    feeBasis = cryptoFeeBasisCreateAsXTZ (networkFee->pricePerCostFactorUnit, tezosTransactionGetFeeBasis(tid));

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

static void
cryptoWalletManagerRecoverTransfersFromTransactionBundleXTZ (BRCryptoWalletManager manager,
                                                             OwnershipKept BRCryptoClientTransactionBundle bundle) {
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
cryptoWalletManagerRecoverTransferFromTransferBundleXTZ (BRCryptoWalletManager manager,
                                                         OwnershipKept BRCryptoClientTransferBundle bundle) {
    // create BRTezosTransfer

    BRTezosAccount xtzAccount = cryptoAccountAsXTZ(manager->account);
    
    BRTezosUnitMutez amountMutez, feeMutez = 0;
    sscanf(bundle->amount, "%" PRIu64, &amountMutez);
    if (NULL != bundle->fee) sscanf(bundle->fee, "%" PRIu64, &feeMutez);
    BRTezosAddress toAddress   = tezosAddressCreateFromString (bundle->to,   false);
    BRTezosAddress fromAddress = tezosAddressCreateFromString (bundle->from, false);

    // Convert the hash string to bytes
    BRCryptoHash bundleHash = cryptoHashCreateFromStringAsXTZ (bundle->hash);
    BRTezosHash txId = cryptoHashAsXTZ (bundleHash);
    cryptoHashGive(bundleHash);

    int error = (CRYPTO_TRANSFER_STATE_ERRORED == bundle->status);

    bool xtzTransferNeedFree = true;
    BRTezosTransfer xtzTransfer = tezosTransferCreate(fromAddress, toAddress, amountMutez, feeMutez, txId, bundle->blockTimestamp, bundle->blockNumber, error);

    // create BRCryptoTransfer
    
    BRCryptoWallet wallet = cryptoWalletManagerGetWallet (manager);
    BRCryptoHash hash = cryptoHashCreateAsXTZ (txId);
    
    // A transaction may include a "burn" transfer to target address 'unknown' in addition to the
    // normal transfer, both sharing the same hash. Typically occurs when sending to an un-revealed
    // address.  It must be included since the burn amount is subtracted from wallet balance, but
    // is not considered a normal fee.
    BRCryptoAddress target = cryptoAddressCreateAsXTZ (toAddress);
    BRCryptoTransfer baseTransfer = cryptoWalletGetTransferByHashOrUIDSAndTargetXTZ (wallet, hash, bundle->uids, target);
    
    cryptoAddressGive (target);
    cryptoHashGive (hash);
    tezosAddressFree (fromAddress);

    BRTezosFeeBasis xtzFeeBasis = tezosFeeBasisCreateActual (feeMutez);

    BRCryptoFeeBasis      feeBasis = cryptoFeeBasisCreateAsXTZ (wallet->unitForFee, xtzFeeBasis);
    BRCryptoTransferState state    = cryptoClientTransferBundleGetTransferState (bundle, feeBasis);

    if (NULL == baseTransfer) {
        baseTransfer = cryptoTransferCreateAsXTZ (wallet->listenerTransfer,
                                                  bundle->uids,
                                                  wallet->unit,
                                                  wallet->unitForFee,
                                                  state,
                                                  xtzAccount,
                                                  xtzTransfer);
        xtzTransferNeedFree = false;
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
    
    if (xtzTransferNeedFree)
        tezosTransferFree (xtzTransfer);
}

static BRCryptoFeeBasis
cryptoWalletManagerRecoverFeeBasisFromFeeEstimateXTZ (BRCryptoWalletManager cwm,
                                                      BRCryptoNetworkFee networkFee,
                                                      BRCryptoFeeBasis initialFeeBasis,
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
    double sizeInKBytes = cryptoFeeBasisCoerceXTZ(initialFeeBasis)->xtzFeeBasis.u.initial.sizeInKBytes;

    BRTezosFeeBasis feeBasis = tezosFeeBasisCreateEstimate (mutezPerKByte,
                                                            sizeInKBytes,
                                                            gasUsed,
                                                            storageUsed,
                                                            counter);
    
    return cryptoFeeBasisCreateAsXTZ (networkFee->pricePerCostFactorUnit, feeBasis);
}

extern BRCryptoWalletSweeperStatus
cryptoWalletManagerWalletSweeperValidateSupportedXTZ (BRCryptoWalletManager manager,
                                                      BRCryptoWallet wallet,
                                                      BRCryptoKey key) {
    return CRYPTO_WALLET_SWEEPER_UNSUPPORTED_CURRENCY;
}

extern BRCryptoWalletSweeper
cryptoWalletManagerCreateWalletSweeperXTZ (BRCryptoWalletManager manager,
                                           BRCryptoWallet wallet,
                                           BRCryptoKey key) {
    // not supported
    return NULL;
}

static BRCryptoWallet
cryptoWalletManagerCreateWalletXTZ (BRCryptoWalletManager manager,
                                    BRCryptoCurrency currency,
                                    Nullable OwnershipKept BRArrayOf(BRCryptoClientTransactionBundle) transactions,
                                    Nullable OwnershipKept BRArrayOf(BRCryptoClientTransferBundle) transfers) {
    BRTezosAccount xtzAccount = cryptoAccountAsXTZ(manager->account);

    // Create the primary BRCryptoWallet
    BRCryptoNetwork  network       = manager->network;
    BRCryptoUnit     unitAsBase    = cryptoNetworkGetUnitAsBase    (network, currency);
    BRCryptoUnit     unitAsDefault = cryptoNetworkGetUnitAsDefault (network, currency);
    
    BRCryptoWallet wallet = cryptoWalletCreateAsXTZ (manager->listenerWallet,
                                                     unitAsDefault,
                                                     unitAsDefault,
                                                     xtzAccount);
    cryptoWalletManagerAddWallet (manager, wallet);
    
    // TODO:XTZ load transfers from fileService
    
    cryptoUnitGive (unitAsDefault);
    cryptoUnitGive (unitAsBase);
    
    return wallet;
}

BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersXTZ = {
    cryptoWalletManagerCreateXTZ,
    cryptoWalletManagerReleaseXTZ,
    crytpWalletManagerCreateFileServiceXTZ,
    cryptoWalletManagerGetEventTypesXTZ,
    crytpWalletManagerCreateP2PManagerXTZ,
    cryptoWalletManagerCreateWalletXTZ,
    cryptoWalletManagerSignTransactionWithSeedXTZ,
    cryptoWalletManagerSignTransactionWithKeyXTZ,
    cryptoWalletManagerEstimateLimitXTZ,
    cryptoWalletManagerEstimateFeeBasisXTZ,
    NULL, // BRCryptoWalletManagerSaveTransactionBundleHandler
    NULL, // BRCryptoWalletManagerSaveTransactionBundleHandler
    cryptoWalletManagerRecoverTransfersFromTransactionBundleXTZ,
    cryptoWalletManagerRecoverTransferFromTransferBundleXTZ,
    cryptoWalletManagerRecoverFeeBasisFromFeeEstimateXTZ,
    cryptoWalletManagerWalletSweeperValidateSupportedXTZ,
    cryptoWalletManagerCreateWalletSweeperXTZ
};
