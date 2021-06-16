//
//  WKAccountXLM.c
//  WalletKitCore
//
//  Created by Bryan Goring on 06/15/2021.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXLM.h"

// Impls for Stellar account handler TODO

static void*
wkAccountCreateFromSeedXLM(UInt512 seed) {

    return NULL;
}

static void*
wkAccountCreateFromBytesXLM(
    uint8_t*    bytes,
    size_t      len ) {

    return NULL;
}

static void
wkAccountReleaseXLM(WKAccount account) {
}

static size_t
wkAccountSerializeXLM(  uint8_t     *accountSerBuf,
                        WKAccount   account ) {

    assert (account != NULL);
    return 0;
}

// TODO
// When these methods are to be enabled
// for Stellar account creation, visit WKHandlers.c
// and expose &wkAccountHandlersXLM for WK_NETWORK_TYPE_XLM
WKAccountHandlers wkAccountHandlersXLM = {
    wkAccountCreateFromSeedXLM,
    wkAccountCreateFromBytesXLM,
    wkAccountReleaseXLM,
    wkAccountSerializeXLM,
};


