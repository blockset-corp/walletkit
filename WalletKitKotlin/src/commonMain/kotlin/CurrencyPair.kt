package com.breadwallet.core

/**
 * "A currency pair is the quotation of the relative value of a currency unit against the unit of
 * another currency in the foreign exchange market.
 *
 * The currency that is used as the reference is
 * called the counter currency, quote currency or currency and the currency that is quoted in
 * relation is called the base currency or transaction currency.
 *
 * "The quotation EUR/USD 1.2500 means that one euro is exchanged for 1.2500 US dollars. Here, EUR
 * is the base currency and USD is the quote currency(counter currency)."
 *
 * Ref: [https://en.wikipedia.org/wiki/Currency_pair](https://en.wikipedia.org/wiki/Currency_pair)
 *
 * Thus BTC/USD=1000 means that one BTC is changed for $1,000.  Here, BTC is the base currency
 * and USD is the quote currency.  You would create such an exchange with:
 *
 *    let BTC_USD_Pair = CurrencyPair (baseUnit:  Bitcoin.Units.BTC,
 *                                     quoteUnit: Fiat.USD.Dollar,
 *                                     exchangeRate: 1000.0)
 *
 * and then use it to find the value of 2 BTC with:
 *
 *    BTC_USD_Pair.exchange (asBase: Amount (value: 2.0, unit: Bitcoin.Units.BTC))
 *
 * which would return: $2,000  (as Amount of 2000.0 in Fiat.USD.Dollar)
 */
data class CurrencyPair(
        /** In EUR/USD=1.2500, the `baseCurrency` is EUR. */
        public val baseUnit: CUnit,
        /** In EUR/USD=1.250, the `quoteCurrency` is USD. */
        public val quoteUnit: CUnit,
        /**
         * In EUR/USD=1.2500, the `exchangeRate` is 1.2500 which represents
         * the number of USD that one EUR can be exchanged for.
         */
        public val exchangeRate: Double
) {

    /**
     * Apply `self` CurrencyPair to convert `asBase` (in `baseCurrency`) to `quoteCurrency`.
     * This is essentially `asBase * exchangeRate`
     *
     * @param amountAsBase The amount of `baseCurrency`
     *
     * @return the amount as `quoteCurrency`
     */
    public fun exchangeAsBase(amountAsBase: Amount): Amount? {
        return amountAsBase.asDouble(baseUnit)
                ?.let { Amount.create(it * exchangeRate, quoteUnit) }
    }

    /**
     * Apply `self` CurrencyPair to convert `asQuote` (in `quoteCurrency`) to `baseCurrency`.
     * This is essentially `asQuote / exchangeRate`.
     *
     * @param amountAsQuote the amount of `quoteCurrency`
     *
     * @return the amount as `baseCurrency`
     */
    public fun exchangeAsQuote(amountAsQuote: Amount): Amount? {
        return amountAsQuote.asDouble(quoteUnit)
                ?.let { Amount.create(it / exchangeRate, baseUnit) }
    }

    override fun toString(): String =
            "${baseUnit.name}/${quoteUnit.name}=$exchangeRate"
}
