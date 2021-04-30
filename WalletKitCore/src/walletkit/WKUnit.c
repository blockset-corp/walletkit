//
//  WKUnit.c
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "WKUnit.h"
#include "support/BRArray.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct WKUnitRecord {
    WKCurrency currency;
    char *uids;
    char *name;
    char *symbol;
    WKUnit base;
    uint8_t decimals;
    WKRef ref;
};

IMPLEMENT_WK_GIVE_TAKE (WKUnit, wkUnit)

static WKUnit
wkUnitCreateInternal (WKCurrency currency,
                          const char *code,
                          const char *name,
                          const char *symbol) {
    WKUnit unit = malloc (sizeof (struct WKUnitRecord));

    unit->currency = wkCurrencyTake (currency);
    unit->name   = strdup (name);
    unit->symbol = strdup (symbol);
    unit->uids   = malloc (strlen (wkCurrencyGetUids(currency)) + 1 + strlen(code) + 1);
    sprintf (unit->uids, "%s:%s", wkCurrencyGetUids(currency), code);

    unit->ref = WK_REF_ASSIGN (wkUnitRelease);
    return unit;
}

extern WKUnit
wkUnitCreateAsBase (WKCurrency currency,
                        const char *code,
                        const char *name,
                        const char *symbol) {
    WKUnit unit = wkUnitCreateInternal (currency, code, name, symbol);

    unit->base = NULL;
    unit->decimals = 0;

    return unit;
}

extern WKUnit
wkUnitCreate (WKCurrency currency,
                  const char *code,
                  const char *name,
                  const char *symbol,
                  WKUnit baseUnit,
                  uint8_t powerOffset) {
    assert (NULL != baseUnit);
    WKUnit unit = wkUnitCreateInternal (currency, code, name, symbol);

    unit->base = wkUnitTake (baseUnit);
    unit->decimals = powerOffset;

    return unit;
}

static void
wkUnitRelease (WKUnit unit) {
    if (NULL != unit->base) wkUnitGive (unit->base);
    wkCurrencyGive (unit->currency);
    free (unit->uids);
    free (unit->name);
    free (unit->symbol);

    memset (unit, 0, sizeof(*unit));
    free (unit);
}

private_extern BRArrayOf(WKUnit)
wkUnitTakeAll (BRArrayOf(WKUnit) units) {
    if (NULL != units)
        for (size_t index = 0; index < array_count (units); index++)
            wkUnitTake(units[index]);
    return units;
}

private_extern BRArrayOf(WKUnit)
wkUnitGiveAll (BRArrayOf(WKUnit) units) {
    for (size_t index = 0; index < array_count (units); index++)
        wkUnitGive(units[index]);
    return units;
}

extern const char *
wkUnitGetUids(WKUnit unit) {
    return unit->uids;
}

extern const char *
wkUnitGetName (WKUnit unit) {
    return unit->name;
}

extern const char *
wkUnitGetSymbol (WKUnit unit) {
    return unit->symbol;
}

extern WKCurrency
wkUnitGetCurrency (WKUnit unit) {
    return wkCurrencyTake (unit->currency);
}

extern WKBoolean
wkUnitHasCurrency (WKUnit unit,
                       WKCurrency currency) {
    return AS_WK_BOOLEAN (unit->currency == currency);
}
extern WKUnit
wkUnitGetBaseUnit (WKUnit unit) {
    return wkUnitTake (NULL == unit->base ? unit : unit->base);
}

extern uint8_t
wkUnitGetBaseDecimalOffset (WKUnit unit) {
    return NULL == unit->base ? 0 : unit->decimals;
}

extern WKBoolean
wkUnitIsCompatible (WKUnit u1,
                        WKUnit u2) {
    return wkCurrencyIsIdentical (u1->currency, u2->currency);
}

extern WKBoolean
wkUnitIsIdentical (WKUnit u1,
                       WKUnit u2) {
    return AS_WK_BOOLEAN (u1 == u2
                              || u1->uids == u2->uids
                              || 0 == strcmp (u1->uids, u2->uids));
}
