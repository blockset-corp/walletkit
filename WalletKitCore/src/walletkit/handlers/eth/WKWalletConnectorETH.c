//
//  WKWalletConnectorETH.c
//  WalletKitCore
//
//  Created by Bryan Goring on 8/23/21.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "walletkit/WKHandlersP.h"
#include "WKETH.h"

#include "support/BRCrypto.h"


// Several RPC methods in WalletConnect 1.0 mandate a prefix as follows
// (https://docs.walletconnect.org/json-rpc-api-methods/ethereum)
static const char* ethereumSignedMessagePrefix = "\x19""Ethereum Signed Message:\n";

static WKWalletConnector
wkWalletConnectorCreateETH (WKWalletManager manager) {
    WKWalletConnector connector =
    wkWalletConnectorAllocAndInit (sizeof (struct WKWalletConnectorETHRecord),
                                   manager->type,
                                   manager);

    // ...

    return connector;
}

static void
wkWalletConnectorReleaseETH (WKWalletConnector connector) {
    // ...
    return;
}

static uint8_t*
wkWalletConnectorGetDigestETH (
        WKWalletConnector       walletConnector,
        const uint8_t           *msg,
        size_t                  msgLen,
        WKBoolean               addPrefix,
        size_t                  *digestLength,
        WKWalletConnectorError  *err    ) {

    // No error
    *err = WK_WALLET_CONNECTOR_ERROR_IS_UNDEFINED;

    uint8_t* digest = NULL;
    uint8_t* finalMsg = NULL;

    if (addPrefix) {
        size_t prefixLen = strlen(ethereumSignedMessagePrefix);
        finalMsg = malloc (msgLen + prefixLen);
        assert (NULL != finalMsg);
        memcpy (finalMsg, ethereumSignedMessagePrefix, prefixLen);
        memcpy (finalMsg + prefixLen, msg, msgLen);
        msg = finalMsg;
        msgLen += prefixLen;
    }

    digest = malloc (ETHEREUM_HASH_BYTES);
    assert (NULL != digest);
    BRKeccak256 (digest, msg, msgLen);
    *digestLength = ETHEREUM_HASH_BYTES;

    if (NULL != finalMsg)
        free (finalMsg);

    return digest;
}

static uint8_t*
wkWalletConnectorSignDataETH (
        WKWalletConnector       walletConnector,
        const uint8_t           *data,
        size_t                  dataLen,
        WKKey                   key,
        size_t                  *signatureLength,
        WKWalletConnectorError  *err    ) {

    return NULL;
}

typedef enum {
    WK_WALLET_CONNECT_ETH_FROM,
    WK_WALLET_CONNECT_ETH_TO,
    WK_WALLET_CONNECT_ETH_DATA,
    WK_WALLET_CONNECT_ETH_GAS,
    WK_WALLET_CONNECT_ETH_GASPRICE,
    WK_WALLET_CONNECT_ETH_VALUE,
    WK_WALLET_CONNECT_ETH_NONCE
} WKWalletConnectEthTransactionFields;
#define WK_WALLET_CONNECT_ETH_FIELD_MAX (WK_WALLET_CONNECT_ETH_NONCE + 1)

static const char* transactionFromArgsFieldNames[WK_WALLET_CONNECT_ETH_FIELD_MAX] = {
    "from", "to", "data", "gas", "gasPrice", "value", "nonce"
};

// Mandatory transaction from arguments fields
#define MANDATORY_FIELDS ((1 << WK_WALLET_CONNECT_ETH_FROM)       | \
                          (1 << WK_WALLET_CONNECT_ETH_TO)         | \
                          (1 << WK_WALLET_CONNECT_ETH_GAS)        | \
                          (1 << WK_WALLET_CONNECT_ETH_GASPRICE)   | \
                          (1 << WK_WALLET_CONNECT_ETH_NONCE) )
#define WK_WALLET_CONNECT_ETH_MET(met, reqmet) (met | 1 << reqmet)
#define WK_WALLET_CONNECT_ETH_MANDATORY_MET(met) ((met & MANDATORY_FIELDS) == MANDATORY_FIELDS)


static WKBoolean isTransactionKeyField(
        const char*                             fieldValue,
        WKWalletConnectEthTransactionFields     field ) {

    return strncmp (transactionFromArgsFieldNames[field],
                    fieldValue,
                    strlen(transactionFromArgsFieldNames[field])) == 0;
}

static uint8_t*
wkWalletConnectorCreateTransactionFromArgumentsETH (
        WKWalletConnector       walletConnector,
        BRArrayOf (char*)       keys,
        BRArrayOf (char*)       values,
        size_t                  *serializationLength,
        WKWalletConnectorError  *err           ) {

    // No error
    *err = WK_WALLET_CONNECTOR_ERROR_IS_UNDEFINED;

    BREthereumAddress   sourceAddress;
    BREthereumAddress   targetAddress;
    BREthereumEther     amount;
    BREthereumGasPrice  gasPrice;
    BREthereumGas       gas;
    const char*         data;
    uint64_t            nonce;

    // Permissible 'optional' field defaults:
    // w/o DATA: Assume "" or NULL
    // w/o VALUE: Assume 0
    amount = ethEtherCreateZero ();
    data = NULL;

    int reqsMet = 0;
    size_t elems = array_count (keys);
    for (size_t elemNo=0; elemNo < elems; elemNo++) {

        char *field = keys[elemNo];
        char *value = values[elemNo];

        if (isTransactionKeyField (field, WK_WALLET_CONNECT_ETH_FROM)) {

            sourceAddress = ethAddressCreate (value);
            if (ETHEREUM_BOOLEAN_FALSE == ethAddressEqual(sourceAddress, ETHEREUM_EMPTY_ADDRESS_INIT))
                reqsMet = WK_WALLET_CONNECT_ETH_MET(reqsMet, WK_WALLET_CONNECT_ETH_FROM);

        } else if (isTransactionKeyField (field, WK_WALLET_CONNECT_ETH_TO)) {

            targetAddress = ethAddressCreate (value);
            if (ETHEREUM_BOOLEAN_FALSE == ethAddressEqual(sourceAddress, ETHEREUM_EMPTY_ADDRESS_INIT))
                reqsMet = WK_WALLET_CONNECT_ETH_MET(reqsMet, WK_WALLET_CONNECT_ETH_TO);

        } else if (isTransactionKeyField(field, WK_WALLET_CONNECT_ETH_DATA)) {

            data = value;
            reqsMet = WK_WALLET_CONNECT_ETH_MET(reqsMet, WK_WALLET_CONNECT_ETH_DATA);

        } else if (isTransactionKeyField(field, WK_WALLET_CONNECT_ETH_GAS)) {

            uint64_t amountOfGas = strtoull (value, NULL, 10);
            gas = ethGasCreate (amountOfGas);
            reqsMet = WK_WALLET_CONNECT_ETH_MET(reqsMet, WK_WALLET_CONNECT_ETH_GAS);

        } else if (isTransactionKeyField(field, WK_WALLET_CONNECT_ETH_GASPRICE)) {

            UInt256 gasPriceInWei = uint256Create(strtoull (value, NULL, 10));
            BREthereumEther gasEthers = ethEtherCreate(gasPriceInWei);
            gasPrice = ethGasPriceCreate (gasEthers);
            reqsMet = WK_WALLET_CONNECT_ETH_MET(reqsMet, WK_WALLET_CONNECT_ETH_GASPRICE);

        } else if (isTransactionKeyField(field, WK_WALLET_CONNECT_ETH_VALUE)) {

            uint64_t amountInGwei = strtoull (value, NULL, 10);
            amount = ethEtherCreateNumber (amountInGwei, GWEI);
            reqsMet = WK_WALLET_CONNECT_ETH_MET(reqsMet, WK_WALLET_CONNECT_ETH_VALUE);

        } else if (isTransactionKeyField(field, WK_WALLET_CONNECT_ETH_NONCE)) {

            nonce = strtoull (value, NULL, 10);
            reqsMet = WK_WALLET_CONNECT_ETH_MET(reqsMet, WK_WALLET_CONNECT_ETH_NONCE);

        } // else ignore
    }

    if (!WK_WALLET_CONNECT_ETH_MANDATORY_MET (reqsMet) ) {
        *err = WK_WALLET_CONNECTOR_INVALID_TRANSACTION_ARGUMENTS;
        return NULL;
    }

    BREthereumTransaction transaction = ethTransactionCreate(sourceAddress,
                                                             targetAddress,
                                                             amount,
                                                             gasPrice,
                                                             gas,
                                                             data,
                                                             nonce);

    BRRlpData rlpData = ethTransactionGetRlpData(transaction,
                                                 wkNetworkAsETH(walletConnector->type),
                                                 RLP_TYPE_TRANSACTION_UNSIGNED);

    uint8_t* serializedData = malloc (rlpData.bytesCount);
    assert (serializedData != NULL);
    memcpy (serializedData, rlpData.bytes, rlpData.bytesCount);
    *serializationLength = rlpData.bytesCount;

    return serializedData;
}

uint8_t*
wkWalletConnectorCreateTransactionFromSerializationETH (
        WKWalletConnector       walletConnector,
        const uint8_t           *data,
        size_t                  dataLength,
        size_t                  *signatureLength,
        WKBoolean               *isSigned,
        WKWalletConnectorError  *err            ) {

    return NULL;
}

WKWalletConnectorHandlers wkWalletConnectorHandlersETH = {
    wkWalletConnectorCreateETH,
    wkWalletConnectorReleaseETH,
    wkWalletConnectorGetDigestETH,
    wkWalletConnectorSignDataETH,
    wkWalletConnectorCreateTransactionFromArgumentsETH,
    wkWalletConnectorCreateTransactionFromSerializationETH
};

