//
//  BRHederaToken
//  WalletKitCore Hedera
//
//  Created by Carl Cherry on Sep. 23, 2021.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BR_Hedera_Token_H
#define BR_Hedera_Token_H

#include "BRHederaAddress.h"
#include "support/BRSet.h"

#ifdef __cplusplus
extern "C" {
#endif

// A Fungible token on Hedera
typedef struct BRHederaTokenRecord *BRHederaToken;

extern BRHederaToken
hederaTokenCreate (const char *address,
                const char *symbol,
                const char *name,
                const char *description,
                unsigned int decimals);

extern void
hederaTokenUpdate (BRHederaToken token,
                const char *symbol,
                const char *name,
                const char *description,
                unsigned int decimals);

extern BRHederaToken
hederaTokenClone(BRHederaToken token);

extern void
hederaTokenFree(BRHederaToken token);

extern BRHederaAddress
hederaTokenGetAddress(BRHederaToken token);

extern const char *
hederaTokenGetAddressAsString (BRHederaToken token);

extern const char *
hederaTokenGetSymbol (BRHederaToken token);

extern const char *
hederaTokenGetName (BRHederaToken token);

extern const char *
hederaTokenGetDescription(BRHederaToken token);

extern unsigned int
hederaTokenGetDecimals (BRHederaToken token);

extern BRSetOf(BRHederaToken) hederaTokenSetCreate(size_t capacity);

#ifdef __cplusplus
}
#endif

#endif
