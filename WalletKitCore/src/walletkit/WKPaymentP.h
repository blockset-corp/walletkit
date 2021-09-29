//
//  WKPaymentP.h
//  WalletKitCore
//
//  Created by Ed Gamble on 12/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKPaymentP_h
#define WKPaymentP_h

#include "WKPayment.h"
#include "bitcoin/BRBitcoinTransaction.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: Handlers

typedef WKPaymentProtocolRequestBitPayBuilder
(*WKPaymentProtocolRquestBitPayBuilderCreate) (WKNetworkType chainType,
                                                     WKNetwork wkNetwork,
                                                     WKCurrency wkCurrency,
                                                     WKPayProtReqBitPayCallbacks callbacks,
                                                     const char *network,
                                                     uint64_t time,
                                                     uint64_t expires,
                                                     double feeCostFactor,
                                                     const char *memo,
                                                     const char *paymentURL,
                                                     const uint8_t *merchantData,
                                                     size_t merchantDataLen);

typedef void
(*WKPaymentProtocolRequestBitPayBuilderAddOutputHandler) (WKPaymentProtocolRequestBitPayBuilder builder,
                                                                const char *address,
                                                                uint64_t satoshis);

typedef void
(*WKPaymentProtocolRequestBitPayBuilderReleaseHandler) (WKPaymentProtocolRequestBitPayBuilder builder);

typedef WKBoolean
(*WKPaymentProtocolRequestValidateSupportedHandler) (WKPaymentProtocolType type);

typedef WKPaymentProtocolRequest
(*WKPaymentProtocolRequestCreateForBitPayHandler) (WKPaymentProtocolRequestBitPayBuilder builder);

typedef WKPaymentProtocolRequest
(*WKPaymentProtocolRequestCreateForBip70Handler) (WKNetwork wkNetwork,
                                                        WKCurrency wkCurrency,
                                                        WKPayProtReqBip70Callbacks callbacks,
                                                        uint8_t *serialization,
                                                        size_t serializationLen);

typedef WKFeeBasis
(*WKPaymentProtocolRequestEstimateFeeBasisHandler) (WKPaymentProtocolRequest request,
                                                          WKWalletManager cwm,
                                                          WKWallet wallet,
                                                          WKCookie cookie,
                                                          WKNetworkFee fee);

typedef WKTransfer
(*WKPaymentProtocolRequestCreateTransferHandler) (WKPaymentProtocolRequest req,
                                                        WKWallet wallet,
                                                        WKFeeBasis estimatedFeeBasis);

typedef WKBoolean
(*WKPaymentProtocolRequestIsSecureHandler) (WKPaymentProtocolRequest req);

typedef const char *
(*WKPaymentProtocolRequestGetMemoHandler) (WKPaymentProtocolRequest req);

typedef const char *
(*WKPaymentProtocolRequestGetPaymentURLHandler) (WKPaymentProtocolRequest req);

typedef WKAmount
(*WKPaymentProtocolRequestGetTotalAmountHandler) (WKPaymentProtocolRequest req);

typedef WKAddress
(*WKPaymentProtocolRequestGetPrimaryTargetAddressHandler) (WKPaymentProtocolRequest req);

typedef char *
(*WKPaymentProtocolRequestGetCommonNameHandler) (WKPaymentProtocolRequest req);

typedef WKPaymentProtocolError
(*WKPaymentProtocolRequestIsValidHandler) (WKPaymentProtocolRequest req);

typedef void
(*WKPaymentProtocolRequestReleaseHandler) (WKPaymentProtocolRequest req);

typedef WKPaymentProtocolPayment
(*WKPaymentProtocolPaymentCreateHandler) (WKPaymentProtocolRequest protoReq,
                                                WKTransfer transfer,
                                                WKAddress refundAddress);

typedef uint8_t *
(*WKPaymentProtocolPaymentEncodeHandler) (WKPaymentProtocolPayment protoPay,
                                                size_t *encodedLen);

typedef void
(*WKPaymentProtocolPaymentReleaseHandler) (WKPaymentProtocolPayment protoPay);

typedef struct {
    WKPaymentProtocolRquestBitPayBuilderCreate createBitPayBuilder;
    WKPaymentProtocolRequestBitPayBuilderAddOutputHandler bitPayBuilderAddOutput;
    WKPaymentProtocolRequestBitPayBuilderReleaseHandler releaseBitPayBuilder;
    
    WKPaymentProtocolRequestValidateSupportedHandler validateSupported;
    WKPaymentProtocolRequestCreateForBitPayHandler requestCreateForBitPay;
    WKPaymentProtocolRequestCreateForBip70Handler requestCreateForBip70;
    WKPaymentProtocolRequestEstimateFeeBasisHandler estimateFeeBasis;
    WKPaymentProtocolRequestCreateTransferHandler createTransfer;
    
    WKPaymentProtocolRequestIsSecureHandler isSecure;
    WKPaymentProtocolRequestGetMemoHandler getMemo;
    WKPaymentProtocolRequestGetPaymentURLHandler getPaymentURL;
    WKPaymentProtocolRequestGetTotalAmountHandler getTotalAmount;
    WKPaymentProtocolRequestGetPrimaryTargetAddressHandler getPrimaryTargetAddress;
    WKPaymentProtocolRequestGetCommonNameHandler getCommonName;
    WKPaymentProtocolRequestIsValidHandler isValid;
    WKPaymentProtocolRequestReleaseHandler releaseRequest;
    
    WKPaymentProtocolPaymentCreateHandler createPayment;
    WKPaymentProtocolPaymentEncodeHandler encodePayment;
    WKPaymentProtocolPaymentReleaseHandler releasePayment;
    
} WKPaymentProtocolHandlers;

// MARK: - BitPay Payment Protocol Request Builder

struct WKPaymentProtocolRequestBitPayBuilderRecord {
    WKNetworkType type;
    const WKPaymentProtocolHandlers *handlers;
    WKRef ref;
    size_t sizeInBytes;
    
    WKNetwork wkNetwork;
    WKCurrency wkCurrency;
    WKPayProtReqBitPayCallbacks callbacks;

    char *network;
    uint64_t time;
    uint64_t expires;
    char *memo;
    char *paymentURL;
    uint8_t *merchantData;
    size_t merchantDataLen;
    double feeCostFactor;
};

private_extern WKPaymentProtocolRequestBitPayBuilder
wkPaymentProtocolRequestBitPayBuilderAllocAndInit (size_t sizeInBytes,
                                                       WKNetworkType type,
                                                       WKNetwork wkNetwork,
                                                       WKCurrency wkCurrency,
                                                       WKPayProtReqBitPayCallbacks callbacks,
                                                       const char *network,
                                                       uint64_t time,
                                                       uint64_t expires,
                                                       double feeCostFactor,
                                                       const char *memo,
                                                       const char *paymentURL,
                                                       const uint8_t *merchantData,
                                                       size_t merchantDataLen);

// MARK: - Payment Protocol Request

struct WKPaymentProtocolRequestRecord {
    WKNetworkType chainType;
    const WKPaymentProtocolHandlers *handlers;
    WKRef ref;
    size_t sizeInBytes;
    
    WKPaymentProtocolType paymentProtocolType;
    WKNetwork wkNetwork;
    WKCurrency wkCurrency;
    
    WKNetworkFee requiredFee;
    WKPayProtReqBitPayAndBip70Callbacks callbacks;
};

private_extern WKPaymentProtocolRequest
wkPaymentProtocolRequestAllocAndInit (size_t sizeInBytes,
                                          WKNetworkType type,
                                          WKPaymentProtocolType paymentProtocolType,
                                          WKNetwork wkNetwork,
                                          WKCurrency wkCurrency,
                                          WKNetworkFee requiredFee,
                                          WKPayProtReqBitPayAndBip70Callbacks callbacks);

// MARK: - Payment Protocol Payment

struct WKPaymentProtocolPaymentRecord {
    WKNetworkType chainType;
    WKRef ref;
    size_t sizeInBytes;
    
    WKPaymentProtocolType paymentProtocolType;
    WKPaymentProtocolRequest request;

    WKNetwork wkNetwork;
    WKCurrency wkCurrency;
};

private_extern WKPaymentProtocolPayment
wkPaymentProtocolPaymentAllocAndInit (size_t sizeInBytes,
                                          WKPaymentProtocolRequest protoReq);

#ifdef __cplusplus
}
#endif



#endif /* WKPaymentP_h */
