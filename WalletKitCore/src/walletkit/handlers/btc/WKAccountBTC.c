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
wkAccountCreateSeedInternal(
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
wkAccountCreateFromBytesBTC(
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
wkAccountSerializeInternal(
    BRMasterPubKey  masterPubKey,
    uint8_t         *accountSerBuf ) {

    size_t keySerSize = BRBIP32SerializeMasterPubKey (NULL, masterPubKey);

    if (accountSerBuf != NULL) {
        BRBIP32SerializeMasterPubKey ((char*) accountSerBuf, masterPubKey);
    }

    return keySerSize;
}

// BTC & BTC/BCH/BSV shared account handling functions

static WKAccountDetails
wkAccountCreateFromSeedBTC(
    WKBoolean   isMainnet,
    UInt512     seed    ) {

    BRMasterPubKey *pubKey = (BRMasterPubKey*) calloc (1, sizeof(BRMasterPubKey));
    assert (pubKey != NULL);

    if (isMainnet)
        *pubKey = BRBIP32MasterPubKey (seed.u8, sizeof (seed.u8));
    else
        *pubKey = BRBIP32MasterPubKeyPath(seed.u8, 
                                          sizeof (seed.u8), 
                                          BITCOIN_BIP32_DEPTH_TEST, 
                                          BITCOIN_BIP32_CHILD_TEST);

    return pubKey;
}

static void
wkAccountReleaseBTC(WKAccountDetails accountDetails) {
    // Cast only for clarity what account details we are concerned with
    free ((BRMasterPubKey*)accountDetails);
}
    
static size_t
wkAccountSerializeBTC(
    uint8_t     *accountSerBuf,
    WKAccount   account         ) {

    assert (account != NULL);

    BRMasterPubKey* masterKey;

    masterKey = (BRMasterPubKey*) wkAccountAs(account,
                                              WK_NETWORK_TYPE_BTC);

    return wkAccountSerializeInternal ( *masterKey,
                                        accountSerBuf );
}

// BCH specific account handling functions

static size_t
wkAccountSerializeBCH(
    uint8_t     *accountSerBuf,
    WKAccount   account         ) {

    assert (account != NULL);

    BRMasterPubKey* masterKey = (BRMasterPubKey*) wkAccountAs(account,
                                                              WK_NETWORK_TYPE_BCH);

    return wkAccountSerializeInternal ( *masterKey,
                                        accountSerBuf );
}

// BSV specific account handling functions

static size_t
wkAccountSerializeBSV(
    uint8_t     *accountSerBuf,
    WKAccount   account         ) {

    assert (account != NULL);

    BRMasterPubKey* masterKey= (BRMasterPubKey*) wkAccountAs(account,
                                                             WK_NETWORK_TYPE_BSV);

    return wkAccountSerializeInternal ( *masterKey,
                                        accountSerBuf );
}

// LTC specific account handling functions

static size_t
wkAccountSerializeLTC(
    uint8_t     *accountSerBuf,
    WKAccount   account         ) {

    assert (account != NULL);

    BRMasterPubKey* masterKey = (BRMasterPubKey*) wkAccountAs (account,
                                                               WK_NETWORK_TYPE_LTC);

    return wkAccountSerializeInternal (*masterKey,
                                       accountSerBuf );
}

static WKAccountDetails
wkAccountCreateFromSeedLTC(
    WKBoolean   isMainnet,
    UInt512     seed    ) {

    return wkAccountCreateSeedInternal(seed,
                                       LTC_BIP32_DEPTH,
                                       LTC_BIP32_CHILD );
}

// DOGE specific account handling functions

static size_t
wkAccountSerializeDOGE(
    uint8_t     *accountSerBuf,
    WKAccount   account         ) {

    assert (account != NULL);

    BRMasterPubKey* masterKey = (BRMasterPubKey*) wkAccountAs(account,
                                                              WK_NETWORK_TYPE_DOGE);

    return wkAccountSerializeInternal (*masterKey,
                                       accountSerBuf );
}

static WKAccountDetails
wkAccountCreateFromSeedDOGE(
    WKBoolean   isMainnet,
    UInt512     seed    ) {

    return wkAccountCreateSeedInternal(seed,
                                       DOGE_BIP32_DEPTH,
                                       DOGE_BIP32_CHILD );
}

// Handlers are functionally equivalent for each of BTC/BCH/BSV
WKAccountHandlers wkAccountHandlersBTC = {
    wkAccountCreateFromSeedBTC,
    wkAccountCreateFromBytesBTC,
    wkAccountReleaseBTC,
    wkAccountSerializeBTC
};

WKAccountHandlers wkAccountHandlersBCH = {
    wkAccountCreateFromSeedBTC,
    wkAccountCreateFromBytesBTC,
    wkAccountReleaseBTC,
    wkAccountSerializeBCH
};

WKAccountHandlers wkAccountHandlersBSV= {
    wkAccountCreateFromSeedBTC,
    wkAccountCreateFromBytesBTC,
    wkAccountReleaseBTC,
    wkAccountSerializeBSV
};

WKAccountHandlers wkAccountHandlersLTC = {
    wkAccountCreateFromSeedLTC,
    wkAccountCreateFromBytesBTC,
    wkAccountReleaseBTC,
    wkAccountSerializeLTC
};

WKAccountHandlers wkAccountHandlersDOGE = {
    wkAccountCreateFromSeedDOGE,
    wkAccountCreateFromBytesBTC,
    wkAccountReleaseBTC,
    wkAccountSerializeDOGE
};

