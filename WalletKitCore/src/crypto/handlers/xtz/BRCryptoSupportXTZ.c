//
//  BRCryptoSupportXTZ.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoXTZ.h"
#include "tezos/BRTezosBase.h"
#include "crypto/BRCryptoHashP.h"
#include "crypto/BRCryptoAmountP.h"
#include "ethereum/util/BRUtilMath.h"

private_extern BRCryptoAmount
cryptoAmountCreateAsXTZ (BRCryptoUnit unit,
                         BRCryptoBoolean isNegative,
                         BRTezosUnitMutez value) {
    return cryptoAmountCreate (unit, isNegative, uint256Create (value));
}

// MARK: -

private_extern const char **
tezosGetTransactionAttributeKeys (int asRequired,
                                  size_t *count) {
    
    static size_t requiredCount = 0;
    static const char **requiredNames = NULL;
    
    static size_t optionalCount = 1;
    static const char *optionalNames[] = {
        FIELD_OPTION_DELEGATION_OP
    };
    
    if (asRequired) { *count = requiredCount; return requiredNames; }
    else {            *count = optionalCount; return optionalNames; }
}
