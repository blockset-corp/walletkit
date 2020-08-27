//
//  BRCryptoStatusP.h
//  BRCore
//
//  Created by Ed Gamble on 12/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoStatusP_h
#define BRCryptoStatusP_h

#include "BRCryptoStatus.h"
#ifdef REFACTOR
#include "ethereum/BREthereum.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef REFACTOR
private_extern BRCryptoStatus
cryptoStatusFromETH (BREthereumStatus status);

private_extern BREthereumStatus
cryptoStatusAsETH (BRCryptoStatus status);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoStatusP_h */
