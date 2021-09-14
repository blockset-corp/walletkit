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
#include "walletkit/WKKeyP.h"
#include "support/BRCrypto.h"

#include <stdlib.h>



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
        size_t                  msgLength,
        WKBoolean               addPrefix,
        size_t                  *digestLength,
        WKWalletConnectorStatus *status    ) {

    // No error
    *status = WK_WALLET_CONNECTOR_STATUS_OK;

    uint8_t* digest = NULL;
    uint8_t* finalMsg = NULL;

    if (addPrefix) {
        size_t prefixLen = strlen(ethereumSignedMessagePrefix);
        finalMsg = malloc (msgLength + prefixLen);
        assert (NULL != finalMsg);
        memcpy (finalMsg, ethereumSignedMessagePrefix, prefixLen);
        memcpy (finalMsg + prefixLen, msg, msgLength);
        msg = finalMsg;
        msgLength += prefixLen;
    }

    digest = malloc (ETHEREUM_HASH_BYTES);
    assert (NULL != digest);
    BRKeccak256 (digest, msg, msgLength);
    *digestLength = ETHEREUM_HASH_BYTES;

    if (NULL != finalMsg)
        free (finalMsg);

    return digest;
}

static uint8_t*
wkWalletConnectorSignDataETH (
        WKWalletConnector       walletConnector,
        const uint8_t           *data,
        size_t                  dataLength,
        WKKey                   key,
        size_t                  *signatureLength,
        WKWalletConnectorStatus *status   ) {

    BRKey *brKey = wkKeyGetCore (key);

    // No error
    *status = WK_WALLET_CONNECTOR_STATUS_OK;

    // Direct sign the data as arbitrary data
    BREthereumSignature signature = ethSignatureCreate (SIGNATURE_TYPE_RECOVERABLE_VRS_EIP,
                                                        data,
                                                        dataLength,
                                                        *brKey   );
    BRKeyClean (brKey);

    uint8_t *signatureData = malloc (sizeof(BREthereumSignatureVRS));
    assert (signatureData != NULL);
    memcpy (signatureData, &signature.sig.vrs, sizeof (BREthereumSignatureVRS));
    *signatureLength = sizeof (BREthereumSignatureVRS);

    return signatureData;
}

WKKey
wkWalletConnectorRecoverKeyETH (
        WKWalletConnector       walletConnector,
        const uint8_t           *digest,
        size_t                  digestLength,
        const uint8_t           *signature,
        size_t                  signatureLength,
        WKWalletConnectorStatus *status) {

    WKKey key = NULL;
    BRKey k;

    *status = WK_WALLET_CONNECTOR_STATUS_OK;
    if (sizeof(UInt256) != digestLength) {
        *status = WK_WALLET_CONNECTOR_STATUS_INVALID_DIGEST;
        return NULL;
    }
    if (digestLength != 65) {
        *status = WK_WALLET_CONNECTOR_STATUS_INVALID_SIGNATURE;
    }
    if (1 == BRKeyRecoverPubKey (&k, UInt256Get (digest), signature, signatureLength) ) {
        key = wkKeyCreateFromKey (&k);
    } else {
        *status = WK_WALLET_CONNECTOR_STATUS_KEY_RECOVERY_FAILED;
    }
    BRKeyClean (&k);

    return key;
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


static WKWalletConnectEthTransactionFields getTransactionFieldFromKey(const char* keyValue) {

    for (WKWalletConnectEthTransactionFields field=WK_WALLET_CONNECT_ETH_FROM;
         field < WK_WALLET_CONNECT_ETH_FIELD_MAX;
         field++ ) {

        if (strncmp(transactionFromArgsFieldNames[field],
                    keyValue,
                    strlen(transactionFromArgsFieldNames[field])) == 0)

            return field;
    }
    return WK_WALLET_CONNECT_ETH_FIELD_MAX;
}

static uint8_t*
wkWalletConnectorCreateTransactionFromArgumentsETH (
        WKWalletConnector       walletConnector,
        BRArrayOf (const char*) keys,
        BRArrayOf (const char*) values,
        size_t                  *serializationLength,
        WKWalletConnectorStatus *status         ) {

    // No error
    *status = WK_WALLET_CONNECTOR_STATUS_OK;

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

        const char *field = keys[elemNo];
        const char *value = values[elemNo];

        switch (getTransactionFieldFromKey (field)) {
            case WK_WALLET_CONNECT_ETH_FROM:

                sourceAddress = ethAddressCreate(value);
                if (ETHEREUM_BOOLEAN_FALSE ==
                    ethAddressEqual(sourceAddress, ETHEREUM_EMPTY_ADDRESS_INIT))
                    reqsMet = WK_WALLET_CONNECT_ETH_MET(reqsMet, WK_WALLET_CONNECT_ETH_FROM);
                break;

            case WK_WALLET_CONNECT_ETH_TO:

                targetAddress = ethAddressCreate (value);
                if (ETHEREUM_BOOLEAN_FALSE == ethAddressEqual(targetAddress, ETHEREUM_EMPTY_ADDRESS_INIT))
                    reqsMet = WK_WALLET_CONNECT_ETH_MET(reqsMet, WK_WALLET_CONNECT_ETH_TO);
                break;

            case WK_WALLET_CONNECT_ETH_DATA:

                data = value;
                reqsMet = WK_WALLET_CONNECT_ETH_MET(reqsMet, WK_WALLET_CONNECT_ETH_DATA);
                break;

            case WK_WALLET_CONNECT_ETH_GAS: {

                uint64_t amountOfGas = strtoull(value, NULL, 0);
                gas = ethGasCreate(amountOfGas);
                reqsMet = WK_WALLET_CONNECT_ETH_MET(reqsMet, WK_WALLET_CONNECT_ETH_GAS);
                break;
            }

            case WK_WALLET_CONNECT_ETH_GASPRICE: {

                UInt256 gasPriceInWei = uint256Create(strtoull(value, NULL, 0));
                BREthereumEther gasEthers = ethEtherCreate(gasPriceInWei);
                gasPrice = ethGasPriceCreate(gasEthers);
                reqsMet = WK_WALLET_CONNECT_ETH_MET(reqsMet, WK_WALLET_CONNECT_ETH_GASPRICE);
                break;
            }

            case WK_WALLET_CONNECT_ETH_VALUE: {

                uint64_t amountInWei = strtoull(value, NULL, 0);
                amount = ethEtherCreateNumber(amountInWei, WEI);
                reqsMet = WK_WALLET_CONNECT_ETH_MET(reqsMet, WK_WALLET_CONNECT_ETH_VALUE);
                break;
            }

            case WK_WALLET_CONNECT_ETH_NONCE:

                nonce = strtoull (value, NULL, 0);
                reqsMet = WK_WALLET_CONNECT_ETH_MET(reqsMet, WK_WALLET_CONNECT_ETH_NONCE);
                break;

            // Ignore
            default:
                break;
        }
    }

    if (!WK_WALLET_CONNECT_ETH_MANDATORY_MET (reqsMet) ) {
        *status = WK_WALLET_CONNECTOR_STATUS_INVALID_TRANSACTION_ARGUMENTS;
        return NULL;
    }

    BREthereumTransaction transaction = ethTransactionCreate(sourceAddress,
                                                             targetAddress,
                                                             amount,
                                                             gasPrice,
                                                             gas,
                                                             data,
                                                             nonce);

    // Allocates memory to rlpData for the serialization.
    BRRlpData rlpData = ethTransactionGetRlpData(transaction,
                                                 wkNetworkAsETH(wkWalletManagerGetNetwork(walletConnector->manager)),
                                                 RLP_TYPE_TRANSACTION_UNSIGNED);

    *serializationLength = rlpData.bytesCount;
    return rlpData.bytes;
}

// Will require validation of the input data adhering to serialized
// RLP transaction before this can be exposed to public data (CORE-1281)
uint8_t*
wkWalletConnectorCreateTransactionFromSerializationETH (
        WKWalletConnector       walletConnector,
        const uint8_t           *data,
        size_t                  dataLength,
        size_t                  *serializationLength,
        WKBoolean               *isSigned,
        WKWalletConnectorStatus *status) {

    WKWalletManagerETH  managerETH  = wkWalletManagerCoerceETH (walletConnector->manager);
    BREthereumNetwork   ethNetwork  = managerETH->network;

    // No result, no error
    *serializationLength = 0;
    *status = WK_WALLET_CONNECTOR_STATUS_OK;

    // Step 1: Get an ETH transaction back from serialization assuming
    //         it's unsigned. The detection and parsing of the signature, if
    //         present will be based on the chain id field (item 6). We will
    //         miss transaction hash and source address, but neither of these are
    //         important for reserialization
    BRRlpCoder              coder           = rlpCoderCreate ();
    BRRlpData               rlpData         = { .bytesCount = dataLength,
                                                .bytes      = (uint8_t*)data };

    // TODO: CORE-1281: data must be checked to adhere to Item structure
    //                  and RLP decode requirements before doing the following...
    //                  Arbitrary data in any form may cause asserts within.
    BRRlpItem               item            = rlpDataGetItem (coder, rlpData);
    BREthereumTransaction   ethTransaction  = ethTransactionRlpDecode (item,
                                                                       ethNetwork,
                                                                       RLP_TYPE_TRANSACTION_UNSIGNED,
                                                                       coder);
    rlpItemRelease  (coder, item);
    rlpCoderRelease (coder);

    // Step 2: Signing status is available on the transaction
    *isSigned = (ETHEREUM_BOOLEAN_TRUE == ethTransactionIsSigned (ethTransaction));

    // Step 3: Reserialize// Allocates memory to rlpData for the serialization.
    BRRlpData serializationData = ethTransactionGetRlpData(ethTransaction,
                                                           wkNetworkAsETH(wkWalletManagerGetNetwork(walletConnector->manager)),
                                                           (*isSigned ? RLP_TYPE_TRANSACTION_SIGNED:
                                                                        RLP_TYPE_TRANSACTION_UNSIGNED));

    *serializationLength = serializationData.bytesCount;
    return serializationData.bytes;
}

static uint8_t*
wkWalletConnectorSignTransactionDataETH (
    WKWalletConnector       walletConnector,
    const uint8_t           *transactionData,
    size_t                  dataLength,
    WKKey                   key,
    size_t                  *signedDataLength,
    WKWalletConnectorStatus *status ) {

    WKWalletManagerETH  managerETH  = wkWalletManagerCoerceETH (walletConnector->manager);
    BREthereumNetwork   ethNetwork  = managerETH->network;
    BRKey               *brKey      = wkKeyGetCore (key);
    BREthereumAccount   ethAccount  = managerETH->account;
    BREthereumAddress   ethAddress  = ethAccountGetPrimaryAddress (ethAccount);

    // No error
    *status = WK_WALLET_CONNECTOR_STATUS_OK;

    // Step 1: Deserialize RLP encoded transaction data into an ETH transaction which can be signed
    BRRlpCoder              coder           = rlpCoderCreate ();
    BRRlpData               rlpData         = { .bytesCount = dataLength,
                                                .bytes      = (uint8_t*)transactionData };
    BRRlpItem               item            = rlpDataGetItem (coder, rlpData);
    BREthereumTransaction   ethTransaction  = ethTransactionRlpDecode (item,
                                                                       ethNetwork,
                                                                       RLP_TYPE_TRANSACTION_UNSIGNED,
                                                                       coder);
    rlpItemRelease  (coder, item);
    rlpCoderRelease (coder);

    // Step 2: Create a signature directly on the input data and add it onto the
    //         ETH transaction
    BREthereumSignature signature = ethAccountSignBytesWithPrivateKey (ethAccount,
                                                                       ethAddress,
                                                                       SIGNATURE_TYPE_RECOVERABLE_VRS_EIP,
                                                                       rlpData.bytes,
                                                                       rlpData.bytesCount,
                                                                       *brKey   );
    ethTransactionSign (ethTransaction, signature);
    BRKeyClean (brKey);

    // Step 3: Add the signature to the ETH transaction and serialize it
    BRRlpData signedData = ethTransactionGetRlpData(ethTransaction,
                                                    ethNetwork,
                                                    RLP_TYPE_TRANSACTION_SIGNED);

    *signedDataLength = signedData.bytesCount;
    return signedData.bytes;
}

WKWalletConnectorHandlers wkWalletConnectorHandlersETH = {
    wkWalletConnectorCreateETH,
    wkWalletConnectorReleaseETH,
    wkWalletConnectorGetDigestETH,
    wkWalletConnectorSignDataETH,
    wkWalletConnectorRecoverKeyETH,
    wkWalletConnectorCreateTransactionFromArgumentsETH,
    wkWalletConnectorCreateTransactionFromSerializationETH,
    wkWalletConnectorSignTransactionDataETH
};

