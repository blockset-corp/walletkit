//
//  WKAmount.h
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKAmount_h
#define WKAmount_h

#include "WKBase.h"
#include "WKCurrency.h"
#include "WKUnit.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * When comparing two amounts, the result can be less than, equal to or greater than.
 */
typedef enum {
    WK_COMPARE_LT,
    WK_COMPARE_EQ,
    WK_COMPARE_GT
} WKComparison;

/**
 * @brief The amount of an asset held or transfered on a network
 *
 * @discussion  An amount is fundamentally represented as a signed 256-bit number in a specfic
 * currency that will be printed using the amount's unit.
 */
typedef struct WKAmountRecord *WKAmount;

/**
 * Create an amount from `value` in `unit`.  If the amount is "2 Bitcoin" then, for example,
 * the amount can be created from either of {2.0, BTC} or {2e8, SAT}.
 *
 * @note The internal representation is always as a UInt256 in the currency's baseUnit (aka
 * the 'integer unit').  This allows easy arithmetic operations w/o conversions.  Conversions
 * to the base unit happens on creation, and conversion from the base unit happens on display.
 *
 * @param value
 * @param unit
 *
 * @return An amount
 */
extern WKAmount
wkAmountCreateDouble (double value,
                      WKUnit unit);

/**
 * Create an amount form `value` in `unit`.  See discusion on `wkAmountCreateDouble()`
 *
 * @param value
 * @param unit
 *
 *@return An Amount
 */
extern WKAmount
wkAmountCreateInteger (int64_t value,
                       WKUnit unit);

/**
 * Create an amount from `value` and `unit`. See discusion on `wkAmountCreateDouble()`
 *
 * @note there are some constraints on `const char *value` that need to be described.
 *
 * @param value
 * @param isNegative
 * @param unit
 *
 * @return An amount
 */
extern WKAmount
wkAmountCreateString (const char *value,
                      WKBoolean isNegative,
                      WKUnit unit);

/**
 * Returns the amount's unit.
 *
 * @param amount The amount
 *
 * @return The amount's unit, typically use for default display w/ an incremented reference
 *    count (aka 'taken')
 */
extern WKUnit
wkAmountGetUnit (WKAmount amount);

/**
 * Returns the amount's currency
 *
 * @param amount The amount
 *
 * @return The currency w/ an incremented reference count (aka 'taken')
 */
extern WKCurrency
wkAmountGetCurrency (WKAmount amount);

/**
 * Check if the amount is in the specified currenty
 */
extern WKBoolean
wkAmountHasCurrency (WKAmount amount,
                     WKCurrency currency);

/**
 * Check if the amount is negative
 */
extern WKBoolean
wkAmountIsNegative (WKAmount amount);

/**
 * Check of two amount's are compatible; they are compatible if they have the same currency and
 * thus can be added.
 *
 * @param a1
 * @param a2
 *
 * @return
 */
extern WKBoolean
wkAmountIsCompatible (WKAmount a1,
                      WKAmount a2);

/**
 * Check if the amount is zero
 */
extern WKBoolean
wkAmountIsZero (WKAmount amount);

/**
 * Compare two amounts
 */
extern WKComparison
wkAmountCompare (WKAmount a1,
                 WKAmount a2);

/**
 * Add two amounts
 */
extern WKAmount
wkAmountAdd (WKAmount a1,
             WKAmount a2);

/**
 * Subtract one amount from another
 */
extern WKAmount
wkAmountSub (WKAmount a1,
             WKAmount a2);

/**
 * Negate the amount
 */
extern WKAmount
wkAmountNegate (WKAmount amount);

/**
 * Convert `amount` into `unit`
 *
 * @param amount
 * @param unit
 *
 * @note: if `unit` is incompatible, then NULL is returned.
 *
 * @return An amount
 */
extern WKAmount
wkAmountConvertToUnit (WKAmount amount,
                       WKUnit unit);

/**
 * Return `amount` as a double in `unit`.  For example, if amount is "2 BTC" and unit is
 * SAT then the returned value will be 2e8; if amount is "2e6 SAT" and unit is "BTC" then the
 * return value will be 0.02.
 *
 * @param amount
 * @param unit
 * @param overflow
 *
 * @return A double
 */
extern double
wkAmountGetDouble (WKAmount amount,
                   WKUnit unit,
                   WKBoolean *overflow);

/**
 * Return `amount` as a UInt64 if the representation of `amount` in the base unit is less than
 * or equal to UINT64_MAX; otherwise set overflow.
 *
 * @param amount
 * @param overflow
 *
 * @return
 */
extern uint64_t
wkAmountGetIntegerRaw (WKAmount amount,
                       WKBoolean *overflow);

/**
 * Create a string representation of amount in the give base () with the provided prefix.  The
 * base must be one of 16, 10 or 2.
 */
extern char *
wkAmountGetStringPrefaced (WKAmount amount,
                           int base,
                           const char *preface);

DECLARE_WK_GIVE_TAKE (WKAmount, wkAmount);

#ifdef __cplusplus
}
#endif

#endif /* WKAmount_h */
