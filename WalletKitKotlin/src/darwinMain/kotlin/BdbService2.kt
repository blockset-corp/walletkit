package com.breadwallet.core.api

import com.breadwallet.core.encodeBase64
import com.breadwallet.core.model.*
import io.ktor.client.*
import io.ktor.client.request.*
import kotlinx.cinterop.*
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.json.Json
import platform.Foundation.*
import platform.Foundation.NSURLComponents.Companion.componentsWithString
import platform.Foundation.NSURLQueryItem.Companion.queryItemWithName

private const val DEFAULT_BDB_BASE_URL = "api.blockset.com"

class BdbService2 private constructor(
        private val http: HttpClient,
        private val bdbBaseURL: String,
        private val clientToken: String
) {

    private val json = Json {
        isLenient = true
        ignoreUnknownKeys = true
    }

    companion object {
        public fun createForTest(
                httpClient: HttpClient,
                bdbAuthToken: String,
                bdbBaseURL: String = DEFAULT_BDB_BASE_URL,
        ) = BdbService2(httpClient, bdbBaseURL, bdbAuthToken)
    }

    private fun sendRequest(url: NSURLComponents): String = memScoped {
        val req = NSMutableURLRequest()
        req.setURL(url.URL)
        req.setHTTPMethod("GET")
        req.addValue("Bearer $clientToken", "Authorization")

        val res = alloc<ObjCObjectVar<NSURLResponse?>>()
        val err = alloc<ObjCObjectVar<NSError?>>()
        val data = NSURLConnection.sendSynchronousRequest(req, res.ptr, err.ptr)
        return data!!.toKString()!!
    }

    public suspend fun getBlockchains(isMainnet: Boolean = true): BdbBlockchains =
            http.get("/blockchains") {
                parameter("testnet", !isMainnet)
            }

    public fun getBlockchain(id: String): BdbBlockchain = memScoped {
        val url = componentsWithString("https://$bdbBaseURL/blockchains/$id")!!

        val req = NSMutableURLRequest()
        req.setURL(url.URL)
        req.setHTTPMethod("GET")
        req.addValue("Bearer $clientToken", "Authorization")
        req.addValue("application/json", "accept")

        val res = alloc<ObjCObjectVar<NSURLResponse?>>()
        val err = alloc<ObjCObjectVar<NSError?>>()
        val data = NSURLConnection.sendSynchronousRequest(req, res.ptr, err.ptr)
        json.decodeFromString(data!!.toKString()!!)
    }
    //http.get("/blockchains/$id")

    public suspend fun getCurrencies(blockchainId: String? = null): BdbCurrencies =
            http.get("/currencies") {
                if (blockchainId != null) {
                    parameter("blockchain_id", blockchainId)
                }
            }

    public suspend fun getCurrency(currencyId: String): BdbCurrency =
            http.get("/currencies/$currencyId")

    public suspend fun getOrCreateSubscription(
            subscription: BdbSubscription
    ): BdbSubscription =
            http.get("/subscriptions/${subscription.subscriptionId}")

    public suspend fun getSubscription(id: String): BdbSubscription =
            http.get("/subscriptions/$id")

    public suspend fun getSubscriptions(): BdbSubscriptions =
            http.get("/subscriptions")

    public suspend fun createSubscription(
            deviceId: String,
            endpoint: BdbSubscription.BdbSubscriptionEndpoint,
            currencies: List<BdbSubscription.BdbSubscriptionCurrency>
    ): BdbSubscription =
            http.post("/subscriptions") {
                body = BdbSubscription.BdbNewSubscription(
                        deviceId, endpoint, currencies
                )
            }

    public suspend fun updateSubscription(subscription: BdbSubscription): BdbSubscription =
            http.put("/subscriptions/${subscription.subscriptionId}") {
                body = subscription
            }

    public suspend fun deleteSubscription(id: String) =
            http.delete<Unit>("/subscriptions/$id")

    public suspend fun getTransfers(
            blockchainId: String,
            addresses: List<String>,
            beginBlockNumber: ULong?,
            endBlockNumber: ULong?,
            maxPageSize: Int? = null
    ): List<BdbTransfer> =
            http.get("/transfers") {
                // TODO: Java implementation chunks this op by 50 addresses
                parameter("blockchain_id", blockchainId)
                parameter("start_height", beginBlockNumber)
                parameter("end_height", endBlockNumber)
                parameter("max_page_size", maxPageSize)
                parameter("address", addresses)
            }

    public suspend fun getTransfer(transferId: String): BdbTransfer =
            http.get("/transfers/$transferId")

    public fun getTransactions(
            blockchainId: String,
            addresses: List<String>,
            beginBlockNumber: ULong?,
            endBlockNumber: ULong?,
            includeRaw: Boolean,
            includeProof: Boolean,
            maxPageSize: Int? = null
    ): BdbTransactions = memScoped {
        val url = componentsWithString("https://$bdbBaseURL/transactions")!!
        val queryItems = listOfNotNull(
                queryItemWithName("blockchain_id", blockchainId),
                queryItemWithName("include_proof", includeProof.toString()),
                queryItemWithName("include_raw", includeRaw.toString()),
                beginBlockNumber?.toString()?.run {
                    queryItemWithName("start_height", this)
                },
                endBlockNumber?.toString()?.run {
                    queryItemWithName("end_height", this)
                },
                maxPageSize?.toString()?.run {
                    queryItemWithName("max_page_size", this)
                }
        )
        addresses.chunked(20).map { chunk ->
            url.queryItems = queryItems + queryItemWithName("address", chunk.joinToString(separator = ","))
            var bdbTxns = json.decodeFromString<BdbTransactions>(sendRequest(url))
            var moreUrl = bdbTxns.links?.next?.href
            while (moreUrl != null) {
                val next = json.decodeFromString<BdbTransactions>(sendRequest(componentsWithString(moreUrl)!!))
                moreUrl = next.links?.next?.href
                val txns = bdbTxns.embedded.transactions + next.embedded.transactions
                bdbTxns = BdbTransactions(BdbTransactions.Embedded(txns))
            }
            bdbTxns
        }.reduce { acc, bdbTxns ->
            val txns = acc.embedded.transactions + bdbTxns.embedded.transactions
            BdbTransactions(BdbTransactions.Embedded(txns))
        }
    }

    public suspend fun getTransaction(
            transactionId: String,
            includeRaw: Boolean,
            includeProof: Boolean
    ): BdbTransaction =
            http.get("/transactions/$transactionId") {
                parameter("include_raw", includeRaw)
                parameter("include_proof", includeProof)
            }

    public suspend fun createTransaction(
            blockchainId: String,
            hashAsHex: String,
            tx: ByteArray
    ): Unit = http.post("/transactions") {
        parameter("blockchain_id", blockchainId)
        parameter("transaction_id", hashAsHex)
        parameter("data", tx.encodeBase64())
    }

    public suspend fun getBlocks(
            blockchainId: String,
            includeRaw: Boolean = true,
            includeTx: Boolean = false,
            includeTxRaw: Boolean = false,
            includeTxProof: Boolean = false,
            beginBlockNumber: ULong? = null,
            endBlockNumber: ULong? = null,
            maxPageSize: Int? = null
    ): List<BdbBlock> =
            http.get("/blocks") {
                parameter("blockchain_id", blockchainId)
                parameter("include_raw", includeRaw)
                parameter("include_tx", includeTx)
                parameter("include_tx_raw", includeTxRaw)
                parameter("include_tx_proof", includeTxProof)
                parameter("start_height", beginBlockNumber)
                parameter("end_height", endBlockNumber)
                parameter("max_page_size", maxPageSize)
            }

    public suspend fun getBlock(
            blockId: String,
            includeTx: Boolean,
            includeTxRaw: Boolean,
            includeTxProof: Boolean
    ): BdbBlock =
            http.get("/blocks/$blockId") {
                parameter("include_raw", false)
                parameter("include_tx", includeTx)
                parameter("include_tx_raw", includeTxRaw)
                parameter("include_tx_proof", includeTxProof)
            }

    public suspend fun getBlockWithRaw(blockId: String): BdbBlock =
            http.get("/blocks/$blockId") {
                parameter("include_raw", true)
                parameter("include_tx", false)
                parameter("include_tx_raw", false)
                parameter("include_tx_proof", false)
            }

    private fun getNetworkName(networkName: String): String? {
        return if (networkName.toLowerCase() == "testnet") "ropsten" else networkName
    }
}


inline fun NSData.toKString(): String? {
    return bytes?.readBytes(length.toInt())?.toKString()
}