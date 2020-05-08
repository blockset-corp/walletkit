#include "BRCryptoBTC.h"

#include "bitcoin/BRWallet.h"

struct BRCryptoWalletBTCRecord {
    struct BRCryptoWalletRecord base;
    BRWallet *wid;
};

static BRCryptoWalletBTC
cryptoWalletCoerce (BRCryptoWallet wallet) {
    assert (CRYPTO_NETWORK_TYPE_BTC == wallet->type);
    return (BRCryptoWalletBTC) wallet;
}


private_extern BRCryptoWallet
cryptoWalletCreateAsBTC (BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
//                         BRWalletManager bwm,
                         BRWallet *wid) {
    BRCryptoWallet walletBase = cryptoWalletAllocAndInit (sizeof (struct BRCryptoWalletBTCRecord),
                                                          CRYPTO_NETWORK_TYPE_BTC,
                                                          unit,
                                                          unitForFee,
                                                          cryptoAmountCreateInteger(0, unit),
                                                          NULL);

    BRCryptoWalletBTC wallet = cryptoWalletCoerce (walletBase);

//    wallet->u.btc.bwm = bwm;
    wallet->wid = wid;

    return walletBase;
}

static void
cryptoWalletReleaseBTC (BRCryptoWallet wallet) {
}

private_extern BRWallet *
cryptoWalletAsBTC (BRCryptoWallet walletBase) {
    BRCryptoWalletBTC wallet = cryptoWalletCoerce(walletBase);
    return wallet->wid;
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

static BRCryptoAddress
cryptoWalletGetAddressBTC (BRCryptoWallet walletBase,
                           BRCryptoAddressScheme addressScheme) {
    assert (CRYPTO_ADDRESS_SCHEME_BTC_LEGACY == addressScheme ||
            CRYPTO_ADDRESS_SCHEME_BTC_SEGWIT == addressScheme);

    BRCryptoWalletBTC wallet = cryptoWalletCoerce(walletBase);

    BRWallet *wid = wallet->wid;
    BRAddress btcAddress = (CRYPTO_ADDRESS_SCHEME_BTC_SEGWIT == addressScheme
                            ? BRWalletReceiveAddress(wid)
                            : BRWalletLegacyAddress (wid));
#ifdef REFACTOR
    return cryptoAddressCreateAsBTC (btcAddress, AS_CRYPTO_BOOLEAN (BRWalletManagerHandlesBTC(wallet->u.btc.bwm)));
#endif
    return cryptoAddressCreateAsBTC (CRYPTO_NETWORK_TYPE_BTC, btcAddress);

}

static bool
cryptoWalletHasAddressBTC (BRCryptoWallet walletBase,
                           BRCryptoAddress address) {
    BRCryptoWalletBTC wallet = cryptoWalletCoerce(walletBase);

    BRCryptoBoolean isBitcoinAddress;

    BRWallet *btcWallet = wallet->wid;
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
cryptoWalletCreateTransferMultipleBTC (BRCryptoWallet walletBase,
                                       size_t outputsCount,
                                       BRCryptoTransferOutput *outputs,
                                       BRCryptoFeeBasis estimatedFeeBasis,
                                       BRCryptoCurrency currency,
                                       BRCryptoUnit unit,
                                       BRCryptoUnit unitForFee) {
    BRCryptoWalletBTC wallet = cryptoWalletCoerce(walletBase);


//    BRWalletManager bwm = wallet->bwm;
    BRWallet *wid = wallet->wid;
    BRAddressParams params = BRWalletGetAddressParams(wid);

    BRTxOutput txOutputs [outputsCount];
    memset (txOutputs, 0, outputsCount * sizeof(BRTxOutput));

    for (size_t index = 0; index < outputsCount; index++) {
        BRCryptoTransferOutput *output = &outputs[index];
        BRTxOutput *txOutput = &txOutputs[index];

        assert (cryptoWalletGetType(walletBase) == cryptoAddressGetType(output->target));
        assert (cryptoAmountHasCurrency (output->amount, currency));

        BRCryptoBoolean isBitcoinAddr = CRYPTO_TRUE;
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
            : cryptoTransferCreateAsBTC (unit, unitForFee, wid, tid, NULL, walletBase->type));
#ifdef REFACTOR
                                         AS_CRYPTO_BOOLEAN(BRWalletManagerHandlesBTC(bwm))));
#endif
}

extern BRCryptoTransfer
cryptoWalletCreateTransferBTC (BRCryptoWallet  walletBase,
                               BRCryptoAddress target,
                               BRCryptoAmount  amount,
                               BRCryptoFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept BRCryptoTransferAttribute *attributes,
                               BRCryptoCurrency currency,
                               BRCryptoUnit unit,
                               BRCryptoUnit unitForFee) {
    BRCryptoWalletBTC wallet = cryptoWalletCoerce(walletBase);

    //    BRWalletManager bwm = wallet->bwm;
    BRWallet *wid = wallet->wid;

    BRCryptoBoolean isBitcoinAddr = CRYPTO_TRUE;
    BRAddress address = cryptoAddressAsBTC (target, &isBitcoinAddr);
//    assert (isBitcoinAddr == AS_CRYPTO_BOOLEAN (BRWalletManagerHandlesBTC (bwm)));

    BRCryptoBoolean overflow = CRYPTO_FALSE;
    uint64_t value = cryptoAmountGetIntegerRaw (amount, &overflow);
    if (CRYPTO_TRUE == overflow) { return NULL; }

    uint64_t feePerKb = cryptoFeeBasisAsBTC(estimatedFeeBasis);

    BRTransaction *tid = BRWalletCreateTransactionWithFeePerKb (wid, feePerKb, value, address.s);

//    BRTransaction *tid = BRWalletManagerCreateTransaction (bwm, wid, value, address,
//                                                           cryptoFeeBasisAsBTC(estimatedFeeBasis));

    return (NULL == tid
            ? NULL
            : cryptoTransferCreateAsBTC (unit, unitForFee, wid, tid, NULL, walletBase->type));
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
    cryptoWalletIsEqualBTC
};
