#include "BRCryptoXRP.h"

struct BRCryptoWalletXRPRecord {
    struct BRCryptoWalletRecord base;
//    BRWallet *wid;
};

static BRCryptoWalletXRP
cryptoWalletCoerce (BRCryptoWallet wallet) {
    assert (CRYPTO_NETWORK_TYPE_XRP == wallet->type);
    return (BRCryptoWalletXRP) wallet;
}

private_extern BRGenericWallet
cryptoWalletAsGEN (BRCryptoWallet wallet);

private_extern BRCryptoWallet
cryptoWalletCreateAsGEN (BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BRGenericWallet wid);

private_extern BRCryptoTransfer
cryptoWalletFindTransferAsGEN (BRCryptoWallet wallet,
                               BRGenericTransfer gen);


private_extern BRCryptoWallet
cryptoWalletCreateAsGEN (BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         OwnershipKept BRGenericWallet wid) {
    BRCryptoWallet wallet = cryptoWalletCreateInternal (BLOCK_CHAIN_TYPE_GEN, unit, unitForFee);

    wallet->u.gen = wid;

    return wallet;
}

static void
cryptoWalletReleaseXRP (BRCryptoWallet wallet) {
}

private_extern BRCryptoTransfer
cryptoWalletFindTransferAsGEN (BRCryptoWallet wallet,
                               BRGenericTransfer gen) {
    BRCryptoTransfer transfer = NULL;
    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        if (CRYPTO_TRUE == cryptoTransferHasGEN (wallet->transfers[index], gen)) {
            transfer = cryptoTransferTake (wallet->transfers[index]);
            break;
        }
    }
    pthread_mutex_unlock (&wallet->lock);
    return transfer;
}

static BRCryptoAddress
cryptoWalletGetAddressXRP (BRCryptoWallet wallet,
                           BRCryptoAddressScheme addressScheme) {
    assert (CRYPTO_ADDRESS_SCHEME_GEN_DEFAULT == addressScheme);
    BRGenericWallet wid = wallet->u.gen;

    BRGenericAddress genAddress = genWalletGetAddress (wid);
    return cryptoAddressCreateAsGEN (genAddress);
}

static BRCryptoBoolean
cryptoWalletHasAddressXRP (BRCryptoWallet wallet,
                        BRCryptoAddress address) {
             return AS_CRYPTO_BOOLEAN (genWalletHasAddress (wallet->u.gen, address->u.gen));
 }

static BRCryptoTransfer
cryptoWalletCreateTransferMultipleXRP (BRCryptoWallet wallet,
                                    size_t outputsCount,
                                    BRCryptoTransferOutput *outputs,
                                    BRCryptoFeeBasis estimatedFeeBasis) {
    assert (cryptoWalletGetType(wallet) == cryptoFeeBasisGetType(estimatedFeeBasis));
    if (0 == outputsCount) return NULL;

    BRCryptoTransfer transfer = NULL;

    BRCryptoUnit unit         = cryptoWalletGetUnit (wallet);
    BRCryptoUnit unitForFee   = cryptoWalletGetUnitForFee(wallet);
    BRCryptoCurrency currency = cryptoUnitGetCurrency(unit);

    cryptoCurrencyGive(currency);
    cryptoUnitGive (unitForFee);
    cryptoUnitGive (unit);

    return transfer;
}

static BRCryptoTransfer
cryptoWalletCreateTransferXRP (BRCryptoWallet  wallet,
                               BRCryptoAddress target,
                               BRCryptoAmount  amount,
                               BRCryptoFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept BRCryptoTransferAttribute *attributes) {
    assert (cryptoWalletGetType(wallet) == cryptoAddressGetType(target));
    assert (cryptoWalletGetType(wallet) == cryptoFeeBasisGetType(estimatedFeeBasis));

    BRCryptoTransfer transfer;

    BRCryptoUnit unit       = cryptoWalletGetUnit (wallet);
    BRCryptoUnit unitForFee = cryptoWalletGetUnitForFee(wallet);

    BRCryptoCurrency currency = cryptoUnitGetCurrency(unit);
    assert (cryptoAmountHasCurrency (amount, currency));
    cryptoCurrencyGive(currency);

    BRGenericWallet wid = wallet->u.gen;

    UInt256 genValue  = cryptoAmountGetValue (amount);
    BRGenericAddress genAddr = cryptoAddressAsGEN (target);
    BRGenericFeeBasis genFeeBasis = cryptoFeeBasisAsGEN (estimatedFeeBasis);

    // The GEN Wallet is holding the `tid` memory
    BRGenericTransfer tid = NULL;

    if (0 == attributesCount)
        tid = genWalletCreateTransfer (wid, genAddr, genValue, genFeeBasis);
    else {
        BRArrayOf(BRGenericTransferAttribute) genAttributes;
        array_new (genAttributes, attributesCount);
        for (size_t index = 0; index < attributesCount; index++) {
            // There is no need to give/take this attribute.  It is OwnershipKept
            // (by the caller) and we only extract info.
            BRCryptoTransferAttribute attribute = attributes[index];
            BRGenericTransferAttribute genAttribute =
            genTransferAttributeCreate (cryptoTransferAttributeGetKey(attribute),
                                        cryptoTransferAttributeGetValue(attribute),
                                        CRYPTO_TRUE == cryptoTransferAttributeIsRequired(attribute));
            array_add (genAttributes, genAttribute);
        }
        tid = genWalletCreateTransferWithAttributes (wid, genAddr, genValue, genFeeBasis, genAttributes);
        genTransferAttributeReleaseAll(genAttributes);
    }

    // The CRYPTO Transfer holds the `tid` memory (w/ REF count of 1)
    transfer = NULL == tid ? NULL : cryptoTransferCreateAsGEN (unit, unitForFee, tid);

    if (NULL != transfer && attributesCount > 0) {
        BRArrayOf (BRCryptoTransferAttribute) transferAttributes;
        array_new (transferAttributes, attributesCount);
        array_add_array (transferAttributes, attributes, attributesCount);
        cryptoTransferSetAttributes (transfer, transferAttributes);
        array_free (transferAttributes);
    }

    cryptoUnitGive (unitForFee);
    cryptoUnitGive (unit);

    return transfer;
}

static bool
cryptoWalletIsEqualXRP (BRCryptoWallet wb1, BRCryptoWallet wb2) {
//    return (w1->u.gen == w2->u.gen);
    return true;
}

