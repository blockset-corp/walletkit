//
//  BRCryptoWallet.c
//  BRCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoWalletP.h"

#include "BRCryptoAmountP.h"
#include "BRCryptoFeeBasisP.h"
#include "BRCryptoKeyP.h"
#include "BRCryptoTransferP.h"
#include "BRCryptoAddressP.h"
#include "BRCryptoNetworkP.h"
#include "BRCryptoPaymentP.h"

#include "BRCryptoHandlersP.h"

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoWallet, cryptoWallet)

extern BRCryptoWallet
cryptoWalletAllocAndInit (size_t sizeInBytes,
                          BRCryptoBlockChainType type,
                          BRCryptoUnit unit,
                          BRCryptoUnit unitForFee,
                          BRCryptoAmount balanceMinimum,
                          BRCryptoAmount balanceMaximum) {
    assert (sizeInBytes >= sizeof (struct BRCryptoWalletRecord));

    BRCryptoWallet wallet = calloc (1, sizeInBytes);

    wallet->sizeInBytes = sizeInBytes;
    wallet->type  = type;
    wallet->handlers = cryptoHandlersLookup(type)->wallet;
    wallet->state = CRYPTO_WALLET_STATE_CREATED;
    
    wallet->unit  = cryptoUnitTake (unit);
    wallet->unitForFee = cryptoUnitTake (unitForFee);

    BRCryptoCurrency currency = cryptoUnitGetCurrency(unit);
    assert (NULL == balanceMinimum || cryptoAmountHasCurrency (balanceMinimum, currency));
    assert (NULL == balanceMaximum || cryptoAmountHasCurrency (balanceMaximum, currency));
    cryptoCurrencyGive (currency);

    wallet->balanceMinimum = cryptoAmountTake (balanceMinimum);
    wallet->balanceMaximum = cryptoAmountTake (balanceMaximum);
    wallet->balance = cryptoAmountCreateInteger(0, unit);

    array_new (wallet->transfers, 5);

    wallet->ref = CRYPTO_REF_ASSIGN (cryptoWalletRelease);

    pthread_mutex_init_brd (&wallet->lock, PTHREAD_MUTEX_NORMAL);  // PTHREAD_MUTEX_RECURSIVE

    return wallet;
}

static void
cryptoWalletRelease (BRCryptoWallet wallet) {
    pthread_mutex_lock (&wallet->lock);

    cryptoUnitGive (wallet->unit);
    cryptoUnitGive (wallet->unitForFee);

    cryptoAmountGive (wallet->balanceMinimum);
    cryptoAmountGive (wallet->balanceMaximum);
    cryptoAmountGive (wallet->balance);

    for (size_t index = 0; index < array_count(wallet->transfers); index++)
        cryptoTransferGive (wallet->transfers[index]);
    array_free (wallet->transfers);

    wallet->handlers->release (wallet);

    pthread_mutex_unlock  (&wallet->lock);
    pthread_mutex_destroy (&wallet->lock);

    memset (wallet, 0, sizeof(*wallet));
    free (wallet);
}

private_extern BRCryptoBlockChainType
cryptoWalletGetType (BRCryptoWallet wallet) {
    return wallet->type;
}

extern BRCryptoCurrency
cryptoWalletGetCurrency (BRCryptoWallet wallet) {
    return cryptoUnitGetCurrency(wallet->unit);
}

extern BRCryptoUnit
cryptoWalletGetUnit (BRCryptoWallet wallet) {
    return cryptoUnitTake (wallet->unit);
}

extern BRCryptoWalletState
cryptoWalletGetState (BRCryptoWallet wallet) {
    return wallet->state;
}

private_extern void
cryptoWalletSetState (BRCryptoWallet wallet,
                      BRCryptoWalletState state) {
    wallet->state = state;
}

extern BRCryptoCurrency
cryptoWalletGetCurrencyForFee (BRCryptoWallet wallet) {
    return cryptoUnitGetCurrency (wallet->unitForFee);
}

extern BRCryptoUnit
cryptoWalletGetUnitForFee (BRCryptoWallet wallet) {
    return cryptoUnitTake (wallet->unitForFee);
}

extern BRCryptoAmount
cryptoWalletGetBalance (BRCryptoWallet wallet) {
    return cryptoAmountTake (wallet->balance);
}

extern BRCryptoAmount /* nullable */
cryptoWalletGetBalanceMinimum (BRCryptoWallet wallet) {
    return cryptoAmountTake (wallet->balanceMinimum);
}

extern BRCryptoAmount /* nullable */
cryptoWalletGetBalanceMaximum (BRCryptoWallet wallet) {
    return cryptoAmountTake (wallet->balanceMaximum);
}

static BRCryptoBoolean
cryptoWalletHasTransferLock (BRCryptoWallet wallet,
                             BRCryptoTransfer transfer,
                             bool needLock) {
    BRCryptoBoolean r = CRYPTO_FALSE;
    if (needLock) pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; index < array_count(wallet->transfers) && CRYPTO_FALSE == r; index++) {
        r = cryptoTransferEqual (transfer, wallet->transfers[index]);
    }
    if (needLock) pthread_mutex_unlock (&wallet->lock);
    return r;
}

extern BRCryptoBoolean
cryptoWalletHasTransfer (BRCryptoWallet wallet,
                         BRCryptoTransfer transfer) {
    return cryptoWalletHasTransferLock(wallet, transfer, true);
}

extern void
cryptoWalletAddTransfer (BRCryptoWallet wallet,
                         BRCryptoTransfer transfer) {
    pthread_mutex_lock (&wallet->lock);
    if (CRYPTO_FALSE == cryptoWalletHasTransferLock (wallet, transfer, false)) {
        array_add (wallet->transfers, cryptoTransferTake(transfer));

        BRCryptoAmount oldBalance = cryptoWalletGetBalance (wallet);
        BRCryptoAmount newBalance = cryptoAmountAdd (oldBalance, cryptoTransferGetAmountDirected(transfer));
        cryptoAmountGive(oldBalance);
        wallet->balance = newBalance;
    }
    pthread_mutex_unlock (&wallet->lock);
}

extern void
cryptoWalletRemTransfer (BRCryptoWallet wallet, BRCryptoTransfer transfer) {
    BRCryptoTransfer walletTransfer = NULL;
    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        if (CRYPTO_TRUE == cryptoTransferEqual (wallet->transfers[index], transfer)) {
            walletTransfer = wallet->transfers[index];
            array_rm (wallet->transfers, index);

            BRCryptoAmount oldBalance = cryptoWalletGetBalance (wallet);
            BRCryptoAmount newBalance = cryptoAmountSub (oldBalance, cryptoTransferGetAmountDirected(transfer));
            cryptoAmountGive(oldBalance);
            wallet->balance = newBalance;
            break;
        }
    }
    pthread_mutex_unlock (&wallet->lock);

    // drop reference outside of lock to avoid potential case where release function runs
    if (NULL != walletTransfer) cryptoTransferGive (transfer);
}

extern BRCryptoTransfer *
cryptoWalletGetTransfers (BRCryptoWallet wallet, size_t *count) {
    pthread_mutex_lock (&wallet->lock);
    *count = array_count (wallet->transfers);
    BRCryptoTransfer *transfers = NULL;
    if (0 != *count) {
        transfers = calloc (*count, sizeof(BRCryptoTransfer));
        for (size_t index = 0; index < *count; index++) {
            transfers[index] = cryptoTransferTake(wallet->transfers[index]);
        }
    }
    pthread_mutex_unlock (&wallet->lock);
    return transfers;
}

private_extern BRCryptoTransfer
cryptoWalletGetTransferByHash (BRCryptoWallet wallet, BRCryptoHash hashToMatch) {
    BRCryptoTransfer transfer = NULL;

    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; NULL == transfer && index < array_count(wallet->transfers); index++) {
        BRCryptoHash hash = cryptoTransferGetHash (wallet->transfers[index]);
        if (cryptoHashEqual(hash, hashToMatch))
            transfer = wallet->transfers[index];
        cryptoHashGive(hash);
    }
    pthread_mutex_unlock (&wallet->lock);

    return cryptoTransferTake (transfer);
}

extern BRCryptoAddress
cryptoWalletGetAddress (BRCryptoWallet wallet,
                        BRCryptoAddressScheme addressScheme) {
    return wallet->handlers->getAddress (wallet, addressScheme);
}

extern bool
cryptoWalletHasAddress (BRCryptoWallet wallet,
                        BRCryptoAddress address) {

    return (wallet->type != cryptoAddressGetType(address)
            ? false
            : wallet->handlers->hasAdress (wallet, address));
}

private_extern OwnershipGiven BRSetOf(BRCyptoAddress)
cryptoWalletGetAddressesForRecovery (BRCryptoWallet wallet) {
    return wallet->handlers->getAddressesForRecovery (wallet);
}

extern BRCryptoFeeBasis
cryptoWalletGetDefaultFeeBasis (BRCryptoWallet wallet) {
    return wallet->defaultFeeBasis;
//    BRCryptoFeeBasis feeBasis;
//
//    BRCryptoUnit feeUnit = cryptoWalletGetUnitForFee (wallet);
//
//    switch (wallet->type) {
//        case BLOCK_CHAIN_TYPE_BTC: {
//            BRWallet *wid = wallet->u.btc.wid;
//
//            assert (0); // TODO: Generic Size not 1000
//            feeBasis = cryptoFeeBasisCreateAsBTC (feeUnit, (uint32_t) BRWalletFeePerKb (wid), 1000);
//            break;
//        }
//        case BLOCK_CHAIN_TYPE_ETH: {
//            BREthereumEWM ewm = wallet->u.eth.ewm;
//            BREthereumWallet wid =wallet->u.eth.wid;
//
//            BREthereumGas gas = ewmWalletGetDefaultGasLimit (ewm, wid);
//            BREthereumGasPrice gasPrice = ewmWalletGetDefaultGasPrice (ewm, wid);
//
//            feeBasis =  cryptoFeeBasisCreateAsETH (feeUnit, gas, gasPrice);
//            break;
//        }
//        case BLOCK_CHAIN_TYPE_GEN: {
//            BRGenericWallet wid = wallet->u.gen;
//
//            BRGenericFeeBasis bid = genWalletGetDefaultFeeBasis (wid);
//            feeBasis =  cryptoFeeBasisCreateAsGEN (feeUnit, bid);
//            break;
//        }
//    }
//
//    cryptoUnitGive (feeUnit);
//
//    return feeBasis;
}

extern void
cryptoWalletSetDefaultFeeBasis (BRCryptoWallet wallet,
                                BRCryptoFeeBasis feeBasis) {
    if (NULL != wallet->defaultFeeBasis) cryptoFeeBasisGive (wallet->defaultFeeBasis);
    wallet->defaultFeeBasis = cryptoFeeBasisTake(feeBasis);

//    assert (cryptoWalletGetType(wallet) == cryptoFeeBasisGetType (feeBasis));
//
//    switch (wallet->type) {
//        case BLOCK_CHAIN_TYPE_BTC:
//            // This will generate a BTC BITCOIN_WALLET_PER_PER_KB_UPDATED event.
//             BRWalletManagerUpdateFeePerKB (wallet->u.btc.bwm,
//                                            wallet->u.btc.wid,
//                                            cryptoFeeBasisAsBTC(feeBasis));
//            break;
//
//        case BLOCK_CHAIN_TYPE_ETH: {
//            BREthereumEWM ewm = wallet->u.eth.ewm;
//            BREthereumWallet wid =wallet->u.eth.wid;
//            BREthereumFeeBasis ethFeeBasis = cryptoFeeBasisAsETH (feeBasis);
//
//            // These will generate EWM WALLET_EVENT_DEFAULT_GAS_{LIMIT,PRICE}_UPDATED events
//            ewmWalletSetDefaultGasLimit (ewm, wid, ethFeeBasis.u.gas.limit);
//            ewmWalletSetDefaultGasPrice (ewm, wid, ethFeeBasis.u.gas.price);
//            break;
//        }
//
//        case BLOCK_CHAIN_TYPE_GEN: {
//            BRGenericWallet wid = wallet->u.gen;
//
//            // This will not generate any GEN events (GEN events don't exist);
//            genWalletSetDefaultFeeBasis (wid, cryptoFeeBasisAsGEN(feeBasis));
//            break;
//        }
//    }
}

extern size_t
cryptoWalletGetTransferAttributeCount (BRCryptoWallet wallet,
                                       BRCryptoAddress target) {
    return wallet->handlers->getTransferAttributeCount (wallet, target);
}

extern BRCryptoTransferAttribute
cryptoWalletGetTransferAttributeAt (BRCryptoWallet wallet,
                                    BRCryptoAddress target,
                                    size_t index) {
    return wallet->handlers->getTransferAttributeAt (wallet, target, index);
}

extern BRCryptoTransferAttributeValidationError
cryptoWalletValidateTransferAttribute (BRCryptoWallet wallet,
                                       OwnershipKept BRCryptoTransferAttribute attribute,
                                       BRCryptoBoolean *validates) {
    return wallet->handlers->validateTransferAttribute (wallet, attribute, validates);
}

extern BRCryptoTransferAttributeValidationError
cryptoWalletValidateTransferAttributes (BRCryptoWallet wallet,
                                        size_t attributesCount,
                                        OwnershipKept BRCryptoTransferAttribute *attributes,
                                        BRCryptoBoolean *validates) {
    *validates = CRYPTO_TRUE;

    for (size_t index = 0; index <attributesCount; index++) {
        cryptoWalletValidateTransferAttribute (wallet, attributes[index], validates);
        if (CRYPTO_FALSE == *validates)
            return CRYPTO_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_RELATIONSHIP_INCONSISTENCY;
    }

    return (BRCryptoTransferAttributeValidationError) 0;
}

extern BRCryptoTransfer
cryptoWalletCreateTransferMultiple (BRCryptoWallet wallet,
                                    size_t outputsCount,
                                    BRCryptoTransferOutput *outputs,
                                    BRCryptoFeeBasis estimatedFeeBasis) {
    //    assert (cryptoWalletGetType(wallet) == cryptoFeeBasisGetType(estimatedFeeBasis));
    if (0 == outputsCount) return NULL;


    BRCryptoUnit unit         = cryptoWalletGetUnit (wallet);
    BRCryptoUnit unitForFee   = cryptoWalletGetUnitForFee(wallet);
    BRCryptoCurrency currency = cryptoUnitGetCurrency(unit);

    BRCryptoTransfer transfer = wallet->handlers->createTransferMultiple (wallet,
                                                                          outputsCount,
                                                                          outputs,
                                                                          estimatedFeeBasis,
                                                                          currency,
                                                                          unit,
                                                                          unitForFee);

    cryptoCurrencyGive(currency);
    cryptoUnitGive (unitForFee);
    cryptoUnitGive (unit);

    return transfer;
}

extern BRCryptoTransfer
cryptoWalletCreateTransfer (BRCryptoWallet  wallet,
                            BRCryptoAddress target,
                            BRCryptoAmount  amount,
                            BRCryptoFeeBasis estimatedFeeBasis,
                            size_t attributesCount,
                            OwnershipKept BRCryptoTransferAttribute *attributes) {
    assert (cryptoWalletGetType(wallet) == cryptoAddressGetType(target));
    //    assert (cryptoWalletGetType(wallet) == cryptoFeeBasisGetType(estimatedFeeBasis));


    BRCryptoUnit unit       = cryptoWalletGetUnit (wallet);
    BRCryptoUnit unitForFee = cryptoWalletGetUnitForFee(wallet);

    BRCryptoCurrency currency = cryptoUnitGetCurrency(unit);
    assert (cryptoAmountHasCurrency (amount, currency));

    BRCryptoTransfer transfer = wallet->handlers->createTransfer (wallet,
                                                                  target,
                                                                  amount,
                                                                  estimatedFeeBasis,
                                                                  attributesCount,
                                                                  attributes,
                                                                  currency,
                                                                  unit,
                                                                  unitForFee);

    if (NULL != transfer && attributesCount > 0) {
        BRArrayOf (BRCryptoTransferAttribute) transferAttributes;
        array_new (transferAttributes, attributesCount);
        array_add_array (transferAttributes, attributes, attributesCount);
        cryptoTransferSetAttributes (transfer, transferAttributes);
        array_free (transferAttributes);
    }

    cryptoCurrencyGive(currency);
    cryptoUnitGive (unitForFee);
    cryptoUnitGive (unit);

    return transfer;
}

#ifdef REFACTOR
extern BRCryptoTransfer
cryptoWalletCreateTransferForWalletSweep (BRCryptoWallet  wallet,
                                          BRCryptoWalletSweeper sweeper,
                                          BRCryptoFeeBasis estimatedFeeBasis) {
    BRCryptoTransfer transfer = NULL;

    BRCryptoUnit unit       = cryptoWalletGetUnit (wallet);
    BRCryptoUnit unitForFee = cryptoWalletGetUnitForFee(wallet);

    switch (wallet->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            BRWalletManager bwm = wallet->u.btc.bwm;
            BRWallet *wid = wallet->u.btc.wid;

            BRTransaction *tid = BRWalletManagerCreateTransactionForSweep (bwm,
                                                                           wid,
                                                                           cryptoWalletSweeperAsBTC(sweeper),
                                                                           cryptoFeeBasisAsBTC(estimatedFeeBasis));
            transfer = NULL == tid ? NULL : cryptoTransferCreateAsBTC (unit,
                                                                       unitForFee,
                                                                       wid,
                                                                       tid,
                                                                       AS_CRYPTO_BOOLEAN(BRWalletManagerHandlesBTC(bwm)));
            break;
        }
        default:
            assert (0);
            break;
    }

    cryptoUnitGive (unitForFee);
    cryptoUnitGive (unit);

    return transfer;
}

extern BRCryptoTransfer
cryptoWalletCreateTransferForPaymentProtocolRequest (BRCryptoWallet wallet,
                                                     BRCryptoPaymentProtocolRequest request,
                                                     BRCryptoFeeBasis estimatedFeeBasis) {
    BRCryptoTransfer transfer = NULL;

    BRCryptoUnit unit       = cryptoWalletGetUnit (wallet);
    BRCryptoUnit unitForFee = cryptoWalletGetUnitForFee(wallet);

    switch (wallet->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            BRWalletManager bwm = wallet->u.btc.bwm;
            BRWallet *wid = wallet->u.btc.wid;

            switch (cryptoPaymentProtocolRequestGetType (request)) {
                case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
                case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
                    BRArrayOf(BRTxOutput) outputs = cryptoPaymentProtocolRequestGetOutputsAsBTC (request);
                    if (NULL != outputs) {
                        BRTransaction *tid = BRWalletManagerCreateTransactionForOutputs (bwm, wid, outputs, array_count (outputs),
                                                                                         cryptoFeeBasisAsBTC(estimatedFeeBasis));
                        transfer = NULL == tid ? NULL : cryptoTransferCreateAsBTC (unit, unitForFee, wid, tid,
                                                                                   AS_CRYPTO_BOOLEAN(BRWalletManagerHandlesBTC(bwm)));
                        array_free (outputs);
                    }
                    break;
                }
                default: {
                    assert (0);
                    break;
                }
            }
            break;
        }
        default: {
            assert (0);
            break;
        }
    }

    cryptoUnitGive (unitForFee);
    cryptoUnitGive (unit);

    return transfer;
}
#endif

extern BRCryptoFeeBasis
cryptoWalletCreateFeeBasis (BRCryptoWallet wallet,
                            BRCryptoAmount pricePerCostFactor,
                            double costFactor) {

    BRCryptoCurrency feeCurrency = cryptoUnitGetCurrency (wallet->unitForFee);
    if (CRYPTO_FALSE == cryptoAmountHasCurrency (pricePerCostFactor, feeCurrency)) {
        cryptoCurrencyGive (feeCurrency);
        return NULL;
    }
    cryptoCurrencyGive (feeCurrency);

    return cryptoFeeBasisCreate(pricePerCostFactor, costFactor);
    
#ifdef REFACTOR
    UInt256 value = cryptoAmountGetValue (pricePerCostFactor);

    switch (wallet->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            uint32_t feePerKB = value.u32[0];

            // Expect all other fields in `value` to be zero
            value.u32[0] = 0;
            if (!uint256EQL (value, UINT256_ZERO)) return NULL;

            uint32_t sizeInBytes = (uint32_t) (1000 * costFactor);

            return cryptoFeeBasisCreateAsBTC (wallet->unitForFee, feePerKB, sizeInBytes);
        }

        case BLOCK_CHAIN_TYPE_ETH: {
            BREthereumGas gas = ethGasCreate((uint64_t) costFactor);
            BREthereumGasPrice gasPrice = ethGasPriceCreate (ethEtherCreate(value));

            return cryptoFeeBasisCreateAsETH (wallet->unitForFee, gas, gasPrice);
        }

        case BLOCK_CHAIN_TYPE_GEN: {
            return cryptoFeeBasisCreateAsGEN (wallet->unitForFee,
                                              (BRGenericFeeBasis) {
                value,
                costFactor
            });
        }
    }
#endif
}

extern BRCryptoBoolean
cryptoWalletEqual (BRCryptoWallet w1, BRCryptoWallet w2) {
    return AS_CRYPTO_BOOLEAN (w1 == w2 ||
                              (w1->type == w2->type &&
                               w1->handlers->isEqual (w1, w2)));
}

extern const char *
cryptoWalletEventTypeString (BRCryptoWalletEventType t) {
    switch (t) {
        case CRYPTO_WALLET_EVENT_CREATED:
        return "CRYPTO_WALLET_EVENT_CREATED";

        case CRYPTO_WALLET_EVENT_CHANGED:
        return "CRYPTO_WALLET_EVENT_CHANGED";

        case CRYPTO_WALLET_EVENT_DELETED:
        return "CRYPTO_WALLET_EVENT_DELETED";

        case CRYPTO_WALLET_EVENT_TRANSFER_ADDED:
        return "CRYPTO_WALLET_EVENT_TRANSFER_ADDED";

        case CRYPTO_WALLET_EVENT_TRANSFER_CHANGED:
        return "CRYPTO_WALLET_EVENT_TRANSFER_CHANGED";

        case CRYPTO_WALLET_EVENT_TRANSFER_SUBMITTED:
        return "CRYPTO_WALLET_EVENT_TRANSFER_SUBMITTED";

        case CRYPTO_WALLET_EVENT_TRANSFER_DELETED:
        return "CRYPTO_WALLET_EVENT_TRANSFER_DELETED";

        case CRYPTO_WALLET_EVENT_BALANCE_UPDATED:
        return "CRYPTO_WALLET_EVENT_BALANCE_UPDATED";

        case CRYPTO_WALLET_EVENT_FEE_BASIS_UPDATED:
        return "CRYPTO_WALLET_EVENT_FEE_BASIS_UPDATED";

        case CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED:
        return "CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED";
    }
    return "<CRYPTO_WALLET_EVENT_TYPE_UNKNOWN>";
}

/// MARK: Wallet Sweeper

#ifdef REFACTOR
struct BRCryptoWalletSweeperRecord {
    BRCryptoBlockChainType type;
    BRCryptoKey key;
    BRCryptoUnit unit;
    union {
        struct {
            BRWalletSweeper sweeper;
        } btc;
    } u;
    ;
};

static BRCryptoWalletSweeperStatus
BRWalletSweeperStatusToCrypto (BRWalletSweeperStatus t) {
    switch (t) {
        case WALLET_SWEEPER_SUCCESS: return CRYPTO_WALLET_SWEEPER_SUCCESS;
        case WALLET_SWEEPER_INVALID_TRANSACTION: return CRYPTO_WALLET_SWEEPER_INVALID_TRANSACTION;
        case WALLET_SWEEPER_INVALID_SOURCE_WALLET: return CRYPTO_WALLET_SWEEPER_INVALID_SOURCE_WALLET;
        case WALLET_SWEEPER_NO_TRANSACTIONS_FOUND: return CRYPTO_WALLET_SWEEPER_NO_TRANSFERS_FOUND;
        case WALLET_SWEEPER_INSUFFICIENT_FUNDS: return CRYPTO_WALLET_SWEEPER_INSUFFICIENT_FUNDS;
        case WALLET_SWEEPER_UNABLE_TO_SWEEP: return CRYPTO_WALLET_SWEEPER_UNABLE_TO_SWEEP;
    }
}

extern BRCryptoWalletSweeperStatus
cryptoWalletSweeperValidateSupported (BRCryptoNetwork network,
                                      BRCryptoCurrency currency,
                                      BRCryptoKey key,
                                      BRCryptoWallet wallet) {
    if (CRYPTO_FALSE == cryptoNetworkHasCurrency (network, currency)) {
        return CRYPTO_WALLET_SWEEPER_INVALID_ARGUMENTS;
    }

    if (cryptoNetworkGetType (network) != cryptoWalletGetType (wallet)) {
        return CRYPTO_WALLET_SWEEPER_INVALID_ARGUMENTS;
    }

    BRCryptoCurrency walletCurrency = cryptoWalletGetCurrency (wallet);
    if (CRYPTO_FALSE == cryptoCurrencyIsIdentical (currency, walletCurrency)) {
        cryptoCurrencyGive (walletCurrency);
        return CRYPTO_WALLET_SWEEPER_INVALID_ARGUMENTS;
    }
    cryptoCurrencyGive (walletCurrency);

    if (CRYPTO_FALSE == cryptoKeyHasSecret (key)) {
        return CRYPTO_WALLET_SWEEPER_INVALID_KEY;
    }

    switch (cryptoWalletGetType (wallet)) {
        case BLOCK_CHAIN_TYPE_BTC: {
            BRWallet * wid             = cryptoWalletAsBTC (wallet);
            BRKey * keyCore            = cryptoKeyGetCore (key);
            BRAddressParams addrParams = cryptoNetworkAsBTC (network)->addrParams;

            return BRWalletSweeperStatusToCrypto (BRWalletSweeperValidateSupported (keyCore,
                                                                                    addrParams,
                                                                                    wid));
        }
        default:{
            break;
        }
    }

    return CRYPTO_WALLET_SWEEPER_UNSUPPORTED_CURRENCY;
}

extern BRCryptoWalletSweeper
cryptoWalletSweeperCreateAsBtc (BRCryptoNetwork network,
                                BRCryptoCurrency currency,
                                BRCryptoKey key,
                                BRCryptoAddressScheme scheme) {
    assert (cryptoKeyHasSecret (key));
    BRCryptoWalletSweeper sweeper = calloc (1, sizeof(struct BRCryptoWalletSweeperRecord));
    sweeper->type = BLOCK_CHAIN_TYPE_BTC;
    sweeper->key = cryptoKeyTake (key);
    sweeper->unit = cryptoNetworkGetUnitAsBase (network, currency);
    sweeper->u.btc.sweeper = BRWalletSweeperNew(cryptoKeyGetCore (key),
                                                cryptoNetworkAsBTC (network)->addrParams,
                                                CRYPTO_ADDRESS_SCHEME_BTC_SEGWIT == scheme);
    return sweeper;
}

extern void
cryptoWalletSweeperRelease (BRCryptoWalletSweeper sweeper) {
    switch (sweeper->type) {
        case BLOCK_CHAIN_TYPE_BTC:
            BRWalletSweeperFree (sweeper->u.btc.sweeper);
            break;
        default:
            assert (0);
            break;
    }
    cryptoKeyGive (sweeper->key);
    cryptoUnitGive (sweeper->unit);

    memset (sweeper, 0, sizeof(struct BRCryptoWalletSweeperRecord));
    free (sweeper);
}

extern BRCryptoWalletSweeperStatus
cryptoWalletSweeperHandleTransactionAsBTC (BRCryptoWalletSweeper sweeper,
                                           OwnershipKept uint8_t *transaction,
                                           size_t transactionLen) {
    BRCryptoWalletSweeperStatus status = CRYPTO_WALLET_SWEEPER_ILLEGAL_OPERATION;

    switch (sweeper->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            status = BRWalletSweeperStatusToCrypto (
                BRWalletSweeperHandleTransaction (sweeper->u.btc.sweeper,
                                                  transaction, transactionLen)
            );
            break;
        }
        default:
            assert (0);
            break;
    }

    return status;
}

extern BRCryptoKey
cryptoWalletSweeperGetKey (BRCryptoWalletSweeper sweeper) {
    return cryptoKeyTake (sweeper->key);
}

extern char *
cryptoWalletSweeperGetAddress (BRCryptoWalletSweeper sweeper) {
    char * address = NULL;

    switch (sweeper->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            address = BRWalletSweeperGetLegacyAddress (sweeper->u.btc.sweeper);
            break;
        }
        default:
            assert (0);
            break;
    }

    return address;
}

extern BRCryptoAmount
cryptoWalletSweeperGetBalance (BRCryptoWalletSweeper sweeper) {
    BRCryptoAmount amount = NULL;

    switch (sweeper->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            UInt256 value = uint256Create (BRWalletSweeperGetBalance (sweeper->u.btc.sweeper));
            amount = cryptoAmountCreate (sweeper->unit, CRYPTO_FALSE, value);
            break;
        }
        default:
            assert (0);
            break;
    }

    return amount;
}

extern BRCryptoWalletSweeperStatus
cryptoWalletSweeperValidate (BRCryptoWalletSweeper sweeper) {
    BRCryptoWalletSweeperStatus status = CRYPTO_WALLET_SWEEPER_ILLEGAL_OPERATION;

    switch (sweeper->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            status = BRWalletSweeperStatusToCrypto (
                BRWalletSweeperValidate(sweeper->u.btc.sweeper)
            );
            break;
        }
        default:
            assert (0);
            break;
    }

    return status;
}

private_extern BRWalletSweeper
cryptoWalletSweeperAsBTC (BRCryptoWalletSweeper sweeper) {
    assert (BLOCK_CHAIN_TYPE_BTC == sweeper->type);
    return sweeper->u.btc.sweeper;
}
#endif
