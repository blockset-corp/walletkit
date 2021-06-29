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
    uint8_t         *accountSerBuf ) {

    size_t keySerSize = BRBIP32SerializeMasterPubKey (NULL, masterPubKey);

    if (accountSerBuf != NULL) {
        BRBIP32SerializeMasterPubKey ((char*) accountSerBuf, masterPubKey);
    }

    return keySerSize;
}

static void
wkAccountReleaseCommon(
    WKAccount       account,
    WKNetworkType   netType) {

    BRMasterPubKey* pubKey;

    pubKey = (BRMasterPubKey*) wkAccountAs (account,
                                            netType);
    
    free (pubKey);
}

// BTC & BTC/BCH/BSV shared account handling functions

static WKAccountDetails
wkAccountCreateFromSeedBTC(UInt512  seed) {

    BRMasterPubKey *pubKey = (BRMasterPubKey*) calloc (1, sizeof(BRMasterPubKey));
    assert (pubKey != NULL);

    *pubKey = BRBIP32MasterPubKey (seed.u8, sizeof (seed.u8));

    return pubKey;
}

static void
wkAccountReleaseBTC(WKAccount account) {
    wkAccountReleaseCommon (account, WK_NETWORK_TYPE_BTC);
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
                                      accountSerBuf );
}

// BCH specific account handling functions

static void
wkAccountReleaseBCH(WKAccount account) {
    wkAccountReleaseCommon (account, WK_NETWORK_TYPE_BCH);
}

static size_t
wkAccountSerializeBCH(
    uint8_t     *accountSerBuf,
    WKAccount   account         ) {

    assert (account != NULL);

    BRMasterPubKey* masterKey = (BRMasterPubKey*) wkAccountAs(account,
                                                              WK_NETWORK_TYPE_BCH);

    return wkAccountSerializeCommon ( *masterKey,
                                      accountSerBuf );
}

// BSV specific account handling functions

static void
wkAccountReleaseBSV(WKAccount account) {
    wkAccountReleaseCommon (account, WK_NETWORK_TYPE_BSV);
}

static size_t
wkAccountSerializeBSV(
    uint8_t     *accountSerBuf,
    WKAccount   account         ) {

    assert (account != NULL);

    BRMasterPubKey* masterKey= (BRMasterPubKey*) wkAccountAs(account,
                                                             WK_NETWORK_TYPE_BSV);

    return wkAccountSerializeCommon ( *masterKey,
                                      accountSerBuf );
}

// LTC specific account handling functions

static void
wkAccountReleaseLTC(WKAccount account) {
    wkAccountReleaseCommon (account, WK_NETWORK_TYPE_LTC);
}

static size_t
wkAccountSerializeLTC(
    uint8_t     *accountSerBuf,
    WKAccount   account         ) {

    assert (account != NULL);

    BRMasterPubKey* masterKey = (BRMasterPubKey*) wkAccountAs (account,
                                                               WK_NETWORK_TYPE_LTC);

    return wkAccountSerializeCommon(*masterKey,
                                    accountSerBuf );
}

static WKAccountDetails
wkAccountCreateFromSeedLTC(UInt512  seed) {

    return wkAccountCreateSeedCommon(seed,
                                     LTC_BIP32_DEPTH,
                                     LTC_BIP32_CHILD );
}

// DOGE specific account handling functions

static void
wkAccountReleaseDOGE(WKAccount account) {
    wkAccountReleaseCommon (account, WK_NETWORK_TYPE_DOGE);
}

static size_t
wkAccountSerializeDOGE(
    uint8_t     *accountSerBuf,
    WKAccount   account         ) {

    assert (account != NULL);

    BRMasterPubKey* masterKey = (BRMasterPubKey*) wkAccountAs(account,
                                                              WK_NETWORK_TYPE_DOGE);

    return wkAccountSerializeCommon(*masterKey,
                                    accountSerBuf );
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
    wkAccountReleaseBTC,
    wkAccountSerializeBTC
};

WKAccountHandlers wkAccountHandlersBCH = {
    wkAccountCreateFromSeedBTC,
    wkAccountCreateFromBytesCommon,
    wkAccountReleaseBCH,
    wkAccountSerializeBCH
};

WKAccountHandlers wkAccountHandlersBSV= {
    wkAccountCreateFromSeedBTC,
    wkAccountCreateFromBytesCommon,
    wkAccountReleaseBSV,
    wkAccountSerializeBSV
};

WKAccountHandlers wkAccountHandlersLTC = {
    wkAccountCreateFromSeedLTC,
    wkAccountCreateFromBytesCommon,
    wkAccountReleaseLTC,
    wkAccountSerializeLTC
};

WKAccountHandlers wkAccountHandlersDOGE = {
    wkAccountCreateFromSeedDOGE,
    wkAccountCreateFromBytesCommon,
    wkAccountReleaseDOGE,
    wkAccountSerializeDOGE
};

