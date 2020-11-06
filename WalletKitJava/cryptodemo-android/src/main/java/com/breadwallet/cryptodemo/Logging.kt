package com.breadwallet.cryptodemo

import android.os.Build
import android.util.Log
import java.util.logging.*

/**
 * Helper class for configuring logging to logcat.
 *
 * The default Android [Handler] used [Log.isLoggable] before determining if
 * a record is logged. This class instead installs a hanlder that does *NOT* factor that into its
 * determination on whether to log or not.
 */
internal object Logging {
    private val TAG = Logging::class.java.name

    @JvmStatic
    fun initialize(minimumLevel: Level? = Level.ALL) {
        // clear out the standard config
        LogManager.getLogManager().reset()

        Logger.getLogger("").apply {
            // add the new permissive handler
            addHandler(LoggingHandler())
            // set the new root (and child) minimum log level
            level = minimumLevel
        }
    }

    internal class LoggingHandler : Handler() {

        init {
            formatter = THE_FORMATTER
        }

        override fun publish(record: LogRecord) {
            val level = logcatLevel(record.level)
            val tag = logcatName(record.loggerName)

            // Don't check `Log.isLoggable`, as that has its level set to
            // INFO (by default, typically) and is not easily changed
            try {
                Log.println(level, tag, formatter.format(record))
            } catch (e: RuntimeException) {
                Log.e(TAG, "Failed to print record", e)
            }
        }

        // not necessary for android.util.Log
        override fun flush() = Unit

        // not necessary for android.util.Log
        @Throws(SecurityException::class)
        override fun close() = Unit

        companion object {
            private const val TAG_SEPARATOR = "."

            // android.util.Log requires tags of length 23 or smaller for up to and including API level 23
            private val MAX_TAG_LENGTH = if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.M) 23 else Int.MAX_VALUE

            private val THE_FORMATTER: Formatter = object : Formatter() {
                override fun format(record: LogRecord): String = buildString {
                    appendLine(record.message)
                    record.thrown
                        ?.run(Log::getStackTraceString)
                        ?.run(::appendLine)
                }
            }

            fun logcatLevel(level: Level): Int {
                val julValue = level.intValue()
                return when {
                    julValue >= Level.SEVERE.intValue() -> Log.ERROR
                    julValue >= Level.WARNING.intValue() -> Log.WARN
                    julValue >= Level.INFO.intValue() -> Log.INFO
                    else -> Log.DEBUG
                }
            }

            fun logcatName(name: String? = null): String {
                return name
                    .orEmpty()
                    .ifEmpty { "<empty>" }
                    .substringAfterLast(TAG_SEPARATOR)
                    .take(MAX_TAG_LENGTH)
            }
        }
    }
}