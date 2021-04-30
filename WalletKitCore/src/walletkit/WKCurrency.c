//
//  WKCurrency.c
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "WKCurrency.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct WKCurrencyRecord {
    char *uids;
    char *name;
    char *code;
    char *type;
    char *issuer;
    WKRef ref;
};

IMPLEMENT_WK_GIVE_TAKE (WKCurrency, wkCurrency)

extern WKCurrency
wkCurrencyCreate (const char *uids,
                      const char *name,
                      const char *code,
                      const char *type,
                      const char *issuer) {
    WKCurrency currency = malloc (sizeof (struct WKCurrencyRecord));

    currency->uids = strdup (uids);
    currency->name = strdup (name);
    currency->code = strdup (code);
    currency->type = strdup (type);
    currency->issuer = (NULL == issuer ? NULL : strdup (issuer));
    currency->ref  = WK_REF_ASSIGN (wkCurrencyRelease);

    return currency;
}

static void
wkCurrencyRelease (WKCurrency currency) {
    free (currency->type);
    free (currency->code);
    free (currency->name);
    free (currency->uids);
    if (NULL != currency->issuer) free (currency->issuer);

    memset (currency, 0, sizeof(*currency));
    free (currency);
}

extern const char *
wkCurrencyGetUids (WKCurrency currency) {
    return currency->uids;
}

extern const char *
wkCurrencyGetName (WKCurrency currency) {
    return currency->name;
}

extern const char *
wkCurrencyGetCode (WKCurrency currency) {
    return currency->code;
}

extern const char *
wkCurrencyGetType (WKCurrency currency) {
    return currency->type;
}

extern const char *
wkCurrencyGetIssuer (WKCurrency currency) {
    return currency->issuer;
}

extern bool
wkCurrencyHasUids (WKCurrency currency,
                       const char *uids) {
    return (NULL != uids
            && (uids == currency->uids
                || 0 == strcmp (uids, currency->uids)));
}
// total supply
// initial supply

// units (aka demoninations)

extern WKBoolean
wkCurrencyIsIdentical (WKCurrency c1,
                           WKCurrency c2) {
    return AS_WK_BOOLEAN (c1 == c2
                              || c1->uids == c2->uids
                              || 0 == strcmp (c1->uids, c2->uids));
}
