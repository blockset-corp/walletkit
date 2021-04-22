//
//  BRCryptoFeeBasis.h
//  WalletKitCore
//
//  Created by Ed Gamble on 5/15/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoFeeBasis_h
#define BRCryptoFeeBasis_h

#include "BRCryptoBase.h"
#include "BRCryptoCurrency.h"
#include "BRCryptoAmount.h"

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
typedef struct BRCryptoFeeBasisRecord *BRCryptoFeeBasis;

/**
 * Get the feeBasis' pricePerCostFactor
 */
extern BRCryptoAmount
cryptoFeeBasisGetPricePerCostFactor (BRCryptoFeeBasis feeBasis);

/**
 * Get the feeBasis' costFactor
 */
extern double
cryptoFeeBasisGetCostFactor (BRCryptoFeeBasis feeBasis);

/**
 * Get the feeBasis' fee, in the unit of the pricePerCostFactor
 */
extern BRCryptoAmount
cryptoFeeBasisGetFee (BRCryptoFeeBasis feeBasis);

/**
 * Check if two feeBasises are identical
 */
extern BRCryptoBoolean
cryptoFeeBasisIsEqual (BRCryptoFeeBasis feeBasis1,
                       BRCryptoFeeBasis feeBasis2);

DECLARE_CRYPTO_GIVE_TAKE (BRCryptoFeeBasis, cryptoFeeBasis);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoFeeBasis_h */
