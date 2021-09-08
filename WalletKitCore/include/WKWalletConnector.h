//
//  WKWalletConnector.h
//  WalletKitCore
//
//  Created by Bryan Goring on 8/23/21
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKWalletConnector_h
#define WKWalletConnector_h

#include "WKWalletManager.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {

    // TBD
    WK_WALLET_CONNECTOR_ERROR_UNSUPPORTED_CONNECTOR,

    // Crucial setup has not been done properly, one of
    // network handlers, WalletConnect handlers, or individual
    // handler methods for this particular network are not defined.
    WK_WALLET_CONNECTOR_ILLEGAL_OPERATION,

    // One or more transaction arguments key-value pairs are not
    // provided, or values are invalid
    WK_WALLET_CONNECTOR_INVALID_TRANSACTION_ARGUMENTS,

    // ...

} WKWalletConnectorError;

/// @brief An undefined WalletConnect error is no error at all (ie Success)
#define WK_WALLET_CONNECTOR_ERROR_IS_UNDEFINED  ((WKWalletConnectorError) -1)

typedef struct WKWalletConnectorRecord *WKWalletConnector;

extern WKWalletConnector
wkWalletConnectorCreate (WKWalletManager manager);

extern void
wkWalletConnectorRelease (WKWalletConnector connector);

/* Use the wallet connector to generate a digest of the provided message.
 *
 * @param connector The wallet connector object
 * @param msg The input message bytes for which to create the digest
 * @param msgLen The length of input message
 * @param addPrefix A flag to indicate prepending an optional prefix known to the network
 * @param digestLength The length of digest created (on success > 0)
 * @param err An error when a failure has occurred
 * @return When successful an allocated digest buffer of length digestLength. The caller
 *         is responsible for freeing native memory allocated here.
 *
 */
extern uint8_t*
wkWalletConnectorGetDigest (
        WKWalletConnector       connector,
        const uint8_t           *msg,
        size_t                  msgLen,
        WKBoolean               addPrefix,
        size_t                  *digestLength,
        WKWalletConnectorError  *err            );

/** Uses the wallet connector to sign the arbitrary data.
 *
 * @param connector The wallet connector object
 * @param data The input serialized transaction data to be signed
 * @param dataLen The length of input transaction data
 * @param key The key for signing
 * @param signatureLength The length of returned signature data (on success > 0)
 * @param err An error when a failure has occurred
 * @return When successful an allocated signature buffer of length signatureLength. The caller
 *         is responsible for freeing native memory allocated here.
 */
extern uint8_t*
wkWalletConnectorSignData (
        WKWalletConnector       connector,
        const uint8_t           *data,
        size_t                  dataLen,
        WKKey                   key,
        size_t                  *signatureLength,
        WKWalletConnectorError  *err            );

/** Uses the wallet connector provided to organize the key-value pairs
 *  into a suitable serialized transaction.
 *
 * (Aside, to be removed)
 * Creates a transaction (e.g. ETHTransactionCreate()) that will verify
 * the keys and values, and serializes the transaction.
 *
 * @param connector The wallet connector object
 * @param keys A NULL terminated series of NULL terminated c-strings.
 * @param values A NULL terminated series of NULL terminated c-strings, corresponding to strings
 *               of 'keys' and in the correct order
 * @param keyValuePairsCount The number of key and corresponding values, of which there are expected
 *                           to be an equal number
 * @param serialization Length The length of the serialization created
 * @param err An error when a failure has occurred
 * @return When successful, a series of bytes of length serializationLength representing
 *         the serialization. The caller is responsible for returning native memory allocated here.
 */
extern uint8_t*
wkWalletConnectorCreateTransactionFromArguments  (
        WKWalletConnector       connector,
        const char              **keys,
        const char              **values,
        size_t                  keyValuePairsCount,
        size_t                  *serializationLength,
        WKWalletConnectorError  *err            );

/** Uses the wallet connector provided to create a serialized transaction
 *  out of the input. In the process, the input serialization is
 *  validated according to the network conventions (TBD: CORE-1281), verifying this data is
 *  formulated correctly. In addition, whether or no the transaction is already
 *  signed, is determined.
 *
 * (Aside, to be removed)
 * Decodes the data into a native, network specific Transaction object (e.g. ETHTransactionRLPDecode())
 * and verifies the composition thereby. Will re-serialize the data from this transaction object.
 *
 * @param connector The wallet connector object
 * @param data The input serialized transaction data
 * @param dataLength The length of this input
 * @param serializationLength The length of the serialization created
 * @param isSigned A flag indicating whether the transaction is signed or no
 * @param err An error when a failure has occurred
 * @return When successful, a series of bytes of length serializationLength representing
 *         the serialization. THe caller is responsible for returning native memory allocated here.
 */
extern uint8_t*
wkWalletConnectorCreateTransactionFromSerialization  (
        WKWalletConnector       connector,
        uint8_t                 *data,
        size_t                  dataLength,
        size_t                  *serializationLength,
        WKBoolean               *isSigned,
        WKWalletConnectorError  *err           );

#ifdef __cplusplus
}
#endif

#endif // WKWalletConnector_h
