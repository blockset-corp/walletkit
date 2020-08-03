package com.breadwallet.core

import com.breadwallet.corenative.crypto.BRCryptoAddressScheme
import com.breadwallet.corenative.crypto.BRCryptoTransfer
import com.breadwallet.corenative.crypto.BRCryptoWallet
import com.breadwallet.corenative.crypto.BRCryptoWalletState
import java.util.*

actual class Wallet internal constructor(
        internal val core: BRCryptoWallet,
        actual val manager: WalletManager,
        actual val callbackCoordinator: SystemCallbackCoordinator
) {

    actual val system: System
        get() = manager.system

    actual val unit: CUnit
        get() = CUnit(core.unit)

    actual val unitForFee: CUnit
        get() = CUnit(core.unitForFee)

    actual val balance: Amount
        get() = core.balance.run(::Amount)

    actual val transfers: List<Transfer> by lazy {
        core.transfers.map { Transfer(it, this) }
    }

    actual val target: Address
        get() = getTargetForScheme(manager.addressScheme)

    actual val currency: Currency
        get() = Currency(core.currency)

    actual val name: String
        get() = unit.currency.name

    actual val state: WalletState
        get() = when (core.state) {
            BRCryptoWalletState.CRYPTO_WALLET_STATE_CREATED -> WalletState.CREATED
            BRCryptoWalletState.CRYPTO_WALLET_STATE_DELETED -> WalletState.DELETED
            else -> error("Invalid core state (${core.state})")
        }

    actual fun hasAddress(address: Address): Boolean {
        return core.containsAddress(address.core)
    }

    actual fun getTransferByHash(hash: TransferHash?): Transfer? =
            transfers.singleOrNull { it.hash == hash }

    actual fun getTargetForScheme(scheme: AddressScheme): Address {
        val coreInt = manager.addressScheme.core.toInt()
        val coreScheme = BRCryptoAddressScheme.fromCore(coreInt)
        return core.getTargetAddress(coreScheme).run(::Address)
    }

    internal actual fun createTransferFeeBasis(pricePerCostFactor: Amount, costFactor: Double): TransferFeeBasis? {
        return core.createTransferFeeBasis(pricePerCostFactor.core, costFactor).orNull()?.run(::TransferFeeBasis)
    }

    actual fun createTransfer(
            target: Address,
            amount: Amount,
            estimatedFeeBasis: TransferFeeBasis,
            transferAttributes: List<TransferAttribute>
    ): Transfer? {
        val attrs = transferAttributes.map(TransferAttribute::core)
        val coreTransfer = core.createTransfer(target.core, amount.core, estimatedFeeBasis.core, attrs).orNull()
        return Transfer(coreTransfer ?: return null, this)
    }

    actual fun estimateFee(
            target: Address,
            amount: Amount,
            fee: NetworkFee,
            completion: CompletionHandler<TransferFeeBasis, FeeEstimationError>
    ) {
        manager.core.estimateFeeBasis(core, null, target.core, amount.core, fee.core)
    }

    actual fun estimateLimitMaximum(
            target: Address,
            fee: NetworkFee,
            completion: CompletionHandler<Amount, LimitEstimationError>
    ) {
        //manager.core.estimateLimit()
        TODO()
    }

    actual fun estimateLimitMinimum(
            target: Address,
            fee: NetworkFee,
            completion: CompletionHandler<Amount, LimitEstimationError>
    ): Unit = TODO()

    actual override fun equals(other: Any?): Boolean =
            other is Wallet && core == other.core

    actual override fun hashCode(): Int = Objects.hash(core)

    internal fun getTransfer(coreTransfer: BRCryptoTransfer): Transfer? {
        return if (core.containsTransfer(coreTransfer)) {
            Transfer(coreTransfer.take(), this)
        } else null
    }
}
