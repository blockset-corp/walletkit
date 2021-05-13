//
//  WKWallet.h
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKWallet_h
#define WKWallet_h

#include "WKKey.h"
#include "WKNetwork.h"        // NetworkFee
#include "WKPayment.h"
#include "WKFeeBasis.h"
#include "WKTransfer.h"
#include "WKClient.h"
#include "WKListener.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Wallet

/**
 * Get the wallet's state
 */
extern WKWalletState
wkWalletGetState (WKWallet wallet);

/**
 * Returns the wallet's currency
 *
 * @param wallet the wallet
 *
 * @return The currency w/ an incremented reference count (aka 'taken')
 */
extern WKCurrency
wkWalletGetCurrency (WKWallet wallet);

/**
 * Check if the wallet has `currency`
 */
extern WKBoolean
wkWalletHasCurrency (WKWallet wallet,
                         WKCurrency currency);

/**
 * Returns the wallet's (default) unit.  Used for *display* of the wallet's balance.
 *
 * @param wallet The wallet
 *
 * @return the unit w/ an incremented reference count (aka 'taken')
 */
extern WKUnit
wkWalletGetUnit (WKWallet wallet);

/**
 * Get the currency used for wallet fees.  This currency might not be the wallet's currency.
 * For example, Ethereum ERC20 wallets will have a currency for the ERC20 token, but fees
 * will be paid in Ethere.
 */
extern WKCurrency
wkWalletGetCurrencyForFee (WKWallet wallet);

/**
 * Check if wallet uses `currency` for fees.
 */
extern WKBoolean
wkWalletHasCurrencyForFee (WKWallet wallet,
                               WKCurrency currency);

/**
 * Returns the wallet's fee unit.
 *
 * @param wallet The wallet
 *
 * @return the fee unit w/ an incremented reference count (aka 'taken')
 */
extern WKUnit
wkWalletGetUnitForFee (WKWallet wallet);

/**
 * Returns the wallets balance
 *
 * @param wallet the wallet
 *
 * @return the balance
 */
extern WKAmount
wkWalletGetBalance (WKWallet wallet);

/**
 * Get the wallet's minimum balance.  Returns NULL if there is no minimum (and thus zero is
 * the implied minimum).
 */
extern WKAmount /* nullable */
wkWalletGetBalanceMinimum (WKWallet wallet);

/**
 * Get the wallet's maximum balance.  Returns NULL if there is no maximum (and thus the wallet's
 * balance is the implied maximum)
 */
extern WKAmount /* nullable */
wkWalletGetBalanceMaximum (WKWallet wallet);

/**
 * Check if wallet contains `transfer`
 */
extern WKBoolean
wkWalletHasTransfer (WKWallet wallet,
                         WKTransfer transfer);

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
extern WKTransfer *
wkWalletGetTransfers (WKWallet wallet,
                          size_t *count);

/**
 * Returns a 'new' adddress from `wallet` according to the provided `addressScheme`.  For BTC
 * this is a segwit or a bech32 address.  Note that the returned address is not associated with
 * `wallet` and thus one runs the risk of using a WKAddress w/ the wrong WKWallet
 */
extern WKAddress
wkWalletGetAddress (WKWallet wallet,
                        WKAddressScheme addressScheme);

/**
 * Check if `wallet` has `address`.  Checks that `address` has been used already by `wallet`
 * or if `address` is the *next* address from `wallet`
 */
extern bool
wkWalletHasAddress (WKWallet wallet,
                        WKAddress address);

/**
 * Get the wallet's default fee basis.
 */
extern WKFeeBasis
wkWalletGetDefaultFeeBasis (WKWallet wallet);

/**
 * Set the wallet's default fee basis.
 */
extern void
wkWalletSetDefaultFeeBasis (WKWallet wallet,
                                WKFeeBasis feeBasis);

/**
 * Get the count of transfer attributes that are relevent to `wallet` when a transfer uses
 * the `target` address.  For example, when transfering XRP with a target that is Coinbase one
 * must provide a transfer attribute of "DestinationTag".  The returned count will be one.
 */
extern size_t
wkWalletGetTransferAttributeCount (WKWallet wallet,
                                       WKAddress target);

/**
 * Get the transfer attribtue at `index` from the attributes that are relevent to 'wallet' when
 * a transfer uses the `target` address.  The value of `index` must be [0, count).
 */
extern WKTransferAttribute
wkWalletGetTransferAttributeAt (WKWallet wallet,
                                    WKAddress target,
                                    size_t index);

/**
 * Validate `attribute` for `wallet` and fill `validates` with the result.  If `validates` is
 * WK_TRUE then the return value described the validation error; otherwise the return
 * value is undefined
 */
extern WKTransferAttributeValidationError
wkWalletValidateTransferAttribute (WKWallet wallet,
                                       OwnershipKept WKTransferAttribute attribute,
                                       WKBoolean *validates);

/**
 * Find the transfer attribute that is relevent to `wallet` when a transfer uses the `target`
 * address and that contain an attribute with `key`.
 */
extern WKTransferAttribute
wkWalletGetTransferAttributeForKey (WKWallet wallet,
                                        WKAddress target,
                                        const char *key);

/**
 * Validate `attributes` for `wallet` and fill `validates` with the result.  If `validates` is
 * WK_TRUE then the return value described the validation error; otherwise the return
 * value is undefined.  The first attribute that is invalid will be returned; subsequent
 * attributes will not be validated.
 */
extern WKTransferAttributeValidationError
wkWalletValidateTransferAttributes (WKWallet wallet,
                                        size_t attributesCount,
                                        OwnershipKept WKTransferAttribute *attributes,
                                        WKBoolean *validates);

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
extern WKTransfer
wkWalletCreateTransfer (WKWallet wallet,
                            WKAddress target,
                            WKAmount amount,
                            WKFeeBasis estimatedFeeBasis,
                            size_t attributesCount,
                            OwnershipKept WKTransferAttribute *attributes);

extern WKTransfer
wkWalletCreateTransferForPaymentProtocolRequest (WKWallet wallet,
                                                     WKPaymentProtocolRequest request,
                                                     WKFeeBasis estimatedFeeBasis);

extern WKTransfer
wkWalletCreateTransferMultiple (WKWallet wallet,
                                    size_t outputsCount,
                                    WKTransferOutput *outputs,
                                    WKFeeBasis estimatedFeeBasis);

/**
 * Check of two wallets are equal.
 */
extern WKBoolean
wkWalletEqual (WKWallet w1, WKWallet w2);

DECLARE_WK_GIVE_TAKE (WKWallet, wkWallet);

#ifdef __cplusplus
}
#endif

#endif /* WKWallet_h */
