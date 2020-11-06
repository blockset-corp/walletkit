/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.cryptodemo

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.net.ConnectivityManager.CONNECTIVITY_ACTION
import com.breadwallet.cryptodemo.Utilities.isNetworkReachable
import java.util.logging.Level
import java.util.logging.Logger

class ConnectivityBroadcastReceiver : BroadcastReceiver() {
    companion object {
        private val Log = Logger.getLogger(ConnectivityBroadcastReceiver::class.java.name)
    }

    @Suppress("DEPRECATION")
    override fun onReceive(context: Context, intent: Intent) {
        when (intent.action) {
            CONNECTIVITY_ACTION -> {
                val isNetworkReachable = isNetworkReachable(context)
                Log.log(Level.FINE, "isNetworkReachable: $isNetworkReachable")
                CoreCryptoApplication.getSystem().setNetworkReachable(isNetworkReachable)
            }
        }
    }
}