package demo

import kotlinx.coroutines.runBlocking
import java.io.File
import java.util.*
import kotlin.system.exitProcess

fun main() = runBlocking {
    Thread.setDefaultUncaughtExceptionHandler { _, error ->
        println("Unhandled Exception: $error")
        error.printStackTrace()
        exitProcess(-1)
    }

    val bdbToken = System.getenv("BDB_CLIENT_TOKEN") ?: BDB_CLIENT_TOKEN
    syncAndPrintBalance(bdbToken)
}

actual fun quit(): Nothing = exitProcess(0)

actual val uids: String = UUID.randomUUID().toString()

actual fun now(): Long = Date().time / 1000

actual val storagePath: String by lazy {
    File(File("").absoluteFile, "walletkit-demo").absolutePath
}
