//
//  WKWalletConnectorP.h
//  WalletKitCore
//
//  Created by Bryan Goring on 8/23/21
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKWalletConnectorP_h
#define WKWalletConnectorP_h

#include "WKWalletConnector.h"
#include "support/BRArray.h"
#include "support/BRInt.h"
#include "support/json/BRJson.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Connector Handlers

typedef WKWalletConnector
(*WKWalletConnectorCreateHandler) (WKWalletManager manager);

typedef void
(*WKWalletConnectorReleaseHandler) (WKWalletConnector connector);

typedef uint8_t*
(*WKWalletConnectorCreateStandardMessageHandler) (
        WKWalletConnector       walletConnector,
        const uint8_t           *msg,
        size_t                  msgLength,
        size_t                  *standardMessageLength );

typedef uint8_t*
(*WKWalletConnectorGetDigestHandler) (
        WKWalletConnector       walletConnector,
        const uint8_t           *msg,
        size_t                  msgLength,
        size_t                  *digestLength,
        WKWalletConnectorStatus *status    );

typedef WKKey
(*WKWalletConnectorCreateKeyFromSeed) (
        WKWalletConnector       walletConnector,
        UInt512                 seed    );

typedef uint8_t*
(*WKWalletConnectorSignDataHandler) (
        WKWalletConnector       walletConnector,
        const uint8_t           *data,
        size_t                  dataLength,
        WKKey                   key,
        size_t                  *signatureLength,
        WKWalletConnectorStatus *status    );

typedef WKKey
(*WKWalletConnectorRecoverKeyHandler) (
        WKWalletConnector       walletConnector,
        const uint8_t           *digest,
        size_t                  digestLength,
        const uint8_t           *signature,
        size_t                  signatureLength,
        WKWalletConnectorStatus *status);

/** It can be assumed that there are equal number of elements in
 *  keys & values, so array_count() of either will be the same.
 */
typedef uint8_t*
(*WKWalletConnectorCreateTransactionFromArgumentsHandler) (
        WKWalletConnector       walletConnector,
        BRArrayOf (const char*) keys,
        BRArrayOf (const char*) values,
        WKNetworkFee            defaultFee,
        size_t                  *serializationLength,
        WKWalletConnectorStatus *status           );

typedef uint8_t*
(*WKWalletConnectorCreateTransactionFromSerializationHandler) (
        WKWalletConnector       walletConnector,
        const uint8_t           *data,
        size_t                  dataLength,
        size_t                  *serializationLength,
        WKBoolean               *isSigned,
        WKWalletConnectorStatus *status            );

typedef uint8_t*
(*WKWalletConnectorSignTransactionDataHandler) (
        WKWalletConnector       walletConnector,
        const uint8_t           *transactionData,
        size_t                  dataLength,
        WKKey                   key,
        uint8_t                 **transactionIdentifier,
        size_t                  *transactionIdentifierLength,
        size_t                  *signedDataLength,
        WKWalletConnectorStatus *status            );

typedef uint8_t*
(*WKWalletConnectorSignTypedDataHandler) (
        WKWalletConnector       walletConnector,
        BRJson                  typedData,
        WKKey                   key,
        uint8_t                 **digestData,
        size_t                  *digestLength,
        size_t                  *signatureLength,
        WKWalletConnectorStatus *status);

typedef struct {
    WKWalletConnectorCreateHandler                              create;
    WKWalletConnectorReleaseHandler                             release;
    WKWalletConnectorCreateStandardMessageHandler               createStandardMessage;
    WKWalletConnectorGetDigestHandler                           getDigest;
    WKWalletConnectorCreateKeyFromSeed                          createKeyFromSeed;
    WKWalletConnectorSignDataHandler                            sign;
    WKWalletConnectorRecoverKeyHandler                          recover;
    WKWalletConnectorCreateTransactionFromArgumentsHandler      createTransactionFromArguments;
    WKWalletConnectorCreateTransactionFromSerializationHandler  createTransactionFromSerialization;
    WKWalletConnectorSignTransactionDataHandler                 signTransactionData;
    WKWalletConnectorSignTypedDataHandler                       signTypedData;
} WKWalletConnectorHandlers;

// MARK: - Connector

struct WKWalletConnectorRecord {
    WKNetworkType                       type;
    const WKWalletConnectorHandlers     *handlers;
    size_t                              sizeInBytes;

    WKWalletManager                     manager;
    // ...
};

private_extern WKWalletConnector
wkWalletConnectorAllocAndInit (size_t sizeInBytes,
                               WKNetworkType type,
                               WKWalletManager manager);

#ifdef __cplusplus
}
#endif

#endif /* WKWalletConnectorP_h */
