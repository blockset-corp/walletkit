#include "BRCryptoETH.h"

struct BRCryptoWalletETHRecord {
    struct BRCryptoWalletRecord base;
//    BRWallet *wid;
};

static BRCryptoWalletETH
cryptoWalletCoerce (BRCryptoWallet wallet) {
    assert (CRYPTO_NETWORK_TYPE_ETH == wallet->type);
    return (BRCryptoWalletETH) wallet;
}

private_extern BREthereumWallet
cryptoWalletAsETH (BRCryptoWallet wallet);

private_extern BRCryptoWallet
cryptoWalletCreateAsETH (BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BREthereumEWM ewm,
                         BREthereumWallet wid);

private_extern BRCryptoTransfer
cryptoWalletFindTransferAsETH (BRCryptoWallet wallet,
                               BREthereumTransfer eth);

private_extern BRCryptoWallet
cryptoWalletCreateAsETH (BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BREthereumEWM ewm,
                         BREthereumWallet wid) {
    BRCryptoWallet wallet = cryptoWalletCreateInternal (BLOCK_CHAIN_TYPE_ETH, unit, unitForFee);

    wallet->u.eth.ewm = ewm;
    wallet->u.eth.wid = wid;

    return wallet;
}

static void
cryptoWalletReleaseETH (BRCryptoWallet wallet) {
}

private_extern BRCryptoTransfer
cryptoWalletFindTransferAsETH (BRCryptoWallet wallet,
                               BREthereumTransfer eth) {
    BRCryptoTransfer transfer = NULL;
    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        if (CRYPTO_TRUE == cryptoTransferHasETH (wallet->transfers[index], eth)) {
            transfer = cryptoTransferTake (wallet->transfers[index]);
            break;
        }
    }
    pthread_mutex_unlock (&wallet->lock);
    return transfer;
}

static BRCryptoAddress
cryptoWalletGetAddressETH (BRCryptoWallet wallet,
                        BRCryptoAddressScheme addressScheme) {
    assert (CRYPTO_ADDRESS_SCHEME_ETH_DEFAULT == addressScheme);
    BREthereumEWM ewm = wallet->u.eth.ewm;

    BREthereumAddress ethAddress = ethAccountGetPrimaryAddress (ewmGetAccount(ewm));
    return cryptoAddressCreateAsETH (ethAddress);
}

static BRCryptoBoolean
cryptoWalletHasAddressETH (BRCryptoWallet wallet,
                           BRCryptoAddress address) {
    BREthereumAddress ethAddress = cryptoAddressAsETH (address);
    return AS_CRYPTO_BOOLEAN (ETHEREUM_BOOLEAN_TRUE == ewmWalletHasAddress(wallet->u.eth.ewm, wallet->u.eth.wid, ethAddress));
}

static BRCryptoTransfer
cryptoWalletCreateTransferMultipleETH (BRCryptoWallet wallet,
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
cryptoWalletCreateTransferETH (BRCryptoWallet  wallet,
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
    
    BREthereumEWM ewm = wallet->u.eth.ewm;
    BREthereumWallet wid = wallet->u.eth.wid;
    
    UInt256 ethValue  = cryptoAmountGetValue (amount);
    BREthereumToken  ethToken  = ewmWalletGetToken (ewm, wid);
    BREthereumAmount ethAmount = (NULL != ethToken
                                  ? ethAmountCreateToken (ethTokenQuantityCreate (ethToken, ethValue))
                                  : ethAmountCreateEther (ethEtherCreate (ethValue)));
    BREthereumFeeBasis ethFeeBasis = cryptoFeeBasisAsETH (estimatedFeeBasis);
    
    char *addr = cryptoAddressAsString(target); // Target address
    
    //
    // We have a race condition here. `ewmWalletCreateTransferWithFeeBasis()` will generate
    // a `TRANSFER_EVENT_CREATED` event; `cwmTransactionEventAsETH()` will eventually get
    // called and attempt to find or create the BRCryptoTransfer.
    //
    // We might think about locking the wallet around the following two statements and then
    // locking in `cwmTransactionEventAsETH()` too - perhaps justifying it with 'we are
    // mucking w/ the wallet's transactions' so we should lock it (over a block of
    // statements).
    //
    BREthereumTransfer tid = ewmWalletCreateTransferWithFeeBasis (ewm, wid, addr, ethAmount, ethFeeBasis);
    transfer = NULL == tid ? NULL : cryptoTransferCreateAsETH (unit, unitForFee, ewm, tid, estimatedFeeBasis);
    
    free (addr);
    
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
cryptoWalletIsEqualETH (BRCryptoWallet wb1, BRCryptoWallet wb2) {
//    return (w1->u.eth.ewm == w2->u.eth.ewm &&
//            w1->u.eth.wid == w2->u.eth.wid);
    return true;
}
