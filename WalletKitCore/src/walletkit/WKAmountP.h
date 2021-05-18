//
//  WKAmountP.h
//  WalletKitCore
//
//  Created by Ed Gamble on 12/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKAmountP_h
#define WKAmountP_h

#include "WKAmount.h"
#include "support/BRInt.h"

#ifdef __cplusplus
extern "C" {
#endif

private_extern WKAmount
wkAmountCreate (WKUnit unit,
                    WKBoolean isNegative,
                    UInt256 value);

private_extern UInt256
wkAmountGetValue (WKAmount amount);

#ifdef __cplusplus
}
#endif

#endif /* WKAmountP_h */
