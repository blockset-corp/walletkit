/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.cryptodemo

import android.content.Context
import android.net.ConnectivityManager

object Utilities {
    @JvmStatic
    @Suppress("DEPRECATION")
    fun isNetworkReachable(context: Context): Boolean {
        return context.getSystemService(ConnectivityManager::class.java)
            .activeNetworkInfo
            ?.isConnectedOrConnecting ?: false
    }
}