//
//  WKPayment.c
//  WalletKitCore
//
//  Created by Michael Carrara on 8/27/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "WKPayment.h"

#include "WKBase.h"

#include "WKPaymentP.h"
#include "WKNetworkP.h"
#include "WKAmountP.h"
#include "WKTransferP.h"
#include "WKAddressP.h"
#include "WKWalletP.h"

#include "WKHandlersP.h"

#include "bitcoin/BRBitcoinPaymentProtocol.h"
#include "support/BRArray.h"


/// Mark: - Private Declarations

/// MARK: - BitPay Payment Protocol Request Builder Declarations

/// MARK: - Payment Protocol Request Declarations

static WKPaymentProtocolRequest
wkPaymentProtocolRequestCreateForBitPay (WKPaymentProtocolRequestBitPayBuilder builder);

/// MARK: - BitPay Payment Protocol Request Builder Implementation

IMPLEMENT_WK_GIVE_TAKE (WKPaymentProtocolRequestBitPayBuilder, wkPaymentProtocolRequestBitPayBuilder)

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
                                                       size_t merchantDataLen) {
    assert (sizeInBytes >= sizeof (struct WKPaymentProtocolRequestBitPayBuilderRecord));
    
    WKPaymentProtocolRequestBitPayBuilder builder = calloc (1, sizeInBytes);
    
    builder->type = type;
    builder->handlers = wkHandlersLookup(type)->payment;
    builder->ref = WK_REF_ASSIGN(wkPaymentProtocolRequestBitPayBuilderRelease);
    builder->sizeInBytes = sizeInBytes;

    builder->wkNetwork = wkNetworkTake (wkNetwork);
    builder->wkCurrency = wkCurrencyTake (wkCurrency);
    builder->callbacks = callbacks;

    builder->time = time;
    builder->expires = expires;
    builder->feeCostFactor = feeCostFactor;
    
    if (network) {
        builder->network = strdup (network);
    }

    if (memo) {
        builder->memo = strdup (memo);
    }

    if (paymentURL) {
        builder->paymentURL = strdup (paymentURL);
    }

    if (merchantData && merchantDataLen) {
        builder->merchantData = malloc (merchantDataLen);
        memcpy (builder->merchantData, merchantData, merchantDataLen);
        builder->merchantDataLen = merchantDataLen;
    }
    
    return builder;
}

extern WKPaymentProtocolRequestBitPayBuilder
wkPaymentProtocolRequestBitPayBuilderCreate (WKNetwork wkNetwork,
                                                 WKCurrency wkCurrency,
                                                 WKPayProtReqBitPayCallbacks callbacks,
                                                 const char *network,
                                                 uint64_t time,
                                                 uint64_t expires,
                                                 double feeCostFactor,
                                                 const char *memo,
                                                 const char *paymentURL,
                                                 const uint8_t *merchantData,
                                                 size_t merchantDataLen) {
    WKNetworkType chainType = wkNetworkGetType (wkNetwork);
    
    const WKPaymentProtocolHandlers * paymentHandlers = wkHandlersLookup (chainType)->payment;
    if (NULL == paymentHandlers) {
        assert (0);
        return NULL;
    }
    
    return paymentHandlers->createBitPayBuilder (chainType,
                                                 wkNetwork,
                                                 wkCurrency,
                                                 callbacks,
                                                 network,
                                                 time,
                                                 expires,
                                                 feeCostFactor,
                                                 memo,
                                                 paymentURL,
                                                 merchantData,
                                                 merchantDataLen);
}

static void
wkPaymentProtocolRequestBitPayBuilderRelease (WKPaymentProtocolRequestBitPayBuilder builder) {
    builder->handlers->releaseBitPayBuilder (builder);
    
    if (builder->network) {
        free (builder->network);
    }

    if (builder->memo) {
        free (builder->memo);
    }

    if (builder->paymentURL) {
        free (builder->paymentURL);
    }

    if (builder->merchantData) {
        free (builder->merchantData);
    }

    wkNetworkGive (builder->wkNetwork);
    wkCurrencyGive (builder->wkCurrency);
    memset (builder, 0, builder->sizeInBytes);
    free (builder);
}

extern void
wkPaymentProtocolRequestBitPayBuilderAddOutput(WKPaymentProtocolRequestBitPayBuilder builder,
                                                   const char *address,
                                                   uint64_t satoshis) {
    builder->handlers->bitPayBuilderAddOutput (builder, address, satoshis);
}

extern WKPaymentProtocolRequest
wkPaymentProtocolRequestBitPayBuilderBuild(WKPaymentProtocolRequestBitPayBuilder builder) {
    return wkPaymentProtocolRequestCreateForBitPay (builder);
}

/// MARK: - Payment Protocol Request Implementation

static void
wkPaymentProtocolRequestRelease (WKPaymentProtocolRequest protoReq);

IMPLEMENT_WK_GIVE_TAKE (WKPaymentProtocolRequest, wkPaymentProtocolRequest)

private_extern WKPaymentProtocolRequest
wkPaymentProtocolRequestAllocAndInit (size_t sizeInBytes,
                                          WKNetworkType type,
                                          WKPaymentProtocolType paymentProtocolType,
                                          WKNetwork wkNetwork,
                                          WKCurrency wkCurrency,
                                          WKNetworkFee requiredFee,
                                          WKPayProtReqBitPayAndBip70Callbacks callbacks) {
    assert (sizeInBytes >= sizeof (struct WKPaymentProtocolRequestRecord));
    
    WKPaymentProtocolRequest protoReq = calloc (1, sizeInBytes);
    
    protoReq->chainType = type;
    protoReq->handlers = wkHandlersLookup(type)->payment;
    protoReq->ref = WK_REF_ASSIGN(wkPaymentProtocolRequestRelease);
    protoReq->sizeInBytes = sizeInBytes;
    
    protoReq->paymentProtocolType = paymentProtocolType;
    protoReq->wkNetwork = wkNetworkTake (wkNetwork);
    protoReq->wkCurrency = wkCurrencyTake (wkCurrency);
    protoReq->requiredFee = wkNetworkFeeTake (requiredFee);
    protoReq->callbacks = callbacks;
    
    return protoReq;
}

extern WKBoolean
wkPaymentProtocolRequestValidateSupported (WKPaymentProtocolType protocolType,
                                               WKNetwork network,
                                               WKCurrency currency,
                                               WKWallet wallet) {
    if (WK_FALSE == wkNetworkHasCurrency (network, currency)) {
        return WK_FALSE;
    }

    if (wkNetworkGetType (network) != wkWalletGetType (wallet)) {
        return WK_FALSE;
    }

    WKCurrency walletCurrency = wkWalletGetCurrency (wallet);
    if (WK_FALSE == wkCurrencyIsIdentical (currency, walletCurrency)) {
        wkCurrencyGive (walletCurrency);
        return WK_FALSE;
    }
    wkCurrencyGive (walletCurrency);

    WKNetworkType chainType = wkNetworkGetType (network);
    const WKPaymentProtocolHandlers * paymentHandlers = wkHandlersLookup (chainType)->payment;
    if (NULL == paymentHandlers) {
        return WK_FALSE;
    }
    
    return paymentHandlers->validateSupported (protocolType);
}

static WKPaymentProtocolRequest
wkPaymentProtocolRequestCreateForBitPay (WKPaymentProtocolRequestBitPayBuilder builder) {
    return builder->handlers->requestCreateForBitPay (builder);
}

extern WKPaymentProtocolRequest
wkPaymentProtocolRequestCreateForBip70 (WKNetwork wkNetwork,
                                            WKCurrency wkCurrency,
                                            WKPayProtReqBip70Callbacks callbacks,
                                            uint8_t *serialization,
                                            size_t serializationLen) {
    WKNetworkType chainType = wkNetworkGetType (wkNetwork);
    const WKPaymentProtocolHandlers * paymentHandlers = wkHandlersLookup (chainType)->payment;
    if (NULL == paymentHandlers) {
        assert(0);
        return NULL;
    }
    
    return paymentHandlers->requestCreateForBip70 (wkNetwork,
                                                   wkCurrency,
                                                   callbacks,
                                                   serialization,
                                                   serializationLen);
}

static void
wkPaymentProtocolRequestRelease (WKPaymentProtocolRequest protoReq) {
    assert (WK_PAYMENT_PROTOCOL_TYPE_BITPAY == protoReq->paymentProtocolType ||
            WK_PAYMENT_PROTOCOL_TYPE_BIP70 == protoReq->paymentProtocolType);
    
    protoReq->handlers->releaseRequest (protoReq);

    wkNetworkFeeGive (protoReq->requiredFee);
    wkNetworkGive (protoReq->wkNetwork);
    wkCurrencyGive (protoReq->wkCurrency);

    memset (protoReq, 0, protoReq->sizeInBytes);
    free (protoReq);
}

extern WKPaymentProtocolType
wkPaymentProtocolRequestGetType (WKPaymentProtocolRequest protoReq) {
    return protoReq->paymentProtocolType;
}

extern WKBoolean
wkPaymentProtocolRequestIsSecure (WKPaymentProtocolRequest protoReq) {
    return protoReq->handlers->isSecure (protoReq);
}

extern const char *
wkPaymentProtocolRequestGetMemo (WKPaymentProtocolRequest protoReq) {
    return protoReq->handlers->getMemo (protoReq);
}

extern const char *
wkPaymentProtocolRequestGetPaymentURL (WKPaymentProtocolRequest protoReq) {
    return protoReq->handlers->getPaymentURL (protoReq);
}

extern WKAmount
wkPaymentProtocolRequestGetTotalAmount (WKPaymentProtocolRequest protoReq) {
    return protoReq->handlers->getTotalAmount (protoReq);
}

extern WKNetworkFee
wkPaymentProtocolRequestGetRequiredNetworkFee (WKPaymentProtocolRequest protoReq) {
    return wkNetworkFeeTake (protoReq->requiredFee);
}

extern WKAddress
wkPaymentProtocolRequestGetPrimaryTargetAddress (WKPaymentProtocolRequest protoReq) {
    return protoReq->handlers->getPrimaryTargetAddress (protoReq);
}

extern char *
wkPaymentProtocolRequestGetCommonName (WKPaymentProtocolRequest protoReq) {
    return protoReq->handlers->getCommonName (protoReq);
}

extern WKPaymentProtocolError
wkPaymentProtocolRequestIsValid (WKPaymentProtocolRequest protoReq) {
    return protoReq->handlers->isValid (protoReq);
}

/// MARK: - Payment Protocol Payment

static void
wkPaymentProtocolPaymentRelease (WKPaymentProtocolPayment protoPay);

IMPLEMENT_WK_GIVE_TAKE (WKPaymentProtocolPayment, wkPaymentProtocolPayment)

private_extern WKPaymentProtocolPayment
wkPaymentProtocolPaymentAllocAndInit (size_t sizeInBytes,
                                          WKPaymentProtocolRequest protoReq) {
    assert (sizeInBytes >= sizeof (struct WKPaymentProtocolPaymentRecord));
    
    WKPaymentProtocolPayment protoPay = calloc (1, sizeInBytes);
    
    protoPay->chainType = protoReq->chainType;
    protoPay->ref = WK_REF_ASSIGN(wkPaymentProtocolPaymentRelease);
    protoPay->sizeInBytes = sizeInBytes;
    
    protoPay->paymentProtocolType = protoReq->paymentProtocolType;
    protoPay->request = wkPaymentProtocolRequestTake (protoReq);
    protoPay->wkNetwork = wkNetworkTake (protoReq->wkNetwork);
    protoPay->wkCurrency = wkCurrencyTake (protoReq->wkCurrency);
    
    return protoPay;
}

extern WKPaymentProtocolPayment
wkPaymentProtocolPaymentCreate (WKPaymentProtocolRequest protoReq,
                                    WKTransfer transfer,
                                    WKAddress refundAddress) {
    return protoReq->handlers->createPayment (protoReq, transfer, refundAddress);
}

static void
wkPaymentProtocolPaymentRelease (WKPaymentProtocolPayment protoPay) {
    protoPay->request->handlers->releasePayment (protoPay);

    wkNetworkGive (protoPay->wkNetwork);
    wkCurrencyGive (protoPay->wkCurrency);
    wkPaymentProtocolRequestGive (protoPay->request);
    memset (protoPay, 0, sizeof(*protoPay));
    free (protoPay);
}

extern uint8_t *
wkPaymentProtocolPaymentEncode(WKPaymentProtocolPayment protoPay,
                                   size_t *encodedLen) {
    return protoPay->request->handlers->encodePayment (protoPay, encodedLen);
}

/// Mark: Payment Protocol Payment ACK

static void
wkPaymentProtocolPaymentACKRelease (WKPaymentProtocolPaymentACK protoAck);

struct WKPaymentProtocolPaymentACKRecord {
    WKPaymentProtocolType type;

    union {
        struct {
            BRBitcoinPaymentProtocolACK *ack;
        } btcBip70;
    } u;

    WKRef ref;
};

IMPLEMENT_WK_GIVE_TAKE (WKPaymentProtocolPaymentACK, wkPaymentProtocolPaymentACK)

extern WKPaymentProtocolPaymentACK
wkPaymentProtocolPaymentACKCreateForBip70 (uint8_t *serialization,
                                               size_t serializationLen) {
    WKPaymentProtocolPaymentACK protoAck = NULL;

    BRBitcoinPaymentProtocolACK *ack = btcPaymentProtocolACKParse (serialization, serializationLen);
    if (NULL != ack) {
        protoAck = calloc (1, sizeof(struct WKPaymentProtocolPaymentACKRecord));
        protoAck->ref = WK_REF_ASSIGN (wkPaymentProtocolPaymentACKRelease);
        protoAck->type = WK_PAYMENT_PROTOCOL_TYPE_BIP70;

        protoAck->u.btcBip70.ack = ack;
    }

    return protoAck;
}

static void
wkPaymentProtocolPaymentACKRelease (WKPaymentProtocolPaymentACK protoAck) {
    switch (protoAck->type) {
        case WK_PAYMENT_PROTOCOL_TYPE_BIP70: {
            btcPaymentProtocolACKFree (protoAck->u.btcBip70.ack);
            break;
        }
        default: {
            break;
        }
    }

    memset (protoAck, 0, sizeof(*protoAck));
    free (protoAck);
}

extern const char *
wkPaymentProtocolPaymentACKGetMemo (WKPaymentProtocolPaymentACK protoAck) {
    const char * memo = NULL;

    switch (protoAck->type) {
        case WK_PAYMENT_PROTOCOL_TYPE_BIP70: {
            memo = protoAck->u.btcBip70.ack->memo;
            break;
        }
        default: {
            assert (0);
            break;
        }
    }

    return memo;
}
