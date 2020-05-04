package com.breadwallet.core

import brcrypto.*
import brcrypto.BRCryptoTransferStateType.*
import com.breadwallet.core.System.Companion.system
import com.breadwallet.core.model.BdbTransaction
import kotlinx.cinterop.*
import platform.Foundation.NSISO8601DateFormatter
import platform.Foundation.timeIntervalSince1970
import kotlin.native.concurrent.TransferMode
import kotlin.native.concurrent.Worker
import kotlin.native.concurrent.freeze

@SharedImmutable
private val worker = Worker.start()

internal fun createCryptoClient(
        c: BRCryptoClientContext
) = nativeHeap.alloc<BRCryptoClient> {
    context = c
    funcGetBlockNumber = staticCFunction { context, cwm, sid ->
        initRuntimeIfNeeded()
        try {
            worker.execute(TransferMode.SAFE, {
                Triple(cwm, sid, context)
            }) { (cwm, sid, context) ->
                memScoped {
                    checkNotNull(context)
                    checkNotNull(cwm)
                    defer { cryptoWalletManagerGive(cwm) }
                    val system = context.system
                    val manager = checkNotNull(system.getWalletManager(cwm))
                    try {
                        val chain = system.query2.getBlockchain(manager.network.uids)
                        cwmAnnounceGetBlockNumberSuccess(cwm, sid, chain.blockHeight.toULong())
                    } catch (e: Exception) {
                        e.printStackTrace()
                        cwmAnnounceGetBlockNumberFailure(manager.core, sid)
                    }
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }
    funcGetTransactions =
            staticCFunction { context, cwm, sid, addrs, addrsCount, currency, begBlockNumber, endBlockNumber ->
                initRuntimeIfNeeded()
                try {
                    val addresses = List(addrsCount.toInt()) { i ->
                        checkNotNull(addrs!![i]).toKStringFromUtf8()
                    }

                    worker.execute(TransferMode.SAFE, {
                        Triple(context, Triple(cwm, sid, addresses), begBlockNumber to endBlockNumber).freeze()
                    }) { (context, cwmSidAddresses, blockNums) ->
                        memScoped {
                            val (b, e) = blockNums
                            val (cwm, sid, addresses) = cwmSidAddresses
                            checkNotNull(context)
                            checkNotNull(cwm)
                            defer { cryptoWalletManagerGive(cwm) }
                            val system = context.system
                            val manager = checkNotNull(system.getWalletManager(cwm))
                            val transactions = system.query2.getTransactions(
                                    manager.network.uids,
                                    addresses,
                                    if (b == BLOCK_HEIGHT_UNBOUND_VALUE) null else b,
                                    if (e == BLOCK_HEIGHT_UNBOUND_VALUE) null else e,
                                    includeRaw = true,
                                    includeProof = false
                            ).embedded.transactions
                            processTransactions(Triple(cwm, sid, transactions))
                        }
                    }
                } catch (e: Exception) {
                    e.printStackTrace()
                    cwmAnnounceGetTransactionsComplete(cwm, sid, CRYPTO_FALSE)
                    cryptoWalletManagerGive(cwm)
                }
            }

    funcGetTransfers =
            staticCFunction { context, cwm, sid, addrs, addrsCount, currency, begBlockNumber, endBlockNumber ->
                initRuntimeIfNeeded()
                try {
                    memScoped {
                        checkNotNull(context)
                        checkNotNull(cwm)
                        defer { cryptoWalletManagerGive(cwm) }

                        val system = context.system
                        val manager = checkNotNull(system.getWalletManager(cwm))
                        val addresses = List(addrsCount.toInt()) { i ->
                            checkNotNull(addrs!![i]).toKStringFromUtf8()
                        }

                        try {
                            val transactions = system.query2.getTransactions(
                                    manager.network.uids,
                                    addresses,
                                    if (begBlockNumber == BLOCK_HEIGHT_UNBOUND_VALUE) null else begBlockNumber,
                                    if (endBlockNumber == BLOCK_HEIGHT_UNBOUND_VALUE) null else endBlockNumber,
                                    includeRaw = true,
                                    includeProof = false
                            ).embedded.transactions

                            val fmt = NSISO8601DateFormatter()
                            transactions.map { bdbTx ->
                                val blockTimestamp = bdbTx.timestamp?.run(fmt::dateFromString)
                                        ?.timeIntervalSince1970
                                        ?.toLong() ?: 0L
                                val blockHeight = bdbTx.blockHeight ?: 0L
                                val blockConfirmations = bdbTx.confirmations ?: 0L
                                val blockTransactionIndex = bdbTx.index ?: 0
                                val blockHash = bdbTx.blockHash
                                val status = when (bdbTx.status) {
                                    "confirmed" -> CRYPTO_TRANSFER_STATE_INCLUDED
                                    "submitted", "reverted" -> CRYPTO_TRANSFER_STATE_SUBMITTED
                                    "failed", "rejected" -> CRYPTO_TRANSFER_STATE_ERRORED
                                    else -> error("Unhandled Transaction status '${bdbTx.status}'")
                                }
                                mergeTransfers(bdbTx, addresses).forEach { (transfer, fee) ->
                                    val metaKeysPtr = transfer.meta.keys.map { it.cstr.ptr }.toCValues()
                                    val metaValsPtr = transfer.meta.values.map { it.cstr.ptr }.toCValues()

                                    cwmAnnounceGetTransferItem(
                                            cwm, sid, status,
                                            bdbTx.hash,
                                            transfer.transferId,
                                            transfer.fromAddress,
                                            transfer.toAddress,
                                            transfer.amount.value,
                                            transfer.amount.currencyId,
                                            fee,
                                            blockTimestamp.toULong(),
                                            blockHeight.toULong(),
                                            blockConfirmations.toULong(),
                                            blockTransactionIndex.toULong(),
                                            blockHash,
                                            metaKeysPtr.size.toULong(),
                                            metaKeysPtr,
                                            metaValsPtr
                                    )
                                }
                            }
                            cwmAnnounceGetTransfersComplete(cwm, sid, CRYPTO_TRUE)
                        } catch (e: Exception) {
                            e.printStackTrace()
                            cwmAnnounceGetTransfersComplete(cwm, sid, CRYPTO_FALSE)
                        }
                    }
                } catch (e: Exception) {
                    e.printStackTrace()
                    cwmAnnounceGetTransfersComplete(cwm, sid, CRYPTO_FALSE)
                }
            }

    funcSubmitTransaction =
            staticCFunction { context, cwm, sid, transactionBytes, transactionBytesLength, hashAsHex ->
                initRuntimeIfNeeded()
                memScoped {
                    checkNotNull(context)
                    checkNotNull(cwm)
                    defer { cryptoWalletManagerGive(cwm) }
                }
            }

    funcEstimateTransactionFee =
            staticCFunction { context, cwm, sid, transactionBytes, transactionBytesLength, hashAsHex ->
                initRuntimeIfNeeded()
                memScoped {
                    checkNotNull(context)
                    checkNotNull(cwm)
                    defer { cryptoWalletManagerGive(cwm) }
                }
            }
}

private fun processTransactions(
        data: Triple<BRCryptoWalletManager?, BRCryptoClientCallbackState?, List<BdbTransaction>>
) = memScoped {
    val (cwm, sid, transactions) = data

    try {
        val fmt = NSISO8601DateFormatter()
        transactions.forEach { bdbTx ->
            val timestamp = bdbTx.timestamp?.run(fmt::dateFromString)
                    ?.timeIntervalSince1970
                    ?.toLong()
                    ?: 0L
            val height = bdbTx.blockHeight ?: 0L
            val status = when (bdbTx.status) {
                "confirmed" -> CRYPTO_TRANSFER_STATE_INCLUDED
                "submitted", "reverted" -> CRYPTO_TRANSFER_STATE_SUBMITTED
                "failed", "rejected" -> CRYPTO_TRANSFER_STATE_ERRORED
                else -> error("Unhandled Transaction status '${bdbTx.status}'")
            }
            bdbTx.raw?.let { data ->
                val rawTxData = data.decodeBase64Bytes().asUByteArray()
                cwmAnnounceGetTransactionsItem(
                        cwm, sid, status,
                        rawTxData.refTo(0),
                        rawTxData.size.toULong(),
                        timestamp.toULong(),
                        height.toULong()
                )
            }
        }
        cwmAnnounceGetTransactionsComplete(cwm, sid, CRYPTO_TRUE)
    } catch (e: Exception) {
        e.printStackTrace()
        cwmAnnounceGetTransactionsComplete(cwm, sid, CRYPTO_FALSE)
    }
}