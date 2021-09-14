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
static BRArrayOf(const char*)
arrayOfStringFromWordlist(
    const char  **strs,
    size_t      stringsCount    ) {

    BRArrayOf(const char*) arrayOfStrs;

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
wkWalletConnectorCreateStandardMessage (
        WKWalletConnector       connector,
        const uint8_t           *msg,
        size_t                  msgLen,
        size_t                  *standardMessageLength,
        WKWalletConnectorStatus *status            ) {

    assert (NULL != connector               &&
            NULL != msg                     &&
            NULL != standardMessageLength   );

    uint8_t* standardMessage = NULL;

    *status = WK_WALLET_CONNECTOR_STATUS_OK;
    const WKHandlers *netHandlers = wkHandlersLookup(connector->type);

    if (NULL != netHandlers             &&
        NULL != netHandlers->connector  &&
        NULL != netHandlers->connector->getDigest) {

        standardMessage =  netHandlers->connector->createStandardMessage(connector,
                                                                         msg,
                                                                         msgLen,
                                                                         standardMessageLength );
    } else {
        *status = WK_WALLET_CONNECTOR_STATUS_ILLEGAL_OPERATION;
    }

    return standardMessage;
}

extern uint8_t*
wkWalletConnectorGetDigest (
        WKWalletConnector       connector,
        const uint8_t           *msg,
        size_t                  msgLen,
        size_t                  *digestLength,
        WKWalletConnectorStatus *status            ) {

    assert (NULL != connector       &&
            NULL != msg             &&
            NULL != digestLength    &&
            NULL != status );

    uint8_t* digest = NULL;

    *status = WK_WALLET_CONNECTOR_STATUS_OK;
    const WKHandlers *netHandlers = wkHandlersLookup(connector->type);

    if (NULL != netHandlers             &&
        NULL != netHandlers->connector  &&
        NULL != netHandlers->connector->getDigest) {

        digest =  netHandlers->connector->getDigest(connector,
                                                    msg,
                                                    msgLen,
                                                    digestLength,
                                                    status);

        if (NULL == digest && WK_WALLET_CONNECTOR_STATUS_OK == *status)
            *status = WK_WALLET_CONNECTOR_STATUS_INVALID_DIGEST;

    } else {
        *status = WK_WALLET_CONNECTOR_STATUS_ILLEGAL_OPERATION;
    }
    return digest;
}

extern WKKey
wkWalletConnectorCreateKey (
        WKWalletConnector           connector,
        const char                  *phrase,
        WKWalletConnectorStatus     *status           ) {

    assert (NULL != connector &&
            NULL != phrase && strlen(phrase) > 0);

    WKKey key = NULL;

    *status = WK_WALLET_CONNECTOR_STATUS_OK;
    const WKHandlers *netHandlers = wkHandlersLookup(connector->type);

    if (NULL != netHandlers             &&
        NULL != netHandlers->connector  &&
        NULL != netHandlers->connector->createKeyFromSeed) {

        UInt512 seed = wkAccountDeriveSeed(phrase);
        key =  netHandlers->connector->createKeyFromSeed(connector,
                                                         seed);

    } else {
        *status = WK_WALLET_CONNECTOR_STATUS_ILLEGAL_OPERATION;
    }
    return key;
}

extern uint8_t*
wkWalletConnectorSignData   (
        WKWalletConnector         connector,
        const uint8_t             *data,
        size_t                    dataLen,
        WKKey                     key,
        size_t                    *signatureLength,
        WKWalletConnectorStatus  *status            ) {

    assert (NULL != connector       &&
            NULL != data            &&
            NULL != key             &&
            NULL != signatureLength &&
            NULL != status );

    uint8_t* signedData = NULL;

    *status = WK_WALLET_CONNECTOR_STATUS_OK;
    const WKHandlers *netHandlers = wkHandlersLookup(connector->type);

    if (NULL != netHandlers             &&
        NULL != netHandlers->connector  &&
        NULL != netHandlers->connector->sign) {

        signedData =  netHandlers->connector->sign(connector,
                                                   data,
                                                   dataLen,
                                                   key,
                                                   signatureLength,
                                                   status);

        if (NULL == signedData && WK_WALLET_CONNECTOR_STATUS_OK == *status)
            *status = WK_WALLET_CONNECTOR_STATUS_INVALID_SIGNATURE;

    } else {
        *status = WK_WALLET_CONNECTOR_STATUS_ILLEGAL_OPERATION;
    }
    return signedData;
}

extern WKKey
wkWalletConnectorRecoverKey (
        WKWalletConnector       connector,
        const uint8_t           *digest,
        size_t                  digestLength,
        const uint8_t           *signature,
        size_t                  signatureLength,
        WKWalletConnectorStatus *status         ) {

    assert (NULL != connector       &&
            NULL != digest          &&
            NULL != signature       &&
            NULL != status );

    WKKey pubKey = NULL;

    *status = WK_WALLET_CONNECTOR_STATUS_OK;
    const WKHandlers *netHandlers = wkHandlersLookup(connector->type);

    if (NULL != netHandlers             &&
        NULL != netHandlers->connector  &&
        NULL != netHandlers->connector->recover) {

        pubKey =  netHandlers->connector->recover(connector,
                                                  digest,
                                                  digestLength,
                                                  signature,
                                                  signatureLength,
                                                  status);

        if (NULL == pubKey && WK_WALLET_CONNECTOR_STATUS_OK == *status)
            *status = WK_WALLET_CONNECTOR_STATUS_KEY_RECOVERY_FAILED;

    } else {
        *status = WK_WALLET_CONNECTOR_STATUS_ILLEGAL_OPERATION;
    }
    return pubKey;
}

extern uint8_t*
wkWalletConnectorCreateTransactionFromArguments  (
        WKWalletConnector         connector,
        const char                **keys,
        const char                **values,
        size_t                    keyValuePairsCount,
        size_t                    *serializationLength,
        WKWalletConnectorStatus *status            ) {

    assert (NULL != connector           &&
            NULL != keys                &&
            NULL != values              &&
            NULL != serializationLength &&
            NULL != status );

    uint8_t* unsignedTransaction = NULL;

    // Transform the 'WordList' based keys and values into BRArrayOf which is more
    // natural for handlers.
    BRArrayOf(const char*) arrayOfKeys = arrayOfStringFromWordlist(keys, keyValuePairsCount);
    BRArrayOf(const char*) arrayOfValues = arrayOfStringFromWordlist(values, keyValuePairsCount);

    *status = WK_WALLET_CONNECTOR_STATUS_OK;
    const WKHandlers *netHandlers = wkHandlersLookup(connector->type);

    if (NULL != netHandlers             &&
        NULL != netHandlers->connector  &&
        NULL != netHandlers->connector->createTransactionFromArguments) {

        unsignedTransaction =  netHandlers->connector->createTransactionFromArguments(
                connector,
                arrayOfKeys,
                arrayOfValues,
                serializationLength,
                status);

        if (NULL == unsignedTransaction && WK_WALLET_CONNECTOR_STATUS_OK == *status)
            *status = WK_WALLET_CONNECTOR_STATUS_INVALID_SERIALIZATION;

    } else {
        *status = WK_WALLET_CONNECTOR_STATUS_ILLEGAL_OPERATION;
    }

    array_free (arrayOfKeys);
    array_free (arrayOfValues);

    return unsignedTransaction;
}

extern uint8_t*
wkWalletConnectorCreateTransactionFromSerialization  (
        WKWalletConnector       connector,
        const uint8_t           *data,
        size_t                  dataLength,
        size_t                  *serializationLength,
        WKBoolean               *isSigned,
        WKWalletConnectorStatus *status           ) {

    assert (NULL != connector           &&
            NULL == data                &&
            NULL == serializationLength &&
            NULL == isSigned            &&
            NULL == status );

    uint8_t* transaction = NULL;

    *status = WK_WALLET_CONNECTOR_STATUS_OK;
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
                status);

        if (NULL == transaction && WK_WALLET_CONNECTOR_STATUS_OK == *status)
            *status = WK_WALLET_CONNECTOR_STATUS_INVALID_SERIALIZATION;

    } else {
        *status = WK_WALLET_CONNECTOR_STATUS_ILLEGAL_OPERATION;
    }
    return transaction;
}

extern uint8_t*
wkWalletConnectorSignTransactionData (
        WKWalletConnector       connector,
        const uint8_t           *transactionData,
        size_t                  dataLength,
        WKKey                   key,
        size_t                  *signedDataLength,
        WKWalletConnectorStatus *status            ) {

    assert (NULL != connector           &&
            NULL == transactionData     &&
            NULL == signedDataLength    &&
            NULL == status );

        uint8_t* transaction = NULL;

        *status = WK_WALLET_CONNECTOR_STATUS_OK;
        const WKHandlers *netHandlers = wkHandlersLookup(connector->type);

        if (NULL != netHandlers             &&
            NULL != netHandlers->connector  &&
            NULL != netHandlers->connector->signTransactionData) {

            transaction =  netHandlers->connector->signTransactionData(
                    connector,
                    transactionData,
                    dataLength,
                    key,
                    signedDataLength,
                    status);

            if (NULL == transaction && WK_WALLET_CONNECTOR_STATUS_OK == *status)
                *status = WK_WALLET_CONNECTOR_STATUS_INVALID_SIGNATURE;

        } else {
            *status = WK_WALLET_CONNECTOR_STATUS_ILLEGAL_OPERATION;
        }
        return transaction;
}
