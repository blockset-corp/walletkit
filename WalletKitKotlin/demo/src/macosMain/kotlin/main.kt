package demo

import co.touchlab.stately.freeze
import kotlinx.cinterop.autoreleasepool
import kotlinx.coroutines.runBlocking
import platform.Foundation.*
import kotlin.system.exitProcess

fun main() = autoreleasepool {
    setUnhandledExceptionHook({ error: Throwable ->
        println("Unhandled Exception: $error")
        error.printStackTrace()
        exitProcess(-1)
    }.freeze())
    runBlocking {
        val bdbToken = NSProcessInfo.processInfo.environment["BDB_CLIENT_TOKEN"]?.toString() ?: BDB_CLIENT_TOKEN
        syncAndPrintBalance(bdbToken)
    }
}

actual fun quit(): Nothing = exitProcess(0)

actual val uids: String = NSUUID().UUIDString

actual fun now(): Long = NSDate.now().timeIntervalSince1970().toLong()

actual val storagePath: String by lazy {
    val basePath = NSURL.fileURLWithPath(NSFileManager.defaultManager.currentDirectoryPath)

    checkNotNull(NSURL(
            fileURLWithPath = "walletkit-demo",
            relativeToURL = basePath
    ).path)
}
