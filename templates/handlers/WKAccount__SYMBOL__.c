//
//  WKAccount__SYMBOL__.c
//  WalletKitCore
//
//  Created by __USER__ on __DATE__.
//  Copyright © __YEAR__ Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WK__SYMBOL__.h"

static WKAccountDetails
wkAccountCreateFromSeed__SYMBOL__(UInt512 seed) {
    return __name__AccountCreateWithSeed(seed);
}

static WKAccountDetails
wkAccountCreateFromBytes__SYMBOL__(
    uint8_t*    bytes,
    size_t      len ) {

    BR__Name__Account __symbol__ = __name__AccountCreateWithSerialization(bytes, len);
    assert (NULL != __symbol__);

    return __symbol__;
}

static void
wkAccountRelease__SYMBOL__(WKAccountDetails accountDetails) {
    __name__AccountFree ((BR__Name__Account)accountDetails);
}

static size_t
wkAccountSerialize__SYMBOL__(  uint8_t     *accountSerBuf,
                        WKAccount   account ) {

    assert (account != NULL);

    size_t          __symbol__Size;
    BR__Name__Account  __symbol__Acct;
    uint8_t         *__symbol__Bytes;

    __symbol__Acct = (BR__Name__Account) wkAccountAs (account,
                                            WK_NETWORK_TYPE___SYMBOL__);
    __symbol__Bytes = __name__AccountGetSerialization (__symbol__Acct,
                                             &__symbol__Size );

    if (accountSerBuf != NULL) {
        memcpy (accountSerBuf, __symbol__Bytes, __symbol__Size);
    }

    free (__symbol__Bytes);

    return __symbol__Size;
}

WKAccountHandlers wkAccountHandlers__SYMBOL__ = {
    wkAccountCreateFromSeed__SYMBOL__,
    wkAccountCreateFromBytes__SYMBOL__,
    wkAccountRelease__SYMBOL__,
    wkAccountSerialize__SYMBOL__
};



