package com.breadwallet.core

import brcrypto.*
import brcrypto.BRCryptoWalletState.CRYPTO_WALLET_STATE_CREATED
import brcrypto.BRCryptoWalletState.CRYPTO_WALLET_STATE_DELETED
import kotlinx.cinterop.*

actual class Wallet internal constructor(
        core: BRCryptoWallet,
        actual val manager: WalletManager,
        actual val callbackCoordinator: SystemCallbackCoordinator,
        take: Boolean
) {

    internal val core: BRCryptoWallet =
            if (take) checkNotNull(cryptoWalletTake(core))
            else core

    actual val system: System
        get() = manager.system

    actual val unit: CUnit
        get() = CUnit(checkNotNull(cryptoWalletGetUnit(core)), false)

    actual val unitForFee: CUnit
        get() = CUnit(checkNotNull(cryptoWalletGetUnitForFee(core)), false)

    actual val balance: Amount
        get() = Amount(checkNotNull(cryptoWalletGetBalance(core)), false)

    actual val transfers: List<Transfer> by lazy {
        memScoped {
            val count = alloc<ULongVar>()
            val coreTransfers = cryptoWalletGetTransfers(core, count.ptr)?.also { pointer ->
                defer { cryptoMemoryFree(pointer) }
            }

            List(count.value.toInt()) { i ->
                Transfer(checkNotNull(coreTransfers!![i]), this@Wallet, false)
            }
        }
    }

    actual fun getTransferByHash(hash: TransferHash?): Transfer? =
            transfers.singleOrNull { it.hash == hash }

    actual val target: Address
        get() = getTargetForScheme(manager.addressScheme)

    actual fun getTargetForScheme(scheme: AddressScheme): Address {
        val coreScheme = BRCryptoAddressScheme.byValue(scheme.core)
        val coreAddress = checkNotNull(cryptoWalletGetAddress(core, coreScheme))
        return Address(coreAddress, false)
    }

    actual val currency: Currency
        get() = Currency(checkNotNull(cryptoWalletGetCurrency(core)), false)

    actual val name: String
        get() = unit.currency.code

    actual val state: WalletState
        get() = when (cryptoWalletGetState(core)) {
            CRYPTO_WALLET_STATE_CREATED -> WalletState.CREATED
            CRYPTO_WALLET_STATE_DELETED -> WalletState.DELETED
        }

    actual fun hasAddress(address: Address): Boolean {
        return CRYPTO_TRUE == cryptoWalletHasAddress(core, address.core)
    }

    internal actual fun createTransferFeeBasis(
            pricePerCostFactor: Amount,
            costFactor: Double
    ): TransferFeeBasis? {
        val coreFeeBasis = cryptoWalletCreateFeeBasis(core, pricePerCostFactor.core, costFactor)
        return TransferFeeBasis(coreFeeBasis ?: return null, false)
    }

    actual fun createTransfer(
            target: Address,
            amount: Amount,
            estimatedFeeBasis: TransferFeeBasis,
            transferAttributes: List<TransferAttribute>
    ): Transfer? = memScoped {
        val attrs = transferAttributes.map(TransferAttribute::core).toCValues()
        val count = attrs.size.toULong()
        val coreTransfer = cryptoWalletCreateTransfer(core, target.core, amount.core, estimatedFeeBasis.core, count, attrs)
        Transfer(coreTransfer ?: return null, this@Wallet, false)
    }

    internal fun transferBy(core: BRCryptoTransfer): Transfer? {
        return if (CRYPTO_TRUE == cryptoWalletHasTransfer(this.core, core)) {
            Transfer(core, this, true)
        } else null
    }

    internal fun transferByCoreOrCreate(core: BRCryptoTransfer): Transfer? {
        return transferBy(core) ?: Transfer(core, this, true)
    }

    actual fun estimateFee(
            target: Address,
            amount: Amount,
            fee: NetworkFee,
            completion: CompletionHandler<TransferFeeBasis, FeeEstimationError>
    ) {
        cryptoWalletManagerEstimateFeeBasis(manager.core, core, null, target.core, amount.core, fee.core)
    }

    actual fun estimateLimitMaximum(
            target: Address,
            fee: NetworkFee,
            completion: CompletionHandler<Amount, LimitEstimationError>
    ): Unit = TODO()

    actual fun estimateLimitMinimum(
            target: Address,
            fee: NetworkFee,
            completion: CompletionHandler<Amount, LimitEstimationError>
    ): Unit = TODO()

    actual override fun equals(other: Any?): Boolean =
            other is Wallet && CRYPTO_TRUE == cryptoWalletEqual(core, other.core)

    actual override fun hashCode(): Int = core.hashCode()

    internal fun getTransfer(coreTransfer: BRCryptoTransfer): Transfer? {
        return if (cryptoWalletHasTransfer(core, coreTransfer) == CRYPTO_TRUE) {
            Transfer(coreTransfer, this, true)
        } else null
    }
}
