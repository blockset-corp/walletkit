/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

/* package */
interface WKBoolean {

    // This type is left as an interface with int constant values, rather than converting to an enum, as
    // this is never exposed outside of the library (boolean is used) so the overhead of creating an enum
    // is unnecessary.

    int WK_FALSE = 0;
    int WK_TRUE = 1;
}
