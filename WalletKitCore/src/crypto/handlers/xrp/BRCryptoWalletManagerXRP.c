//
//  BRCryptoWalletManagerXRP.c
//  
//
//  Created by Ehsan Rezaie on 2020-05-12.
//

#include "BRCryptoXRP.h"

#include "crypto/BRCryptoAccountP.h"
#include "crypto/BRCryptoNetworkP.h"
#include "crypto/BRCryptoKeyP.h"
#include "crypto/BRCryptoClientP.h"
#include "crypto/BRCryptoWalletP.h"
#include "crypto/BRCryptoAmountP.h"
#include "crypto/BRCryptoWalletManagerP.h"
#include "crypto/BRCryptoFileService.h"

#include "ripple/BRRippleWallet.h"
#include "ripple/BRRippleAccount.h"


// MARK: - Events

//TODO:XRP make common
const BREventType *xrpEventTypes[] = {
    // ...
};

const unsigned int
xrpEventTypesCount = (sizeof (xrpEventTypes) / sizeof (BREventType*));


static BRCryptoWalletManagerXRP
cryptoWalletManagerCoerce (BRCryptoWalletManager wm) {
    assert (CRYPTO_NETWORK_TYPE_XRP == wm->type);
    return (BRCryptoWalletManagerXRP) wm;
}

// MARK: - Handlers

static BRCryptoWalletManager
cryptoWalletManagerCreateHandlerXRP (BRCryptoListener listener,
                                     BRCryptoClient client,
                                     BRCryptoAccount account,
                                     BRCryptoNetwork network,
                                     BRCryptoSyncMode mode,
                                     BRCryptoAddressScheme scheme,
                                     const char *path) {
    BRCryptoWalletManager managerBase = cryptoWalletManagerAllocAndInit (sizeof (struct BRCryptoWalletManagerXRPRecord),
                                                                         cryptoNetworkGetType(network),
                                                                         listener,
                                                                         client,
                                                                         account,
                                                                         network,
                                                                         scheme,
                                                                         path,
                                                                         CRYPTO_CLIENT_REQUEST_USE_TRANSACTIONS);
    BRCryptoWalletManagerXRP manager = cryptoWalletManagerCoerce (managerBase);

    // XRP Stuff
    manager->ignoreTBD = 0;

    return managerBase;
}

static void
cryptoWalletManagerReleaseHandlerXRP (BRCryptoWalletManager manager) {

}

static void
cryptoWalletManagerInitializeHandlerXRP (BRCryptoWalletManager manager) {
    BRRippleAccount xrpAccount = cryptoAccountAsXRP(manager->account);
    BRRippleWallet xrpWallet = rippleWalletCreate(xrpAccount);
    
    // Create the primary BRCryptoWallet
    BRCryptoNetwork  network       = manager->network;
    BRCryptoCurrency currency      = cryptoNetworkGetCurrency (network);
    BRCryptoUnit     unitAsBase    = cryptoNetworkGetUnitAsBase    (network, currency);
    BRCryptoUnit     unitAsDefault = cryptoNetworkGetUnitAsDefault (network, currency);
    
    manager->wallet = cryptoWalletCreateAsXRP (unitAsDefault, unitAsBase, xrpWallet);
    array_add (manager->wallets, manager->wallet);
    
    //TODO:XRP load transfers from fileService
    
    cryptoUnitGive (unitAsDefault);
    cryptoUnitGive (unitAsBase);
    cryptoCurrencyGive (currency);
}

static BRFileService
crytpWalletManagerCreateFileServiceXRP (BRCryptoWalletManager manager,
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
cryptoWalletManagerGetEventTypesXRP (BRCryptoWalletManager manager,
                                     size_t *eventTypesCount) {
    assert (NULL != eventTypesCount);
    *eventTypesCount = xrpEventTypesCount;
    return xrpEventTypes;
}

static BRCryptoBoolean
cryptoWalletManagerSignTransactionWithSeedHandlerXRP (BRCryptoWalletManager manager,
                                                      BRCryptoWallet walletBase,
                                                      BRCryptoTransfer transferBase,
                                                      UInt512 seed) {
    BRRippleAccount account = cryptoAccountAsXRP (manager->account);
    BRRippleTransfer transfer = cryptoTransferCoerceXRP(transferBase)->xrpTransfer;
    BRRippleTransaction transaction = rippleTransferGetTransaction (transfer);
    if (transaction) {
        size_t tx_size = rippleAccountSignTransaction (account, transaction, seed);
        return AS_CRYPTO_BOOLEAN(tx_size > 0);
    } else {
        return CRYPTO_FALSE;
    }
}

static BRCryptoBoolean
cryptoWalletManagerSignTransactionWithKeyHandlerXRP (BRCryptoWalletManager manager,
                                                     BRCryptoWallet wallet,
                                                     BRCryptoTransfer transfer,
                                                     BRCryptoKey key) {
    assert(0);
    return CRYPTO_FALSE;
}

static BRCryptoAmount
cryptoWalletManagerEstimateLimitHandlerXRP (BRCryptoWalletManager cwm,
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
        
        // Ripple has fixed network fee (costFactor = 1.0)
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
cryptoWalletManagerEstimateFeeBasisHandlerXRP (BRCryptoWalletManager cwm,
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
cryptoWalletManagerRecoverTransfersFromTransactionBundleHandlerXRP (BRCryptoWalletManager manager,
                                                                    OwnershipKept BRCryptoClientTransactionBundle bundle) {
    // Not XRP functionality
    assert (0);
}

static void
cryptoWalletManagerRecoverTransferFromTransferBundleHandlerXRP (BRCryptoWalletManager cwm,
                                                                OwnershipKept BRCryptoClientTransferBundle bundle) {
    // create BRRippleTransfer
    
    BRRippleWallet xrpWallet = cryptoWalletAsXRP (cwm->wallet);
    
    BRRippleUnitDrops amountDrops, feeDrops = 0;
    sscanf(bundle->amount, "%" PRIu64, &amountDrops);
    if (NULL != bundle->fee) sscanf(bundle->fee, "%" PRIu64, &feeDrops);
    BRRippleAddress toAddress   = rippleAddressCreateFromString (bundle->to,   false);
    BRRippleAddress fromAddress = rippleAddressCreateFromString (bundle->from, false);
    // Convert the hash string to bytes
    BRRippleTransactionHash txId;
    hexDecode(txId.bytes, sizeof(txId.bytes), bundle->hash, strlen(bundle->hash));
    int error = (CRYPTO_TRANSFER_STATE_ERRORED == bundle->status);

    BRRippleTransfer xrpTransfer = rippleTransferCreate(fromAddress, toAddress, amountDrops, feeDrops, txId, bundle->blockTimestamp, bundle->blockNumber, error);

    rippleAddressFree (toAddress);
    rippleAddressFree (fromAddress);
    
    rippleWalletAddTransfer (xrpWallet, xrpTransfer); //TODO:XRP needed?
    
    // create BRCryptoTransfer
    
    BRCryptoWallet wallet = cryptoWalletManagerGetWallet (cwm);
    
    BRCryptoTransfer baseTransfer = cryptoTransferCreateAsXRP (wallet->unit,
                                                               wallet->unitForFee,
                                                               xrpWallet,
                                                               xrpTransfer);
    cryptoWalletAddTransfer (wallet, baseTransfer);
    
    //TODO:XRP attributes
    //TODO:XRP save to fileService
    //TODO:XRP announce
    
    rippleTransferFree (xrpTransfer);
}

static BRCryptoClientP2PManager
crytpWalletManagerCreateP2PManagerHandlerXRP (BRCryptoWalletManager cwm) {
    // not supported
    return NULL;
}

extern BRCryptoWalletSweeperStatus
cryptoWalletManagerWalletSweeperValidateSupportedXRP (BRCryptoWalletManager cwm,
                                                      BRCryptoWallet wallet,
                                                      BRCryptoKey key) {
    return CRYPTO_WALLET_SWEEPER_UNSUPPORTED_CURRENCY;
}

extern BRCryptoWalletSweeper
cryptoWalletManagerCreateWalletSweeperXRP (BRCryptoWalletManager cwm,
                                           BRCryptoWallet wallet,
                                           BRCryptoKey key) {
    // not supported
    return NULL;
}

BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersXRP = {
    cryptoWalletManagerCreateHandlerXRP,
    cryptoWalletManagerReleaseHandlerXRP,
    cryptoWalletManagerInitializeHandlerXRP,
    crytpWalletManagerCreateFileServiceXRP,
    cryptoWalletManagerGetEventTypesXRP,
    cryptoWalletManagerSignTransactionWithSeedHandlerXRP,
    cryptoWalletManagerSignTransactionWithKeyHandlerXRP,
    cryptoWalletManagerEstimateLimitHandlerXRP,
    cryptoWalletManagerEstimateFeeBasisHandlerXRP,
    crytpWalletManagerCreateP2PManagerHandlerXRP,
    cryptoWalletManagerRecoverTransfersFromTransactionBundleHandlerXRP,
    cryptoWalletManagerRecoverTransferFromTransferBundleHandlerXRP,
    cryptoWalletManagerWalletSweeperValidateSupportedXRP,
    cryptoWalletManagerCreateWalletSweeperXRP
};
