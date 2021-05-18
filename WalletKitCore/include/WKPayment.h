//
//  WKPayment.h
//  WalletKitCore
//
//  Created by Michael Carrara on 8/27/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKPayment_h
#define WKPayment_h

#include "WKBase.h"
#include "WKAddress.h"
#include "WKAmount.h"
#include "WKCurrency.h"
#include "WKNetwork.h"
#include "WKTransfer.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Mark: Forward Declarations

typedef struct WKPaymentProtocolRequestBitPayBuilderRecord *WKPaymentProtocolRequestBitPayBuilder;

typedef struct WKPaymentProtocolRequestRecord *WKPaymentProtocolRequest;

typedef struct WKPaymentProtocolPaymentRecord *WKPaymentProtocolPayment;

typedef struct WKPaymentProtocolPaymentACKRecord *WKPaymentProtocolPaymentACK;

typedef enum {
    WK_PAYMENT_PROTOCOL_TYPE_BITPAY,
    WK_PAYMENT_PROTOCOL_TYPE_BIP70,
} WKPaymentProtocolType;

typedef enum {
    WK_PAYMENT_PROTOCOL_ERROR_NONE,
    WK_PAYMENT_PROTOCOL_ERROR_CERT_MISSING,
    WK_PAYMENT_PROTOCOL_ERROR_CERT_NOT_TRUSTED,
    WK_PAYMENT_PROTOCOL_ERROR_SIGNATURE_TYPE_NOT_SUPPORTED,
    WK_PAYMENT_PROTOCOL_ERROR_SIGNATURE_VERIFICATION_FAILED,
    WK_PAYMENT_PROTOCOL_ERROR_EXPIRED
} WKPaymentProtocolError;

typedef void * WKPayProtReqContext;

typedef char * (*WKPayProtReqBitPayAndBip70CommonNameExtractor) (WKPaymentProtocolRequest request,
                                                                 WKPayProtReqContext context,
                                                                 const char *pkiType,
                                                                 uint8_t *certBytes[],
                                                                 size_t certLengths[],
                                                                 size_t certCount);

typedef WKPaymentProtocolError (*WKPayProtReqBitPayAndBip70Validator) (WKPaymentProtocolRequest request,
                                                                       WKPayProtReqContext context,
                                                                       const char *pkiType,
                                                                       uint64_t expires,
                                                                       uint8_t *certBytes[],
                                                                       size_t certLengths[],
                                                                       size_t certCount,
                                                                       const uint8_t *digest,
                                                                       size_t digestLength,
                                                                       const uint8_t *signature,
                                                                       size_t signatureLength);


typedef struct {
    WKPayProtReqContext context;
    WKPayProtReqBitPayAndBip70Validator validator;
    WKPayProtReqBitPayAndBip70CommonNameExtractor nameExtractor;
} WKPayProtReqBitPayAndBip70Callbacks;

typedef WKPayProtReqBitPayAndBip70Callbacks WKPayProtReqBitPayCallbacks;
typedef WKPayProtReqBitPayAndBip70Callbacks WKPayProtReqBip70Callbacks;

/// Mark: BitPay Payment Protocol Request Builder

extern WKPaymentProtocolRequestBitPayBuilder
wkPaymentProtocolRequestBitPayBuilderCreate (WKNetwork wkNetwork,
                                             WKCurrency wkCurrency,
                                             WKPayProtReqBitPayCallbacks callbacks,
                                             const char *network,
                                             uint64_t time,
                                             uint64_t expires,
                                             double feePerByte,
                                             const char *memo,
                                             const char *paymentURL,
                                             const uint8_t *merchantData,
                                             size_t merchDataLen);

DECLARE_WK_GIVE_TAKE (WKPaymentProtocolRequestBitPayBuilder, wkPaymentProtocolRequestBitPayBuilder);

extern void
wkPaymentProtocolRequestBitPayBuilderAddOutput(WKPaymentProtocolRequestBitPayBuilder builder,
                                               const char *address,
                                               uint64_t amount);

extern WKPaymentProtocolRequest
wkPaymentProtocolRequestBitPayBuilderBuild(WKPaymentProtocolRequestBitPayBuilder builder);

/// Mark: Payment Protocol Request

extern WKBoolean
wkPaymentProtocolRequestValidateSupported (WKPaymentProtocolType type,
                                           WKNetwork network,
                                           WKCurrency currency,
                                           WKWallet wallet);

extern WKPaymentProtocolRequest
wkPaymentProtocolRequestCreateForBip70 (WKNetwork wkNetwork,
                                        WKCurrency wkCurrency,
                                        WKPayProtReqBip70Callbacks callbacks,
                                        uint8_t *serialization,
                                        size_t serializationLen);

DECLARE_WK_GIVE_TAKE (WKPaymentProtocolRequest, wkPaymentProtocolRequest);

extern WKPaymentProtocolType
wkPaymentProtocolRequestGetType (WKPaymentProtocolRequest protoReq);

extern WKBoolean
wkPaymentProtocolRequestIsSecure (WKPaymentProtocolRequest protoReq);

extern const char *
wkPaymentProtocolRequestGetMemo (WKPaymentProtocolRequest protoReq);

extern const char *
wkPaymentProtocolRequestGetPaymentURL (WKPaymentProtocolRequest protoReq);

extern WKAmount
wkPaymentProtocolRequestGetTotalAmount (WKPaymentProtocolRequest protoReq);

extern WKNetworkFee
wkPaymentProtocolRequestGetRequiredNetworkFee (WKPaymentProtocolRequest protoReq);

extern WKAddress
wkPaymentProtocolRequestGetPrimaryTargetAddress (WKPaymentProtocolRequest protoReq);

// If the return value is not NULL, it must be deallocated using free()
extern char *
wkPaymentProtocolRequestGetCommonName (WKPaymentProtocolRequest protoReq);

extern WKPaymentProtocolError
wkPaymentProtocolRequestIsValid(WKPaymentProtocolRequest protoReq);

/// Mark: Payment Protocol Payment

extern WKPaymentProtocolPayment
wkPaymentProtocolPaymentCreate (WKPaymentProtocolRequest protoReq,
                                WKTransfer transfer,
                                WKAddress refundAddress);

DECLARE_WK_GIVE_TAKE (WKPaymentProtocolPayment, wkPaymentProtocolPayment);

extern uint8_t *
wkPaymentProtocolPaymentEncode(WKPaymentProtocolPayment protoPay,
                               size_t *encodedLen);

/// Mark: Payment Protocol ACK

extern WKPaymentProtocolPaymentACK
wkPaymentProtocolPaymentACKCreateForBip70 (uint8_t *serialization,
                                           size_t serializationLen);

DECLARE_WK_GIVE_TAKE (WKPaymentProtocolPaymentACK, wkPaymentProtocolPaymentACK);

extern const char *
wkPaymentProtocolPaymentACKGetMemo (WKPaymentProtocolPaymentACK protoAck);

#ifdef __cplusplus
}
#endif

#endif /* WKPayment_h */
