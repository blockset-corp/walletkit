//
//  WKSupportBTC.c
//  WalletKitCore
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "WKBTC.h"
#include "walletkit/WKHashP.h"
#include "walletkit/WKAmountP.h"
#include "support/util/BRUtilMath.h"


private_extern WKHash
wkHashCreateAsBTC (UInt256 btc) {
    UInt256 revBtc = UInt256Reverse (btc);

    return wkHashCreateInternal (btc.u32[0],
                                     sizeof (revBtc.u8),
                                     revBtc.u8,
                                     WK_NETWORK_TYPE_BTC);
}

