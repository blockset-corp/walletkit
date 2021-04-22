//
//  BRCryptoWallet.h
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoWallet_h
#define BRCryptoWallet_h

#include "BRCryptoKey.h"
#include "BRCryptoNetwork.h"        // NetworkFee
#include "BRCryptoPayment.h"
#include "BRCryptoFeeBasis.h"
#include "BRCryptoTransfer.h"
#include "BRCryptoClient.h"
#include "BRCryptoListener.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Wallet

/**
 * Get the wallet's state
 */
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
 * Check if the wallet has `currency`
 */
extern BRCryptoBoolean
cryptoWalletHasCurrency (BRCryptoWallet wallet,
                         BRCryptoCurrency currency);

/**
 * Returns the wallet's (default) unit.  Used for *display* of the wallet's balance.
 *
 * @param wallet The wallet
 *
 * @return the unit w/ an incremented reference count (aka 'taken')
 */
extern BRCryptoUnit
cryptoWalletGetUnit (BRCryptoWallet wallet);

/**
 * Get the currency used for wallet fees.  This currency might not be the wallet's currency.
 * For example, Ethereum ERC20 wallets will have a currency for the ERC20 token, but fees
 * will be paid in Ethere.
 */
extern BRCryptoCurrency
cryptoWalletGetCurrencyForFee (BRCryptoWallet wallet);

/**
 * Check if wallet uses `currency` for fees.
 */
extern BRCryptoBoolean
cryptoWalletHasCurrencyForFee (BRCryptoWallet wallet,
                               BRCryptoCurrency currency);

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

/**
 * Get the wallet's minimum balance.  Returns NULL if there is no minimum (and thus zero is
 * the implied minimum).
 */
extern BRCryptoAmount /* nullable */
cryptoWalletGetBalanceMinimum (BRCryptoWallet wallet);

/**
 * Get the wallet's maximum balance.  Returns NULL if there is no maximum (and thus the wallet's
 * balance is the implied maximum)
 */
extern BRCryptoAmount /* nullable */
cryptoWalletGetBalanceMaximum (BRCryptoWallet wallet);

/**
 * Check if wallet contains `transfer`
 */
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

/**
 * Get the wallet's default fee basis.
 */
extern BRCryptoFeeBasis
cryptoWalletGetDefaultFeeBasis (BRCryptoWallet wallet);

/**
 * Set the wallet's default fee basis.
 */
extern void
cryptoWalletSetDefaultFeeBasis (BRCryptoWallet wallet,
                                BRCryptoFeeBasis feeBasis);

/**
 * Get the count of transfer attributes that are relevent to `wallet` when a transfer uses
 * the `target` address.  For example, when transfering XRP with a target that is Coinbase one
 * must provide a transfer attribute of "DestinationTag".  The returned count will be one.
 */
extern size_t
cryptoWalletGetTransferAttributeCount (BRCryptoWallet wallet,
                                       BRCryptoAddress target);

/**
 * Get the transfer attribtue at `index` from the attributes that are relevent to 'wallet' when
 * a transfer uses the `target` address.  The value of `index` must be [0, count).
 */
extern BRCryptoTransferAttribute
cryptoWalletGetTransferAttributeAt (BRCryptoWallet wallet,
                                    BRCryptoAddress target,
                                    size_t index);

/**
 * Validate `attribute` for `wallet` and fill `validates` with the result.  If `validates` is
 * CRYPTO_TRUE then the return value described the validation error; otherwise the return
 * value is undefined
 */
extern BRCryptoTransferAttributeValidationError
cryptoWalletValidateTransferAttribute (BRCryptoWallet wallet,
                                       OwnershipKept BRCryptoTransferAttribute attribute,
                                       BRCryptoBoolean *validates);

/**
 * Validate `attributes` for `wallet` and fill `validates` with the result.  If `validates` is
 * CRYPTO_TRUE then the return value described the validation error; otherwise the return
 * value is undefined.  The first attribute that is invalid will be returned; subsequent
 * attributes will not be validated.
 */
extern BRCryptoTransferAttributeValidationError
cryptoWalletValidateTransferAttributes (BRCryptoWallet wallet,
                                        size_t attributesCount,
                                        OwnershipKept BRCryptoTransferAttribute *attributes,
                                        BRCryptoBoolean *validates);

/**
 * Check the the transfer attributes that are relevent to `wallet` when a transfer uses
 * the `target` address contain an attribute with `key`.  Fill in `isRequired` if the attribute
 * matching `key` is a required attribute.
 */
extern BRCryptoBoolean
cryptoWalletHasTransferAttributeForKey (BRCryptoWallet wallet,
                                        BRCryptoAddress target,
                                        const char *key,
                                        BRCryptoBoolean *isRequired);

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

/**
 * Check of two wallets are equal.
 */
extern BRCryptoBoolean
cryptoWalletEqual (BRCryptoWallet w1, BRCryptoWallet w2);

DECLARE_CRYPTO_GIVE_TAKE (BRCryptoWallet, cryptoWallet);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoWallet_h */
