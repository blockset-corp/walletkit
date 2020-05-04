package com.breadwallet.core

import com.breadwallet.core.model.BdbTransaction
import com.breadwallet.core.model.BdbTransfer

internal fun mergeTransfers(transaction: BdbTransaction, addresses: List<String>): List<Pair<BdbTransfer, String?>> {
    // Only consider transfers w/ `address`
    var transfers = transaction.transfers
            .filter { addresses.contains(it.fromAddress) || addresses.contains(it.toAddress) }
            .toMutableList()
    val transfersWithFee = transfers.filter { it.toAddress == "__fee__" }
    val transfersWithoutFee = transfers.filter { it.toAddress != "__fee__" }
    val transfersMerged = ArrayList<Pair<BdbTransfer, String?>>(transfers.size)

    // Get the transferWithFee if we have one
    check(transfersWithFee.size <= 1)
    val transferWithFee = transfersWithFee.getOrNull(0)

    // There is no "__fee__" entry
    if (transferWithFee == null) {
        // Announce transfers with no fee
        transfers.forEach {
            transfersMerged.add(Pair(it, null))
        }

        // There is a single "__fee__" entry, due to `checkState(transfersWithFee.size() <= 1)` above
    } else {
        // We may or may not have a non-fee transfer matching `transferWithFee`.  We
        // may or may not have more than one non-fee transfers matching `transferWithFee`

        // Find the first of the non-fee transfers matching `transferWithFee`
        val transferMatchingFee = transfers.firstOrNull { transfer ->
            transferWithFee.transactionId == transfer.transactionId &&
                    transferWithFee.fromAddress == transfer.fromAddress
        }

        // We must have a transferMatchingFee; if we don't add one
        transfers = transfersWithoutFee.toMutableList()
        if (null == transferMatchingFee) {
            transfers.add(
                    BdbTransfer(
                            transferWithFee.transferId,
                            transferWithFee.blockchainId,
                            transferWithFee.index,
                            transferWithFee.amount,
                            transferWithFee.meta,
                            transferWithFee.fromAddress,
                            "unknown",
                            "0",
                            transferWithFee.acknowledgements)
            )
        }

        // Hold the Id for the transfer that we'll add a fee to.
        val transferForFeeId = transferMatchingFee?.transferId ?: transferWithFee.transferId

        // Announce transfers adding the fee to the `transferforFeeId`
        transfers.forEach { transfer ->
            val fee = if (transfer.transferId == transferForFeeId) {
                transferWithFee.amount.value
            } else null

            transfersMerged.add(Pair(transfer, fee))
        }
    }

    return transfersMerged.toList()
}
