//
//  BRCryptoPayment.c
//  BRCore
//
//  Created by Michael Carrara on 8/27/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoPayment.h"

#include "BRCryptoBase.h"

#include "BRCryptoPaymentP.h"
#include "BRCryptoNetworkP.h"
#include "BRCryptoAmountP.h"
#include "BRCryptoTransferP.h"
#include "BRCryptoAddressP.h"
#include "BRCryptoWalletP.h"

#include "BRCryptoHandlersP.h"

#include "bitcoin/BRPaymentProtocol.h"
#include "support/BRArray.h"


/// Mark: - Private Declarations

/// MARK: - BitPay Payment Protocol Request Builder Declarations

/// MARK: - Payment Protocol Request Declarations

static BRCryptoPaymentProtocolRequest
cryptoPaymentProtocolRequestCreateForBitPay (BRCryptoPaymentProtocolRequestBitPayBuilder builder);

/// MARK: - BitPay Payment Protocol Request Builder Implementation

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoPaymentProtocolRequestBitPayBuilder, cryptoPaymentProtocolRequestBitPayBuilder)

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
                                                       size_t merchantDataLen) {
    assert (sizeInBytes >= sizeof (struct BRCryptoPaymentProtocolRequestBitPayBuilderRecord));
    
    BRCryptoPaymentProtocolRequestBitPayBuilder builder = calloc (1, sizeInBytes);
    
    builder->type = type;
    builder->handlers = cryptoHandlersLookup(type)->payment;
    builder->ref = CRYPTO_REF_ASSIGN(cryptoPaymentProtocolRequestBitPayBuilderRelease);
    builder->sizeInBytes = sizeInBytes;

    builder->cryptoNetwork = cryptoNetworkTake (cryptoNetwork);
    builder->cryptoCurrency = cryptoCurrencyTake (cryptoCurrency);
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

extern BRCryptoPaymentProtocolRequestBitPayBuilder
cryptoPaymentProtocolRequestBitPayBuilderCreate (BRCryptoNetwork cryptoNetwork,
                                                 BRCryptoCurrency cryptoCurrency,
                                                 BRCryptoPayProtReqBitPayCallbacks callbacks,
                                                 const char *network,
                                                 uint64_t time,
                                                 uint64_t expires,
                                                 double feeCostFactor,
                                                 const char *memo,
                                                 const char *paymentURL,
                                                 const uint8_t *merchantData,
                                                 size_t merchantDataLen) {
    BRCryptoBlockChainType chainType = cryptoNetworkGetType (cryptoNetwork);
    
    const BRCryptoPaymentProtocolHandlers * paymentHandlers = cryptoHandlersLookup (chainType)->payment;
    if (NULL == paymentHandlers) {
        assert (0);
        return NULL;
    }
    
    return paymentHandlers->createBitPayBuilder (chainType,
                                                 cryptoNetwork,
                                                 cryptoCurrency,
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
cryptoPaymentProtocolRequestBitPayBuilderRelease (BRCryptoPaymentProtocolRequestBitPayBuilder builder) {
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

    cryptoNetworkGive (builder->cryptoNetwork);
    cryptoCurrencyGive (builder->cryptoCurrency);
    memset (builder, 0, builder->sizeInBytes);
    free (builder);
}

extern void
cryptoPaymentProtocolRequestBitPayBuilderAddOutput(BRCryptoPaymentProtocolRequestBitPayBuilder builder,
                                                   const char *address,
                                                   uint64_t satoshis) {
    builder->handlers->bitPayBuilderAddOutput (builder, address, satoshis);
}

extern BRCryptoPaymentProtocolRequest
cryptoPaymentProtocolRequestBitPayBuilderBuild(BRCryptoPaymentProtocolRequestBitPayBuilder builder) {
    return cryptoPaymentProtocolRequestCreateForBitPay (builder);
}

/// MARK: - Payment Protocol Request Implementation

static void
cryptoPaymentProtocolRequestRelease (BRCryptoPaymentProtocolRequest protoReq);

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoPaymentProtocolRequest, cryptoPaymentProtocolRequest)

private_extern BRCryptoPaymentProtocolRequest
cryptoPaymentProtocolRequestAllocAndInit (size_t sizeInBytes,
                                          BRCryptoBlockChainType type,
                                          BRCryptoPaymentProtocolType paymentProtocolType,
                                          BRCryptoNetwork cryptoNetwork,
                                          BRCryptoCurrency cryptoCurrency,
                                          BRCryptoNetworkFee requiredFee,
                                          BRCryptoPayProtReqBitPayAndBip70Callbacks callbacks) {
    assert (sizeInBytes >= sizeof (struct BRCryptoPaymentProtocolRequestRecord));
    
    BRCryptoPaymentProtocolRequest protoReq = calloc (1, sizeInBytes);
    
    protoReq->chainType = type;
    protoReq->handlers = cryptoHandlersLookup(type)->payment;
    protoReq->ref = CRYPTO_REF_ASSIGN(cryptoPaymentProtocolRequestRelease);
    protoReq->sizeInBytes = sizeInBytes;
    
    protoReq->paymentProtocolType = paymentProtocolType;
    protoReq->cryptoNetwork = cryptoNetworkTake (cryptoNetwork);
    protoReq->cryptoCurrency = cryptoCurrencyTake (cryptoCurrency);
    protoReq->requiredFee = cryptoNetworkFeeTake (requiredFee);
    protoReq->callbacks = callbacks;
    
    return protoReq;
}

extern BRCryptoBoolean
cryptoPaymentProtocolRequestValidateSupported (BRCryptoPaymentProtocolType protocolType,
                                               BRCryptoNetwork network,
                                               BRCryptoCurrency currency,
                                               BRCryptoWallet wallet) {
    if (CRYPTO_FALSE == cryptoNetworkHasCurrency (network, currency)) {
        return CRYPTO_FALSE;
    }

    if (cryptoNetworkGetType (network) != cryptoWalletGetType (wallet)) {
        return CRYPTO_FALSE;
    }

    BRCryptoCurrency walletCurrency = cryptoWalletGetCurrency (wallet);
    if (CRYPTO_FALSE == cryptoCurrencyIsIdentical (currency, walletCurrency)) {
        cryptoCurrencyGive (walletCurrency);
        return CRYPTO_FALSE;
    }
    cryptoCurrencyGive (walletCurrency);

    BRCryptoBlockChainType chainType = cryptoNetworkGetType (network);
    const BRCryptoPaymentProtocolHandlers * paymentHandlers = cryptoHandlersLookup (chainType)->payment;
    if (NULL == paymentHandlers) {
        return CRYPTO_FALSE;
    }
    
    return paymentHandlers->validateSupported (protocolType);
}

static BRCryptoPaymentProtocolRequest
cryptoPaymentProtocolRequestCreateForBitPay (BRCryptoPaymentProtocolRequestBitPayBuilder builder) {
    return builder->handlers->requestCreateForBitPay (builder);
}

extern BRCryptoPaymentProtocolRequest
cryptoPaymentProtocolRequestCreateForBip70 (BRCryptoNetwork cryptoNetwork,
                                            BRCryptoCurrency cryptoCurrency,
                                            BRCryptoPayProtReqBip70Callbacks callbacks,
                                            uint8_t *serialization,
                                            size_t serializationLen) {
    BRCryptoBlockChainType chainType = cryptoNetworkGetType (cryptoNetwork);
    const BRCryptoPaymentProtocolHandlers * paymentHandlers = cryptoHandlersLookup (chainType)->payment;
    if (NULL == paymentHandlers) {
        assert(0);
        return NULL;
    }
    
    return paymentHandlers->requestCreateForBip70 (cryptoNetwork,
                                                   cryptoCurrency,
                                                   callbacks,
                                                   serialization,
                                                   serializationLen);
}

static void
cryptoPaymentProtocolRequestRelease (BRCryptoPaymentProtocolRequest protoReq) {
    assert (CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY == protoReq->paymentProtocolType ||
            CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70 == protoReq->paymentProtocolType);
    
    protoReq->handlers->releaseRequest (protoReq);

    cryptoNetworkFeeGive (protoReq->requiredFee);
    cryptoNetworkGive (protoReq->cryptoNetwork);
    cryptoCurrencyGive (protoReq->cryptoCurrency);

    memset (protoReq, 0, protoReq->sizeInBytes);
    free (protoReq);
}

extern BRCryptoPaymentProtocolType
cryptoPaymentProtocolRequestGetType (BRCryptoPaymentProtocolRequest protoReq) {
    return protoReq->paymentProtocolType;
}

extern BRCryptoBoolean
cryptoPaymentProtocolRequestIsSecure (BRCryptoPaymentProtocolRequest protoReq) {
    return protoReq->handlers->isSecure (protoReq);
}

extern const char *
cryptoPaymentProtocolRequestGetMemo (BRCryptoPaymentProtocolRequest protoReq) {
    return protoReq->handlers->getMemo (protoReq);
}

extern const char *
cryptoPaymentProtocolRequestGetPaymentURL (BRCryptoPaymentProtocolRequest protoReq) {
    return protoReq->handlers->getPaymentURL (protoReq);
}

extern BRCryptoAmount
cryptoPaymentProtocolRequestGetTotalAmount (BRCryptoPaymentProtocolRequest protoReq) {
    return protoReq->handlers->getTotalAmount (protoReq);
}

extern BRCryptoNetworkFee
cryptoPaymentProtocolRequestGetRequiredNetworkFee (BRCryptoPaymentProtocolRequest protoReq) {
    return cryptoNetworkFeeTake (protoReq->requiredFee);
}

extern BRCryptoAddress
cryptoPaymentProtocolRequestGetPrimaryTargetAddress (BRCryptoPaymentProtocolRequest protoReq) {
    return protoReq->handlers->getPrimaryTargetAddress (protoReq);
}

extern char *
cryptoPaymentProtocolRequestGetCommonName (BRCryptoPaymentProtocolRequest protoReq) {
    return protoReq->handlers->getCommonName (protoReq);
}

extern BRCryptoPaymentProtocolError
cryptoPaymentProtocolRequestIsValid (BRCryptoPaymentProtocolRequest protoReq) {
    return protoReq->handlers->isValid (protoReq);
}

/// MARK: - Payment Protocol Payment

static void
cryptoPaymentProtocolPaymentRelease (BRCryptoPaymentProtocolPayment protoPay);

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoPaymentProtocolPayment, cryptoPaymentProtocolPayment)

private_extern BRCryptoPaymentProtocolPayment
cryptoPaymentProtocolPaymentAllocAndInit (size_t sizeInBytes,
                                          BRCryptoPaymentProtocolRequest protoReq) {
    assert (sizeInBytes >= sizeof (struct BRCryptoPaymentProtocolPaymentRecord));
    
    BRCryptoPaymentProtocolPayment protoPay = calloc (1, sizeInBytes);
    
    protoPay->chainType = protoReq->chainType;
    protoPay->ref = CRYPTO_REF_ASSIGN(cryptoPaymentProtocolPaymentRelease);
    protoPay->sizeInBytes = sizeInBytes;
    
    protoPay->paymentProtocolType = protoReq->paymentProtocolType;
    protoPay->request = cryptoPaymentProtocolRequestTake (protoReq);
    protoPay->cryptoNetwork = cryptoNetworkTake (protoReq->cryptoNetwork);
    protoPay->cryptoCurrency = cryptoCurrencyTake (protoReq->cryptoCurrency);
    
    return protoPay;
}

extern BRCryptoPaymentProtocolPayment
cryptoPaymentProtocolPaymentCreate (BRCryptoPaymentProtocolRequest protoReq,
                                    BRCryptoTransfer transfer,
                                    BRCryptoAddress refundAddress) {
    return protoReq->handlers->createPayment (protoReq, transfer, refundAddress);
}

static void
cryptoPaymentProtocolPaymentRelease (BRCryptoPaymentProtocolPayment protoPay) {
    protoPay->request->handlers->releasePayment (protoPay);

    cryptoNetworkGive (protoPay->cryptoNetwork);
    cryptoCurrencyGive (protoPay->cryptoCurrency);
    cryptoPaymentProtocolRequestGive (protoPay->request);
    memset (protoPay, 0, sizeof(*protoPay));
    free (protoPay);
}

extern uint8_t *
cryptoPaymentProtocolPaymentEncode(BRCryptoPaymentProtocolPayment protoPay,
                                   size_t *encodedLen) {
    return protoPay->request->handlers->encodePayment (protoPay, encodedLen);
}

/// Mark: Payment Protocol Payment ACK

static void
cryptoPaymentProtocolPaymentACKRelease (BRCryptoPaymentProtocolPaymentACK protoAck);

struct BRCryptoPaymentProtocolPaymentACKRecord {
    BRCryptoPaymentProtocolType type;

    union {
        struct {
            BRPaymentProtocolACK *ack;
        } btcBip70;
    } u;

    BRCryptoRef ref;
};

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoPaymentProtocolPaymentACK, cryptoPaymentProtocolPaymentACK)

extern BRCryptoPaymentProtocolPaymentACK
cryptoPaymentProtocolPaymentACKCreateForBip70 (uint8_t *serialization,
                                               size_t serializationLen) {
    BRCryptoPaymentProtocolPaymentACK protoAck = NULL;

    BRPaymentProtocolACK *ack = BRPaymentProtocolACKParse (serialization, serializationLen);
    if (NULL != ack) {
        protoAck = calloc (1, sizeof(struct BRCryptoPaymentProtocolPaymentACKRecord));
        protoAck->ref = CRYPTO_REF_ASSIGN (cryptoPaymentProtocolPaymentACKRelease);
        protoAck->type = CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70;

        protoAck->u.btcBip70.ack = ack;
    }

    return protoAck;
}

static void
cryptoPaymentProtocolPaymentACKRelease (BRCryptoPaymentProtocolPaymentACK protoAck) {
    switch (protoAck->type) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRPaymentProtocolACKFree (protoAck->u.btcBip70.ack);
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
cryptoPaymentProtocolPaymentACKGetMemo (BRCryptoPaymentProtocolPaymentACK protoAck) {
    const char * memo = NULL;

    switch (protoAck->type) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
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
