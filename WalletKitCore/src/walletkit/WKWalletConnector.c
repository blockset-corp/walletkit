//
//  WKWalletConnector.c
//  WalletKitCore
//
//  Created by Bryan Goring on 8/23/21
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "WKWalletConnectorP.h"
#include "WKHandlersP.h"


/** Creates a shallow copy of strings in the input strs 'WordList' into
 *  a BRArray of c-strings.
 */
static BRArrayOf(char*)
arrayOfStringFromWordlist(
    char    **strs,
    size_t  stringsCount    ) {

    BRArrayOf(char*) arrayOfStrs;

    array_new(arrayOfStrs, stringsCount);
    array_add_array(arrayOfStrs, strs, stringsCount);
    return arrayOfStrs;
}

private_extern WKWalletConnector
wkWalletConnectorAllocAndInit (size_t sizeInBytes,
                               WKNetworkType type,
                               WKWalletManager manager) {
    WKWalletConnector connector = calloc (1, sizeof (struct WKWalletConnectorRecord));

    connector->type = type;
    connector->handlers = wkHandlersLookup(type)->connector;
    connector->sizeInBytes = sizeInBytes;

    connector->manager = wkWalletManagerTake (manager);
    
    return connector;
}

extern WKWalletConnector
wkWalletConnectorCreate (WKWalletManager manager) {
    const WKHandlers *handlers = wkHandlersLookup (manager->type);

    return (NULL != handlers &&
            NULL != handlers->connector &&
            NULL != handlers->connector->create
            ? handlers->connector->create (manager)
            : NULL);
}

extern void
wkWalletConnectorRelease (WKWalletConnector connector) {
    if (NULL != connector->handlers->release)
        connector->handlers->release (connector);

    wkWalletManagerGive (connector->manager);
    memset (connector, 0, connector->sizeInBytes);
    free (connector);
}

extern uint8_t*
wkWalletConnectorGetDigest (
        WKWalletConnector       connector,
        const uint8_t           *msg,
        size_t                  msgLen,
        WKBoolean               addPrefix,
        size_t                  *digestLength,
        WKWalletConnectorError  *err            ) {

    assert (NULL != connector       &&
            NULL != msg             &&
            NULL != digestLength    &&
            NULL != err );

    uint8_t* digest = NULL;

    *err = WK_WALLET_CONNECTOR_ERROR_IS_UNDEFINED;
    const WKHandlers *netHandlers = wkHandlersLookup(connector->type);

    if (NULL != netHandlers             &&
        NULL != netHandlers->connector  &&
        NULL != netHandlers->connector->getDigest) {

        digest =  netHandlers->connector->getDigest(connector,
                                                    msg,
                                                    msgLen,
                                                    addPrefix,
                                                    digestLength,
                                                    err);
    } else {
        *err = WK_WALLET_CONNECTOR_ILLEGAL_OPERATION;
    }
    return digest;
}

extern uint8_t*
wkWalletConnectorSignData   (
        WKWalletConnector         connector,
        const uint8_t             *data,
        size_t                    dataLen,
        WKKey                     key,
        size_t                    *signedLength,
        WKWalletConnectorError    *err            ) {

    assert (NULL != connector       &&
            NULL != data            &&
            NULL != key             &&
            NULL != signedLength &&
            NULL != err );

    uint8_t* signedData = NULL;

    *err = WK_WALLET_CONNECTOR_ERROR_IS_UNDEFINED;
    const WKHandlers *netHandlers = wkHandlersLookup(connector->type);

    if (NULL != netHandlers             &&
        NULL != netHandlers->connector  &&
        NULL != netHandlers->connector->sign) {

        signedData =  netHandlers->connector->sign(connector,
                                                   data,
                                                   dataLen,
                                                   key,
                                                   signedLength,
                                                   err);
    } else {
        *err = WK_WALLET_CONNECTOR_ILLEGAL_OPERATION;
    }
    return signedData;
}

extern uint8_t*
wkWalletConnectorCreateTransactionFromArguments  (
        WKWalletConnector         connector,
        const char                **keys,
        const char                **values,
        size_t                    keyValuePairsCount,
        size_t                    *serializationLength,
        WKWalletConnectorError    *err            ) {

    assert (NULL != connector           &&
            NULL != keys                &&
            NULL != values              &&
            NULL != serializationLength &&
            NULL != err );

    uint8_t* unsignedTransaction = NULL;

    // Transform the 'WordList' based keys and values into BRArrayOf which is more
    // natural for handlers.
    BRArrayOf(char*) arrayOfKeys = arrayOfStringFromWordlist(keys, keyValuePairsCount);
    BRArrayOf(char*) arrayOfValues = arrayOfStringFromWordlist(values, keyValuePairsCount);

    *err = WK_WALLET_CONNECTOR_ERROR_IS_UNDEFINED;
    const WKHandlers *netHandlers = wkHandlersLookup(connector->type);

    if (NULL != netHandlers             &&
        NULL != netHandlers->connector  &&
        NULL != netHandlers->connector->createTransactionFromArguments) {

        unsignedTransaction =  netHandlers->connector->createTransactionFromArguments(
                connector,
                keys,
                values,
                serializationLength,
                err);
    } else {
        *err = WK_WALLET_CONNECTOR_ILLEGAL_OPERATION;
    }

    array_free (arrayOfKeys);
    array_free (arrayOfValues);

    return unsignedTransaction;
}

extern uint8_t*
wkWalletConnectorCreateTransactionFromSerialization  (
        WKWalletConnector       connector,
        uint8_t                 *data,
        size_t                  dataLength,
        size_t                  *serializationLength,
        WKBoolean               *isSigned,
        WKWalletConnectorError  *err           ) {

    assert (NULL != connector           &&
            NULL == data                &&
            NULL == serializationLength &&
            NULL == isSigned            &&
            NULL == err );

    uint8_t* transaction = NULL;

    *err = WK_WALLET_CONNECTOR_ERROR_IS_UNDEFINED;
    const WKHandlers *netHandlers = wkHandlersLookup(connector->type);

    if (NULL != netHandlers             &&
        NULL != netHandlers->connector  &&
        NULL != netHandlers->connector->createTransactionFromArguments) {

        transaction =  netHandlers->connector->createTransactionFromSerialization(
                connector,
                data,
                dataLength,
                serializationLength,
                isSigned,
                err);
    } else {
        *err = WK_WALLET_CONNECTOR_ILLEGAL_OPERATION;
    }
    return transaction;
}
