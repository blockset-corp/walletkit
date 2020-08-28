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

#include "tezos/BRTezosAccount.h"


// MARK: - Events

//TODO:XTZ make common
const BREventType *xtzEventTypes[] = {
    // ...
};

const unsigned int
xtzEventTypesCount = (sizeof (xtzEventTypes) / sizeof (BREventType*));


//static BRCryptoWalletManagerXTZ
//cryptoWalletManagerCoerce (BRCryptoWalletManager wm) {
//    assert (CRYPTO_NETWORK_TYPE_XTZ == wm->type);
//    return (BRCryptoWalletManagerXTZ) wm;
//}

// MARK: - Handlers

static BRCryptoWalletManager
cryptoWalletManagerCreateXTZ (BRCryptoWalletManagerListener listener,
                              BRCryptoClient client,
                              BRCryptoAccount account,
                              BRCryptoNetwork network,
                              BRCryptoSyncMode mode,
                              BRCryptoAddressScheme scheme,
                              const char *path) {
    BRCryptoWalletManager manager = cryptoWalletManagerAllocAndInit (sizeof (struct BRCryptoWalletManagerXTZRecord),
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

    pthread_mutex_unlock (&manager->lock);
    return manager;
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
    return fileServiceCreateFromTypeSpecfications (basePath, currency, network,
                                                   context, handler,
                                                   fileServiceSpecificationsCount,
                                                   fileServiceSpecifications);
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
    BRTezosBlockHash lastBlockHash; //TODO:TEZOS get from network (/blockchains)
    BRTezosAccount account = cryptoAccountAsXTZ (manager->account);
    BRTezosTransaction tid = tezosTransferGetTransaction (cryptoTransferCoerceXTZ(transfer)->xtzTransfer);
    bool needsReveal = (TEZOS_OP_TRANSACTION == tezosTransactionGetOperationKind(tid)) && cryptoWalletNeedsRevealXTZ(wallet);
    
    if (tid) {
        size_t tx_size = tezosTransactionSignTransaction (tid, account, seed, lastBlockHash, needsReveal);
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
    UInt256 amount = UINT256_ZERO;
    
    *needEstimate = CRYPTO_FALSE;
    *isZeroIfInsuffientFunds = CRYPTO_FALSE;
    
    if (CRYPTO_TRUE == asMaximum) {
        BRCryptoAmount minBalance = wallet->balanceMinimum;
        assert(minBalance);
        
        // Available balance based on minimum wallet balance
        BRCryptoAmount balance = cryptoAmountSub(wallet->balance, minBalance);
        
        // Tezos has fixed network fee (costFactor = 1.0)
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
    
    return cryptoAmountCreateInternal (unit,
                                       CRYPTO_FALSE,
                                       amount,
                                       0);
}

static BRCryptoFeeBasis
cryptoWalletManagerEstimateFeeBasisXTZ (BRCryptoWalletManager manager,
                                        BRCryptoWallet wallet,
                                        BRCryptoCookie cookie,
                                        BRCryptoAddress target,
                                        BRCryptoAmount amount,
                                        BRCryptoNetworkFee networkFee) {
    BRCryptoAmount pricePerCostFactor = cryptoNetworkFeeGetPricePerCostFactor (networkFee);
    double costFactor = 1.0;  // 'cost factor' is 'transaction'
    
    return cryptoFeeBasisCreate (pricePerCostFactor, costFactor);
}

static void
cryptoWalletManagerRecoverTransfersFromTransactionBundleXTZ (BRCryptoWalletManager manager,
                                                             OwnershipKept BRCryptoClientTransactionBundle bundle) {
    // Not XTZ functionality
    assert (0);
}

#ifdef REFACTOR
extern BRGenericTransferState
genManagerRecoverTransferState (BRGenericManager gwm,
                                uint64_t timestamp,
                                uint64_t blockHeight,
                                BRGenericFeeBasis feeBasis,
                                BRCryptoBoolean error) {
    return (blockHeight == BLOCK_HEIGHT_UNBOUND && CRYPTO_TRUE == error
            ? genTransferStateCreateOther (GENERIC_TRANSFER_STATE_ERRORED)
            : (blockHeight == BLOCK_HEIGHT_UNBOUND
               ? genTransferStateCreateOther(GENERIC_TRANSFER_STATE_SUBMITTED)
               : genTransferStateCreateIncluded (blockHeight,
                                                 GENERIC_TRANSFER_TRANSACTION_INDEX_UNKNOWN,
                                                 timestamp,
                                                 feeBasis,
                                                 error,
                                                 NULL)));
}

BRGenericTransferState transferState = genManagerRecoverTransferState (gwm,
                                                                       timestamp,
                                                                       blockHeight,
                                                                       feeBasis,
                                                                       AS_CRYPTO_BOOLEAN (0 == error));
genTransferSetState (transfer, transferState);
#endif

static void
cryptoWalletManagerRecoverTransferFromTransferBundleXTZ (BRCryptoWalletManager manager,
                                                         OwnershipKept BRCryptoClientTransferBundle bundle) {
    // create BRTezosTransfer

    BRTezosAccount xtzAccount = cryptoAccountAsXTZ(manager->account);
    
    BRTezosUnitMutez amountDrops, feeDrops = 0;
    sscanf(bundle->amount, "%" PRIu64, &amountDrops);
    if (NULL != bundle->fee) sscanf(bundle->fee, "%" PRIu64, &feeDrops);
    BRTezosAddress toAddress   = tezosAddressCreateFromString (bundle->to,   false);
    BRTezosAddress fromAddress = tezosAddressCreateFromString (bundle->from, false);
    // Convert the hash string to bytes
    BRTezosTransactionHash txId;
    hexDecode(txId.bytes, sizeof(txId.bytes), bundle->hash, strlen(bundle->hash));
    int error = (CRYPTO_TRANSFER_STATE_ERRORED == bundle->status);
#ifdef REFACTOR
    ||
    (CRYPTO_TRANSFER_STATE_INCLUDED == bundle->status
     && CRYPTO_FALSE == state.u.included.success));
#endif
    BRTezosTransfer xtzTransfer = tezosTransferCreate(fromAddress, toAddress, amountDrops, feeDrops, txId, bundle->blockTimestamp, bundle->blockNumber, error);
    
    tezosAddressFree (toAddress);
    tezosAddressFree (fromAddress);

    // create BRCryptoTransfer
    
    BRCryptoWallet wallet = cryptoWalletManagerGetWallet (manager);
    
    BRCryptoTransfer baseTransfer = cryptoTransferCreateAsXTZ (wallet->listenerTransfer,
                                                               wallet->unit,
                                                               wallet->unitForFee,
                                                               xtzAccount,
                                                               xtzTransfer);
    cryptoWalletAddTransfer (wallet, baseTransfer);
    
    BRCryptoTransferState transferState =
    (CRYPTO_TRANSFER_STATE_INCLUDED == bundle->status
     ? cryptoTransferStateIncludedInit (bundle->blockNumber,
                                        bundle->blockTransactionIndex,
                                        bundle->blockTimestamp,
                                        NULL,
                                        CRYPTO_TRUE,
                                        NULL)
     : (CRYPTO_TRANSFER_STATE_ERRORED == bundle->status
        ? cryptoTransferStateErroredInit ((BRCryptoTransferSubmitError) { CRYPTO_TRANSFER_SUBMIT_ERROR_UNKNOWN })
        : cryptoTransferStateInit (bundle->status)));
    
    cryptoTransferSetState (baseTransfer, transferState);
    
    //TODO:XTZ attributes
    //TODO:XTZ save to fileService
    //TODO:XTZ announce
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
                                    BRCryptoCurrency currency) {
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
    cryptoWalletManagerRecoverTransfersFromTransactionBundleXTZ,
    cryptoWalletManagerRecoverTransferFromTransferBundleXTZ,
    cryptoWalletManagerWalletSweeperValidateSupportedXTZ,
    cryptoWalletManagerCreateWalletSweeperXTZ
};
