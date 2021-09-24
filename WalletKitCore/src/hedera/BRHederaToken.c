//
//  BRHederaToken
//  WalletKitCore Hedera
//
//  Created by Carl Cherry on Sep. 23, 2021.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <string.h>
#include "support/BRArray.h"
#include "support/BRSet.h"
#include "BRHederaToken.h"

//
// Token
//
struct BRHederaTokenRecord {

    /**
     * The address for the Hedera token (an Hedera account)
     */
    BRHederaAddress address;
    char * addressAsString;

    /**
     * The (exchange) symbol - "BRD"
     */
    char *symbol;

    /**
     * The name - "Bread Token"
     */
    char *name;

    /**
     * The description - "The Bread Token ..."
     */
    char *description;

    /**
     * The maximum decimals
     */
    unsigned int decimals;

    /**
     * True(1) if allocated statically
     */
    int staticallyAllocated;
};

extern BRHederaToken
hederaTokenCreate (const char *address,
                const char *symbol,
                const char *name,
                const char *description,
                unsigned int decimals)
{
    BRHederaToken token = malloc (sizeof(struct BRHederaTokenRecord));

    token->addressAsString = strdup(address);
    token->address     = hederaAddressCreateFromString(address, true);
    token->symbol      = strdup (symbol);
    token->name        = strdup (name);
    token->description = strdup (description);
    token->decimals    = decimals;
    return token;
}

extern BRHederaToken
hederaTokenClone(BRHederaToken token)
{
    if (token) {
        BRHederaToken newToken = malloc (sizeof(struct BRHederaTokenRecord));
        newToken->addressAsString = strdup(token->addressAsString);
        newToken->address     = hederaAddressClone(token->address);
        newToken->symbol      = strdup (token->symbol);
        newToken->name        = strdup (token->name);
        newToken->description = strdup (token->description);
        newToken->decimals    = token->decimals;
        return newToken;
    } else {
        return NULL;
    }
}

extern void
hederaTokenFree(BRHederaToken token)
{
    assert(token);
    hederaAddressFree(token->address);
    free(token->addressAsString);
    free(token->name);
    free(token->description);
    free(token->symbol);
    free(token);
}

extern const char *
hederaTokenGetAddressAsString (BRHederaToken token)
{
    return token->addressAsString;
}

extern const char *
hederaTokenGetSymbol (BRHederaToken token)
{
    return token->symbol;
}

extern const char *
hederaTokenGetName (BRHederaToken token)
{
    return token->name;
}

extern const char *
hederaTokenGetDescription(BRHederaToken token)
{
    return token->description;
}

extern unsigned int
hederaTokenGetDecimals (BRHederaToken token)
{
    return token->decimals;
}

extern BRHederaAddress
hederaTokenGetAddress(BRHederaToken token)
{
    return token->address;
}

extern void
hederaTokenUpdate (BRHederaToken token,
                const char *symbol,
                const char *name,
                const char *description,
                unsigned int decimals) {

    if (0 != strcasecmp (symbol     , token->symbol     )) { free (token->symbol     ); token->symbol      = strdup (symbol     ); }
    if (0 != strcasecmp (name       , token->name       )) { free (token->name       ); token->name        = strdup (name       ); }
    if (0 != strcasecmp (description, token->description)) { free (token->description); token->description = strdup (description); }
    token->decimals = decimals;
}


static inline size_t
tokenHashValue (const void *t)
{
    return hederaAddressHashValue(((BRHederaToken)t)->address);
}

static inline int
tokenHashEqual (const void *t1, const void *t2) {
    return t1 == t2 || hederaAddressHashEqual (((BRHederaToken)t1)->address,
                                            ((BRHederaToken)t2)->address);
}

extern BRSetOf(BRHederaToken)
hederaTokenSetCreate (size_t capacity)
{
    return BRSetNew (tokenHashValue, tokenHashEqual, capacity);
}
