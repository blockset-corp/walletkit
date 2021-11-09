//
//  WKAccountHBAR.c
//  WalletKitCore
//
//  Created by Bryan Goring on 06/15/2021.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKHBAR.h"

static WKAccountDetails
wkAccountCreateFromSeedHBAR(
    WKBoolean   isMainnet,
    UInt512     seed    ) {
    return hederaAccountCreateWithSeed(seed);
}

static WKAccountDetails
wkAccountCreateFromBytesHBAR(
    uint8_t*    bytes,
    size_t      len ) {

    BRHederaAccount hbar = hederaAccountCreateWithSerialization(bytes, len);
    assert (NULL != hbar);

    return hbar;
}

static void
wkAccountReleaseHBAR(WKAccountDetails accountDetails) {
    hederaAccountFree ((BRHederaAccount)accountDetails);
}

static size_t
wkAccountSerializeHBAR(  uint8_t     *accountSerBuf,
                         WKAccount   account ) {

    assert (account != NULL);

    size_t          hbarSize;
    BRHederaAccount hbarAcct;
    uint8_t         *hbarBytes;

    hbarAcct = (BRHederaAccount) wkAccountAs (account,
                                              WK_NETWORK_TYPE_HBAR);
    hbarBytes = hederaAccountGetSerialization (hbarAcct,
                                               &hbarSize );

    if (accountSerBuf != NULL) {
        memcpy (accountSerBuf, hbarBytes, hbarSize);
    }

    free (hbarBytes);

    return hbarSize;
}

WKAccountHandlers wkAccountHandlersHBAR = {
    wkAccountCreateFromSeedHBAR,
    wkAccountCreateFromBytesHBAR,
    wkAccountReleaseHBAR,
    wkAccountSerializeHBAR
};
