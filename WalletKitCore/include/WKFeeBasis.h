//
//  WKFeeBasis.h
//  WalletKitCore
//
//  Created by Ed Gamble on 5/15/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKFeeBasis_h
#define WKFeeBasis_h

#include "WKBase.h"
#include "WKCurrency.h"
#include "WKAmount.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A FeeBasis is the basis for a transaction's fee.
 *
 * @discussion Different networks compute fees in different ways; it is in part one of the ways
 * that crptocurrency blockchains distinguish themselves.  For example, a BTC transaction's fee
 * is the User specified 'feePerKb' and the transaction's size on 'Kb'.  For an ETH transaction the
 * fee is the User-specified 'gasPrice' times the 'gas', which is a representation of the
 * computational cost to include the transaction in the Ethereum blockchain.
 *
 * At the present, for the known WalletKit blockchains, fees can be represented as
 * `pricePerCostFactor * costFactor`.
 */
typedef struct WKFeeBasisRecord *WKFeeBasis;

/**
 * Get the feeBasis' pricePerCostFactor
 */
extern WKAmount
wkFeeBasisGetPricePerCostFactor (WKFeeBasis feeBasis);

/**
 * Get the feeBasis' costFactor
 */
extern double
wkFeeBasisGetCostFactor (WKFeeBasis feeBasis);

/**
 * Get the feeBasis' fee, in the unit of the pricePerCostFactor
 */
extern WKAmount
wkFeeBasisGetFee (WKFeeBasis feeBasis);

/**
 * Check if two feeBasises are identical
 */
extern WKBoolean
wkFeeBasisIsEqual (WKFeeBasis feeBasis1,
                       WKFeeBasis feeBasis2);

DECLARE_WK_GIVE_TAKE (WKFeeBasis, wkFeeBasis);

#ifdef __cplusplus
}
#endif

#endif /* WKFeeBasis_h */
