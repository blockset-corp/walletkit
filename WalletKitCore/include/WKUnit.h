//
//  WKUnit.h
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKUnit_h
#define WKUnit_h

#include "WKBase.h"
#include "WKCurrency.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A Unit is a 'unit of measure' for a Currency.  For example, Bitcoin has units of 'Satoshi' and
 * 'BTC'; Ethereum has units of 'Ether', 'GWEI' and 'WEI'.
 */
typedef struct WKUnitRecord *WKUnit;

/**
 * Create a unit for `currency` as a 'base unit'.  A base unit is the unit with a integer value
 * which is essentially the highest resolution measure for the currency.  For Bitcoin this is
 * 'Satoshi'; for Ethereum this is 'WEI'.
 */
private_extern WKUnit
wkUnitCreateAsBase (WKCurrency currency,
                        const char *code,
                        const char *name,
                        const char *symbol);

/**
 * Create a unit for `currency` as a 'derived unit'.  A derived unit is a unit with a base 10
 *  number of digits as a noffset from a 'base unit'.  For Bitcoin a derived unit would be
 *  'BTC' as 8 digits (10^8) from 'Satoshi'.  For Ethereum, 'Ether' is 18 digits offset from
 *  'WEI'.
 */
private_extern WKUnit
wkUnitCreate (WKCurrency currency,
                  const char *code,
                  const char *name,
                  const char *symbol,
                  WKUnit baseUnit,
                  uint8_t powerOffset);


/**
 * Get the unit's unique identifier.  This should be considered unique only among the
 * currency's units.
 */
extern const char *
wkUnitGetUids(WKUnit unit);

/**
 * Get the unit's name
 */
extern const char *
wkUnitGetName (WKUnit unit);

/**
 * Get the unit's symbol
 */
extern const char *
wkUnitGetSymbol (WKUnit unit);

/**
 * Returns the unit's currency
 *
 * @param unit the Unit
 *
 * @return The currency w/ an incremented reference count (aka 'taken')
 */
extern WKCurrency
wkUnitGetCurrency (WKUnit unit);

/**
 * Check if unit has `currency`
 */
extern WKBoolean
wkUnitHasCurrency (WKUnit unit,
                       WKCurrency currency);

/**
 * Returns the unit's base unit.  If unit is itself the base unit then unit is returned
 *
 * @param unit The unit
 *
 * @return the base unit w/ an incremented reference count (aka 'taken')
 */
extern WKUnit
wkUnitGetBaseUnit (WKUnit unit);

/**
 * Get the unit's decimal offset.  This will be 0 for a base unit.
 */
extern uint8_t
wkUnitGetBaseDecimalOffset (WKUnit unit);

/**
 * Check if two units are compatible; they are compatible if the units share a currency.
 */
extern WKBoolean
wkUnitIsCompatible (WKUnit u1,
                        WKUnit u2);

/**
 * Check if two units are identical
 */
extern WKBoolean
wkUnitIsIdentical (WKUnit u1,
                       WKUnit u2);

DECLARE_WK_GIVE_TAKE (WKUnit, wkUnit);

#ifdef __cplusplus
}
#endif

#endif /* WKUnit_h */
