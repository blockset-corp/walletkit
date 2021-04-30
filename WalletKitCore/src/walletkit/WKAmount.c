//
//  WKAmount.c
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "WKAmount.h"

#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "WKAmount.h"

#include "support/BRInt.h"
#include "support/util/BRUtilMath.h"

struct WKAmountRecord {
    WKUnit unit;
    WKBoolean isNegative;
    UInt256 value;
    WKRef ref;
};

IMPLEMENT_WK_GIVE_TAKE (WKAmount, wkAmount);

static WKAmount
wkAmountCreateInternal (WKUnit unit,
                            WKBoolean isNegative,
                            UInt256 value,
                            int takeUnit) {
    WKAmount amount = malloc (sizeof (struct WKAmountRecord));

    amount->unit = takeUnit ? wkUnitTake (unit) : unit;
    amount->isNegative = isNegative;
    amount->value = value;
    amount->ref = WK_REF_ASSIGN (wkAmountRelease);

    return amount;
}

private_extern WKAmount
wkAmountCreate (WKUnit unit,
                    WKBoolean isNegative,
                    UInt256 value) {
    // Given a UInt256 -> conversion to unit already complete.
    return wkAmountCreateInternal (unit, isNegative, value, 1);
}

static WKAmount
wkAmountCreateUInt256 (UInt256 v,
                           WKBoolean isNegative,
                           WKUnit unit) {
    int powOverflow = 0, mulOverflow = 0;

    uint8_t decimals = wkUnitGetBaseDecimalOffset (unit);

    if (0 != decimals)
        v = uint256Mul_Overflow (v, uint256CreatePower(decimals, &powOverflow), &mulOverflow);

    return (powOverflow || mulOverflow ? NULL
            : wkAmountCreateInternal (unit, isNegative, v, 1));
}

extern WKAmount
wkAmountCreateDouble (double value,
                          WKUnit unit) {
    int overflow;
    UInt256 valueAsUInt256 = uint256CreateDouble (value, wkUnitGetBaseDecimalOffset (unit), &overflow);

    return (overflow
            ? NULL
            : wkAmountCreateInternal (unit,
                                          (value < 0.0 ? WK_TRUE : WK_FALSE),
                                          valueAsUInt256,
                                          1));
}

extern WKAmount
wkAmountCreateInteger (int64_t value,
                           WKUnit unit) {

    UInt256 v = uint256Create ((uint64_t) (value < 0 ? -value : value));
    return wkAmountCreateUInt256 (v, (value < 0 ? WK_TRUE : WK_FALSE), unit);
}

extern WKAmount
wkAmountCreateString (const char *value,
                          WKBoolean isNegative,
                          WKUnit unit) {
    UInt256 v;
    BRCoreParseStatus status;

    // Try to parse as an integer
    v = uint256CreateParse (value, 0, &status);

    // if that fails, try to parse as a decimal
    if (CORE_PARSE_OK != status) {
        v = uint256CreateParseDecimal (value, wkUnitGetBaseDecimalOffset (unit), &status);
        unit = wkUnitGetBaseUnit(unit);
        wkUnitGive(unit);
    }

    return (CORE_PARSE_OK != status ? NULL : wkAmountCreateUInt256 (v, isNegative, unit));
}

static void
wkAmountRelease (WKAmount amount) {
    wkUnitGive (amount->unit);

    memset (amount, 0, sizeof(*amount));
    free (amount);
}

extern WKUnit
wkAmountGetUnit (WKAmount amount) {
    return wkUnitTake (amount->unit);
}

extern WKCurrency
wkAmountGetCurrency (WKAmount amount) {
    return wkUnitGetCurrency(amount->unit);
}

extern WKBoolean
wkAmountHasCurrency (WKAmount amount,
                         WKCurrency currency) {
    return wkUnitHasCurrency(amount->unit, currency);
}

extern WKBoolean
wkAmountIsNegative (WKAmount amount) {
    return amount->isNegative;
}

extern WKBoolean
wkAmountIsCompatible (WKAmount a1,
                          WKAmount a2) {
    return wkUnitIsCompatible (a1->unit, a2->unit);
}

extern WKBoolean
wkAmountIsZero (WKAmount amount) {
    return AS_WK_BOOLEAN (uint256EQL (amount->value, UINT256_ZERO));
}

static WKComparison
wkCompareUInt256 (UInt256 v1, UInt256 v2) {
    switch (uint256Compare (v1, v2)) {
        case -1: return WK_COMPARE_LT;
        case  0: return WK_COMPARE_EQ;
        case +1: return WK_COMPARE_GT;
        default: assert (0); return WK_COMPARE_EQ;
    }
}

extern WKComparison
wkAmountCompare (WKAmount a1,
                     WKAmount a2) {
    assert (WK_TRUE == wkAmountIsCompatible(a1, a2));

    if (WK_TRUE == a1->isNegative && WK_TRUE != a2->isNegative)
        return WK_COMPARE_LT;
    else if (WK_TRUE != a1->isNegative && WK_TRUE == a2->isNegative)
        return WK_COMPARE_GT;
    else if (WK_TRUE == a1->isNegative && WK_TRUE == a2->isNegative)
        // both negative -> swap comparison
        return wkCompareUInt256 (a2->value, a1->value);
    else
        // both positive -> same comparison
        return wkCompareUInt256 (a1->value, a2->value);
}

extern WKAmount
wkAmountAdd (WKAmount a1,
                 WKAmount a2) {
    assert (WK_TRUE == wkAmountIsCompatible (a1, a2));

    int overflow = 0;
    int negative = 0;

    if (WK_TRUE == a1->isNegative && WK_TRUE != a2->isNegative) {
        // (-x) + y = (y - x)
        UInt256 value = uint256Sub_Negative (a2->value, a1->value, &negative);
        return wkAmountCreate (a1->unit, AS_WK_BOOLEAN(negative), value);
    }
    else if (WK_TRUE != a1->isNegative && WK_TRUE == a2->isNegative) {
        // x + (-y) = x - y
        UInt256 value = uint256Sub_Negative (a1->value, a2->value, &negative);
        return wkAmountCreate(a1->unit, AS_WK_BOOLEAN(negative), value);
    }
    else if (WK_TRUE == a1->isNegative && WK_TRUE == a2->isNegative) {
        // (-x) + (-y) = - (x + y)
        UInt256 value = uint256Add_Overflow (a2->value, a1->value, &overflow);
        return overflow ? NULL :  wkAmountCreate (a1->unit, WK_TRUE, value);
    }
    else {
        UInt256 value = uint256Add_Overflow (a1->value, a2->value, &overflow);
        return overflow ? NULL :  wkAmountCreate (a1->unit, WK_FALSE, value);
    }
}

extern WKAmount
wkAmountSub (WKAmount a1,
                 WKAmount a2) {
    assert (WK_TRUE == wkAmountIsCompatible (a1, a2));

    int overflow = 0;
    int negative = 0;

    if (WK_TRUE == a1->isNegative && WK_TRUE != a2->isNegative) {
        // (-x) - y = - (x + y)
        UInt256 value = uint256Add_Overflow (a1->value, a2->value, &overflow);
        return overflow ? NULL : wkAmountCreate (a1->unit, WK_TRUE, value);
    }
    else if (WK_TRUE != a1->isNegative && WK_TRUE == a2->isNegative) {
        // x - (-y) = x + y
        UInt256 value = uint256Add_Overflow (a1->value, a2->value, &overflow);
        return overflow ? NULL : wkAmountCreate(a1->unit, WK_FALSE, value);
    }
    else if (WK_TRUE == a1->isNegative && WK_TRUE == a2->isNegative) {
        // (-x) - (-y) = y - x
        UInt256 value = uint256Sub_Negative (a2->value, a1->value, &negative);
        return wkAmountCreate (a1->unit, AS_WK_BOOLEAN(negative), value);
    }
    else {
        UInt256 value = uint256Sub_Negative (a1->value, a2->value, &negative);
        return wkAmountCreate (a1->unit, AS_WK_BOOLEAN(negative), value);
    }
}

extern WKAmount
wkAmountNegate (WKAmount amount) {
    return wkAmountCreate (amount->unit,
                               WK_TRUE == amount->isNegative ? WK_FALSE : WK_TRUE,
                               amount->value);
}

extern WKAmount
wkAmountConvertToUnit (WKAmount amount,
                           WKUnit unit) {
    return (WK_TRUE != wkUnitIsCompatible (amount->unit, unit)
            ? NULL
            : wkAmountCreate (unit, amount->isNegative, amount->value));
}

extern double
wkAmountGetDouble (WKAmount amount,
                       WKUnit unit,
                       WKBoolean *overflow) {
    assert (NULL != overflow);
    long double power  = powl (10.0, wkUnitGetBaseDecimalOffset(unit));
    long double result = uint256CoerceDouble (amount->value, (int*) overflow) / power;
    return (double) (WK_TRUE == amount->isNegative ? -result : result);
}

extern uint64_t
wkAmountGetIntegerRaw (WKAmount amount,
                           WKBoolean *overflow) {
    *overflow = (0 != amount->value.u64 [3] ||
                 0 != amount->value.u64 [2] ||
                 0 != amount->value.u64 [1]);
    return *overflow ? 0 : amount->value.u64[0];
}

extern char *
wkAmountGetStringPrefaced (WKAmount amount,
                               int base,
                               const char *preface) {
    return uint256CoerceStringPrefaced (amount->value, base, preface);
}

extern UInt256
wkAmountGetValue (WKAmount amount) {
    return amount->value;
}
