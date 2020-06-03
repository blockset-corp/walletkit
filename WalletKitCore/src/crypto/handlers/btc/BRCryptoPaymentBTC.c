//
//  BRCryptoPaymentBTC.c
//  BRCore
//
//  Created by Ehsan Rezaie on 5/26/20
//  Copyright © 2020 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoBTC.h"
#include "bcash/BRBCashAddr.h"
#include "support/BRArray.h"

// MARK: - BitPay Payment Protocol Request Builder

static BRCryptoPaymentProtocolRequestBitPayBuilderBTC
cryptoPaymentProtocolRequestBitPayBuilderCoerceBTC (BRCryptoPaymentProtocolRequestBitPayBuilder builder) {
    assert (CRYPTO_NETWORK_TYPE_BTC == builder->type ||
            CRYPTO_NETWORK_TYPE_BCH == builder->type);
    return (BRCryptoPaymentProtocolRequestBitPayBuilderBTC) builder;
}

extern BRCryptoPaymentProtocolRequestBitPayBuilder
cryptoPaymentProtocolRequestBitPayBuilderCreateAsBTC (BRCryptoBlockChainType type,
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
    BRCryptoPaymentProtocolRequestBitPayBuilder builderBase = cryptoPaymentProtocolRequestBitPayBuilderAllocAndInit (sizeof (struct BRCryptoPaymentProtocolRequestBitPayBuilderBTCRecord),
                                                                                                                     type,
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
    BRCryptoPaymentProtocolRequestBitPayBuilderBTC builder = cryptoPaymentProtocolRequestBitPayBuilderCoerceBTC (builderBase);
    
    array_new (builder->outputs, 10);

    return builderBase;
}

static void cryptoPaymentProtocolRequestBitPayBuilderAddOutputBTC (BRCryptoPaymentProtocolRequestBitPayBuilder builderBase,
                                                            const char *address,
                                                            uint64_t satoshis) {
    BRCryptoPaymentProtocolRequestBitPayBuilderBTC builder = cryptoPaymentProtocolRequestBitPayBuilderCoerceBTC (builderBase);
    
    if (satoshis) {
        const BRChainParams * chainParams = cryptoNetworkAsBTC (builderBase->cryptoNetwork);
        int isBTC = BRChainParamsIsBitcoin (chainParams);

        if (isBTC) {
            if (BRAddressIsValid (chainParams->addrParams, address)) {
                BRTxOutput output = {0};
                BRTxOutputSetAddress (&output, chainParams->addrParams, address);
                output.amount = satoshis;
                array_add (builder->outputs, output);
            }
        } else {
            char cashAddr[36];
            if (0 != BRBCashAddrDecode (cashAddr, address) && !BRAddressIsValid(chainParams->addrParams, address)) {
                BRTxOutput output = {0};
                BRTxOutputSetAddress (&output, chainParams->addrParams, cashAddr);
                output.amount = satoshis;
                array_add (builder->outputs, output);
            }
        }
    }
}

static void
cryptoPaymentProtocolRequestBitPayBuilderReleaseBTC (BRCryptoPaymentProtocolRequestBitPayBuilder builderBase) {
    BRCryptoPaymentProtocolRequestBitPayBuilderBTC builder = cryptoPaymentProtocolRequestBitPayBuilderCoerceBTC (builderBase);
    for (size_t index = 0; index < array_count (builder->outputs); index++ ) {
        const BRChainParams * chainParams = cryptoNetworkAsBTC (builderBase->cryptoNetwork);
        BRTxOutputSetAddress (&builder->outputs[index], chainParams->addrParams, NULL);
    }
    array_free (builder->outputs);
}

// MARK: - Payment Protocol Request

static BRCryptoPaymentProtocolRequestBTC
cryptoPaymentProtocolRequestCoerceBTC (BRCryptoPaymentProtocolRequest protoReq) {
    assert (CRYPTO_NETWORK_TYPE_BTC == protoReq->type ||
            CRYPTO_NETWORK_TYPE_BCH == protoReq->type);
    return (BRCryptoPaymentProtocolRequestBTC) protoReq;
}

extern BRCryptoBoolean
cryptoPaymentProtocolRequestValidateSupportedBTC (BRCryptoPaymentProtocolType type) {
    return AS_CRYPTO_BOOLEAN (CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY == type ||
                              CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70 == type);
}

extern BRCryptoPaymentProtocolRequest
cryptoPaymentProtocolRequestCreateForBitPayBTC (BRCryptoPaymentProtocolRequestBitPayBuilder builderBase) {
    BRCryptoPaymentProtocolRequestBitPayBuilderBTC builder = cryptoPaymentProtocolRequestBitPayBuilderCoerceBTC (builderBase);
    
    BRCryptoBlockChainType type = cryptoNetworkGetType (builderBase->cryptoNetwork);
    
    BRCryptoPaymentProtocolRequest protoReqBase = NULL;

    if ((CRYPTO_NETWORK_TYPE_BTC == type) &&
        cryptoNetworkHasCurrency(builderBase->cryptoNetwork, builderBase->cryptoCurrency) &&
        0 != array_count (builder->outputs) && 0 != builder->outputs[0].amount && 0 != builder->outputs[0].scriptLen) {
        
        BRPaymentProtocolDetails *details = BRPaymentProtocolDetailsNew (builderBase->network,
                                                                         builder->outputs,
                                                                         array_count (builder->outputs),
                                                                         builderBase->time,
                                                                         builderBase->expires,
                                                                         builderBase->memo,
                                                                         builderBase->paymentURL,
                                                                         builderBase->merchantData,
                                                                         builderBase->merchantDataLen);
        
        BRPaymentProtocolRequest *request = BRPaymentProtocolRequestNew (1,
                                                                         "none",
                                                                         NULL,
                                                                         0,
                                                                         details,
                                                                         NULL,
                                                                         0);
        
        if (NULL != request) {
            BRCryptoNetworkFee requiredFee = NULL;
            if (0 != builderBase->feeCostFactor) {
                BRCryptoUnit feeUnit = cryptoNetworkGetUnitAsBase (builderBase->cryptoNetwork, builderBase->cryptoCurrency);
                BRCryptoAmount feeAmount = cryptoAmountCreateDouble (builderBase->feeCostFactor, feeUnit);
                requiredFee = cryptoNetworkFeeCreate (0, feeAmount, feeUnit);
                cryptoAmountGive (feeAmount);
                cryptoUnitGive (feeUnit);
            }
            
            protoReqBase = cryptoPaymentProtocolRequestAllocAndInit(sizeof (struct BRCryptoPaymentProtocolRequestBTCRecord),
                                                                    type,
                                                                    CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY,
                                                                    builderBase->cryptoNetwork,
                                                                    builderBase->cryptoCurrency,
                                                                    requiredFee,
                                                                    builderBase->callbacks);
            cryptoNetworkFeeGive (requiredFee);
            
            BRCryptoPaymentProtocolRequestBTC protoReq = cryptoPaymentProtocolRequestCoerceBTC (protoReqBase);
            protoReq->request = request;
        } else {
            BRPaymentProtocolDetailsFree (details);
        }
    }
    
    return protoReqBase;
}

extern BRCryptoPaymentProtocolRequest
cryptoPaymentProtocolRequestCreateForBip70BTC (BRCryptoNetwork cryptoNetwork,
                                               BRCryptoCurrency cryptoCurrency,
                                               BRCryptoPayProtReqBip70Callbacks callbacks,
                                               uint8_t *serialization,
                                               size_t serializationLen) {
    BRCryptoBlockChainType type = cryptoNetworkGetType (cryptoNetwork);
    
    BRCryptoPaymentProtocolRequest protoReqBase = NULL;
    
    if ((CRYPTO_NETWORK_TYPE_BTC == type) &&
        (cryptoNetworkHasCurrency(cryptoNetwork, cryptoCurrency))) {
        
        BRPaymentProtocolRequest *request = BRPaymentProtocolRequestParse (serialization,
                                                                        serializationLen);
        if (NULL != request) {
            protoReqBase = cryptoPaymentProtocolRequestAllocAndInit(sizeof (struct BRCryptoPaymentProtocolRequestBTCRecord),
                                                                                               type,
                                                                                               CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70, cryptoNetwork,
                                                                                               cryptoCurrency,
                                                                                               NULL,
                                                                                               callbacks);
            
            BRCryptoPaymentProtocolRequestBTC protoReq = cryptoPaymentProtocolRequestCoerceBTC (protoReqBase);
            protoReq->request = request;
        }
    }
    
    return protoReqBase;
}

static void
cryptoPaymentProtocolRequestReleaseBTC (BRCryptoPaymentProtocolRequest protoReqBase) {
    BRCryptoPaymentProtocolRequestBTC protoReq = cryptoPaymentProtocolRequestCoerceBTC(protoReqBase);
    BRPaymentProtocolRequestFree (protoReq->request);
}

// MARK: -

BRCryptoPaymentProtocolHandlers cryptoPaymentProtocolHandlersBTC = {
    cryptoPaymentProtocolRequestBitPayBuilderAddOutputBTC,
    cryptoPaymentProtocolRequestBitPayBuilderReleaseBTC,
    
    cryptoPaymentProtocolRequestValidateSupportedBTC,
    cryptoPaymentProtocolRequestCreateForBitPayBTC,
    cryptoPaymentProtocolRequestCreateForBip70BTC,
    cryptoPaymentProtocolRequestReleaseBTC
};
