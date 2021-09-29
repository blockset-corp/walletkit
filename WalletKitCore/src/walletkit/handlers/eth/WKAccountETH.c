//
//  WKAccountETH.c
//  WalletKitCore
//
//  Created by Bryan Goring on 06/15/2021.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "WKETH.h"

static WKAccountDetails
wkAccountCreateFromSeedETH(UInt512 seed) {
    return ethAccountCreateWithBIP32Seed(seed);
}

static WKAccountDetails
wkAccountCreateFromBytesETH(
    uint8_t*    bytes,
    size_t      len )
{
    BREthereumAccount   eth;
    BRKey               ethPublicKey;

    assert (65 == len);

    BRKeySetPubKey (&ethPublicKey, bytes, len);
    eth = ethAccountCreateWithPublicKey(ethPublicKey);

    return eth;
}

static void
wkAccountReleaseETH(WKAccountDetails accountDetails) {
    ethAccountRelease ((BREthereumAccount)accountDetails);
}

static size_t
wkAccountSerializeETH(  uint8_t     *accountSerBuf,
                        WKAccount   account ) {

    assert (account != NULL);

    BREthereumAccount ethAcct;

    ethAcct = (BREthereumAccount) wkAccountAs (account,
                                               WK_NETWORK_TYPE_ETH);
    BRKey ethPublicKey = ethAccountGetPrimaryAddressPublicKey (ethAcct);

    ethPublicKey.compressed = 0;
    size_t ethSize = BRKeyPubKey (&ethPublicKey, NULL, 0);

    if (accountSerBuf != NULL) {
        BRKeyPubKey (&ethPublicKey,
                     accountSerBuf,
                     ethSize    );
    }

    return ethSize;
}

WKAccountHandlers wkAccountHandlersETH = {
    wkAccountCreateFromSeedETH,
    wkAccountCreateFromBytesETH,
    wkAccountReleaseETH,
    wkAccountSerializeETH
};

