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


static void
cryptoWalletUpdTransfer (BRCryptoWallet wallet,
                                  BRCryptoTransfer transfer,
                                  OwnershipKept BRCryptoTransferState newState);

static void
cryptoWalletUpdBalanceOnTransferConfirmation (BRCryptoWallet wallet,
                                              BRCryptoTransfer transfer);

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoWallet, cryptoWallet)

extern BRCryptoWallet
cryptoWalletAllocAndInit (size_t sizeInBytes,
                          BRCryptoBlockChainType type,
                          BRCryptoWalletListener listener,
                          BRCryptoUnit unit,
                          BRCryptoUnit unitForFee,
                          BRCryptoAmount balanceMinimum,
                          BRCryptoAmount balanceMaximum,
                          BRCryptoFeeBasis defaultFeeBasis,
                          BRCryptoWalletCreateContext createContext,
                          BRCryptoWalletCreateCallbak createCallback) {
    assert (sizeInBytes >= sizeof (struct BRCryptoWalletRecord));

    BRCryptoWallet wallet = calloc (1, sizeInBytes);

    wallet->sizeInBytes = sizeInBytes;
    wallet->type  = type;
    wallet->handlers = cryptoHandlersLookup(type)->wallet;

    wallet->listener   = listener;
    wallet->state      = CRYPTO_WALLET_STATE_CREATED;
    wallet->unit       = cryptoUnitTake (unit);
    wallet->unitForFee = cryptoUnitTake (unitForFee);

    BRCryptoCurrency currency = cryptoUnitGetCurrency(unit);
    assert (NULL == balanceMinimum || cryptoAmountHasCurrency (balanceMinimum, currency));
    assert (NULL == balanceMaximum || cryptoAmountHasCurrency (balanceMaximum, currency));
    cryptoCurrencyGive (currency);

    wallet->balanceMinimum = cryptoAmountTake (balanceMinimum);
    wallet->balanceMaximum = cryptoAmountTake (balanceMaximum);
    wallet->balance = cryptoAmountCreateInteger(0, unit);

    wallet->defaultFeeBasis = cryptoFeeBasisTake (defaultFeeBasis);

    array_new (wallet->transfers, 5);

    wallet->ref = CRYPTO_REF_ASSIGN (cryptoWalletRelease);

    wallet->listenerTransfer = cryptoListenerCreateTransferListener (&wallet->listener, wallet, cryptoWalletUpdTransfer);

    pthread_mutex_init_brd (&wallet->lock, PTHREAD_MUTEX_NORMAL);  // PTHREAD_MUTEX_RECURSIVE

    if (NULL != createCallback) createCallback (createContext, wallet);

    cryptoWalletGenerateEvent (wallet, (BRCryptoWalletEvent) {
        CRYPTO_WALLET_EVENT_CREATED
    });

    return wallet;
}

static void
cryptoWalletRelease (BRCryptoWallet wallet) {
    pthread_mutex_lock (&wallet->lock);
    cryptoWalletSetState (wallet, CRYPTO_WALLET_STATE_DELETED);

    cryptoUnitGive (wallet->unit);
    cryptoUnitGive (wallet->unitForFee);

    cryptoAmountGive (wallet->balanceMinimum);
    cryptoAmountGive (wallet->balanceMaximum);
    cryptoAmountGive (wallet->balance);

    cryptoFeeBasisGive (wallet->defaultFeeBasis);

    for (size_t index = 0; index < array_count(wallet->transfers); index++)
        cryptoTransferGive (wallet->transfers[index]);
    array_free (wallet->transfers);

    wallet->handlers->release (wallet);

    pthread_mutex_unlock  (&wallet->lock);
    pthread_mutex_destroy (&wallet->lock);

    cryptoWalletGenerateEvent (wallet, (BRCryptoWalletEvent) {
        CRYPTO_WALLET_EVENT_DELETED
    });

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

extern BRCryptoBoolean
cryptoWalletHasCurrency (BRCryptoWallet wallet,
                         BRCryptoCurrency currency) {
    return cryptoUnitHasCurrency (wallet->unit, currency);
}

extern BRCryptoWalletState
cryptoWalletGetState (BRCryptoWallet wallet) {
    return wallet->state;
}

private_extern void
cryptoWalletSetState (BRCryptoWallet wallet,
                      BRCryptoWalletState state) {
    BRCryptoWalletState newState = state;
    BRCryptoWalletState oldState = wallet->state;

    wallet->state = state;

    if (oldState != newState)
         cryptoWalletGenerateEvent (wallet, (BRCryptoWalletEvent) {
             CRYPTO_WALLET_EVENT_CHANGED,
             { .state = { oldState, newState }}
         });
}

extern BRCryptoCurrency
cryptoWalletGetCurrencyForFee (BRCryptoWallet wallet) {
    return cryptoUnitGetCurrency (wallet->unitForFee);
}

extern BRCryptoUnit
cryptoWalletGetUnitForFee (BRCryptoWallet wallet) {
    return cryptoUnitTake (wallet->unitForFee);
}

extern BRCryptoBoolean
cryptoWalletHasCurrencyForFee (BRCryptoWallet wallet,
                               BRCryptoCurrency currency) {
    return cryptoUnitHasCurrency (wallet->unitForFee, currency);
}

extern BRCryptoAmount
cryptoWalletGetBalance (BRCryptoWallet wallet) {
    return cryptoAmountTake (wallet->balance);
}

static void
cryptoWalletSetBalance (BRCryptoWallet wallet,
                        OwnershipGiven BRCryptoAmount newBalance) {
    BRCryptoAmount oldBalance = wallet->balance;

    wallet->balance = newBalance;

    if (CRYPTO_COMPARE_EQ != cryptoAmountCompare (oldBalance, newBalance)) {
        cryptoWalletGenerateEvent (wallet, (BRCryptoWalletEvent) {
            CRYPTO_WALLET_EVENT_BALANCE_UPDATED,
            { .balanceUpdated = { cryptoAmountTake (newBalance) }}
        });
    }

    cryptoAmountGive(oldBalance);
}

static void
cryptoWalletIncBalance (BRCryptoWallet wallet,
                        BRCryptoAmount amount) {
    cryptoWalletSetBalance (wallet, cryptoAmountAdd (wallet->balance, amount));
}

static void
cryptoWalletDecBalance (BRCryptoWallet wallet,
                        BRCryptoAmount amount) {
    cryptoWalletSetBalance (wallet, cryptoAmountSub (wallet->balance, amount));
}

//
// Recompute the balance by iterating over all transfers and summing the 'amount directed net'.
// This is appropriately used when the 'amount directed net' has changed; typically when a
// transfer's state is become 'included' and thus the fee has been finalized.  Note, however, we
// handle an estimated vs confirmed fee explicitly in the subsequent function.
//
#pragma clang diagnostic push
#pragma GCC   diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma GCC   diagnostic ignored "-Wunused-function"
static void
cryptoWalletUpdBalance (BRCryptoWallet wallet) {
    pthread_mutex_lock (&wallet->lock);
    BRCryptoAmount balance = cryptoAmountCreateInteger (0, wallet->unit);

    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        BRCryptoAmount amount     = cryptoTransferGetAmountDirectedNet (wallet->transfers[index]);
        BRCryptoAmount newBalance = cryptoAmountAdd (balance, amount);

        cryptoAmountGive(amount);
        cryptoAmountGive(balance);

        balance = newBalance;
    }
    pthread_mutex_unlock (&wallet->lock);

    cryptoWalletSetBalance (wallet, balance);
}
#pragma clang diagnostic pop
#pragma GCC   diagnostic pop

//
// When a transfer is confirmed, the fee can change.  A typical example is that for ETH a transfer
// has an estimated fee based on `gasLimit` but an actual, included fee based on `gasUsed`.  The
// following code handles a fee change - and only a fee change.
//
// There are BTC cases where the amount changes when *another* transfer is confirmed.  Specifically,
// BRWallet has a BRTransaction 'in the future' where one of the inputs is an early UTXO.  The
// other inputs and the outputs of the BRTransaction aren't resolved until early transactions are
// added to the BRWallet, and then addresses are generated upto the gap_limit.  Only then can the
// later transactions inputs and outputs be resovled.  We don't handle that case in this function.
//
static void
cryptoWalletUpdBalanceOnTransferConfirmation (BRCryptoWallet wallet,
                                              BRCryptoTransfer transfer) {
    // if this wallet does not pay fees, then there is nothing to do
    if (CRYPTO_FALSE == cryptoUnitIsCompatible (wallet->unit, wallet->unitForFee))
        return;

    BRCryptoAmount feeEstimated = cryptoTransferGetEstimatedFee (transfer);
    BRCryptoAmount feeConfirmed = cryptoTransferGetConfirmedFee (transfer);
    assert (NULL == feeConfirmed || NULL == feeEstimated || cryptoAmountIsCompatible (feeEstimated, feeConfirmed));
    assert (NULL == feeConfirmed || cryptoAmountIsCompatible (feeConfirmed, wallet->balance));
    // TODO: assert (NULL != feeConfirmed)

    BRCryptoAmount change = NULL;

    if (NULL != feeConfirmed && NULL != feeEstimated)
        change = cryptoAmountSub (feeConfirmed, feeEstimated);
    else if (NULL != feeConfirmed)
        change = cryptoAmountTake (feeConfirmed);
    else if (NULL != feeEstimated)
        change = cryptoAmountNegate (feeEstimated);

    if (NULL != change && CRYPTO_FALSE == cryptoAmountIsZero(change))
        cryptoWalletIncBalance (wallet, change);

    cryptoAmountGive (change);
    cryptoAmountGive (feeEstimated);
    cryptoAmountGive (feeConfirmed);
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

static void
cryptoWalletAnnounceTransfer (BRCryptoWallet wallet,
                              BRCryptoTransfer transfer,
                              BRCryptoWalletEventType type) {  // TRANSFER_{ADDED,DELETED}
    assert (CRYPTO_WALLET_EVENT_TRANSFER_ADDED == type ||
            CRYPTO_WALLET_EVENT_TRANSFER_DELETED == type);
    if (NULL != wallet->handlers->announceTransfer)
        wallet->handlers->announceTransfer (wallet, transfer, type);
}

extern void
cryptoWalletAddTransfer (BRCryptoWallet wallet,
                         BRCryptoTransfer transfer) {
    pthread_mutex_lock (&wallet->lock);
    if (CRYPTO_FALSE == cryptoWalletHasTransferLock (wallet, transfer, false)) {
        array_add (wallet->transfers, cryptoTransferTake(transfer));
        cryptoWalletAnnounceTransfer (wallet, transfer, CRYPTO_WALLET_EVENT_TRANSFER_ADDED);
        cryptoWalletGenerateEvent (wallet, (BRCryptoWalletEvent) {
            CRYPTO_WALLET_EVENT_TRANSFER_ADDED,
            { .transfer = cryptoTransferTake (transfer) }
        });
        cryptoWalletIncBalance (wallet, cryptoTransferGetAmountDirectedNet(transfer));
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
            cryptoWalletAnnounceTransfer (wallet, transfer, CRYPTO_WALLET_EVENT_TRANSFER_DELETED);
            cryptoWalletGenerateEvent (wallet, (BRCryptoWalletEvent) {
                CRYPTO_WALLET_EVENT_TRANSFER_DELETED,
                { .transfer = cryptoTransferTake (transfer) }
            });
            cryptoWalletDecBalance (wallet, cryptoTransferGetAmountDirectedNet(transfer));
            break;
        }
    }
    pthread_mutex_unlock (&wallet->lock);

    // drop reference outside of lock to avoid potential case where release function runs
    if (NULL != walletTransfer) cryptoTransferGive (transfer);
}

static void
cryptoWalletUpdTransfer (BRCryptoWallet wallet,
                         BRCryptoTransfer transfer,
                         OwnershipKept BRCryptoTransferState newState) {
    // The transfer's state has changed.  This implies a possible amount/fee change.
    if (newState.type == CRYPTO_TRANSFER_STATE_INCLUDED &&
        CRYPTO_TRUE == cryptoWalletHasTransfer (wallet, transfer))
        cryptoWalletUpdBalanceOnTransferConfirmation (wallet, transfer);
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
        if (CRYPTO_TRUE == cryptoHashEqual(hash, hashToMatch))
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
    return cryptoFeeBasisTake (wallet->defaultFeeBasis);
}

extern void
cryptoWalletSetDefaultFeeBasis (BRCryptoWallet wallet,
                                BRCryptoFeeBasis feeBasis) {
    if (NULL != wallet->defaultFeeBasis) cryptoFeeBasisGive (wallet->defaultFeeBasis);
    wallet->defaultFeeBasis = cryptoFeeBasisTake(feeBasis);
    cryptoWalletGenerateEvent (wallet, (BRCryptoWalletEvent) {
        CRYPTO_WALLET_EVENT_FEE_BASIS_UPDATED,
        { .feeBasisUpdated = { cryptoFeeBasisTake (wallet->defaultFeeBasis) }}
    });
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

    if (NULL != transfer && attributesCount > 0)
        cryptoTransferSetAttributes (transfer, attributesCount, attributes);

    cryptoCurrencyGive(currency);
    cryptoUnitGive (unitForFee);
    cryptoUnitGive (unit);

    return transfer;
}

extern BRCryptoTransfer
cryptoWalletCreateTransferForPaymentProtocolRequest (BRCryptoWallet wallet,
                                                     BRCryptoPaymentProtocolRequest request,
                                                     BRCryptoFeeBasis estimatedFeeBasis) {
    const BRCryptoPaymentProtocolHandlers * paymentHandlers = cryptoHandlersLookup(cryptoWalletGetType(wallet))->payment;
    
    assert (NULL != paymentHandlers);
    
    return paymentHandlers->createTransfer (request,
                                            wallet,
                                            estimatedFeeBasis);
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
