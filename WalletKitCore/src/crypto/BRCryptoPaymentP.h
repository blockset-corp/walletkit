//
//  BRCryptoPaymentP.h
//  BRCore
//
//  Created by Ed Gamble on 12/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoPaymentP_h
#define BRCryptoPaymentP_h

#include "BRCryptoPayment.h"
#include "bitcoin/BRTransaction.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void
(*BRCryptoPaymentProtocolRequestBitPayBuilderAddOutputHandler) (BRCryptoPaymentProtocolRequestBitPayBuilder builder,
                                                                const char *address,
                                                                uint64_t satoshis);

typedef void
(*BRCryptoPaymentProtocolRequestBitPayBuilderReleaseHandler) (BRCryptoPaymentProtocolRequestBitPayBuilder builder);

typedef BRCryptoBoolean
(*BRCryptoPaymentProtocolRequestValidateSupportedHandler) (BRCryptoPaymentProtocolType type);


typedef BRCryptoPaymentProtocolRequest
(*BRCryptoPaymentProtocolRequestCreateForBitPayHandler) (BRCryptoPaymentProtocolRequestBitPayBuilder builder);

typedef BRCryptoPaymentProtocolRequest
(*BRCryptoPaymentProtocolRequestCreateForBip70Handler) (BRCryptoNetwork cryptoNetwork,
                                                        BRCryptoCurrency cryptoCurrency,
                                                        BRCryptoPayProtReqBip70Callbacks callbacks,
                                                        uint8_t *serialization,
                                                        size_t serializationLen);

typedef BRCryptoBoolean
(*BRCryptoPaymentProtocolRequestIsSecureHandler) (BRCryptoPaymentProtocolRequest req);

typedef const char *
(*BRCryptoPaymentProtocolRequestGetMemoHandler) (BRCryptoPaymentProtocolRequest req);

typedef const char *
(*BRCryptoPaymentProtocolRequestGetPaymentURLHandler) (BRCryptoPaymentProtocolRequest req);

typedef BRCryptoAmount
(*BRCryptoPaymentProtocolRequestGetTotalAmountHandler) (BRCryptoPaymentProtocolRequest req);

typedef BRCryptoAddress
(*BRCryptoPaymentProtocolRequestGetPrimaryTargetAddressHandler) (BRCryptoPaymentProtocolRequest req);

typedef const char *
(*BRCryptoPaymentProtocolRequestGetCommonNameHandler) (BRCryptoPaymentProtocolRequest req);

typedef BRCryptoPaymentProtocolError
(*BRCryptoPaymentProtocolRequestIsValidHandler) (BRCryptoPaymentProtocolRequest req);

typedef const uint8_t*
(*BRCryptoPaymentProtocolRequestGetMerchantDataHandler) (BRCryptoPaymentProtocolRequest req, size_t *merchantDataLen);

typedef void
(*BRCryptoPaymentProtocolRequestReleaseHandler) (BRCryptoPaymentProtocolRequest req);

typedef struct {
    BRCryptoPaymentProtocolRequestBitPayBuilderAddOutputHandler bitPayBuilderAddOutput;
    BRCryptoPaymentProtocolRequestBitPayBuilderReleaseHandler releaseBitPayBuilder;
    
    BRCryptoPaymentProtocolRequestValidateSupportedHandler validateSupported;
    BRCryptoPaymentProtocolRequestCreateForBitPayHandler requestCreateForBitPay;
    BRCryptoPaymentProtocolRequestCreateForBip70Handler requestCreateForBip70;
    BRCryptoPaymentProtocolRequestReleaseHandler release;
} BRCryptoPaymentProtocolHandlers;

// MARK: - BitPay Payment Protocol Request Builder

struct BRCryptoPaymentProtocolRequestBitPayBuilderRecord {
    BRCryptoBlockChainType type;
    const BRCryptoPaymentProtocolHandlers *handlers;
    BRCryptoRef ref;
    size_t sizeInBytes;
    
    BRCryptoNetwork cryptoNetwork;
    BRCryptoCurrency cryptoCurrency;
    BRCryptoPayProtReqBitPayCallbacks callbacks;

    char *network;
    uint64_t time;
    uint64_t expires;
    char *memo;
    char *paymentURL;
    uint8_t *merchantData;
    size_t merchantDataLen;
    double feeCostFactor;
};

private_extern BRCryptoPaymentProtocolRequestBitPayBuilder
cryptoPaymentProtocolRequestBitPayBuilderAllocAndInit (size_t sizeInBytes,
                                                       BRCryptoBlockChainType type,
                                                       BRCryptoNetwork cryptoNetwork,
                                                       BRCryptoCurrency cryptoCurrency,
                                                       BRCryptoPayProtReqBitPayCallbacks callbacks,
                                                       const char *network,
                                                       uint64_t time,
                                                       uint64_t expires,
                                                       double feeCostFactor,
                                                       const char *memo,
                                                       const char *paymentURL,
                                                       const uint8_t *merchantData,
                                                       size_t merchantDataLen);

// MARK: - Payment Protocol Request

struct BRCryptoPaymentProtocolRequestRecord {
    BRCryptoBlockChainType type;
    const BRCryptoPaymentProtocolHandlers *handlers;
    BRCryptoRef ref;
    size_t sizeInBytes;
    
    BRCryptoPaymentProtocolType paymentProtocolType;
    BRCryptoNetwork cryptoNetwork;
    BRCryptoCurrency cryptoCurrency;
    
    BRCryptoNetworkFee requiredFee;
    BRCryptoPayProtReqBitPayAndBip70Callbacks callbacks;
};

private_extern BRCryptoPaymentProtocolRequest
cryptoPaymentProtocolRequestAllocAndInit (size_t sizeInBytes,
                                          BRCryptoBlockChainType type,
                                          BRCryptoPaymentProtocolType paymentProtocolType,
                                          BRCryptoNetwork cryptoNetwork,
                                          BRCryptoCurrency cryptoCurrency,
                                          BRCryptoNetworkFee requiredFee,
                                          BRCryptoPayProtReqBitPayAndBip70Callbacks callbacks);

// MARK: - Payment Protocol Payment

struct BRCryptoPaymentProtocolPaymentRecord {
    BRCryptoPaymentProtocolType type;
    BRCryptoPaymentProtocolRequest request;

    BRCryptoNetwork cryptoNetwork;
    BRCryptoCurrency cryptoCurrency;

    BRCryptoRef ref;
};

private_extern BRCryptoPaymentProtocolPayment
cryptoPaymentProtocolPaymentAllocAndInit (size_t sizeInBytes,
                                          BRCryptoBlockChainType type,
                                          BRCryptoPaymentProtocolRequest request,
                                          BRCryptoNetwork cryptoNetwork,
                                          BRCryptoCurrency cryptoCurrency);

// MARK: -

//private_extern BRArrayOf(BRTxOutput)
//cryptoPaymentProtocolRequestGetOutputsAsBTC (BRCryptoPaymentProtocolRequest request);

#ifdef __cplusplus
}
#endif



#endif /* BRCryptoPaymentP_h */
