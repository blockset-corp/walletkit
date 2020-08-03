package com.breadwallet.core

import com.breadwallet.core.System.Companion.system
import com.breadwallet.corenative.crypto.BRCryptoClient
import com.breadwallet.corenative.crypto.BRCryptoClient.*
import com.breadwallet.corenative.crypto.BRCryptoTransferStateType.*
import com.breadwallet.corenative.support.BRConstants.BLOCK_HEIGHT_UNBOUND
import com.breadwallet.corenative.utility.Cookie
import com.google.common.primitives.UnsignedLong
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import java.time.Instant

private val scope = CoroutineScope(SupervisorJob() + Dispatchers.Default)

internal fun cryptoClient(c: Cookie) = BRCryptoClient(
        c,
        funcGetBlockNumber,
        funcGetTransactions,
        funcGetTransfers,
        funcSubmitTransaction,
        funcEstimateTransactionFee
)

private val funcGetBlockNumber = GetBlockNumberCallback { cookie, manager, callbackState ->
    scope.launch {

    }
    manager.give()
}

private val funcGetTransactions = GetTransactionsCallback { cookie, coreManager, callbackState, addrs, currency, begBlockNumber, endBlockNumber ->
    scope.launch {
        val system = checkNotNull(cookie.system)
        val manager = checkNotNull(system.getWalletManager(coreManager))
        runCatching {
            val transactions = system.query.getTransactions(
                    manager.network.uids,
                    addrs,
                    if (begBlockNumber == BLOCK_HEIGHT_UNBOUND.toLong()) null else begBlockNumber.toULong(),
                    if (endBlockNumber == BLOCK_HEIGHT_UNBOUND.toLong()) null else endBlockNumber.toULong(),
                    includeRaw = true,
                    includeProof = false,
                    maxPageSize = null
            ).embedded.transactions

            transactions.forEach { bdbTx ->
                val rawTxData = checkNotNull(bdbTx.raw).decodeBase64Bytes()
                val timestamp = UnsignedLong.valueOf(Instant.parse(bdbTx.timestamp).toEpochMilli())
                val height = UnsignedLong.valueOf(bdbTx.blockHeight ?: 0L)
                val status = when (bdbTx.status) {
                    "confirmed" -> CRYPTO_TRANSFER_STATE_INCLUDED
                    "submitted", "reverted" -> CRYPTO_TRANSFER_STATE_SUBMITTED
                    "failed", "rejected" -> CRYPTO_TRANSFER_STATE_ERRORED
                    else -> error("Unhandled Transaction status '${bdbTx.status}'")
                }
                manager.core.announceGetTransactionsItem(callbackState, status, rawTxData, timestamp, height)
            }
        }.onSuccess {
            manager.core.announceGetTransactionsComplete(callbackState, true)
        }.onFailure { error ->
            error.printStackTrace()
            manager.core.announceGetTransactionsComplete(callbackState, false)
        }
    }
    coreManager.give()
}

private val funcGetTransfers = GetTransfersCallback { cookie, manager, callbackState, addrs, currency, begBlockNumber, endBlockNumber ->
    manager.give()
}

private val funcSubmitTransaction = SubmitTransactionCallback { cookie, manager, callbackState, data, hashHex ->
    manager.give()
}

private val funcEstimateTransactionFee = EstimateTransactionFeeCallback { cookie, manager, callbackState, bytes, s ->
    manager.give()
}