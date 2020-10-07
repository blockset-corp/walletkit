//
//  BRCryptoSupportBTC.c
//  Core
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRCryptoBTC.h"
#include "crypto/BRCryptoHashP.h"
#include "crypto/BRCryptoAmountP.h"
#include "ethereum/util/BRUtilMath.h"


private_extern BRCryptoHash
cryptoHashCreateAsBTC (UInt256 btc) {
    UInt256 revBtc = UInt256Reverse (btc);

    return cryptoHashCreateInternal (btc.u32[0],
                                     sizeof (revBtc.u8),
                                     revBtc.u8,
                                     CRYPTO_NETWORK_TYPE_BTC);
}

