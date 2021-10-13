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
#include "ethereum/base/BREthereumStructure.h"
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
wkWalletConnectorCreateStandardMessageETH (
        WKWalletConnector       walletConnector,
        const uint8_t           *msg,
        size_t                  msgLength,
        size_t                  *standardMessageLength ) {

    char msgLengthStr[32];
    snprintf (msgLengthStr, 32, "%lu", msgLength);
    
    size_t prefixLen = strlen(ethereumSignedMessagePrefix);
    uint8_t *standardMessage = malloc (msgLength + strlen (msgLengthStr) + prefixLen);
    assert (NULL != standardMessage);

    memcpy (standardMessage, ethereumSignedMessagePrefix, prefixLen);
    memcpy (standardMessage + prefixLen, msgLengthStr, strlen (msgLengthStr));
    memcpy (standardMessage + prefixLen + strlen (msgLengthStr), msg, msgLength);
    *standardMessageLength = msgLength + strlen (msgLengthStr) + prefixLen;

    return standardMessage;
}

static uint8_t*
wkWalletConnectorGetDigestETH (
        WKWalletConnector       walletConnector,
        const uint8_t           *msg,
        size_t                  msgLength,
        size_t                  *digestLength,
        WKWalletConnectorStatus *status    ) {

    // No error
    *status = WK_WALLET_CONNECTOR_STATUS_OK;

    uint8_t *digest = malloc (ETHEREUM_HASH_BYTES);
    assert (NULL != digest);
    BRKeccak256 (digest, msg, msgLength);
    *digestLength = ETHEREUM_HASH_BYTES;

    return digest;
}

static WKKey
wkWalletConnectorCreateKeyFromSeedETH(
        WKWalletConnector       walletConnector,
        UInt512                 seed    ) {

    WKWalletManagerETH  managerETH  = wkWalletManagerCoerceETH (walletConnector->manager);
    BREthereumAccount   ethAccount  = managerETH->account;
    BREthereumAddress   ethAddress  = ethAccountGetPrimaryAddress (ethAccount);

    BRKey key = ethAccountDerivePrivateKeyFromSeed (seed,
                                                    ethAccountGetAddressIndex (ethAccount, ethAddress));

    return wkKeyCreateFromKey(&key);
}

static BREthereumSignatureRSV
walletConnectRsvSignatureFromVrs(BREthereumSignatureVRS vrs) {
    BREthereumSignatureRSV rsv;
    rsv.v = vrs.v;
    memcpy (rsv.r, vrs.r, sizeof (rsv.r));
    memcpy (rsv.s, vrs.s, sizeof (rsv.s));
    
    return rsv;
}

static BREthereumSignatureVRS
walletConnectVrsSignatureFromRsv(BREthereumSignatureRSV rsv) {
    BREthereumSignatureVRS vrs;
    vrs.v = rsv.v;
    memcpy (vrs.r, rsv.r, sizeof (vrs.r));
    memcpy (vrs.s, rsv.s, sizeof (vrs.s));
    
    return vrs;
}

static uint8_t*
wkWalletConnectorSignDataETH (
        WKWalletConnector       walletConnector,
        const uint8_t           *data,
        size_t                  dataLength,
        WKKey                   key,
        size_t                  *signatureLength,
        WKWalletConnectorStatus *status   ) {

    BRKey brKey = *wkKeyGetCore (key);

    // Ensure private key uncompressed
    BRKeySetCompressed(&brKey, 0);

    // No error
    *status = WK_WALLET_CONNECTOR_STATUS_OK;

    // Direct sign the data as arbitrary data
    BREthereumSignature signature = ethSignatureCreate (SIGNATURE_TYPE_RECOVERABLE_VRS_EIP,
                                                        data,
                                                        dataLength,
                                                        brKey,
                                                        NULL    );
    BRKeyClean (&brKey);
    assert (27 == signature.sig.vrs.v || 28 == signature.sig.vrs.v);  // uncompressed

    // Get the Ethereum network's chainId for EIP-155 encoding.
    WKWalletManagerETH managerETH = wkWalletManagerCoerceETH (walletConnector->manager);
    BREthereumChainId chainId = ethNetworkGetChainId(managerETH->network);

    // Update the signature `V` field with the EIP-155 encoding as per https://eips.ethereum.org/EIPS/eip-155
    //    "the v of the signature MUST be set to {0,1} + CHAIN_ID * 2 + 35 where {0,1} is the parity of y"
    // Because the VRS_EIP signing produces `v` of {27|28}, we transfer the EIP-155 encoding to:
    signature.sig.vrs.v += (8 + 2 * chainId);

    // The WalletConnect specification, apparently, wants V, R and S ordered as R, S and V.
    BREthereumSignatureRSV rsv = walletConnectRsvSignatureFromVrs(signature.sig.vrs);

    // Fill in the signature bytes.
    uint8_t *signatureData = malloc (sizeof(BREthereumSignatureRSV));
    assert (signatureData != NULL);

    memcpy (signatureData, &rsv, sizeof (BREthereumSignatureRSV));
    *signatureLength = sizeof (BREthereumSignatureRSV);

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
    if (65 != signatureLength) {
        *status = WK_WALLET_CONNECTOR_STATUS_INVALID_SIGNATURE;
    }

    // The WalletConnect specification, apparently, provides R, S and V; we want V, R and S.
    BREthereumSignatureVRS vrs = walletConnectVrsSignatureFromRsv(*((BREthereumSignatureRSV*)signature));

    // Get the Ethereum network's chainId for EIP-155 decoding.
    WKWalletManagerETH managerETH = wkWalletManagerCoerceETH (walletConnector->manager);
    BREthereumChainId chainId = ethNetworkGetChainId(managerETH->network);

    // Undo the EIP-155 encoding if it exists.  See above `wkWalletConnectorSignDataETH()`
    if (vrs.v > 28) vrs.v -= (8 + 2 * chainId);

    if (1 == BRKeyRecoverPubKey (&k, UInt256Get (digest), &vrs, signatureLength) ) {
        key = wkKeyCreateFromKey (&k);
    } else {
        *status = WK_WALLET_CONNECTOR_STATUS_KEY_RECOVERY_FAILED;
    }
    BRKeyClean (&k);

    return key;
}

typedef enum {

    // The recipient address
    WK_WALLET_CONNECT_ETH_TO,

    // Optional data of transaction
    WK_WALLET_CONNECT_ETH_DATA,

    // The amount of gas presented
    WK_WALLET_CONNECT_ETH_GAS,

    // Gas price
    WK_WALLET_CONNECT_ETH_GASPRICE,

    // Transaction amount
    WK_WALLET_CONNECT_ETH_VALUE,
} WKWalletConnectEthTransactionField;
#define WK_WALLET_CONNECT_ETH_FIELD_MAX (WK_WALLET_CONNECT_ETH_VALUE + 1)

static const char* transactionFromArgsFieldNames[WK_WALLET_CONNECT_ETH_FIELD_MAX] = {
    "to", "data", "gas", "gasPrice", "value"
};

// Mandatory transaction from arguments fields
#define MANDATORY_FIELDS ((1 << WK_WALLET_CONNECT_ETH_TO)           | \
                          (1 << WK_WALLET_CONNECT_ETH_GAS)          | \
                          (1 << WK_WALLET_CONNECT_ETH_GASPRICE) )
#define WK_WALLET_CONNECT_ETH_MET(met, reqmet) (met | 1 << reqmet)
#define WK_WALLET_CONNECT_ETH_MANDATORY_MET(met) ((met & MANDATORY_FIELDS) == MANDATORY_FIELDS)

static WKWalletConnectEthTransactionField getTransactionFieldFromKey(const char* keyValue) {

    for (WKWalletConnectEthTransactionField field=WK_WALLET_CONNECT_ETH_TO;
         field < WK_WALLET_CONNECT_ETH_FIELD_MAX;
         field++ ) {


        if ((strlen (transactionFromArgsFieldNames[field]) == strlen (keyValue)) &&
            (0 == strcmp (transactionFromArgsFieldNames[field], keyValue)) )

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

    BREthereumAddress   targetAddress;
    BREthereumEther     amount;
    BREthereumGasPrice  gasPrice;
    BREthereumGas       gas;
    const char*         data;
    uint64_t            nonce = ETHEREUM_TRANSACTION_NONCE_IS_NOT_ASSIGNED;

    // Permissible 'optional' field defaults:
    // w/o DATA: Assume "" or NULL
    // w/o VALUE: Assume 0
    //
    // Nonce is provided by walletkit, if supplied
    // by arguements, that value is ignored.
    amount = ethEtherCreateZero ();
    data = NULL;

    int reqsMet = 0;
    size_t elems = array_count (keys);
    for (size_t elemNo=0; elemNo < elems; elemNo++) {

        const char *field = keys[elemNo];
        const char *value = values[elemNo];
        WKWalletConnectEthTransactionField ethTransField = getTransactionFieldFromKey (field);

        switch (ethTransField) {

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
                reqsMet = WK_WALLET_CONNECT_ETH_MET(reqsMet, ethTransField);
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

            // Ignore: For example 'from', 'chainId', 'nonce' ...
            default:
                break;
        }
    }

    if (!WK_WALLET_CONNECT_ETH_MANDATORY_MET (reqsMet) ) {
        *status = WK_WALLET_CONNECTOR_STATUS_INVALID_TRANSACTION_ARGUMENTS;
        return NULL;
    }

    // Set the source address 'from', and 'nonce' based on the current account
    WKWalletManagerETH managerETH = wkWalletManagerCoerceETH (walletConnector->manager);
    BREthereumNetwork ethNetwork = managerETH->network;
    BREthereumAccount ethAccount = managerETH->account;
    BREthereumAddress sourceAddress = ethAccountGetPrimaryAddress (ethAccount);

    BREthereumTransaction transaction = ethTransactionCreate(sourceAddress,
                                                             targetAddress,
                                                             amount,
                                                             gasPrice,
                                                             gas,
                                                             data,
                                                             nonce);
    ethTransactionSetNonce (transaction,
                            ethAccountGetThenIncrementAddressNonce (ethAccount, sourceAddress));

    // Allocates memory to rlpData for the serialization.
    BRRlpData rlpData = ethTransactionGetRlpData(transaction,
                                                 ethNetwork,
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
    BRKey               brKey       = *wkKeyGetCore (key);
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
    //         ETH transaction. Ensure that private key is uncompressed
    BRKeySetCompressed(&brKey, 0);
    BREthereumSignature signature = ethAccountSignBytesWithPrivateKey (ethAccount,
                                                                       ethAddress,
                                                                       SIGNATURE_TYPE_RECOVERABLE_VRS_EIP,
                                                                       rlpData.bytes,
                                                                       rlpData.bytesCount,
                                                                       brKey   );
    ethTransactionSign (ethTransaction, signature);
    BRKeyClean (&brKey);

    // Step 3: Add the signature to the ETH transaction and serialize it
    BRRlpData signedData = ethTransactionGetRlpData(ethTransaction,
                                                    ethNetwork,
                                                    RLP_TYPE_TRANSACTION_SIGNED);

    *signedDataLength = signedData.bytesCount;
    return signedData.bytes;
}

static uint8_t*
wkWalletConnectorSignTypedDataETH (
        WKWalletConnector       connector,
        BRJson                  typedData,
        WKKey                   key,
        uint8_t                 **digestData,
        size_t                  *digestLength,
        size_t                  *signatureLength,
        WKWalletConnectorStatus *status) {
    
    uint8_t*                        signatureData = NULL;
    BRKey                           brKey;
    BREthereumStructureErrorType    error;

    *status = WK_WALLET_CONNECTOR_STATUS_OK;
    *digestLength = 0;
    *signatureLength = 0;
    *digestData = NULL;
    
    brKey = *wkKeyGetCore (key);
    BREthereumStructureCoder coder = ethStructureCoderCreateFromTypedData (typedData, &error);
    if (NULL == coder) {
        *status = WK_WALLET_CONNECTOR_STATUS_INVALID_TYPED_DATA;
        return NULL;
    }

    BREthereumStructureSignResult signResult = ethStructureSignData (coder, brKey);
    BRKeyClean (&brKey);
    
    *digestLength = sizeof (BREthereumHash);
    *signatureLength = sizeof (BREthereumSignatureVRS);
    *digestData = malloc (*digestLength);
    signatureData = malloc (*signatureLength);
    assert (NULL != *digestData && NULL != signatureData);

    // Signature type, SIGNATURE_TYPE_RECOVERABLE_VRS_EIP
    memcpy (*digestData, signResult.digest.bytes, *digestLength);
    memcpy (signatureData, &signResult.signature.sig.vrs, *signatureLength);

    return signatureData;
}

WKWalletConnectorHandlers wkWalletConnectorHandlersETH = {
    wkWalletConnectorCreateETH,
    wkWalletConnectorReleaseETH,
    wkWalletConnectorCreateStandardMessageETH,
    wkWalletConnectorGetDigestETH,
    wkWalletConnectorCreateKeyFromSeedETH,
    wkWalletConnectorSignDataETH,
    wkWalletConnectorRecoverKeyETH,
    wkWalletConnectorCreateTransactionFromArgumentsETH,
    wkWalletConnectorCreateTransactionFromSerializationETH,
    wkWalletConnectorSignTransactionDataETH,
    wkWalletConnectorSignTypedDataETH
};

