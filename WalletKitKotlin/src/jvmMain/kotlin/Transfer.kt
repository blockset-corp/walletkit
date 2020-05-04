package com.breadwallet.core

import com.breadwallet.corenative.cleaner.ReferenceCleaner
import com.breadwallet.corenative.crypto.BRCryptoTransfer
import com.breadwallet.corenative.crypto.BRCryptoTransferDirection
import com.breadwallet.corenative.crypto.BRCryptoTransferDirection.*
import com.breadwallet.corenative.crypto.BRCryptoTransferStateType
import com.breadwallet.corenative.crypto.BRCryptoTransferStateType.*
import com.breadwallet.corenative.crypto.BRCryptoTransferSubmitErrorType
import java.util.*

actual class Transfer internal constructor(
        internal val core: BRCryptoTransfer,
        actual val wallet: Wallet

) {

    init {
        ReferenceCleaner.register(core, core::give)
    }

    actual val source: Address?
        get() = core.sourceAddress.orNull()?.run(::Address)

    actual val target: Address?
        get() = core.targetAddress.orNull()?.run(::Address)

    actual val amount: Amount
        get() = Amount(core.amount)

    actual val amountDirected: Amount
        get() = Amount(core.amountDirected)

    actual val fee: Amount
        get() = checkNotNull(confirmedFeeBasis?.fee ?: estimatedFeeBasis?.fee) {
            "Missed confirmed+estimated feeBasis"
        }

    actual val estimatedFeeBasis: TransferFeeBasis?
        get() = core.estimatedFeeBasis.orNull()?.run(::TransferFeeBasis)

    actual val confirmedFeeBasis: TransferFeeBasis?
        get() = core.confirmedFeeBasis.orNull()?.run(::TransferFeeBasis)

    actual val direction: TransferDirection by lazy {
        when (core.direction) {
            CRYPTO_TRANSFER_SENT -> TransferDirection.SENT
            CRYPTO_TRANSFER_RECEIVED -> TransferDirection.RECEIVED
            CRYPTO_TRANSFER_RECOVERED -> TransferDirection.RECOVERED
            else -> error("Unknown core transfer direction (${core.direction})")
        }
    }

    actual val hash: TransferHash?
        get() = core.hash.orNull()?.run(::TransferHash)

    actual val unit: CUnit
        get() = CUnit(core.unitForAmount)

    actual val unitForFee: CUnit
        get() = CUnit(core.unitForFee)

    actual val confirmation: TransferConfirmation?
        get() = (state as? TransferState.INCLUDED)?.confirmation

    actual val confirmations: ULong?
        get() = getConfirmationsAt(wallet.manager.network.height)

    actual val state: TransferState
        get() = when (core.state.type()) {
            CRYPTO_TRANSFER_STATE_CREATED -> TransferState.CREATED
            CRYPTO_TRANSFER_STATE_SIGNED -> TransferState.SIGNED
            CRYPTO_TRANSFER_STATE_SUBMITTED -> TransferState.SUBMITTED
            CRYPTO_TRANSFER_STATE_DELETED -> TransferState.DELETED
            CRYPTO_TRANSFER_STATE_INCLUDED ->
                core.state.u.included.let { included ->
                    TransferState.INCLUDED(TransferConfirmation(
                            blockNumber = included.blockNumber.toULong(),
                            transactionIndex = included.transactionIndex.toULong(),
                            timestamp = included.timestamp.toULong(),
                            fee = Amount(checkNotNull(included.feeBasis.fee.orNull()))
                    ))
                }
            CRYPTO_TRANSFER_STATE_ERRORED ->
                core.state.u.errored.error.let { coreError ->
                    TransferState.FAILED(
                            error = when (coreError.type()) {
                                BRCryptoTransferSubmitErrorType.CRYPTO_TRANSFER_SUBMIT_ERROR_UNKNOWN ->
                                    TransferSubmitError.UNKNOWN
                                BRCryptoTransferSubmitErrorType.CRYPTO_TRANSFER_SUBMIT_ERROR_POSIX ->
                                    TransferSubmitError.POSIX(
                                            errNum = coreError.u.posix.errnum,
                                            errMessage = coreError.message.orNull()
                                    )
                                else -> error("Unknown core error type (${coreError.type()})")
                            }
                    )
                }
            else -> error("Unknown core transfer state type (${core.state.type()})")
        }

    actual fun getConfirmationsAt(blockHeight: ULong): ULong? {
        return confirmation?.run {
            if (blockHeight >= blockNumber) {
                1u + blockHeight - blockNumber
            } else null
        }
    }

    actual override fun equals(other: Any?): Boolean =
            other is Transfer && core == other.core

    actual override fun hashCode(): Int = Objects.hash(core)
}
