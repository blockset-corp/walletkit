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

// MARK: Handlers

typedef BRCryptoPaymentProtocolRequestBitPayBuilder
(*BRCryptoPaymentProtocolRquestBitPayBuilderCreate) (BRCryptoBlockChainType chainType,
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

typedef BRCryptoFeeBasis
(*BRCryptoPaymentProtocolRequestEstimateFeeBasisHandler) (BRCryptoPaymentProtocolRequest request,
                                                          BRCryptoWalletManager cwm,
                                                          BRCryptoWallet wallet,
                                                          BRCryptoCookie cookie,
                                                          BRCryptoNetworkFee fee);

typedef BRCryptoTransfer
(*BRCryptoPaymentProtocolRequestCreateTransferHandler) (BRCryptoPaymentProtocolRequest req,
                                                        BRCryptoWallet wallet,
                                                        BRCryptoFeeBasis estimatedFeeBasis);

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

typedef char *
(*BRCryptoPaymentProtocolRequestGetCommonNameHandler) (BRCryptoPaymentProtocolRequest req);

typedef BRCryptoPaymentProtocolError
(*BRCryptoPaymentProtocolRequestIsValidHandler) (BRCryptoPaymentProtocolRequest req);

typedef void
(*BRCryptoPaymentProtocolRequestReleaseHandler) (BRCryptoPaymentProtocolRequest req);

typedef BRCryptoPaymentProtocolPayment
(*BRCryptoPaymentProtocolPaymentCreateHandler) (BRCryptoPaymentProtocolRequest protoReq,
                                                BRCryptoTransfer transfer,
                                                BRCryptoAddress refundAddress);

typedef uint8_t *
(*BRCryptoPaymentProtocolPaymentEncodeHandler) (BRCryptoPaymentProtocolPayment protoPay,
                                                size_t *encodedLen);

typedef void
(*BRCryptoPaymentProtocolPaymentReleaseHandler) (BRCryptoPaymentProtocolPayment protoPay);

typedef struct {
    BRCryptoPaymentProtocolRquestBitPayBuilderCreate createBitPayBuilder;
    BRCryptoPaymentProtocolRequestBitPayBuilderAddOutputHandler bitPayBuilderAddOutput;
    BRCryptoPaymentProtocolRequestBitPayBuilderReleaseHandler releaseBitPayBuilder;
    
    BRCryptoPaymentProtocolRequestValidateSupportedHandler validateSupported;
    BRCryptoPaymentProtocolRequestCreateForBitPayHandler requestCreateForBitPay;
    BRCryptoPaymentProtocolRequestCreateForBip70Handler requestCreateForBip70;
    BRCryptoPaymentProtocolRequestEstimateFeeBasisHandler estimateFeeBasis;
    BRCryptoPaymentProtocolRequestCreateTransferHandler createTransfer;
    
    BRCryptoPaymentProtocolRequestIsSecureHandler isSecure;
    BRCryptoPaymentProtocolRequestGetMemoHandler getMemo;
    BRCryptoPaymentProtocolRequestGetPaymentURLHandler getPaymentURL;
    BRCryptoPaymentProtocolRequestGetTotalAmountHandler getTotalAmount;
    BRCryptoPaymentProtocolRequestGetPrimaryTargetAddressHandler getPrimaryTargetAddress;
    BRCryptoPaymentProtocolRequestGetCommonNameHandler getCommonName;
    BRCryptoPaymentProtocolRequestIsValidHandler isValid;
    BRCryptoPaymentProtocolRequestReleaseHandler releaseRequest;
    
    BRCryptoPaymentProtocolPaymentCreateHandler createPayment;
    BRCryptoPaymentProtocolPaymentEncodeHandler encodePayment;
    BRCryptoPaymentProtocolPaymentReleaseHandler releasePayment;
    
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
    BRCryptoBlockChainType chainType;
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
    BRCryptoBlockChainType chainType;
    BRCryptoRef ref;
    size_t sizeInBytes;
    
    BRCryptoPaymentProtocolType paymentProtocolType;
    BRCryptoPaymentProtocolRequest request;

    BRCryptoNetwork cryptoNetwork;
    BRCryptoCurrency cryptoCurrency;
};

private_extern BRCryptoPaymentProtocolPayment
cryptoPaymentProtocolPaymentAllocAndInit (size_t sizeInBytes,
                                          BRCryptoPaymentProtocolRequest protoReq);

#ifdef __cplusplus
}
#endif



#endif /* BRCryptoPaymentP_h */
