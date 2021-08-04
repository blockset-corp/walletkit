//
//  WKAccountAVAX.c
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKAVAX.h"

static WKAccountDetails
wkAccountCreateFromSeedAVAX(UInt512 seed) {
    return avalancheAccountCreateWithSeed(seed);
}

static WKAccountDetails
wkAccountCreateFromBytesAVAX(
    uint8_t*    bytes,
    size_t      len ) {

    BRAvalancheAccount avax = avalancheAccountCreateWithSerialization(bytes, len);
    assert (NULL != avax);

    return avax;
}

static void
wkAccountReleaseAVAX(WKAccountDetails accountDetails) {
    avalancheAccountFree ((BRAvalancheAccount)accountDetails);
}

static size_t
wkAccountSerializeAVAX(  uint8_t     *accountSerBuf,
                        WKAccount   account ) {

    assert (account != NULL);
    BRAvalancheAccount avaxAccount = wkAccountGetAsAVAX (account);

    size_t avaxSize;
    uint8_t *avaxBytes = avalancheAccountGetSerialization (avaxAccount, &avaxSize );

    if (accountSerBuf != NULL) {
        memcpy (accountSerBuf, avaxBytes, avaxSize);
    }

    free (avaxBytes);

    return avaxSize;
}

WKAccountHandlers wkAccountHandlersAVAX = {
    wkAccountCreateFromSeedAVAX,
    wkAccountCreateFromBytesAVAX,
    wkAccountReleaseAVAX,
    wkAccountSerializeAVAX
};



