//
//  WKKeyP.h
//  WalletKitCore
//
//  Created by Ed Gamble on 12/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKKeyP_h
#define WKKeyP_h

#include "WKKey.h"
#include "support/BRKey.h"

#ifdef __cplusplus
extern "C" {
#endif

private_extern WKKey
wkKeyCreateFromKey (BRKey *key);

private_extern BRKey *
wkKeyGetCore (WKKey key);

#ifdef __cplusplus
}
#endif

#endif /* WKKeyP_h */
