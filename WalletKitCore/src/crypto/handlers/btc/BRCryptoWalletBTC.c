//
//  BRCryptoWalletBTC.c
//  Core
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoBTC.h"

#include "bitcoin/BRWallet.h"

#define DEFAULT_FEE_BASIS_SIZE_IN_BYTES     (200)
#define DEFAULT_TIDS_UNRESOLVED_COUNT         (2)

private_extern BRCryptoWalletBTC
cryptoWalletCoerceBTC (BRCryptoWallet wallet) {
    assert (CRYPTO_NETWORK_TYPE_BTC == wallet->type ||
            CRYPTO_NETWORK_TYPE_BCH == wallet->type ||
            CRYPTO_NETWORK_TYPE_BSV == wallet->type);
    return (BRCryptoWalletBTC) wallet;
}

typedef struct {
    BRWallet *wid;
} BRCryptoWalletCreateContextBTC;

static void
cryptoWalletCreateCallbackBTC (BRCryptoWalletCreateContext context,
                               BRCryptoWallet wallet) {
    BRCryptoWalletCreateContextBTC *contextBTC = (BRCryptoWalletCreateContextBTC*) context;
    BRCryptoWalletBTC walletBTC = cryptoWalletCoerceBTC (wallet);

    walletBTC->wid = contextBTC->wid;
    array_new (walletBTC->tidsUnresolved, DEFAULT_TIDS_UNRESOLVED_COUNT);
}


private_extern BRCryptoWallet
cryptoWalletCreateAsBTC (BRCryptoBlockChainType type,
                         BRCryptoWalletListener listener,
                         BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BRWallet *wid) {
    BRCryptoAmount minBalance = cryptoAmountCreateInteger(0, unit);

    BRCryptoFeeBasis feeBasis = cryptoFeeBasisCreateAsBTC (unitForFee,
                                                           CRYPTO_FEE_BASIS_BTC_FEE_UNKNOWN,
                                                           BRWalletFeePerKb(wid),
                                                           DEFAULT_FEE_BASIS_SIZE_IN_BYTES);

    BRCryptoWalletCreateContextBTC contextBTC = {
        wid
    };

    BRCryptoWallet wallet = cryptoWalletAllocAndInit (sizeof (struct BRCryptoWalletBTCRecord),
                                                      type,
                                                      listener,
                                                      unit,
                                                      unitForFee,
                                                      minBalance,
                                                      NULL,
                                                      feeBasis,
                                                      &contextBTC,
                                                      cryptoWalletCreateCallbackBTC);

    cryptoAmountGive (minBalance);
    cryptoFeeBasisGive (feeBasis);

    return wallet;
}

static void
cryptoWalletReleaseBTC (BRCryptoWallet wallet) {
    BRCryptoWalletBTC walletBTC = cryptoWalletCoerceBTC(wallet);
    array_free_all (walletBTC->tidsUnresolved, BRTransactionFree);
}

private_extern BRWallet *
cryptoWalletAsBTC (BRCryptoWallet wallet) {
    BRCryptoWalletBTC walletBTC = cryptoWalletCoerceBTC(wallet);
    return walletBTC->wid;
}

private_extern BRCryptoTransfer
cryptoWalletFindTransferAsBTC (BRCryptoWallet wallet,
                               BRTransaction *btc) {
    BRCryptoTransfer transfer = NULL;
    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        if (CRYPTO_TRUE == cryptoTransferHasBTC (wallet->transfers[index], btc)) {
            transfer = cryptoTransferTake (wallet->transfers[index]);
            break;
        }
    }
    pthread_mutex_unlock (&wallet->lock);
    return transfer;
}

private_extern BRCryptoTransferBTC
cryptoWalletFindTransferByHashAsBTC (BRCryptoWallet wallet,
                                     UInt256 hash) {

    BRCryptoTransferBTC transfer = NULL;
    if (! UInt256IsZero(hash)) {
        pthread_mutex_lock (&wallet->lock);
        for (size_t index = 0; index < array_count(wallet->transfers); index++) {
            transfer = (BRCryptoTransferBTC) wallet->transfers[index];
            if (UInt256Eq (hash, transfer->tid->txHash))
                break;
            transfer = NULL;
        }
        pthread_mutex_unlock (&wallet->lock);
    }
    return transfer;
}

private_extern void
cryptoWalletAddUnresolvedAsBTC (BRCryptoWallet wallet,
                                OwnershipGiven BRTransaction *tid) {
    BRCryptoWalletBTC walletBTC = cryptoWalletCoerceBTC(wallet);

    pthread_mutex_lock (&wallet->lock);
    array_add (walletBTC->tidsUnresolved, tid);
    pthread_mutex_unlock (&wallet->lock);
}

private_extern void
cryptoWalletUpdUnresolvedAsBTC (BRCryptoWallet wallet,
                                const UInt256 *hash,
                                uint32_t blockHeight,
                                uint32_t timestamp) {
    BRCryptoWalletBTC walletBTC = cryptoWalletCoerceBTC(wallet);

    pthread_mutex_lock (&wallet->lock);

    for (size_t index = 0; index < array_count (walletBTC->tidsUnresolved); index++) {
        BRTransaction *tid = walletBTC->tidsUnresolved[index];
        if (BRTransactionEq (tid, hash)) {
            tid->blockHeight = blockHeight;
            tid->timestamp   = timestamp;
        }
    }

    pthread_mutex_unlock (&wallet->lock);
}

private_extern size_t
cryptoWalletRemResolvedAsBTC (BRCryptoWallet wallet,
                              BRTransaction **tids,
                              size_t tidsCount) {
    BRCryptoWalletBTC walletBTC = cryptoWalletCoerceBTC(wallet);

    pthread_mutex_lock (&wallet->lock);
    size_t count = array_count(walletBTC->tidsUnresolved);

    if (NULL != tids) {
        size_t  rIndex = 0;

        for (ssize_t tIndex = (ssize_t) MIN (tidsCount, count) - 1; tIndex >= 0; tIndex--)
            if (BRWalletTransactionIsResolved (walletBTC->wid, walletBTC->tidsUnresolved[tIndex])) {
                tids[rIndex++] = walletBTC->tidsUnresolved[tIndex];
                array_rm (walletBTC->tidsUnresolved, (size_t) tIndex);
            }

        count = rIndex;
    }
    pthread_mutex_unlock (&wallet->lock);
    return count;
}

static BRCryptoAddress
cryptoWalletGetAddressBTC (BRCryptoWallet wallet,
                           BRCryptoAddressScheme addressScheme) {
    assert (CRYPTO_ADDRESS_SCHEME_BTC_LEGACY == addressScheme ||
            CRYPTO_ADDRESS_SCHEME_BTC_SEGWIT == addressScheme);

    assert (CRYPTO_ADDRESS_SCHEME_BTC_SEGWIT != addressScheme ||
            CRYPTO_NETWORK_TYPE_BTC == wallet->type);

    BRCryptoWalletBTC walletBTC = cryptoWalletCoerceBTC(wallet);

    BRWallet *wid = walletBTC->wid;

    BRAddress btcAddress = (CRYPTO_ADDRESS_SCHEME_BTC_SEGWIT == addressScheme
                            ? BRWalletReceiveAddress(wid)
                            : BRWalletLegacyAddress (wid));

    return cryptoAddressCreateAsBTC (wallet->type, btcAddress);
}

static bool
cryptoWalletHasAddressBTC (BRCryptoWallet wallet,
                           BRCryptoAddress address) {
    BRCryptoWalletBTC walletBTC = cryptoWalletCoerceBTC(wallet);

    BRWallet *btcWallet = walletBTC->wid;

    BRCryptoBlockChainType type;
    BRAddress btcAddress = cryptoAddressAsBTC (address, &type);

    if (BRWalletAddressIsUsed (btcWallet, btcAddress.s))
        return true;

    BRAddress btcLegacyAddress = BRWalletLegacyAddress (btcWallet);
    if (0 == memcmp (btcAddress.s, btcLegacyAddress.s, sizeof (btcAddress.s)))
        return true;

    if (CRYPTO_NETWORK_TYPE_BTC == type) {
        BRAddress btcSegwitAddress = BRWalletReceiveAddress (btcWallet);
        if (0 == memcmp (btcAddress.s, btcSegwitAddress.s, sizeof (btcAddress.s)))
            return true;
    }

    return false;
}

static bool
cryptoWalletIsEqualBTC (BRCryptoWallet wb1, BRCryptoWallet wb2) {
    BRCryptoWalletBTC w1 = cryptoWalletCoerceBTC(wb1);
    BRCryptoWalletBTC w2 = cryptoWalletCoerceBTC(wb2);

    // This does not compare the properties of `t1` to `t2`, just the 'id-ness'.  If the properties
    // are compared, one needs to be careful about the BRTransaction's timestamp.  Two transactions
    // with an identical hash can have different timestamps depending on how the transaction
    // is identified.  Specifically P2P and API found transactions *will* have different timestamps.
    return w1->wid == w2->wid;
}

extern size_t
cryptoWalletGetTransferAttributeCountBTC (BRCryptoWallet wallet,
                                          BRCryptoAddress target) {
    return 0;
}

extern BRCryptoTransferAttribute
cryptoWalletGetTransferAttributeAtBTC (BRCryptoWallet wallet,
                                       BRCryptoAddress target,
                                       size_t index) {
    return NULL;
}

extern BRCryptoTransferAttributeValidationError
cryptoWalletValidateTransferAttributeBTC (BRCryptoWallet wallet,
                                          OwnershipKept BRCryptoTransferAttribute attribute,
                                          BRCryptoBoolean *validates) {
    *validates = CRYPTO_TRUE;
    return (BRCryptoTransferAttributeValidationError) 0;
}

extern BRCryptoTransfer
cryptoWalletCreateTransferBTC (BRCryptoWallet  wallet,
                               BRCryptoAddress target,
                               BRCryptoAmount  amount,
                               BRCryptoFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept BRCryptoTransferAttribute *attributes,
                               BRCryptoCurrency currency,
                               BRCryptoUnit unit,
                               BRCryptoUnit unitForFee) {
    BRCryptoWalletBTC walletBTC = cryptoWalletCoerceBTC(wallet);

    BRWallet *wid = walletBTC->wid;

    BRCryptoBlockChainType addressType;
    BRAddress address = cryptoAddressAsBTC (target, &addressType);
    assert (addressType == wallet->type);

    BRCryptoBoolean overflow = CRYPTO_FALSE;
    uint64_t value = cryptoAmountGetIntegerRaw (amount, &overflow);
    if (CRYPTO_TRUE == overflow) { return NULL; }

    uint64_t feePerKb = cryptoFeeBasisAsBTC(estimatedFeeBasis);

    BRTransaction *tid = BRWalletCreateTransactionWithFeePerKb (wid, feePerKb, value, address.s);

    return (NULL == tid
            ? NULL
            : cryptoTransferCreateAsBTC (wallet->listenerTransfer,
                                         unit,
                                         unitForFee,
                                         wid,
                                         tid,
                                         wallet->type));
}

extern BRCryptoTransfer
cryptoWalletCreateTransferMultipleBTC (BRCryptoWallet wallet,
                                       size_t outputsCount,
                                       BRCryptoTransferOutput *outputs,
                                       BRCryptoFeeBasis estimatedFeeBasis,
                                       BRCryptoCurrency currency,
                                       BRCryptoUnit unit,
                                       BRCryptoUnit unitForFee) {
    BRCryptoWalletBTC walletBTC = cryptoWalletCoerceBTC(wallet);

    BRWallet *wid = walletBTC->wid;
    BRAddressParams params = BRWalletGetAddressParams(wid);

    BRTxOutput txOutputs [outputsCount];
    memset (txOutputs, 0, outputsCount * sizeof(BRTxOutput));

    for (size_t index = 0; index < outputsCount; index++) {
        BRCryptoTransferOutput *output = &outputs[index];
        BRTxOutput *txOutput = &txOutputs[index];

        assert (cryptoWalletGetType(wallet) == cryptoAddressGetType(output->target));
        assert (cryptoAmountHasCurrency (output->amount, currency));

        BRCryptoBlockChainType outputTargetType;
        BRAddress address = cryptoAddressAsBTC (output->target, &outputTargetType);
        assert (outputTargetType == wallet->type);

        BRCryptoBoolean overflow = CRYPTO_FALSE;
        uint64_t value = cryptoAmountGetIntegerRaw (output->amount, &overflow);
        assert (CRYPTO_TRUE != overflow);

        txOutput->amount = value;
        BRTxOutputSetAddress (txOutput, params, address.s);
    }

    uint64_t feePerKb = cryptoFeeBasisAsBTC(estimatedFeeBasis);

    BRTransaction *tid = BRWalletCreateTxForOutputsWithFeePerKb (wid, feePerKb, txOutputs, outputsCount);

    return (NULL == tid
            ? NULL
            : cryptoTransferCreateAsBTC (wallet->listenerTransfer,
                                         unit,
                                         unitForFee,
                                         wid,
                                         tid,
                                         wallet->type));
}

static OwnershipGiven BRSetOf(BRCryptoAddress)
cryptoWalletGetAddressesForRecoveryBTC (BRCryptoWallet wallet) {
    BRCryptoWalletBTC walletBTC = cryptoWalletCoerceBTC(wallet);
    BRWallet *btcWallet = walletBTC->wid;

    size_t btcAddressesCount = BRWalletAllAddrs (btcWallet, NULL, 0);
    BRAddress *btcAddresses = calloc (btcAddressesCount, sizeof (BRAddress));
    BRWalletAllAddrs (btcWallet, btcAddresses, btcAddressesCount);

    BRCryptoAddress replacedAddress = NULL;

    BRSetOf(BRCryptoAddress) addresses = cryptoAddressSetCreate (btcAddressesCount);

    for (size_t index = 0; index < btcAddressesCount; index++) {
        // The currency, may or may not have a legacy address;
        BRAddress btcPrimaryAddress = btcAddresses[index];
        BRAddress btcLegacyAddress  = BRWalletAddressToLegacy(btcWallet, &btcAddresses[index]);

        // Add in the primaryAddress
        replacedAddress = BRSetAdd (addresses, cryptoAddressCreateAsBTC (wallet->type, btcPrimaryAddress));
        if (replacedAddress) cryptoAddressGive (replacedAddress);


        // If the primaryAddress nd legacyAddress differ, then add it in
        if (!BRAddressEq (&btcPrimaryAddress, &btcLegacyAddress)) {
            replacedAddress = BRSetAdd (addresses, cryptoAddressCreateAsBTC (wallet->type, btcLegacyAddress));
            if (replacedAddress) cryptoAddressGive (replacedAddress);
        }
    }

    free (btcAddresses);

    return addresses;
}

BRCryptoWalletHandlers cryptoWalletHandlersBTC = {
    cryptoWalletReleaseBTC,
    cryptoWalletGetAddressBTC,
    cryptoWalletHasAddressBTC,
    cryptoWalletGetTransferAttributeCountBTC,
    cryptoWalletGetTransferAttributeAtBTC,
    cryptoWalletValidateTransferAttributeBTC,
    cryptoWalletCreateTransferBTC,
    cryptoWalletCreateTransferMultipleBTC,
    cryptoWalletGetAddressesForRecoveryBTC,
    NULL,
    cryptoWalletIsEqualBTC
};

BRCryptoWalletHandlers cryptoWalletHandlersBCH = {
    cryptoWalletReleaseBTC,
    cryptoWalletGetAddressBTC,
    cryptoWalletHasAddressBTC,
    cryptoWalletGetTransferAttributeCountBTC,
    cryptoWalletGetTransferAttributeAtBTC,
    cryptoWalletValidateTransferAttributeBTC,
    cryptoWalletCreateTransferBTC,
    cryptoWalletCreateTransferMultipleBTC,
    cryptoWalletGetAddressesForRecoveryBTC,
    NULL,
    cryptoWalletIsEqualBTC
};

BRCryptoWalletHandlers cryptoWalletHandlersBSV = {
    cryptoWalletReleaseBTC,
    cryptoWalletGetAddressBTC,
    cryptoWalletHasAddressBTC,
    cryptoWalletGetTransferAttributeCountBTC,
    cryptoWalletGetTransferAttributeAtBTC,
    cryptoWalletValidateTransferAttributeBTC,
    cryptoWalletCreateTransferBTC,
    cryptoWalletCreateTransferMultipleBTC,
    cryptoWalletGetAddressesForRecoveryBTC,
    NULL,
    cryptoWalletIsEqualBTC
};
