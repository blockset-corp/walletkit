//
//  WKAccountXRP.c
//  WalletKitCore
//
//  Created by Bryan Goring on 06/15/2021.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXRP.h"

static WKAccountDetails
wkAccountCreateFromSeedXRP(UInt512 seed) {
    return rippleAccountCreateWithSeed(seed);
}

static WKAccountDetails
wkAccountCreateFromBytesXRP(
    uint8_t*    bytes,
    size_t      len ) {

    BRRippleAccount xrp = rippleAccountCreateWithSerialization(bytes, len);
    assert (NULL != xrp);

    return xrp;
}

static void
wkAccountReleaseXRP(WKAccount account) {

    BRRippleAccount rip;

    rip = (BRRippleAccount) wkAccountAs (account,
                                         WK_NETWORK_TYPE_XRP);
    rippleAccountFree (rip);
}

static size_t
wkAccountSerializeXRP(  uint8_t     *accountSerBuf,
                        WKAccount   account ) {

    assert (account != NULL);

    size_t          xrpSize;
    BRRippleAccount xrpAcct;
    uint8_t         *xrpBytes;

    xrpAcct = (BRRippleAccount) wkAccountAs (account,
                                             WK_NETWORK_TYPE_XRP);
    xrpBytes = rippleAccountGetSerialization (xrpAcct,
                                              &xrpSize );

    if (accountSerBuf != NULL) {
        memcpy (accountSerBuf, xrpBytes, xrpSize);
    }

    free (xrpBytes);

    return xrpSize;
}

WKAccountHandlers wkAccountHandlersXRP = {
    wkAccountCreateFromSeedXRP,
    wkAccountCreateFromBytesXRP,
    wkAccountReleaseXRP,
    wkAccountSerializeXRP
};
