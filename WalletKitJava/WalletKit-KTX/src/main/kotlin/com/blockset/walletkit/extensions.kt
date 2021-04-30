/**
 * Created by Drew Carlson <drew.carlson@breadwallet.com> on 12/11/20.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit

import com.blockset.walletkit.*
import com.blockset.walletkit.errors.*
import com.blockset.walletkit.utility.CompletionHandler
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException

/** See [Wallet.estimateFee]. */
@Throws(FeeEstimationError::class)
suspend fun Wallet.estimateFee(
        address: Address,
        amount: Amount,
        networkFee: NetworkFee = walletManager.defaultNetworkFee
): TransferFeeBasis {
    return suspendForCompletion<TransferFeeBasis, FeeEstimationError> {
        estimateFee(address, amount, networkFee, null, it)
    }
}

/** See [WalletManager.createSweeper]. */
@Throws(WalletSweeperError::class)
suspend fun WalletManager.createSweeper(wallet: Wallet, key: Key): WalletSweeper {
    return suspendForCompletion<WalletSweeper, WalletSweeperError> {
        createSweeper(wallet, key, it)
    }
}

/** See [WalletSweeper.estimate]. */
@Throws(FeeEstimationError::class)
suspend fun WalletSweeper.estimate(networkFee: NetworkFee): TransferFeeBasis {
    return suspendForCompletion<TransferFeeBasis, FeeEstimationError> {
        estimate(networkFee, it)
    }
}

/** See [System.accountInitialize]. */
@Throws(AccountInitializationError::class)
suspend fun System.accountInitialize(account: Account, network: Network, create: Boolean): ByteArray {
    return suspendForCompletion<ByteArray, AccountInitializationError> {
        accountInitialize(account, network, create, it)
    }
}

/** See [Wallet.estimateLimitMaximum]. */
@Throws(LimitEstimationError::class)
suspend fun Wallet.estimateLimitMaximum(address: Address, networkFee: NetworkFee): Amount {
    return suspendForCompletion<Amount, LimitEstimationError> {
        estimateLimitMaximum(address, networkFee, it)
    }
}

/** See [Wallet.estimateLimitMinimum]. */
@Throws(LimitEstimationError::class)
suspend fun Wallet.estimateLimitMinimum(address: Address, networkFee: NetworkFee): Amount {
    return suspendForCompletion<Amount, LimitEstimationError> {
        estimateLimitMinimum(address, networkFee, it)
    }
}

/** See [PaymentProtocolRequest.estimate]. */
@Throws(FeeEstimationError::class)
suspend fun PaymentProtocolRequest.estimate(fee: NetworkFee): TransferFeeBasis {
    return suspendForCompletion<TransferFeeBasis, FeeEstimationError> {
        estimate(fee, it)
    }
}

private suspend fun <R, E : Exception> suspendForCompletion(
        block: (CompletionHandler<R, E>) -> kotlin.Unit
): R = suspendCancellableCoroutine { continuation ->
    block(object : CompletionHandler<R, E> {
        override fun handleData(data: R) {
            if (continuation.isActive) {
                continuation.resume(data)
            }
        }

        override fun handleError(error: E) {
            if (continuation.isActive) {
                continuation.resumeWithException(error)
            }
        }
    })
}
