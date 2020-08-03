//
//  BRCryptoWallet.h
//  BRCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoWallet_h
#define BRCryptoWallet_h

#include "BRCryptoStatus.h"
#include "BRCryptoKey.h"
#include "BRCryptoNetwork.h"        // NetworkFee
#include "BRCryptoPayment.h"
#include "BRCryptoFeeBasis.h"
#include "BRCryptoTransfer.h"
#include "BRCryptoClient.h"

#ifdef __cplusplus
extern "C" {
#endif

    /// MARK: Wallet Event

    typedef enum {
        CRYPTO_WALLET_STATE_CREATED,
        CRYPTO_WALLET_STATE_DELETED
    } BRCryptoWalletState;

    typedef enum {
        /// Signaled when a wallet is *allocated*; the wallet may not, in fact generally is
        /// not, fully initialized.  Thus the wallet should only be used for 'identity' purposes.
        CRYPTO_WALLET_EVENT_CREATED,

        /// Signaled when a wallet's state change - such as when the state transitions from
        /// CREATED to DELETED.
        CRYPTO_WALLET_EVENT_CHANGED,

        /// Signaled when a wallet is deleted; the wallet must not be 'dereferenced' and thus
        /// the pointer value can be used.  Surely the wallets's memory will be gone by the time
        /// that thread handling the event first sees the deleted wallet.  If any dereference
        /// occurs, the result will be an instant crash.
        CRYPTO_WALLET_EVENT_DELETED,

        /// Signaled when a transfer is added to the wallet
        CRYPTO_WALLET_EVENT_TRANSFER_ADDED,

        /// Signaled when a transfer is changed.
        CRYPTO_WALLET_EVENT_TRANSFER_CHANGED,

        /// Signaled when a transfer is submitted.
        CRYPTO_WALLET_EVENT_TRANSFER_SUBMITTED,

        /// Signaled when a transfer is removed from the wallet.
        CRYPTO_WALLET_EVENT_TRANSFER_DELETED,

        /// Signaled when the wallet's balance changes.
        CRYPTO_WALLET_EVENT_BALANCE_UPDATED,

        /// Signaled when the wallet's default feeBasis changes.
        CRYPTO_WALLET_EVENT_FEE_BASIS_UPDATED,

        /// Signaled when the wallet's feeBaiss is estimated.
        CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED,
    } BRCryptoWalletEventType;

    extern const char *
    cryptoWalletEventTypeString (BRCryptoWalletEventType t);

    typedef struct {
        BRCryptoWalletEventType type;
        union {
            struct {
                BRCryptoWalletState oldState;
                BRCryptoWalletState newState;
            } state;
            
            struct {
                /// Handler must 'give'
                BRCryptoTransfer value;
            } transfer;
            
            struct {
                /// Handler must 'give'
                BRCryptoAmount amount;
            } balanceUpdated;
            
            struct {
                /// Handler must 'give'
                BRCryptoFeeBasis basis;
            } feeBasisUpdated;
            
            struct {
                /// Handler must 'give' basis
                BRCryptoStatus status;
                BRCryptoCookie cookie;
                BRCryptoFeeBasis basis;
            } feeBasisEstimated;
        } u;
    } BRCryptoWalletEvent;

    // MARK: - Wallet Listener

    typedef void (*BRCryptoWalletListenerCallback) (BRCryptoListenerContext context,
                                                    BRCryptoWalletManager manager,
                                                    BRCryptoWallet wallet,
                                                    BRCryptoWalletEvent event);

    // MARK: - Wallet

    extern BRCryptoWalletState
    cryptoWalletGetState (BRCryptoWallet wallet);

    /**
     * Returns the wallet's currency
     *
     * @param wallet the wallet
     *
     * @return The currency w/ an incremented reference count (aka 'taken')
     */
    extern BRCryptoCurrency
    cryptoWalletGetCurrency (BRCryptoWallet wallet);

    /**
     * Returns the wallet's (default) unit.  Used for *display* of the wallet's balance.
     *
     * @param wallet The wallet
     *
     * @return the unit w/ an incremented reference count (aka 'taken')
     */
    extern BRCryptoUnit
    cryptoWalletGetUnit (BRCryptoWallet wallet);

    extern BRCryptoCurrency
    cryptoWalletGetCurrencyForFee (BRCryptoWallet wallet);

    /**
     * Returns the wallet's fee unit.
     *
     * @param wallet The wallet
     *
     * @return the fee unit w/ an incremented reference count (aka 'taken')
     */
    extern BRCryptoUnit
    cryptoWalletGetUnitForFee (BRCryptoWallet wallet);

    /**
     * Returns the wallets balance
     *
     * @param wallet the wallet
     *
     * @return the balance
     */
    extern BRCryptoAmount
    cryptoWalletGetBalance (BRCryptoWallet wallet);

    extern BRCryptoAmount /* nullable */
    cryptoWalletGetBalanceMinimum (BRCryptoWallet wallet);

    extern BRCryptoAmount /* nullable */
    cryptoWalletGetBalanceMaximum (BRCryptoWallet wallet);

    extern BRCryptoBoolean
    cryptoWalletHasTransfer (BRCryptoWallet wallet,
                             BRCryptoTransfer transfer);

    /**
     * Returns a newly allocated array of the wallet's transfers.
     *
     * The caller is responsible for deallocating the returned array using
     * free().
     *
     * @param wallet the wallet
     * @param count the number of transfers returned
     *
     * @return An array of transfers w/ an incremented reference count (aka 'taken')
     *         or NULL if there are no transfers in the wallet.
     */
    extern BRCryptoTransfer *
    cryptoWalletGetTransfers (BRCryptoWallet wallet,
                              size_t *count);

    /**
     * Returns a 'new' adddress from `wallet` according to the provided `addressScheme`.  For BTC
     * this is a segwit or a bech32 address.  Note that the returned address is not associated with
     * `wallet` and thus one runs the risk of using a BRCryptoAddress w/ the wrong BRCryptoWallet
     */
    extern BRCryptoAddress
    cryptoWalletGetAddress (BRCryptoWallet wallet,
                            BRCryptoAddressScheme addressScheme);

    /**
     * Check if `wallet` has `address`.  Checks that `address` has been used already by `wallet`
     * or if `address` is the *next* address from `wallet`
     */
    extern bool
    cryptoWalletHasAddress (BRCryptoWallet wallet,
                            BRCryptoAddress address);

    extern BRCryptoFeeBasis
    cryptoWalletGetDefaultFeeBasis (BRCryptoWallet wallet);

    extern void
    cryptoWalletSetDefaultFeeBasis (BRCryptoWallet wallet,
                                    BRCryptoFeeBasis feeBasis);

    extern size_t
    cryptoWalletGetTransferAttributeCount (BRCryptoWallet wallet,
                                           BRCryptoAddress target);

    extern BRCryptoTransferAttribute
    cryptoWalletGetTransferAttributeAt (BRCryptoWallet wallet,
                                        BRCryptoAddress target,
                                        size_t index);

    extern BRCryptoTransferAttributeValidationError
    cryptoWalletValidateTransferAttribute (BRCryptoWallet wallet,
                                           OwnershipKept BRCryptoTransferAttribute attribute,
                                           BRCryptoBoolean *validates);

    extern BRCryptoTransferAttributeValidationError
    cryptoWalletValidateTransferAttributes (BRCryptoWallet wallet,
                                            size_t attributesCount,
                                            OwnershipKept BRCryptoTransferAttribute *attribute,
                                            BRCryptoBoolean *validates);

    /**
     * Create a transfer.
     *
     * @param wallet The wallet providing the amount
     * @param target The target address; this must be consistent with the provied wallet's address
     * @param amount the amount to transfer
     * @param estimatedFeeBasis the fees one is willing to
     *
     * @return the transfer or NULL
     */
    extern BRCryptoTransfer
    cryptoWalletCreateTransfer (BRCryptoWallet wallet,
                                BRCryptoAddress target,
                                BRCryptoAmount amount,
                                BRCryptoFeeBasis estimatedFeeBasis,
                                size_t attributesCount,
                                OwnershipKept BRCryptoTransferAttribute *attributes);

    extern BRCryptoTransfer
    cryptoWalletCreateTransferForPaymentProtocolRequest (BRCryptoWallet wallet,
                                                         BRCryptoPaymentProtocolRequest request,
                                                         BRCryptoFeeBasis estimatedFeeBasis);

    extern BRCryptoTransfer
    cryptoWalletCreateTransferMultiple (BRCryptoWallet wallet,
                                        size_t outputsCount,
                                        BRCryptoTransferOutput *outputs,
                                        BRCryptoFeeBasis estimatedFeeBasis);

    extern void
    cryptoWalletAddTransfer (BRCryptoWallet wallet, BRCryptoTransfer transfer);

    extern void
    cryptoWalletRemTransfer (BRCryptoWallet wallet, BRCryptoTransfer transfer);

    extern BRCryptoFeeBasis
    cryptoWalletCreateFeeBasis (BRCryptoWallet wallet,
                                BRCryptoAmount pricePerCostFactor,
                                double costFactor);

    extern BRCryptoBoolean
    cryptoWalletEqual (BRCryptoWallet w1, BRCryptoWallet w2);

    DECLARE_CRYPTO_GIVE_TAKE (BRCryptoWallet, cryptoWallet);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoWallet_h */
