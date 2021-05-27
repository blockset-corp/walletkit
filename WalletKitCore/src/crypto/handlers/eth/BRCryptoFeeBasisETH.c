//
//  BRCryptoFeeBasisETH.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-09-04.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoETH.h"
#include "crypto/BRCryptoFeeBasisP.h"
#include "crypto/BRCryptoAmountP.h"

static BRCryptoFeeBasisETH
cryptoFeeBasisCoerce (BRCryptoFeeBasis feeBasis) {
    assert (CRYPTO_NETWORK_TYPE_ETH == feeBasis->type);
    return (BRCryptoFeeBasisETH) feeBasis;
}

typedef struct {
    BREthereumFeeBasis ethFeeBasis;
} BRCryptoFeeBasisCreateContextETH;

static void
cryptoFeeBasisCreateCallbackETH (BRCryptoFeeBasisCreateContext context,
                                 BRCryptoFeeBasis feeBasis) {
    BRCryptoFeeBasisCreateContextETH *contextETH = (BRCryptoFeeBasisCreateContextETH*) context;
    BRCryptoFeeBasisETH feeBasisETH = cryptoFeeBasisCoerce (feeBasis);
    
    feeBasisETH->ethFeeBasis = contextETH->ethFeeBasis;
}

private_extern BRCryptoFeeBasis
cryptoFeeBasisCreateAsETH (BRCryptoUnit unit,
                           BREthereumFeeBasis ethFeeBasis) {
    BRCryptoFeeBasisCreateContextETH contextETH = {
        ethFeeBasis
    };
    
    return cryptoFeeBasisAllocAndInit (sizeof (struct BRCryptoFeeBasisETHRecord),
                                       CRYPTO_NETWORK_TYPE_ETH,
                                       unit,
                                       &contextETH,
                                       cryptoFeeBasisCreateCallbackETH);
}

private_extern BREthereumFeeBasis
cryptoFeeBasisAsETH (BRCryptoFeeBasis feeBasis) {
    BRCryptoFeeBasisETH ethFeeBasis = cryptoFeeBasisCoerce (feeBasis);
    return ethFeeBasis->ethFeeBasis;
}

static void
cryptoFeeBasisReleaseETH (BRCryptoFeeBasis feeBasis) {
}

static double
cryptoFeeBasisGetCostFactorETH (BRCryptoFeeBasis feeBasis) {
    BRCryptoFeeBasisETH ethFeeBasis = cryptoFeeBasisCoerce (feeBasis);
    return ethFeeBasis->ethFeeBasis.u.gas.limit.amountOfGas;
}

static BRCryptoAmount
cryptoFeeBasisGetPricePerCostFactorETH (BRCryptoFeeBasis feeBasis) {
    BREthereumFeeBasis ethFeeBasis = cryptoFeeBasisCoerce (feeBasis)->ethFeeBasis;
    return cryptoAmountCreate (feeBasis->unit,
                               CRYPTO_FALSE,
                               ethFeeBasis.u.gas.price.etherPerGas.valueInWEI);
}

static BRCryptoAmount
cryptoFeeBasisGetFeeETH (BRCryptoFeeBasis feeBasis) {
    BREthereumFeeBasis ethFeeBasis = cryptoFeeBasisCoerce (feeBasis)->ethFeeBasis;
    UInt256 gasPrice = ethFeeBasis.u.gas.price.etherPerGas.valueInWEI;
    double  gasAmount = cryptoFeeBasisGetCostFactor (feeBasis);
    
    int overflow = 0, negative = 0;
    double rem;
    
    UInt256 value = uint256Mul_Double (gasPrice, gasAmount, &overflow, &negative, &rem);
    
    return (overflow
            ? NULL
            : cryptoAmountCreate (feeBasis->unit, CRYPTO_FALSE, value));
}

static BRCryptoBoolean
cryptoFeeBasisIsEqualETH (BRCryptoFeeBasis feeBasis1, BRCryptoFeeBasis feeBasis2) {
    BRCryptoFeeBasisETH fb1 = cryptoFeeBasisCoerce (feeBasis1);
    BRCryptoFeeBasisETH fb2 = cryptoFeeBasisCoerce (feeBasis2);

    return AS_CRYPTO_BOOLEAN (ETHEREUM_BOOLEAN_TRUE == ethFeeBasisEqual (&fb1->ethFeeBasis, &fb2->ethFeeBasis));
}

// MARK: - Handlers

BRCryptoFeeBasisHandlers cryptoFeeBasisHandlersETH = {
    cryptoFeeBasisReleaseETH,
    cryptoFeeBasisGetCostFactorETH,
    cryptoFeeBasisGetPricePerCostFactorETH,
    cryptoFeeBasisGetFeeETH,
    cryptoFeeBasisIsEqualETH
};
