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

    // A status code indicating success
    WK_WALLET_CONNECTOR_STATUS_OK,
    
    // Unsupported for this network
    WK_WALLET_CONNECTOR_STATUS_UNSUPPORTED_CONNECTOR,

    // Crucial setup has not been done properly, one of
    // network handlers, WalletConnect handlers, or individual
    // handler methods for this particular network are not defined.
    WK_WALLET_CONNECTOR_STATUS_ILLEGAL_OPERATION,

    // One or more transaction arguments key-value pairs are not
    // provided, or values are invalid
    WK_WALLET_CONNECTOR_STATUS_INVALID_TRANSACTION_ARGUMENTS,

    // The digest creation has failed or the digest is of an expected length
    WK_WALLET_CONNECTOR_STATUS_INVALID_DIGEST,

    // Signature creation has failed or the signature is an invalid length
    WK_WALLET_CONNECTOR_STATUS_INVALID_SIGNATURE,

    // A general error specifying a failure to produce serialization
    WK_WALLET_CONNECTOR_STATUS_INVALID_SERIALIZATION,

    // The public key cannot be recovered from the digest + signature
    WK_WALLET_CONNECTOR_STATUS_KEY_RECOVERY_FAILED
    
    // ...

} WKWalletConnectorStatus;

typedef struct WKWalletConnectorRecord *WKWalletConnector;

extern WKWalletConnector
wkWalletConnectorCreate (WKWalletManager manager);

extern void
wkWalletConnectorRelease (WKWalletConnector connector);

/// N.B. WalletConnector functions return the sought after object and
///      indicate any possible failure condition through a WKWalletConnectorStatus.
///
///      A NULL return object is always indicative of an error and should
///      be accompanied by a descriptive status code. In the normal course
///      of successful completion, WalletConnect functions indicate
///      'WK_WALLET_CONNECTOR_STATUS_OK'

/* Create a WalletConnect specific 'standard message' out of the input message.
 *
 * @param connector The wallet connector object
 * @param msg The input message bytes
 * @param msgLen The length of input message
 * @param standardMessageLength The length of the standard message created
 * @param status A status of the operation
 * @return The standard message bytes or NULL on error
 *
 */
extern uint8_t*
wkWalletConnectorCreateStandardMessage (
        WKWalletConnector       connector,
        const uint8_t           *msg,
        size_t                  msgLen,
        size_t                  *standardMessageLength,
        WKWalletConnectorStatus *status          );

/* Use the wallet connector to generate a digest of the provided message.
 *
 * @param connector The wallet connector object
 * @param msg The input message bytes for which to create the digest
 * @param msgLen The length of input message
 * @param digestLength The length of digest created (on success > 0)
 * @param status A status of the operation
 * @return When successful an allocated digest buffer of length digestLength. The caller
 *         is responsible for freeing native memory allocated here.
 *
 */
extern uint8_t*
wkWalletConnectorGetDigest (
        WKWalletConnector       connector,
        const uint8_t           *msg,
        size_t                  msgLen,
        size_t                  *digestLength,
        WKWalletConnectorStatus *status          );

/** Uses the wallet connector to sign the arbitrary data.
 *
 * @param connector The wallet connector object
 * @param data The input serialized transaction data to be signed
 * @param dataLength The length of input transaction data
 * @param key The key for signing
 * @param signatureLength The length of returned signature data (on success > 0)
 * @param status A status of the operation
 * @return When successful an allocated signature buffer of length signatureLength. The caller
 *         is responsible for freeing native memory allocated here.
 */
extern uint8_t*
wkWalletConnectorSignData (
        WKWalletConnector       connector,
        const uint8_t           *data,
        size_t                  dataLength,
        WKKey                   key,
        size_t                  *signatureLength,
        WKWalletConnectorStatus *status            );

/** Returns the public key from the provide digest and signature.
 *
 * @param connector The wallet connector object.
 * @param digest The digest
 * @param digestLength The number of bytes of digest
 * @param signature The signature
 * @param signatureLength The number of bytes of signature
 * @param status A status of the operation
 * @return When successful, the recovered public key. Otherwise an error is set in status
 *         and the returned key is NULL.
 */
extern WKKey
wkWalletConnectorRecoverKey (
        WKWalletConnector       connector,
        const uint8_t           *digest,
        size_t                  digestLength,
        const uint8_t           *signature,
        size_t                  signatureLength,
        WKWalletConnectorStatus *status         );

/** Uses the wallet connector provided to organize the key-value pairs
 *  into a suitable serialized transaction.
 *
 * @param connector The wallet connector object
 * @param keys A NULL terminated series of NULL terminated c-strings.
 * @param values A NULL terminated series of NULL terminated c-strings, corresponding to strings
 *               of 'keys' and in the correct order
 * @param keyValuePairsCount The number of key and corresponding values, of which there are expected
 *                           to be an equal number
 * @param serialization Length The length of the serialization created
 * @param status A status of the operation
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
        WKWalletConnectorStatus *status            );

/** Uses the wallet connector provided to create a serialized transaction
 *  out of the input. In the process, the input serialization is
 *  validated according to the network conventions (TBD: CORE-1281), verifying this data is
 *  formulated correctly. In addition, whether or no the transaction is already
 *  signed, is determined.
 *
 * @param connector The wallet connector object
 * @param data The input serialized transaction data
 * @param dataLength The length of this input
 * @param serializationLength The length of the serialization created
 * @param isSigned A flag indicating whether the transaction is signed or no
 * @param status A status of the operation
 * @return When successful, a series of bytes of length serializationLength representing
 *         the serialization. THe caller is responsible for returning native memory allocated here.
 */
extern uint8_t*
wkWalletConnectorCreateTransactionFromSerialization  (
        WKWalletConnector       connector,
        const uint8_t           *data,
        size_t                  dataLength,
        size_t                  *serializationLength,
        WKBoolean               *isSigned,
        WKWalletConnectorStatus *status           );

/** Uses the wallet connector to sign data representing a
 *  validated, serialized transaction.
 *
 * @param connector The wallet connector object
 * @param transactionData The input serialized transaction data to be signed
 * @param dataLength The length of input transaction data
 * @param key The key for signing
 * @param signedDataLength The length of returned signed transaction data (on success > 0)
 * @param status A status of the operation
 * @return When successful the signed transaction serialization of length signedDataLength. The caller
 *         is responsible for freeing native memory allocated here.
 */
extern uint8_t*
wkWalletConnectorSignTransactionData (
        WKWalletConnector       connector,
        const uint8_t           *transactionData,
        size_t                  dataLength,
        WKKey                   key,
        size_t                  *signedDataLength,
        WKWalletConnectorStatus *status            );

#ifdef __cplusplus
}
#endif

#endif // WKWalletConnector_h
