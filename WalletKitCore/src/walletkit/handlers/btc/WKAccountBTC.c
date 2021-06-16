//
//  WKAccountBTC.c
//  WalletKitCore
//
//  Created by Bryan Goring on 06/15/2021.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKBTC.h"
#include "litecoin/BRLitecoinParams.h" // For LTC_BIP32_DEPTH & CHILD
#include "dogecoin/BRDogecoinParams.h" // For DOGE_BIP32_DEPTH & CHILD

static WKAccountDetails
wkAccountCreateSeedCommon(
    UInt512         seed,
    int             depth,
    const uint32_t  child[] ) {

    BRMasterPubKey *pubKey = (BRMasterPubKey*) calloc (1, sizeof(BRMasterPubKey));
    assert (pubKey != NULL);

    *pubKey = BRBIP32MasterPubKeyPath(seed.u8,
                                      sizeof (seed.u8),
                                      depth,
                                      child    );

    return pubKey;
}

static WKAccountDetails
wkAccountCreateFromBytesCommon(
    uint8_t*    bytes,
    size_t      len    ) {

    BRMasterPubKey  *pubKey;

    pubKey = (BRMasterPubKey*) calloc (1, sizeof(BRMasterPubKey));
    assert (pubKey != NULL);

    // There is a slight chance that this fails IF AND ONLY IF the serialized format
    // of a MasterPublicKey either changes or is key dependent.  That is, we parse the MPK
    // from `bytes` but if THIS PARSE needs more than the original parse we might run
    // off the end of the provided `bytes`.  Must be REALLY UNLIKELY.
    //
    // TODO: Add `acctsSerSize` to BRBIP32ParseMasterPubKey()

    *pubKey = BRBIP32ParseMasterPubKey ((const char *) bytes);
    if (len != BRBIP32SerializeMasterPubKey (NULL, *pubKey)) {

        free (pubKey);
        return NULL;
    }

    return pubKey;
}

static size_t
wkAccountSerializeCommon(
    BRMasterPubKey  masterPubKey,
    uint8_t         *accountSerBuf,
    WKAccount       account         ) {

    size_t keySerSize = BRBIP32SerializeMasterPubKey (NULL, masterPubKey);

    if (accountSerBuf != NULL) {
        BRBIP32SerializeMasterPubKey ((char*) accountSerBuf, masterPubKey);
    }

    return keySerSize;
}

static void
wkAccountReleaseCommon(WKAccount account) {

    BRMasterPubKey* pubKey;

    pubKey = (BRMasterPubKey*) wkAccountAs (account,
                                            WK_NETWORK_TYPE_BTC);
    free (pubKey);
}

// BTC account handling functions

static WKAccountDetails
wkAccountCreateFromSeedBTC(UInt512  seed) {

    BRMasterPubKey *pubKey = (BRMasterPubKey*) calloc (1, sizeof(BRMasterPubKey));
    assert (pubKey != NULL);

    *pubKey = BRBIP32MasterPubKey (seed.u8, sizeof (seed.u8));

    return pubKey;
}

static size_t
wkAccountSerializeBTC(
    uint8_t     *accountSerBuf,
    WKAccount   account         ) {

    assert (account != NULL);

    BRMasterPubKey* masterKey;

    masterKey = (BRMasterPubKey*) wkAccountAs(account,
                                              WK_NETWORK_TYPE_BTC);

    return wkAccountSerializeCommon ( *masterKey,
                                      accountSerBuf,
                                      account   );
}

// LTC specific account handling functions

static size_t
wkAccountSerializeLTC(
    uint8_t     *accountSerBuf,
    WKAccount   account         ) {

    assert (account != NULL);

    BRMasterPubKey* masterKey = (BRMasterPubKey*) wkAccountAs (account,
                                                               WK_NETWORK_TYPE_LTC);

    return wkAccountSerializeCommon(*masterKey,
                                    accountSerBuf,
                                    account );
}

static WKAccountDetails
wkAccountCreateFromSeedLTC(UInt512  seed) {

    return wkAccountCreateSeedCommon(seed,
                                     LTC_BIP32_DEPTH,
                                     LTC_BIP32_CHILD );
}

// DOGE specific account handling functions

static size_t
wkAccountSerializeDOGE(
    uint8_t     *accountSerBuf,
    WKAccount   account         ) {

    assert (account != NULL);

    BRMasterPubKey* masterKey;

    masterKey = (BRMasterPubKey*) wkAccountAs(account,
                                              WK_NETWORK_TYPE_DOGE);

    return wkAccountSerializeCommon(*masterKey,
                                    accountSerBuf,
                                    account );
}

static WKAccountDetails
wkAccountCreateFromSeedDOGE(UInt512  seed) {

    return wkAccountCreateSeedCommon(seed,
                                     DOGE_BIP32_DEPTH,
                                     DOGE_BIP32_CHILD );
}

// Handlers are functionally equivalent for each of BTC/BCH/BSV
WKAccountHandlers wkAccountHandlersBTC = {
    wkAccountCreateFromSeedBTC,
    wkAccountCreateFromBytesCommon,
    wkAccountReleaseCommon,
    wkAccountSerializeBTC
};

WKAccountHandlers wkAccountHandlersBCH = {
    wkAccountCreateFromSeedBTC,
    wkAccountCreateFromBytesCommon,
    wkAccountReleaseCommon,
    wkAccountSerializeBTC
};

WKAccountHandlers wkAccountHandlersBSV= {
    wkAccountCreateFromSeedBTC,
    wkAccountCreateFromBytesCommon,
    wkAccountReleaseCommon,
    wkAccountSerializeBTC
};

WKAccountHandlers wkAccountHandlersLTC = {
    wkAccountCreateFromSeedLTC,
    wkAccountCreateFromBytesCommon,
    wkAccountReleaseCommon,
    wkAccountSerializeLTC
};

WKAccountHandlers wkAccountHandlersDOGE = {
    wkAccountCreateFromSeedDOGE,
    wkAccountCreateFromBytesCommon,
    wkAccountReleaseCommon,
    wkAccountSerializeDOGE
};

