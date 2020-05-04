package com.breadwallet.core

import brcrypto.*
import kotlinx.cinterop.*
import kotlinx.io.core.Closeable
import platform.posix.size_tVar


fun Boolean.toCryptoBoolean(): UInt =
        if (this) CRYPTO_TRUE else CRYPTO_FALSE

actual class Network(
        core: BRCryptoNetwork,
        take: Boolean
) : Closeable {

    internal val core: BRCryptoNetwork =
            if (take) checkNotNull(cryptoNetworkTake(core))
            else core

    internal actual val uids: String =
            checkNotNull(cryptoNetworkGetUids(core)).toKStringFromUtf8()

    actual val name: String =
            checkNotNull(cryptoNetworkGetName(core)).toKStringFromUtf8()

    actual val type: NetworkType
        get() = NetworkType.fromCoreInt(cryptoNetworkGetCanonicalType(core).value.toInt())

    actual val isMainnet: Boolean =
            CRYPTO_TRUE == cryptoNetworkIsMainnet(core)

    actual var height: ULong
        get() = cryptoNetworkGetHeight(core)
        internal set(value) {
            cryptoNetworkSetHeight(core, value)
        }

    actual var fees: List<NetworkFee>
        get() = memScoped {
            val count = alloc<BRCryptoCountVar>()
            val cryptoFees = checkNotNull(cryptoNetworkGetNetworkFees(core, count.ptr))

            List(count.value.toInt()) { i ->
                NetworkFee(cryptoFees[i]!!, false)
            }
        }
        set(value) {
            require(value.isNotEmpty())
            val feeValues = value.map(NetworkFee::core).toCValues()

            cryptoNetworkSetNetworkFees(core, feeValues, feeValues.size.toULong())
        }

    actual val minimumFee: NetworkFee
        get() = checkNotNull(fees.maxByOrNull(NetworkFee::timeIntervalInMilliseconds))

    actual val confirmationsUntilFinal: UInt
        get() = cryptoNetworkGetConfirmationsUntilFinal(core)

    actual fun createPeer(address: String, port: UShort, publicKey: String?): NetworkPeer? =
            runCatching { NetworkPeer(this, address, port, publicKey) }.getOrNull()

    actual val currency: Currency by lazy {
        Currency(checkNotNull(cryptoNetworkGetCurrency(core)), false)
    }

    actual val currencies: Set<Currency> by lazy {
        List(cryptoNetworkGetCurrencyCount(core).toInt()) { i ->
            Currency(cryptoNetworkGetCurrencyAt(core, i.toULong())!!, false)
        }.toSet()
    }

    actual fun currencyByCode(code: String): Currency? {
        val coreCurrency = cryptoNetworkGetCurrencyForCode(core, code) ?: return null
        return Currency(coreCurrency, false)
    }

    actual fun currencyByIssuer(issuer: String): Currency? {
        val coreCurrency = cryptoNetworkGetCurrencyForIssuer(core, issuer) ?: return null
        return Currency(coreCurrency, false)
    }

    actual fun hasCurrency(currency: Currency): Boolean =
            CRYPTO_TRUE == cryptoNetworkHasCurrency(core, currency.core)

    actual fun baseUnitFor(currency: Currency): CUnit? {
        if (!hasCurrency(currency)) return null
        return CUnit(checkNotNull(cryptoNetworkGetUnitAsBase(core, currency.core)), false)
    }

    actual fun defaultUnitFor(currency: Currency): CUnit? {
        if (!hasCurrency(currency)) return null
        return CUnit(checkNotNull(cryptoNetworkGetUnitAsDefault(core, currency.core)), false)
    }

    actual fun unitsFor(currency: Currency): Set<CUnit>? {
        if (!hasCurrency(currency)) return null
        val networkCount = cryptoNetworkGetUnitCount(core, currency.core)
        return List(networkCount.toInt()) { i ->
            CUnit(checkNotNull(cryptoNetworkGetUnitAt(core, currency.core, i.toULong())), false)
        }.toSet()
    }

    actual fun hasUnitFor(currency: Currency, unit: CUnit): Boolean? =
            unitsFor(currency)?.contains(unit)

    actual fun addressFor(string: String): Address? {
        val cryptoAddress = cryptoAddressCreateFromString(core, string) ?: return null
        return Address(cryptoAddress, false)
    }

    actual val defaultAddressScheme: AddressScheme
        get() = AddressScheme.fromCoreInt(cryptoNetworkGetDefaultAddressScheme(core).value)

    actual val supportedAddressSchemes: List<AddressScheme>
        get() = memScoped {
            val count = alloc<BRCryptoCountVar>()
            val coreSchemes = checkNotNull(cryptoNetworkGetSupportedAddressSchemes(core, count.ptr))
            return List(count.value.toInt()) { i ->
                AddressScheme.fromCoreInt(coreSchemes[i].value.value)
            }
        }

    actual val supportedWalletManagerModes: List<WalletManagerMode>
        get() = memScoped {
            val count = alloc<BRCryptoCountVar>()
            val coreModes = checkNotNull(cryptoNetworkGetSupportedSyncModes(core, count.ptr))
            return List(count.value.toInt()) { i ->
                WalletManagerMode.fromCoreInt(coreModes[i].value.value)
            }
        }

    actual val defaultWalletManagerMode: WalletManagerMode
        get() = WalletManagerMode.fromCoreInt(cryptoNetworkGetDefaultSyncMode(core).value)

    actual fun supportsWalletManagerMode(mode: WalletManagerMode): Boolean {
        val coreMode = BRCryptoSyncMode.byValue(mode.core)
        return CRYPTO_TRUE == cryptoNetworkSupportsSyncMode(core, coreMode)
    }

    actual fun supportsAddressScheme(addressScheme: AddressScheme): Boolean {
        val coreScheme = BRCryptoAddressScheme.byValue(addressScheme.core)
        return CRYPTO_TRUE == cryptoNetworkSupportsAddressScheme(core, coreScheme)
    }

    actual fun addCurrency(currency: Currency, baseUnit: CUnit, defaultUnit: CUnit) {
        require(baseUnit.hasCurrency(currency))
        require(defaultUnit.hasCurrency(currency))
        if (!hasCurrency(currency)) {
            cryptoNetworkAddCurrency(core, currency.core, baseUnit.core, defaultUnit.core)
        }
    }

    actual fun addUnitFor(currency: Currency, unit: CUnit) {
        require(unit.hasCurrency(currency))
        require(hasCurrency(currency))
        if (hasUnitFor(currency, unit) != true) {
            cryptoNetworkAddCurrencyUnit(core, currency.core, unit.core)
        }
    }

    actual fun requiresMigration(): Boolean {
        return CRYPTO_TRUE == cryptoNetworkRequiresMigration(core)
    }

    actual override fun hashCode(): Int = uids.hashCode()
    actual override fun equals(other: Any?): Boolean =
            other is Network && uids == other.uids

    actual override fun toString(): String = name
    actual override fun close() {
        cryptoNetworkGive(core)
    }

    actual companion object {

        internal actual fun installBuiltins(): List<Network> = memScoped {
            val networkCount = alloc<size_tVar>()
            val builtinCores = checkNotNull(cryptoNetworkInstallBuiltins(networkCount.ptr))
            return List(networkCount.value.toInt()) { i ->
                Network(checkNotNull(builtinCores[i]), false)
            }
        }

        actual fun findBuiltin(uids: String): Network? {
            val core = cryptoNetworkFindBuiltin(uids) ?: return null
            return Network(core, false)
        }
    }
}
