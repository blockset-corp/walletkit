package com.breadwallet.core

import com.breadwallet.corenative.cleaner.ReferenceCleaner
import com.breadwallet.corenative.crypto.BRCryptoAddressScheme
import com.breadwallet.corenative.crypto.BRCryptoNetwork
import com.breadwallet.corenative.crypto.BRCryptoPeer
import com.breadwallet.corenative.crypto.BRCryptoSyncMode
import com.google.common.primitives.UnsignedInteger
import com.google.common.primitives.UnsignedLong
import kotlinx.io.core.Closeable

actual class Network internal constructor(
        internal val core: BRCryptoNetwork
) : Closeable {

    init {
        ReferenceCleaner.register(core, ::close)
    }

    internal actual val uids: String = core.uids

    actual val name: String = core.name

    actual val type: NetworkType
        get() = NetworkType.fromCoreInt(core.canonicalType.toCore())

    actual val isMainnet: Boolean = core.isMainnet

    actual var height: ULong
        get() = core.height.toLong().toULong()
        internal set(value) {
            core.height = UnsignedLong.valueOf(value.toLong())
        }

    actual var fees: List<NetworkFee>
        get() = core.fees.map(::NetworkFee)
        set(value) {
            require(value.isNotEmpty())
            core.fees = value.map(NetworkFee::core)
        }

    actual val minimumFee: NetworkFee
        get() = checkNotNull(fees.maxBy(NetworkFee::timeIntervalInMilliseconds))

    actual val confirmationsUntilFinal: UInt
        get() = core.confirmationsUntilFinal.toByte().toUInt()

    actual fun createPeer(address: String, port: UShort, publicKey: String?): NetworkPeer? =
            BRCryptoPeer.create(
                    core,
                    address,
                    UnsignedInteger.valueOf(port.toLong()),
                    publicKey
            ).orNull()?.run(::NetworkPeer)

    actual val currency: Currency by lazy { Currency(core.currency) }

    actual val currencies: Set<Currency> by lazy {
        (0 until core.currencyCount.toLong())
                .map { core.getCurrency(UnsignedLong.valueOf(it)) }
                .map(::Currency)
                .toSet()
    }

    actual val defaultAddressScheme: AddressScheme
        get() = AddressScheme.fromCoreInt(core.defaultAddressScheme.toCore().toUInt())

    actual val supportedAddressSchemes: List<AddressScheme>
        get() = core.supportedAddressSchemes.map {
            AddressScheme.fromCoreInt(it.toCore().toUInt())
        }

    actual val supportedWalletManagerModes: List<WalletManagerMode>
        get() = core.supportedSyncModes.map { WalletManagerMode.fromCoreInt(it.toCore().toUInt()) }

    actual val defaultWalletManagerMode: WalletManagerMode
        get() = WalletManagerMode.fromCoreInt(core.defaultSyncMode.toCore().toUInt())

    actual fun supportsWalletManagerMode(mode: WalletManagerMode): Boolean {
        return core.supportsSyncMode(BRCryptoSyncMode.fromCore(mode.core.toInt()))
    }

    actual fun supportsAddressScheme(addressScheme: AddressScheme): Boolean {
        return core.supportsAddressScheme(BRCryptoAddressScheme.fromCore(addressScheme.core.toInt()))
    }

    actual fun currencyByCode(code: String): Currency? =
            currencies.firstOrNull { it.code == code }

    actual fun currencyByIssuer(issuer: String): Currency? {
        val issuerLowerCase = issuer.toLowerCase()
        return currencies.firstOrNull { currency ->
            currency.issuer?.toLowerCase() == issuerLowerCase
        }
    }

    actual fun hasCurrency(currency: Currency): Boolean =
            core.hasCurrency(currency.core)

    actual fun baseUnitFor(currency: Currency): CUnit? {
        if (!hasCurrency(currency)) return null
        val cryptoUnit = core.getUnitAsBase(currency.core).orNull() ?: return null
        return CUnit(cryptoUnit)
    }

    actual fun defaultUnitFor(currency: Currency): CUnit? {
        if (!hasCurrency(currency)) return null
        val cryptoUnit = core.getUnitAsDefault(currency.core).orNull() ?: return null
        return CUnit(cryptoUnit)
    }

    actual fun unitsFor(currency: Currency): Set<CUnit>? {
        if (!hasCurrency(currency)) return null
        return (0 until core.getUnitCount(currency.core).toLong())
                .map { checkNotNull(core.getUnitAt(currency.core, UnsignedLong.valueOf(it)).orNull()) }
                .map { CUnit(it) }
                .toSet()
    }

    actual fun hasUnitFor(currency: Currency, unit: CUnit): Boolean? =
            unitsFor(currency)?.contains(unit)

    actual fun addressFor(string: String): Address? {
        return Address.create(string, this)
    }

    actual fun addCurrency(currency: Currency, baseUnit: CUnit, defaultUnit: CUnit) {
        require(baseUnit.hasCurrency(currency))
        require(defaultUnit.hasCurrency(currency))
        if (!hasCurrency(currency)) {
            core.addCurrency(currency.core, baseUnit.core, defaultUnit.core)
        }
    }

    actual fun addUnitFor(currency: Currency, unit: CUnit) {
        require(unit.hasCurrency(currency))
        require(hasCurrency(currency))
        if (hasUnitFor(currency, unit) == null) {
            core.addCurrencyUnit(currency.core, unit.core)
        }
    }

    actual fun requiresMigration(): Boolean =
            core.requiresMigration()

    actual override fun hashCode(): Int = uids.hashCode()
    actual override fun equals(other: Any?): Boolean =
            other is Network && core.uids == other.uids

    actual override fun toString(): String = name

    actual override fun close() {
        core.give()
    }

    actual companion object {
        internal actual fun installBuiltins(): List<Network> =
                BRCryptoNetwork.installBuiltins().map(::Network)

        actual fun findBuiltin(uids: String): Network? =
                BRCryptoNetwork.findBuiltin(uids).orNull()?.run(::Network)
    }
}
