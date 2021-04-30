//
//  WKWalletBTC.c
//  WalletKitCore
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKBTC.h"

#include "bitcoin/BRBitcoinWallet.h"

#define DEFAULT_FEE_BASIS_SIZE_IN_BYTES     (200)
#define DEFAULT_TIDS_UNRESOLVED_COUNT         (2)

private_extern WKWalletBTC
wkWalletCoerceBTC (WKWallet wallet) {
    assert (WK_NETWORK_TYPE_BTC == wallet->type ||
            WK_NETWORK_TYPE_BCH == wallet->type ||
            WK_NETWORK_TYPE_BSV == wallet->type);
    return (WKWalletBTC) wallet;
}

typedef struct {
    BRBitcoinWallet *wid;
} WKWalletCreateContextBTC;

static void
wkWalletCreateCallbackBTC (WKWalletCreateContext context,
                               WKWallet wallet) {
    WKWalletCreateContextBTC *contextBTC = (WKWalletCreateContextBTC*) context;
    WKWalletBTC walletBTC = wkWalletCoerceBTC (wallet);

    walletBTC->wid = contextBTC->wid;
    array_new (walletBTC->tidsUnresolved, DEFAULT_TIDS_UNRESOLVED_COUNT);
}


private_extern WKWallet
wkWalletCreateAsBTC (WKNetworkType type,
                         WKWalletListener listener,
                         WKUnit unit,
                         WKUnit unitForFee,
                         BRBitcoinWallet *wid) {
    WKAmount minBalance = wkAmountCreateInteger(0, unit);

    WKFeeBasis feeBasis = wkFeeBasisCreateAsBTC (unitForFee,
                                                           WK_FEE_BASIS_BTC_FEE_UNKNOWN,
                                                           btcWalletFeePerKb(wid),
                                                           DEFAULT_FEE_BASIS_SIZE_IN_BYTES);

    WKWalletCreateContextBTC contextBTC = {
        wid
    };

    WKWallet wallet = wkWalletAllocAndInit (sizeof (struct WKWalletBTCRecord),
                                                      type,
                                                      listener,
                                                      unit,
                                                      unitForFee,
                                                      minBalance,
                                                      NULL,
                                                      feeBasis,
                                                      &contextBTC,
                                                      wkWalletCreateCallbackBTC);

    wkAmountGive (minBalance);
    wkFeeBasisGive (feeBasis);

    return wallet;
}

static void
wkWalletReleaseBTC (WKWallet wallet) {
    WKWalletBTC walletBTC = wkWalletCoerceBTC(wallet);
    array_free_all (walletBTC->tidsUnresolved, btcTransactionFree);
}

private_extern BRBitcoinWallet *
wkWalletAsBTC (WKWallet wallet) {
    WKWalletBTC walletBTC = wkWalletCoerceBTC(wallet);
    return walletBTC->wid;
}

private_extern WKTransfer
wkWalletFindTransferAsBTC (WKWallet wallet,
                               BRBitcoinTransaction *btc) {
    WKTransfer transfer = NULL;
    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        if (WK_TRUE == wkTransferHasBTC (wallet->transfers[index], btc)) {
            transfer = wkTransferTake (wallet->transfers[index]);
            break;
        }
    }
    pthread_mutex_unlock (&wallet->lock);
    return transfer;
}

private_extern WKTransferBTC
wkWalletFindTransferByHashAsBTC (WKWallet wallet,
                                     UInt256 hash) {

    WKTransferBTC transfer = NULL;
    if (! UInt256IsZero(hash)) {
        pthread_mutex_lock (&wallet->lock);
        for (size_t index = 0; index < array_count(wallet->transfers); index++) {
            transfer = (WKTransferBTC) wallet->transfers[index];
            if (UInt256Eq (hash, transfer->tid->txHash))
                break;
            transfer = NULL;
        }
        pthread_mutex_unlock (&wallet->lock);
    }
    return transfer;
}

private_extern void
wkWalletAddUnresolvedAsBTC (WKWallet wallet,
                                OwnershipGiven BRBitcoinTransaction *tid) {
    WKWalletBTC walletBTC = wkWalletCoerceBTC(wallet);

    pthread_mutex_lock (&wallet->lock);
    array_add (walletBTC->tidsUnresolved, tid);
    pthread_mutex_unlock (&wallet->lock);
}

private_extern void
wkWalletUpdUnresolvedAsBTC (WKWallet wallet,
                                const UInt256 *hash,
                                uint32_t blockHeight,
                                uint32_t timestamp) {
    WKWalletBTC walletBTC = wkWalletCoerceBTC(wallet);

    pthread_mutex_lock (&wallet->lock);

    for (size_t index = 0; index < array_count (walletBTC->tidsUnresolved); index++) {
        BRBitcoinTransaction *tid = walletBTC->tidsUnresolved[index];
        if (btcTransactionEq (tid, hash)) {
            tid->blockHeight = blockHeight;
            tid->timestamp   = timestamp;
        }
    }

    pthread_mutex_unlock (&wallet->lock);
}

private_extern size_t
wkWalletRemResolvedAsBTC (WKWallet wallet,
                              BRBitcoinTransaction **tids,
                              size_t tidsCount) {
    WKWalletBTC walletBTC = wkWalletCoerceBTC(wallet);

    pthread_mutex_lock (&wallet->lock);
    size_t count = array_count(walletBTC->tidsUnresolved);

    if (NULL != tids) {
        size_t  rIndex = 0;

        for (ssize_t tIndex = (ssize_t) MIN (tidsCount, count) - 1; tIndex >= 0; tIndex--)
            if (btcWalletTransactionIsResolved (walletBTC->wid, walletBTC->tidsUnresolved[tIndex])) {
                tids[rIndex++] = walletBTC->tidsUnresolved[tIndex];
                array_rm (walletBTC->tidsUnresolved, (size_t) tIndex);
            }

        count = rIndex;
    }
    pthread_mutex_unlock (&wallet->lock);
    return count;
}

static WKAddress
wkWalletGetAddressBTC (WKWallet wallet,
                           WKAddressScheme addressScheme) {
    assert (WK_ADDRESS_SCHEME_BTC_LEGACY == addressScheme ||
            WK_ADDRESS_SCHEME_BTC_SEGWIT == addressScheme);

    assert (WK_ADDRESS_SCHEME_BTC_SEGWIT != addressScheme ||
            WK_NETWORK_TYPE_BTC == wallet->type);

    WKWalletBTC walletBTC = wkWalletCoerceBTC(wallet);

    BRBitcoinWallet *wid = walletBTC->wid;

    BRAddress btcAddress = (WK_ADDRESS_SCHEME_BTC_SEGWIT == addressScheme
                            ? btcWalletReceiveAddress(wid)
                            : btcWalletLegacyAddress (wid));

    return wkAddressCreateAsBTC (wallet->type, btcAddress);
}

static bool
wkWalletHasAddressBTC (WKWallet wallet,
                           WKAddress address) {
    WKWalletBTC walletBTC = wkWalletCoerceBTC(wallet);

    BRBitcoinWallet *btcWallet = walletBTC->wid;

    WKNetworkType type;
    BRAddress btcAddress = wkAddressAsBTC (address, &type);

    if (btcWalletAddressIsUsed (btcWallet, btcAddress.s))
        return true;

    BRAddress btcLegacyAddress = btcWalletLegacyAddress (btcWallet);
    if (0 == memcmp (btcAddress.s, btcLegacyAddress.s, sizeof (btcAddress.s)))
        return true;

    if (WK_NETWORK_TYPE_BTC == type) {
        BRAddress btcSegwitAddress = btcWalletReceiveAddress (btcWallet);
        if (0 == memcmp (btcAddress.s, btcSegwitAddress.s, sizeof (btcAddress.s)))
            return true;
    }

    return false;
}

static bool
wkWalletIsEqualBTC (WKWallet wb1, WKWallet wb2) {
    WKWalletBTC w1 = wkWalletCoerceBTC(wb1);
    WKWalletBTC w2 = wkWalletCoerceBTC(wb2);

    // This does not compare the properties of `t1` to `t2`, just the 'id-ness'.  If the properties
    // are compared, one needs to be careful about the BRTransaction's timestamp.  Two transactions
    // with an identical hash can have different timestamps depending on how the transaction
    // is identified.  Specifically P2P and API found transactions *will* have different timestamps.
    return w1->wid == w2->wid;
}

extern size_t
wkWalletGetTransferAttributeCountBTC (WKWallet wallet,
                                          WKAddress target) {
    return 0;
}

extern WKTransferAttribute
wkWalletGetTransferAttributeAtBTC (WKWallet wallet,
                                       WKAddress target,
                                       size_t index) {
    return NULL;
}

extern WKTransferAttributeValidationError
wkWalletValidateTransferAttributeBTC (WKWallet wallet,
                                          OwnershipKept WKTransferAttribute attribute,
                                          WKBoolean *validates) {
    *validates = WK_TRUE;
    return (WKTransferAttributeValidationError) 0;
}

extern WKTransfer
wkWalletCreateTransferBTC (WKWallet  wallet,
                               WKAddress target,
                               WKAmount  amount,
                               WKFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept WKTransferAttribute *attributes,
                               WKCurrency currency,
                               WKUnit unit,
                               WKUnit unitForFee) {
    WKWalletBTC walletBTC = wkWalletCoerceBTC(wallet);

    BRBitcoinWallet *wid = walletBTC->wid;

    WKNetworkType addressType;
    BRAddress address = wkAddressAsBTC (target, &addressType);
    assert (addressType == wallet->type);

    WKBoolean overflow = WK_FALSE;
    uint64_t value = wkAmountGetIntegerRaw (amount, &overflow);
    if (WK_TRUE == overflow) { return NULL; }

    uint64_t feePerKb = wkFeeBasisAsBTC(estimatedFeeBasis);

    BRBitcoinTransaction *tid = btcWalletCreateTransactionWithFeePerKb (wid, feePerKb, value, address.s);

    return (NULL == tid
            ? NULL
            : wkTransferCreateAsBTC (wallet->listenerTransfer,
                                         unit,
                                         unitForFee,
                                         wid,
                                         tid,
                                         wallet->type));
}

extern WKTransfer
wkWalletCreateTransferMultipleBTC (WKWallet wallet,
                                       size_t outputsCount,
                                       WKTransferOutput *outputs,
                                       WKFeeBasis estimatedFeeBasis,
                                       WKCurrency currency,
                                       WKUnit unit,
                                       WKUnit unitForFee) {
    WKWalletBTC walletBTC = wkWalletCoerceBTC(wallet);

    BRBitcoinWallet *wid = walletBTC->wid;
    BRAddressParams params = btcWalletGetAddressParams(wid);

    BRBitcoinTxOutput txOutputs [outputsCount];
    memset (txOutputs, 0, outputsCount * sizeof(BRBitcoinTxOutput));

    for (size_t index = 0; index < outputsCount; index++) {
        WKTransferOutput *output = &outputs[index];
        BRBitcoinTxOutput *txOutput = &txOutputs[index];

        assert (wkWalletGetType(wallet) == wkAddressGetType(output->target));
        assert (wkAmountHasCurrency (output->amount, currency));

        WKNetworkType outputTargetType;
        BRAddress address = wkAddressAsBTC (output->target, &outputTargetType);
        assert (outputTargetType == wallet->type);

        WKBoolean overflow = WK_FALSE;
        uint64_t value = wkAmountGetIntegerRaw (output->amount, &overflow);
        assert (WK_TRUE != overflow);

        txOutput->amount = value;
        btcTxOutputSetAddress (txOutput, params, address.s);
    }

    uint64_t feePerKb = wkFeeBasisAsBTC(estimatedFeeBasis);

    BRBitcoinTransaction *tid = btcWalletCreateTxForOutputsWithFeePerKb (wid, feePerKb, txOutputs, outputsCount);

    return (NULL == tid
            ? NULL
            : wkTransferCreateAsBTC (wallet->listenerTransfer,
                                         unit,
                                         unitForFee,
                                         wid,
                                         tid,
                                         wallet->type));
}

static OwnershipGiven BRSetOf(WKAddress)
wkWalletGetAddressesForRecoveryBTC (WKWallet wallet) {
    WKWalletBTC walletBTC = wkWalletCoerceBTC(wallet);
    BRBitcoinWallet *btcWallet = walletBTC->wid;

    size_t btcAddressesCount = btcWalletAllAddrs (btcWallet, NULL, 0);
    BRAddress *btcAddresses = calloc (btcAddressesCount, sizeof (BRAddress));
    btcWalletAllAddrs (btcWallet, btcAddresses, btcAddressesCount);

    WKAddress replacedAddress = NULL;

    BRSetOf(WKAddress) addresses = wkAddressSetCreate (btcAddressesCount);

    for (size_t index = 0; index < btcAddressesCount; index++) {
        // The currency, may or may not have a legacy address;
        BRAddress btcPrimaryAddress = btcAddresses[index];
        BRAddress btcLegacyAddress  = btcWalletAddressToLegacy(btcWallet, &btcAddresses[index]);

        // Add in the primaryAddress
        replacedAddress = BRSetAdd (addresses, wkAddressCreateAsBTC (wallet->type, btcPrimaryAddress));
        if (replacedAddress) wkAddressGive (replacedAddress);


        // If the primaryAddress nd legacyAddress differ, then add it in
        if (!BRAddressEq (&btcPrimaryAddress, &btcLegacyAddress)) {
            replacedAddress = BRSetAdd (addresses, wkAddressCreateAsBTC (wallet->type, btcLegacyAddress));
            if (replacedAddress) wkAddressGive (replacedAddress);
        }
    }

    free (btcAddresses);

    return addresses;
}

WKWalletHandlers wkWalletHandlersBTC = {
    wkWalletReleaseBTC,
    wkWalletGetAddressBTC,
    wkWalletHasAddressBTC,
    wkWalletGetTransferAttributeCountBTC,
    wkWalletGetTransferAttributeAtBTC,
    wkWalletValidateTransferAttributeBTC,
    wkWalletCreateTransferBTC,
    wkWalletCreateTransferMultipleBTC,
    wkWalletGetAddressesForRecoveryBTC,
    NULL,
    wkWalletIsEqualBTC
};

WKWalletHandlers wkWalletHandlersBCH = {
    wkWalletReleaseBTC,
    wkWalletGetAddressBTC,
    wkWalletHasAddressBTC,
    wkWalletGetTransferAttributeCountBTC,
    wkWalletGetTransferAttributeAtBTC,
    wkWalletValidateTransferAttributeBTC,
    wkWalletCreateTransferBTC,
    wkWalletCreateTransferMultipleBTC,
    wkWalletGetAddressesForRecoveryBTC,
    NULL,
    wkWalletIsEqualBTC
};

WKWalletHandlers wkWalletHandlersBSV = {
    wkWalletReleaseBTC,
    wkWalletGetAddressBTC,
    wkWalletHasAddressBTC,
    wkWalletGetTransferAttributeCountBTC,
    wkWalletGetTransferAttributeAtBTC,
    wkWalletValidateTransferAttributeBTC,
    wkWalletCreateTransferBTC,
    wkWalletCreateTransferMultipleBTC,
    wkWalletGetAddressesForRecoveryBTC,
    NULL,
    wkWalletIsEqualBTC
};
