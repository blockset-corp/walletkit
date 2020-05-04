package com.breadwallet.core

import kotlinx.io.core.Closeable


/**
 * An amount of currency.
 *
 * This can be negative (as in, 'currency owed' rather then 'currency owned').
 * Supports basic arithmetic operations (addition, subtraction, comparison);
 * will assert on !isCompatible for mismatched currency.
 */
expect class Amount : Comparable<Amount>, Closeable {
    companion object {
        /** Create [Amount] from [double]. */
        public fun create(double: Double, unit: CUnit): Amount

        /** Create [Amount] from [long] */
        public fun create(long: Long, unit: CUnit): Amount

        /**
         * Parse [string] into an [Amount].
         *
         * The string has some limitations:
         *  * it cannot start with '-' or '+' (no sign character)
         *  * if it starts with '0x', it is interpreted as a 'hex string'
         *  * if it has a decimal point, it is interpreted as a 'decimal string'
         *  * otherwise, it is interpreted as an 'integer string'
         *
         * If it is a 'decimal string' and the string includes values after the decimal point, then
         * the number of values must be less than or equal to the unit's decimals.  For example a
         * string of "1.1" in BTC_SATOSHI won't parse as BTC_SATOSHI has 0 decimals (it is a base unit
         * and thus must be an integer).  Whereas, a string of "1.1" in BTC_BTC will parse as BTC_BTC
         * has 8 decimals and the string has but one.  ("0.123456789" won't parse as it has 9 digits
         * after the decimal; both "1." and "1.0" will parse.)
         *
         * Additionally, `string` cannot have any extraneous starting or ending characters.  Said
         * another way, `string` must be fully consumed.  Thus "10w" and "w10" and "1.1w" won't parse.
         *
         * @param string the string to parse
         * @param unit the string's unit
         * @param isNegative true if negative; false otherwise
         *
         * @return The `Amount` if the string can be parsed.
         */
        public fun create(
                string: String,
                unit: CUnit,
                isNegative: Boolean = false
        ): Amount?
    }

    /**
     * The (default) unit.
     *
     * Without this there is no reasonable implementation of [toString].
     * This property is only used in the [toString] function and to ascertain
     * the Amount's currency typically for consistency in [plus]/[minus] functions.
     */
    public val unit: CUnit

    /** The currency */
    public val currency: Currency
    public val isNegative: Boolean
    public val negate: Amount
    public val isZero: Boolean

    /**
     * Convert [Amount] into [Double] using [unit].
     *
     * - *Note*: This can introduce inaccuracy as `Double` precision is less than the possible
     * number of UInt256 decimals (78).  For example, an Amount of 123456789012345678.0 WEI
     * (approx 1.234e17) when converted will have a different double representation:
     *
     * ```
     * val a7 = Amount.create("123456789012345678.0", unitWei, false)
     * assertEquals(1.2345678901234568e17, a7?.asDouble(unitWei))
     * ```
     * (the final three digits of '678' are rounded to '680')
     */
    public fun asDouble(unit: CUnit): Double?

    /**
     * Convert [Amount] into [String] using [unit].
     *
     * - *Note*: This can introduce inaccuracy.  Use of the `formatter` *requires* that Amount be
     * converted to a Double and the limit in Double precision can compromise the UInt256 value.
     * For example, an Amount of 123456789012345678.0 WEI (approx 1.234e20) when converted will
     * have different string representation:
     *
     *
     * ```
     * val a7 = Amount.create("123456789012345678.0", unitWei, false)
     * assertEquals("123456789012345678", a7?.asString(10, ""))
     * assertEquals("wei123,456,789,012,346,000", a7?.asString(unitWei))
     * assertNotEquals("wei123,456,789,012,345,678", a7?.asString(unitWei))
     * ```
     * (the final 6 digits of '345,678' are rounded to '346,000')
     * TODO: No multiplatform number formatting APIs exists yet
     *   For now this method will only use the default
     *   format defined in the Swift/Java implementations
     */
    public fun asString(unit: CUnit): String?

    /**
     * Return a 'raw string' (as an integer in self's base unit) using `base` and `preface`.
     *
     * Caution is warranted: this is a string w/o any context (currency in a particular Unit).
     * Don't use this to subvert the other `asString(CUnit, ...)` function.  Should only be
     * used for 'partner' interfaces when their API requires a string value in the base unit.
     *
     * - *Note*: The amount's sign is utterly ignored.
     * - *Note*: Unless there is a 'preface', the result may have leading zeros.
     * - *Note*: For base 16, lowercased hex digits are returned.
     *
     * @param base the numeric base - one of {2, 10, 16}.  Defaults to 16
     * @param preface a string preface, defaults to '0x'
     *
     * @return the amount in the base unit as a 'raw string'
     */
    public fun asString(base: Int = 16, preface: String = "0x"): String?

    /**
     * Convert [Amount] from [CurrencyPair.baseUnit] to [CurrencyPair.baseUnit] formatted
     * as a string.
     */
    public fun asString(pair: CurrencyPair): String?

    public operator fun plus(that: Amount): Amount
    public operator fun minus(that: Amount): Amount

    public fun convert(unit: CUnit): Amount?
    public fun isCompatible(amount: Amount): Boolean
    public fun hasCurrency(currency: Currency): Boolean

    /**
     * Note that incompatible units may return 'false' for all comparisons.
     *
     * This violates the expectation that `lhs` and `rhs` satisfy one of: ==, >, and <.
     * *Caution*.
     */
    override operator fun compareTo(other: Amount): Int
    override fun equals(other: Any?): Boolean
    override fun hashCode(): Int
    override fun toString(): String
}
