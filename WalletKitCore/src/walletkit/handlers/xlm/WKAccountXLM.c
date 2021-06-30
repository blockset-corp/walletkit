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

static WKAccountDetails
wkAccountCreateFromSeedXLM(UInt512 seed) {

    return stellarAccountCreateWithSeed(seed);
}

static WKAccountDetails
wkAccountCreateFromBytesXLM(
    uint8_t*    bytes,
    size_t      len ) {

    return stellarAccountCreateWithSerialization(bytes, len);
}

static void
wkAccountReleaseXLM(WKAccountDetails accountDetails) {
    stellarAccountFree((BRStellarAccount)accountDetails);
}

static size_t
wkAccountSerializeXLM(  uint8_t     *accountSerBuf,
                        WKAccount   account ) {

    BRStellarAccount xlmAcct;
    size_t           serializationSize;
    uint8_t          *xlmSerBuf;

    xlmAcct = (BRStellarAccount) wkAccountAs (account,
                                              WK_NETWORK_TYPE_XLM);

    xlmSerBuf = stellarAccountGetSerialization (xlmAcct,
                                                &serializationSize);
    if (accountSerBuf != NULL)
        memcpy (accountSerBuf, xlmSerBuf, serializationSize);

    free (xlmSerBuf);

    return serializationSize;
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


