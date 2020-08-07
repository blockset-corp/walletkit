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

#define DEFAULT_FEE_BASIS_SIZE_IN_BYTES     200

static BRCryptoWalletBTC
cryptoWalletCoerce (BRCryptoWallet wallet) {
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
    BRCryptoWalletBTC walletBTC = cryptoWalletCoerce (wallet);

    walletBTC->wid = contextBTC->wid;
}


private_extern BRCryptoWallet
cryptoWalletCreateAsBTC (BRCryptoBlockChainType type,
                         BRCryptoWalletListener listener,
                         BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BRWallet *wid) {
    BRCryptoFeeBasis feeBasis = cryptoFeeBasisCreateAsBTC (unitForFee,
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
                                                      cryptoAmountCreateInteger(0, unit),
                                                      NULL,
                                                      feeBasis,
                                                      &contextBTC,
                                                      cryptoWalletCreateCallbackBTC);
    cryptoFeeBasisGive (feeBasis);

    return wallet;
}

static void
cryptoWalletReleaseBTC (BRCryptoWallet wallet) {
}

private_extern BRWallet *
cryptoWalletAsBTC (BRCryptoWallet wallet) {
    BRCryptoWalletBTC walletBTC = cryptoWalletCoerce(wallet);
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

static BRCryptoAddress
cryptoWalletGetAddressBTC (BRCryptoWallet wallet,
                           BRCryptoAddressScheme addressScheme) {
    // TODO: Match Wallet type - not always SEGWIT/LEGACY
    assert (CRYPTO_ADDRESS_SCHEME_BTC_LEGACY == addressScheme ||
            CRYPTO_ADDRESS_SCHEME_BTC_SEGWIT == addressScheme);

    BRCryptoWalletBTC walletBTC = cryptoWalletCoerce(wallet);

    BRWallet *wid = walletBTC->wid;
#ifdef REFACTOR
    // BSV
#endif
    BRAddress btcAddress = (CRYPTO_ADDRESS_SCHEME_BTC_SEGWIT == addressScheme
                            ? BRWalletReceiveAddress(wid)
                            : BRWalletLegacyAddress (wid));

    return cryptoAddressCreateAsBTC (wallet->type, btcAddress);
}

static bool
cryptoWalletHasAddressBTC (BRCryptoWallet wallet,
                           BRCryptoAddress address) {
    BRCryptoWalletBTC walletBTC = cryptoWalletCoerce(wallet);

    BRCryptoBoolean isBitcoinAddress;
#ifdef REFACTOR
    // BSV
#endif
    BRWallet *btcWallet = walletBTC->wid;
    BRAddress btcAddress = cryptoAddressAsBTC (address, &isBitcoinAddress);

    if (BRWalletAddressIsUsed (btcWallet, btcAddress.s))
        return true;

    BRAddress btcLegacyAddress = BRWalletLegacyAddress (btcWallet);
    if (0 == memcmp (btcAddress.s, btcLegacyAddress.s, sizeof (btcAddress.s)))
        return true;

    if (CRYPTO_TRUE == isBitcoinAddress) {
        BRAddress btcSegwitAddress = BRWalletReceiveAddress (btcWallet);
        if (0 == memcmp (btcAddress.s, btcSegwitAddress.s, sizeof (btcAddress.s)))
            return true;
    }

    return false;
}

static bool
cryptoWalletIsEqualBTC (BRCryptoWallet wb1, BRCryptoWallet wb2) {
    BRCryptoWalletBTC w1 = cryptoWalletCoerce(wb1);
    BRCryptoWalletBTC w2 = cryptoWalletCoerce(wb2);

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
cryptoWalletCreateTransferMultipleBTC (BRCryptoWallet wallet,
                                       size_t outputsCount,
                                       BRCryptoTransferOutput *outputs,
                                       BRCryptoFeeBasis estimatedFeeBasis,
                                       BRCryptoCurrency currency,
                                       BRCryptoUnit unit,
                                       BRCryptoUnit unitForFee) {
    BRCryptoWalletBTC walletBTC = cryptoWalletCoerce(wallet);


//    BRWalletManager bwm = wallet->bwm;
    BRWallet *wid = walletBTC->wid;
    BRAddressParams params = BRWalletGetAddressParams(wid);

    BRTxOutput txOutputs [outputsCount];
    memset (txOutputs, 0, outputsCount * sizeof(BRTxOutput));

    for (size_t index = 0; index < outputsCount; index++) {
        BRCryptoTransferOutput *output = &outputs[index];
        BRTxOutput *txOutput = &txOutputs[index];

        assert (cryptoWalletGetType(wallet) == cryptoAddressGetType(output->target));
        assert (cryptoAmountHasCurrency (output->amount, currency));

        BRCryptoBoolean isBitcoinAddr = CRYPTO_TRUE;
#ifdef REFACTOR
        // BSV
#endif
        BRAddress address = cryptoAddressAsBTC (output->target, &isBitcoinAddr);
 //       assert (isBitcoinAddr == AS_CRYPTO_BOOLEAN (BRWalletManagerHandlesBTC (bwm)));

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
#ifdef REFACTOR
                                         AS_CRYPTO_BOOLEAN(BRWalletManagerHandlesBTC(bwm))));
#endif
}

static OwnershipGiven BRSetOf(BRCryptoAddress)
cryptoWalletGetAddressesForRecoveryBTC (BRCryptoWallet wallet) {
    BRSetOf(BRCryptoAddress) addresses = cryptoAddressSetCreate (10);

    BRCryptoWalletBTC walletBTC = cryptoWalletCoerce(wallet);
    BRWallet *btcWallet = walletBTC->wid;

    size_t btcAddressesCount = BRWalletAllAddrs (btcWallet, NULL, 0);
    BRAddress *btcAddresses = calloc (btcAddressesCount, sizeof (BRAddress));
    BRWalletAllAddrs (btcWallet, btcAddresses, btcAddressesCount);

    for (size_t index = 0; index < btcAddressesCount; index++) {
        BRSetAdd (addresses, cryptoAddressCreateAsBTC (wallet->type, btcAddresses[index]));
        // TODO: BCH vs BTC vs BSV
        BRSetAdd (addresses, cryptoAddressCreateAsBTC (wallet->type, BRWalletAddressToLegacy(btcWallet, &btcAddresses[index])));
    }

    free (btcAddresses);

    return addresses;
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
    BRCryptoWalletBTC walletBTC = cryptoWalletCoerce(wallet);

    //    BRWalletManager bwm = wallet->bwm;
    BRWallet *wid = walletBTC->wid;

    BRCryptoBoolean isBitcoinAddr = CRYPTO_TRUE;
    #ifdef REFACTOR
            // BSV
    #endif
    BRAddress address = cryptoAddressAsBTC (target, &isBitcoinAddr);
//    assert (isBitcoinAddr == AS_CRYPTO_BOOLEAN (BRWalletManagerHandlesBTC (bwm)));

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
#ifdef REFACTOR
                                         AS_CRYPTO_BOOLEAN(BRWalletManagerHandlesBTC(bwm))));
#endif
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
    cryptoWalletIsEqualBTC
};
