package demo

import com.breadwallet.core.*
import com.breadwallet.core.api.BdbService
import io.ktor.client.*
import io.ktor.utils.io.core.*
import kotlinx.coroutines.yield
import kotlin.native.concurrent.SharedImmutable

suspend fun syncAndPrintBalance(bdbToken: String) {
    val bdbService = BdbService.createForTest(HttpClient(), bdbToken)
    val account = Account.createFromPhrase(PHRASE.toByteArray(), 0, uids)
    val system = System.create(
            "",
            systemListener,
            checkNotNull(account),
            isMainnet = false,
            storagePath,
            bdbService
    )

    checkNotNull(system).configure(emptyList())

    while (true) yield()
}

@SharedImmutable
private val systemListener = object : SystemListener {

    override fun handleNetworkEvent(system: System, network: Network, event: NetworkEvent) {
        when (event) {
            NetworkEvent.Created -> {
                if (!network.currency.code.equals("btc", true)) return
                printlnMagenta("${network.currency.name} Network Created")

                val created = system.createWalletManager(network)
                check(created) { "Failed to create wallet manager $network" }
            }
        }
    }

    override fun handleManagerEvent(system: System, manager: WalletManager, event: WalletManagerEvent) {
        when (event) {
            WalletManagerEvent.Created -> {
                printlnMagenta("${manager.currency.name} Manager Created")
                manager.connect()
            }
            WalletManagerEvent.SyncStarted ->
                printlnMagenta("${manager.currency.name} Sync Started")
            is WalletManagerEvent.SyncProgress ->
                printlnMagenta("${manager.currency.name} Sync Progress")
            is WalletManagerEvent.SyncStopped -> {
                printlnMagenta("${manager.currency.name} Sync ${event.reason}")
                printlnGreen(manager.primaryWallet.balance)
                quit()
            }
        }
    }
}

fun printlnGreen(message: Any) = println("\u001b[32m$message\u001b[0m")
fun printlnMagenta(message: Any) = println("\u001b[35m$message\u001b[0m")
