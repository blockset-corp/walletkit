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

#include <ctype.h>
#include <string.h>

#include "BRCryptoBase.h"

#include "BRCryptoPaymentP.h"
#include "BRCryptoNetworkP.h"
#include "BRCryptoAmountP.h"
#include "BRCryptoTransferP.h"
#include "BRCryptoAddressP.h"
#include "BRCryptoWalletP.h"

#include "BRCryptoHandlersP.h"

#include "handlers/btc/BRCryptoBTC.h"

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
    BRCryptoBlockChainType type = cryptoNetworkGetType (cryptoNetwork);
    
    //TODO:PPR need a handler in wallet(manager) to create
    assert (CRYPTO_NETWORK_TYPE_BTC == type ||
            CRYPTO_NETWORK_TYPE_BCH == type);
    
    return cryptoPaymentProtocolRequestBitPayBuilderCreateAsBTC (type,
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

//TODO:PPR refactor as a generic interface?
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
    
    protoReq->type = type;
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
cryptoPaymentProtocolRequestValidateSupported (BRCryptoPaymentProtocolType type,
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

    //TODO:PPR
    //return wallet->paymentProtocolHandlers->validateSupported (type);
    switch (cryptoWalletGetType (wallet)) {
        case CRYPTO_NETWORK_TYPE_BTC: {
            return AS_CRYPTO_BOOLEAN (CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY == type ||
                                      CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70 == type);
        }
        default: {
            break;
        }
    }

    return CRYPTO_FALSE;
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
    return NULL;//TODO:PPR where to get the handler for this? BRCryptoWallet ?
}

static void
cryptoPaymentProtocolRequestRelease (BRCryptoPaymentProtocolRequest protoReq) {
    assert (CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY == protoReq->paymentProtocolType ||
            CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70 == protoReq->paymentProtocolType);
    
    protoReq->handlers->release (protoReq);

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
    BRCryptoBoolean isSecure = CRYPTO_FALSE;
    switch (protoReq->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRPaymentProtocolRequest *request = protoReq->u.btc.request;
            isSecure = AS_CRYPTO_BOOLEAN (NULL != request->pkiType && 0 != strcmp (request->pkiType, "none"));
            break;
        }
        default: {
            assert (0);
            break;
        }
    }
    return isSecure;
}

extern const char *
cryptoPaymentProtocolRequestGetMemo (BRCryptoPaymentProtocolRequest protoReq) {
    const char *memo = NULL;
    switch (protoReq->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            memo = protoReq->u.btc.request->details->memo;
            break;
        }
        default: {
            assert (0);
            break;
        }
    }
    return memo;
}

extern const char *
cryptoPaymentProtocolRequestGetPaymentURL (BRCryptoPaymentProtocolRequest protoReq) {
    const char *paymentURL = NULL;
    switch (protoReq->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            paymentURL = protoReq->u.btc.request->details->paymentURL;
            break;
        }
        default: {
            assert (0);
            break;
        }
    }
    return paymentURL;
}

extern BRCryptoAmount
cryptoPaymentProtocolRequestGetTotalAmount (BRCryptoPaymentProtocolRequest protoReq) {
    BRCryptoAmount amount = NULL;
    switch (protoReq->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRPaymentProtocolRequest *request = protoReq->u.btc.request;
            uint64_t satoshis = 0;
            for (size_t index = 0; index < request->details->outCount; index++) {
                BRTxOutput *output = &request->details->outputs[index];
                satoshis += output->amount;
            }

            BRCryptoUnit baseUnit = cryptoNetworkGetUnitAsBase (protoReq->cryptoNetwork, protoReq->cryptoCurrency);
            amount = cryptoAmountCreate (baseUnit, CRYPTO_FALSE, uint256Create (satoshis));
            cryptoUnitGive (baseUnit);
            break;
        }
        default: {
            assert (0);
            break;
        }
    }
    return amount;
}

extern BRCryptoNetworkFee
cryptoPaymentProtocolRequestGetRequiredNetworkFee (BRCryptoPaymentProtocolRequest protoReq) {
    return cryptoNetworkFeeTake (protoReq->requiredFee);
}

extern BRCryptoAddress
cryptoPaymentProtocolRequestGetPrimaryTargetAddress (BRCryptoPaymentProtocolRequest protoReq) {
    BRCryptoAddress address = NULL;
    switch (protoReq->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRPaymentProtocolRequest *request = protoReq->u.btc.request;
            if (0 != request->details->outCount) {
                const BRChainParams * chainParams = cryptoNetworkAsBTC (protoReq->cryptoNetwork);
                int isBTC = BRChainParamsIsBitcoin (chainParams);

                BRTxOutput *output = &request->details->outputs[0];
                size_t addressSize = BRTxOutputAddress (output, NULL, 0, chainParams->addrParams);
                char *addressString = malloc (addressSize);
                BRTxOutputAddress (output, addressString, addressSize, chainParams->addrParams);

                address = cryptoAddressCreateAsBTC (BRAddressFill (chainParams->addrParams, addressString),
                                                    AS_CRYPTO_BOOLEAN (isBTC));
                free (addressString);
            }
            break;
        }
        default: {
            assert (0);
            break;
        }
    }
    return address;
}

extern char *
cryptoPaymentProtocolRequestGetCommonName (BRCryptoPaymentProtocolRequest protoReq) {
    char * name = NULL;
    switch (protoReq->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRArrayOf(uint8_t *) certBytes;
            BRArrayOf(size_t) certLens;

            array_new (certBytes, 10);
            array_new (certLens, 10);

            size_t certLen, index = 0;
            while (( certLen = BRPaymentProtocolRequestCert (protoReq->u.btc.request, NULL, 0, index)) > 0) {
                uint8_t *cert = malloc (certLen);
                BRPaymentProtocolRequestCert (protoReq->u.btc.request, cert, certLen, index);

                array_add (certBytes, cert);
                array_add (certLens, certLen);

                index++;
            }

            name = protoReq->u.btc.callbacks.nameExtractor (protoReq,
                                                            protoReq->u.btc.callbacks.context,
                                                            protoReq->u.btc.request->pkiType,
                                                            certBytes, certLens, array_count (certBytes));

            for (index = 0; index < array_count(certBytes); index++) {
                free (certBytes[index]);
            }
            array_free (certBytes);
            array_free (certLens);
            break;
        }
        default: {
            assert (0);
            break;
        }
    }
    return name;
}

extern BRCryptoPaymentProtocolError
cryptoPaymentProtocolRequestIsValid (BRCryptoPaymentProtocolRequest protoReq) {
    BRCryptoPaymentProtocolError error = CRYPTO_PAYMENT_PROTOCOL_ERROR_NONE;
    switch (protoReq->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            uint8_t *digest = NULL;
            size_t digestLen = BRPaymentProtocolRequestDigest(protoReq->u.btc.request, NULL, 0);
            if (digestLen) {
                digest = malloc (digestLen);
                BRPaymentProtocolRequestDigest(protoReq->u.btc.request, digest, digestLen);
            }

            BRArrayOf(uint8_t *) certs;
            BRArrayOf(size_t) certLens;

            array_new (certs, 10);
            array_new (certLens, 10);

            size_t certLen, index = 0;
            while (( certLen = BRPaymentProtocolRequestCert (protoReq->u.btc.request, NULL, 0, index)) > 0) {
                uint8_t *cert = malloc (certLen);
                BRPaymentProtocolRequestCert (protoReq->u.btc.request, cert, certLen, index);

                array_add (certs, cert);
                array_add (certLens, certLen);

                index++;
            }

            error = protoReq->u.btc.callbacks.validator (protoReq,
                                                         protoReq->u.btc.callbacks.context,
                                                         protoReq->u.btc.request->pkiType,
                                                         protoReq->u.btc.request->details->expires,
                                                         certs, certLens, array_count (certs),
                                                         digest, digestLen,
                                                         protoReq->u.btc.request->signature, protoReq->u.btc.request->sigLen);

            for (index = 0; index < array_count(certs); index++) {
                free (certs[index]);
            }
            array_free (certs);
            array_free (certLens);

            if (digest) {
                free (digest);
            }
            break;
        }
        default: {
            assert (0);
            break;
        }
    }
    return error;
}

private_extern BRArrayOf(BRTxOutput)
cryptoPaymentProtocolRequestGetOutputsAsBTC (BRCryptoPaymentProtocolRequest protoReq) {
    BRArrayOf(BRTxOutput) outputs = NULL;
    switch (protoReq->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRPaymentProtocolRequest *request = protoReq->u.btc.request;
            if (0 != request->details->outCount) {
                array_new(outputs, request->details->outCount);
                array_add_array(outputs, request->details->outputs, request->details->outCount);
            }
            break;
        }
        default: {
            assert (0);
            break;
        }
    }
    return outputs;
}

const static uint8_t*
cryptoPaymentProtocolRequestGetMerchantData (BRCryptoPaymentProtocolRequest protoReq, size_t *merchantDataLen) {
    uint8_t *merchantData = NULL;
    switch (protoReq->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRPaymentProtocolRequest *request = protoReq->u.btc.request;
            merchantData = request->details->merchantData;
            *merchantDataLen = request->details->merchDataLen;
            break;
        }
        default: {
            assert (0);
            break;
        }
    }
    return merchantData;
}

static BRCryptoNetwork
cryptoPaymentProtocolRequestGetNetwork (BRCryptoPaymentProtocolRequest protoReq) {
    return cryptoNetworkTake (protoReq->cryptoNetwork);
}

static BRCryptoCurrency
cryptoPaymentProtocolRequestGetCurrency (BRCryptoPaymentProtocolRequest protoReq) {
    return cryptoCurrencyTake (protoReq->cryptoCurrency);
}

/// Mark: Payment Protocol Payment

static void
cryptoPaymentProtocolPaymentRelease (BRCryptoPaymentProtocolPayment protoPay);

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoPaymentProtocolPayment, cryptoPaymentProtocolPayment)

static BRCryptoPaymentProtocolPayment
cryptoPaymentProtocolPaymentCreateInternal(BRCryptoPaymentProtocolType type, BRCryptoPaymentProtocolRequest protoReq) {
    BRCryptoPaymentProtocolPayment protoPay = calloc (1, sizeof (struct BRCryptoPaymentProtocolPaymentRecord));
    protoPay->ref = CRYPTO_REF_ASSIGN (cryptoPaymentProtocolPaymentRelease);
    protoPay->type = type;

    protoPay->request = cryptoPaymentProtocolRequestTake (protoReq);
    protoPay->cryptoNetwork = cryptoPaymentProtocolRequestGetNetwork (protoReq);
    protoPay->cryptoCurrency = cryptoPaymentProtocolRequestGetCurrency (protoReq);

    return protoPay;
}

extern BRCryptoPaymentProtocolPayment
cryptoPaymentProtocolPaymentCreate (BRCryptoPaymentProtocolRequest protoReq,
                                    BRCryptoTransfer transfer,
                                    BRCryptoAddress refundAddress) {
    BRCryptoPaymentProtocolPayment protoPay = NULL;

    switch (cryptoPaymentProtocolRequestGetType (protoReq)) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY: {
            protoPay = cryptoPaymentProtocolPaymentCreateInternal (CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY, protoReq);
            protoPay->u.btcBitPay.transaction = BRTransactionCopy (cryptoTransferAsBTC (transfer));
            break;
        }
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRCryptoNetwork cryptoNetwork = cryptoPaymentProtocolRequestGetNetwork (protoReq);
            BRCryptoCurrency cryptoCurrency = cryptoPaymentProtocolRequestGetCurrency (protoReq);

            BRCryptoAmount refundAmount = cryptoPaymentProtocolRequestGetTotalAmount (protoReq);
            BRCryptoUnit baseUnit = cryptoNetworkGetUnitAsBase (cryptoNetwork, cryptoCurrency);

            BRCryptoAmount baseAmount = cryptoAmountConvertToUnit (refundAmount, baseUnit);
            if (NULL != baseAmount) {
                BRCryptoBoolean overflow = CRYPTO_TRUE;
                uint64_t refundAmountInt = cryptoAmountGetIntegerRaw (baseAmount, &overflow);

                BRCryptoBoolean isAddressBTC = 0;
                BRAddress refundAddressBtc = cryptoAddressAsBTC (refundAddress, &isAddressBTC);

                const BRChainParams * chainParams = cryptoNetworkAsBTC (cryptoNetwork);
                BRCryptoBoolean isBTC = AS_CRYPTO_BOOLEAN (BRChainParamsIsBitcoin (chainParams));

                if (isBTC == isAddressBTC && CRYPTO_FALSE == overflow) {
                    size_t merchantDataLen = 0;
                    const uint8_t *merchantData = cryptoPaymentProtocolRequestGetMerchantData (protoReq, &merchantDataLen);

                    protoPay = cryptoPaymentProtocolPaymentCreateInternal (CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70, protoReq);
                    protoPay->u.btcBip70.transaction = BRTransactionCopy (cryptoTransferAsBTC (transfer));
                    protoPay->u.btcBip70.payment     = BRPaymentProtocolPaymentNew (merchantData,
                                                                                    merchantDataLen,
                                                                                    &protoPay->u.btcBip70.transaction, 1,
                                                                                    &refundAmountInt,
                                                                                    chainParams->addrParams,
                                                                                    &refundAddressBtc,
                                                                                    1, NULL);
                }

                cryptoAmountGive (baseAmount);
            }

            cryptoUnitGive (baseUnit);
            cryptoAmountGive (refundAmount);

            cryptoCurrencyGive (cryptoCurrency);
            cryptoNetworkGive (cryptoNetwork);
            break;
        }
        default: {
            assert (0);
            break;
        }
    }

    return protoPay;
}

static void
cryptoPaymentProtocolPaymentRelease (BRCryptoPaymentProtocolPayment protoPay) {
    switch (protoPay->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY: {
            BRTransactionFree (protoPay->u.btcBitPay.transaction);
            break;
        }
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRTransactionFree (protoPay->u.btcBip70.transaction);
            BRPaymentProtocolPaymentFree (protoPay->u.btcBip70.payment);
            break;
        }
        default: {
            break;
        }
    }

    cryptoNetworkGive (protoPay->cryptoNetwork);
    cryptoCurrencyGive (protoPay->cryptoCurrency);
    cryptoPaymentProtocolRequestGive (protoPay->request);
    memset (protoPay, 0, sizeof(*protoPay));
    free (protoPay);
}

extern uint8_t *
cryptoPaymentProtocolPaymentEncode(BRCryptoPaymentProtocolPayment protoPay,
                                   size_t *encodedLen) {
    uint8_t * encoded = NULL;

    assert (NULL != encodedLen);
    switch (protoPay->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY: {
            size_t transactionBufLen = BRTransactionSerialize (protoPay->u.btcBitPay.transaction, NULL, 0);
            if (0 != transactionBufLen) {
                BRArrayOf(uint8_t) encodedArray;
                array_new (encodedArray, 512);
                array_add (encodedArray, '{');

                #define PP_JSON_CURRENCY_PRE    "\"currency\":\""
                #define PP_JSON_CURRENCY_PRE_SZ (sizeof(PP_JSON_CURRENCY_PRE) - 1)
                array_add_array (encodedArray, PP_JSON_CURRENCY_PRE, PP_JSON_CURRENCY_PRE_SZ);

                const char *currencyCode = cryptoCurrencyGetCode (protoPay->cryptoCurrency);
                size_t currencyCodeLen = strlen(currencyCode);
                for (size_t index = 0; index < currencyCodeLen; index++) {
                    array_add (encodedArray, toupper(currencyCode[index]));
                }

                #define PP_JSON_CURRENCY_PST    "\","
                #define PP_JSON_CURRENCY_PST_SZ (sizeof(PP_JSON_CURRENCY_PST) - 1)
                array_add_array (encodedArray, PP_JSON_CURRENCY_PST, PP_JSON_CURRENCY_PST_SZ);

                #define PP_JSON_TXNS_PRE        "\"transactions\": [\""
                #define PP_JSON_TXNS_PRE_SZ     (sizeof(PP_JSON_TXNS_PRE) - 1)
                array_add_array (encodedArray, PP_JSON_TXNS_PRE, PP_JSON_TXNS_PRE_SZ);

                uint8_t *transactionBuf = malloc (transactionBufLen);
                BRTransactionSerialize (protoPay->u.btcBitPay.transaction, transactionBuf, transactionBufLen);
                size_t transactionHexLen = 0;
                char *transactionHex = hexEncodeCreate (&transactionHexLen, transactionBuf, transactionBufLen);

                array_add_array (encodedArray, transactionHex, transactionHexLen - 1);

                free (transactionHex);
                free (transactionBuf);

                #define PP_JSON_TXNS_PST        "\"]"
                #define PP_JSON_TXNS_PST_SZ     (sizeof(PP_JSON_TXNS_PST) - 1)
                array_add_array (encodedArray, PP_JSON_TXNS_PST, PP_JSON_TXNS_PST_SZ);

                array_add (encodedArray, '}');

                // This function returns a `uint8_t*`.  Normally to convert such an array to
                // a `char*` once needs to encode the result, typically as 'hex' or 'baseXYZ'.
                // However, as the actual data in the `uint8_t*` can be a true string in the
                // BitPay case and thus down the line a User might simply cast as `char*`, we'll
                // add a trailing `\0` to make sure that that thoughtless cast succeeds.
                array_add (encodedArray, '\0');

                *encodedLen = array_count (encodedArray);
                encoded = malloc(*encodedLen);
                memcpy (encoded, encodedArray, *encodedLen);
                *encodedLen -= 1; // don't include the NULL terminator in the count

                array_free (encodedArray);
            }
            break;
        }
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            *encodedLen = BRPaymentProtocolPaymentSerialize(protoPay->u.btcBip70.payment,
                                                            NULL,
                                                            0);
            if (0 != *encodedLen) {
                encoded = malloc(*encodedLen);
                BRPaymentProtocolPaymentSerialize(protoPay->u.btcBip70.payment,
                                                  encoded,
                                                  *encodedLen);
            }
            break;
        }
        default: {
            assert (0);
            break;
        }
    }

    return encoded;
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
        protoAck->paymentProtocolType = CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70;

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
