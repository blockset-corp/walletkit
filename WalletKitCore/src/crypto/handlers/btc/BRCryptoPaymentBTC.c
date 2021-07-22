//
//  BRCryptoPaymentBTC.c
//  BRCore
//
//  Created by Ehsan Rezaie on 5/26/20.
//  Based on implementation by Michael Carrara.
//  Copyright © 2020 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoBTC.h"
#include "crypto/BRCryptoPaymentP.h"

#include <ctype.h>
#include <string.h>

#include "bcash/BRBCashAddr.h"
#include "bcash/BRBCashParams.h"
#include "support/BRArray.h"
#include "crypto/BRCryptoAmountP.h"
#include "ethereum/util/BRUtil.h"

// MARK: Forward Declaration

static BRArrayOf(BRTxOutput)
cryptoPaymentProtocolRequestGetOutputsAsBTC (BRCryptoPaymentProtocolRequest protoReqBase);

// MARK: - BitPay Payment Protocol Request Builder

static BRCryptoPaymentProtocolRequestBitPayBuilderBTC
cryptoPaymentProtocolRequestBitPayBuilderCoerceBTC (BRCryptoPaymentProtocolRequestBitPayBuilder builder) {
    assert (builder->type == CRYPTO_NETWORK_TYPE_BTC ||
            builder->type == CRYPTO_NETWORK_TYPE_BCH );
    return (BRCryptoPaymentProtocolRequestBitPayBuilderBTC) builder;
}

static BRCryptoPaymentProtocolRequestBitPayBuilder
cryptoPaymentProtocolRequestBitPayBuilderCreateBTC (BRCryptoBlockChainType chainType,
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
                                                                                                                     chainType,
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

        // TODO: Use CRYPTO_NETWORK_TYPE_{BTC,BCH,BSV}
        if (BRChainParamsIsBitcoin (chainParams)) {
            if (BRAddressIsValid (chainParams->addrParams, address)) {
                BRTxOutput output = {0};
                BRTxOutputSetAddress (&output, chainParams->addrParams, address);
                output.amount = satoshis;
                array_add (builder->outputs, output);
            }
        } else if (BRChainParamsIsBitcash (chainParams)) {
            char cashAddr[36];
            if (0 != BRBCashAddrDecode (cashAddr, address) && !BRAddressIsValid(chainParams->addrParams, address)) {
                BRTxOutput output = {0};
                BRTxOutputSetAddress (&output, chainParams->addrParams, cashAddr);
                output.amount = satoshis;
                array_add (builder->outputs, output);
            }
        } else {
            // No BitcoinSV
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
    assert (CRYPTO_NETWORK_TYPE_BTC == protoReq->chainType ||
            CRYPTO_NETWORK_TYPE_BCH == protoReq->chainType);
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

    if ((CRYPTO_NETWORK_TYPE_BTC == type || CRYPTO_NETWORK_TYPE_BCH == type) &&
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
    
    if ((CRYPTO_NETWORK_TYPE_BTC == type || CRYPTO_NETWORK_TYPE_BCH == type) &&
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

extern BRCryptoFeeBasis
cryptoPaymentProtocolRequestEstimateFeeBasisBTC (BRCryptoPaymentProtocolRequest protoReqBase,
                                                 BRCryptoWalletManager cwm,
                                                 BRCryptoWallet wallet,
                                                 BRCryptoCookie cookie,
                                                 BRCryptoNetworkFee networkFee) {
    BRWallet *wid = cryptoWalletAsBTC (wallet);
    uint64_t btcFeePerKB = 1000 * cryptoNetworkFeeAsBTC (networkFee);
    uint64_t btcFee = 0;
    
    assert (CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY == protoReqBase->paymentProtocolType ||
            CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70 == protoReqBase->paymentProtocolType);
    
    BRArrayOf(BRTxOutput) outputs = cryptoPaymentProtocolRequestGetOutputsAsBTC (protoReqBase);
    if (NULL != outputs) {
        BRTransaction *transaction = BRWalletCreateTxForOutputsWithFeePerKb (wid, btcFeePerKB, outputs, array_count (outputs));
        
        if (NULL != transaction) {
            btcFee = BRWalletFeeForTx(wid, transaction);
            BRTransactionFree(transaction);
        }
        array_free (outputs);
    }
    
    return cryptoFeeBasisCreateAsBTC (wallet->unitForFee, btcFee, btcFeePerKB, CRYPTO_FEE_BASIS_BTC_SIZE_UNKNOWN);
}

extern BRCryptoTransfer
cryptoPaymentProtocolRequestCreateTransferBTC (BRCryptoPaymentProtocolRequest protoReq,
                                               BRCryptoWallet wallet,
                                               BRCryptoFeeBasis estimatedFeeBasis) {
    BRCryptoTransfer transfer = NULL;
    
    BRCryptoUnit unit       = cryptoWalletGetUnit (wallet);
    BRCryptoUnit unitForFee = cryptoWalletGetUnitForFee (wallet);
    
    switch (wallet->type) {
        case CRYPTO_NETWORK_TYPE_BTC:
        case CRYPTO_NETWORK_TYPE_BCH: {
            BRWallet *wid = cryptoWalletAsBTC (wallet);
            
            switch (cryptoPaymentProtocolRequestGetType (protoReq)) {
                case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
                case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
                    BRArrayOf(BRTxOutput) outputs = cryptoPaymentProtocolRequestGetOutputsAsBTC (protoReq);
                    if (NULL != outputs) {
                        uint64_t feePerKb = cryptoFeeBasisAsBTC (estimatedFeeBasis);
                        BRTransaction *tid = BRWalletCreateTxForOutputsWithFeePerKb (wid, feePerKb, outputs, array_count (outputs));

                        transfer = NULL == tid ? NULL : cryptoTransferCreateAsBTC (wallet->listenerTransfer,
                                                                                   unit,
                                                                                   unitForFee,
                                                                                   wid,
                                                                                   tid,
                                                                                   wallet->type);
                        array_free (outputs);
                    }
                    break;
                }
                default: {
                    assert (0);
                    break;
                }
            }
            break;
        }
        default: {
            assert (0);
            break;
        }
    }
    
    cryptoUnitGive (unitForFee);
    cryptoUnitGive (unit);
    
    return transfer;
}

extern BRCryptoBoolean
cryptoPaymentProtocolRequestIsSecureBTC (BRCryptoPaymentProtocolRequest protoReqBase) {
    BRCryptoPaymentProtocolRequestBTC protoReq = cryptoPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    BRCryptoBoolean isSecure = CRYPTO_FALSE;
    switch (protoReqBase->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRPaymentProtocolRequest *request = protoReq->request;
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
cryptoPaymentProtocolRequestGetMemoBTC (BRCryptoPaymentProtocolRequest protoReqBase) {
    BRCryptoPaymentProtocolRequestBTC protoReq = cryptoPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    const char *memo = NULL;
    switch (protoReqBase->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRPaymentProtocolRequest *request = protoReq->request;
            memo = request->details->memo;
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
cryptoPaymentProtocolRequestGetPaymentURLBTC (BRCryptoPaymentProtocolRequest protoReqBase) {
    BRCryptoPaymentProtocolRequestBTC protoReq = cryptoPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    const char *paymentURL = NULL;
    switch (protoReqBase->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRPaymentProtocolRequest *request = protoReq->request;
            paymentURL = request->details->paymentURL;
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
cryptoPaymentProtocolRequestGetTotalAmountBTC (BRCryptoPaymentProtocolRequest protoReqBase) {
    BRCryptoPaymentProtocolRequestBTC protoReq = cryptoPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    BRCryptoAmount amount = NULL;
    switch (protoReqBase->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRPaymentProtocolRequest *request = protoReq->request;
            uint64_t satoshis = 0;
            for (size_t index = 0; index < request->details->outCount; index++) {
                BRTxOutput *output = &request->details->outputs[index];
                satoshis += output->amount;
            }

            BRCryptoUnit baseUnit = cryptoNetworkGetUnitAsBase (protoReqBase->cryptoNetwork, protoReqBase->cryptoCurrency);
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

extern BRCryptoAddress
cryptoPaymentProtocolRequestGetPrimaryTargetAddressBTC (BRCryptoPaymentProtocolRequest protoReqBase) {
    BRCryptoPaymentProtocolRequestBTC protoReq = cryptoPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    BRCryptoAddress address = NULL;
    switch (protoReqBase->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRPaymentProtocolRequest *request = protoReq->request;
            if (0 != request->details->outCount) {
                const BRChainParams * chainParams = cryptoNetworkAsBTC (protoReqBase->cryptoNetwork);

                BRTxOutput *output = &request->details->outputs[0];
                size_t addressSize = BRTxOutputAddress (output, NULL, 0, chainParams->addrParams);
                char *addressString = malloc (addressSize);
                BRTxOutputAddress (output, addressString, addressSize, chainParams->addrParams);

                address = cryptoAddressCreateAsBTC (protoReqBase->chainType, BRAddressFill (chainParams->addrParams, addressString));
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
cryptoPaymentProtocolRequestGetCommonNameBTC (BRCryptoPaymentProtocolRequest protoReqBase) {
    BRCryptoPaymentProtocolRequestBTC protoReq = cryptoPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    char * name = NULL;
    switch (protoReqBase->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRArrayOf(uint8_t *) certBytes;
            BRArrayOf(size_t) certLens;
            
            array_new (certBytes, 10);
            array_new (certLens, 10);
            
            size_t certLen, index = 0;
            while (( certLen = BRPaymentProtocolRequestCert (protoReq->request, NULL, 0, index)) > 0) {
                uint8_t *cert = malloc (certLen);
                BRPaymentProtocolRequestCert (protoReq->request, cert, certLen, index);
                
                array_add (certBytes, cert);
                array_add (certLens, certLen);
                
                index++;
            }
            
            name = protoReqBase->callbacks.nameExtractor (protoReqBase,
                                                          protoReqBase->callbacks.context,
                                                          protoReq->request->pkiType,
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
cryptoPaymentProtocolRequestIsValidBTC (BRCryptoPaymentProtocolRequest protoReqBase) {
    BRCryptoPaymentProtocolRequestBTC protoReq = cryptoPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    BRCryptoPaymentProtocolError error = CRYPTO_PAYMENT_PROTOCOL_ERROR_NONE;
    switch (protoReqBase->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            uint8_t *digest = NULL;
            size_t digestLen = BRPaymentProtocolRequestDigest(protoReq->request, NULL, 0);
            if (digestLen) {
                digest = malloc (digestLen);
                BRPaymentProtocolRequestDigest(protoReq->request, digest, digestLen);
            }
            
            BRArrayOf(uint8_t *) certs;
            BRArrayOf(size_t) certLens;
            
            array_new (certs, 10);
            array_new (certLens, 10);
            
            size_t certLen, index = 0;
            while (( certLen = BRPaymentProtocolRequestCert (protoReq->request, NULL, 0, index)) > 0) {
                uint8_t *cert = malloc (certLen);
                BRPaymentProtocolRequestCert (protoReq->request, cert, certLen, index);
                
                array_add (certs, cert);
                array_add (certLens, certLen);
                
                index++;
            }
            
            error = protoReqBase->callbacks.validator (protoReqBase,
                                                       protoReqBase->callbacks.context,
                                                       protoReq->request->pkiType,
                                                       protoReq->request->details->expires,
                                                       certs, certLens, array_count (certs),
                                                       digest, digestLen,
                                                       protoReq->request->signature, protoReq->request->sigLen);
            
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

static const uint8_t*
cryptoPaymentProtocolRequestGetMerchantDataBTC (BRCryptoPaymentProtocolRequest protoReqBase, size_t *merchantDataLen) {
    BRCryptoPaymentProtocolRequestBTC protoReq = cryptoPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    uint8_t *merchantData = NULL;
    switch (protoReqBase->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRPaymentProtocolRequest *request = protoReq->request;
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

extern void
cryptoPaymentProtocolRequestReleaseBTC (BRCryptoPaymentProtocolRequest protoReqBase) {
    BRCryptoPaymentProtocolRequestBTC protoReq = cryptoPaymentProtocolRequestCoerceBTC(protoReqBase);
    BRPaymentProtocolRequestFree (protoReq->request);
}

// MARK: - Payment Protocol Payment

static BRCryptoPaymentProtocolPaymentBTC
cryptoPaymentProtocolPaymentCoerceBTC (BRCryptoPaymentProtocolPayment payment) {
    assert (CRYPTO_NETWORK_TYPE_BTC == payment->chainType ||
            CRYPTO_NETWORK_TYPE_BCH == payment->chainType);
    return (BRCryptoPaymentProtocolPaymentBTC) payment;
}

extern BRCryptoPaymentProtocolPayment
cryptoPaymentProtocolPaymentCreateBTC (BRCryptoPaymentProtocolRequest protoReqBase,
                                       BRCryptoTransfer transfer,
                                       BRCryptoAddress refundAddress) {
    BRCryptoPaymentProtocolPayment protoPayBase = NULL;

    switch (cryptoPaymentProtocolRequestGetType (protoReqBase)) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY: {
            
            protoPayBase = cryptoPaymentProtocolPaymentAllocAndInit(sizeof (struct BRCryptoPaymentProtocolPaymentBTCRecord),
                                                                    protoReqBase);
            BRCryptoPaymentProtocolPaymentBTC protoPay = cryptoPaymentProtocolPaymentCoerceBTC (protoPayBase);
            protoPay->transaction = BRTransactionCopy (cryptoTransferAsBTC (transfer));
            protoPay->payment = NULL;
            break;
        }
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRCryptoNetwork cryptoNetwork = cryptoNetworkTake (protoReqBase->cryptoNetwork);
            BRCryptoCurrency cryptoCurrency = cryptoCurrencyTake (protoReqBase->cryptoCurrency);
            
            BRCryptoAmount refundAmount = cryptoPaymentProtocolRequestGetTotalAmount (protoReqBase);
            BRCryptoUnit baseUnit = cryptoNetworkGetUnitAsBase (cryptoNetwork, cryptoCurrency);
            
            BRCryptoAmount baseAmount = cryptoAmountConvertToUnit (refundAmount, baseUnit);
            if (NULL != baseAmount) {
                BRCryptoBoolean overflow = CRYPTO_TRUE;
                uint64_t refundAmountInt = cryptoAmountGetIntegerRaw (baseAmount, &overflow);

                BRCryptoBlockChainType refundType;
                BRAddress refundAddressBtc = cryptoAddressAsBTC (refundAddress, &refundType);
                
                const BRChainParams * chainParams = cryptoNetworkAsBTC (cryptoNetwork);
                BRCryptoBoolean isBTC = AS_CRYPTO_BOOLEAN (BRChainParamsIsBitcoin (chainParams));
                
                if (CRYPTO_TRUE == isBTC && CRYPTO_NETWORK_TYPE_BTC == refundType && CRYPTO_FALSE == overflow) {
                    size_t merchantDataLen = 0;
                    const uint8_t *merchantData = cryptoPaymentProtocolRequestGetMerchantDataBTC (protoReqBase, &merchantDataLen);
                    
                    protoPayBase = cryptoPaymentProtocolPaymentAllocAndInit(sizeof (struct BRCryptoPaymentProtocolPaymentBTCRecord),
                                                                            protoReqBase);
                    BRCryptoPaymentProtocolPaymentBTC protoPay = cryptoPaymentProtocolPaymentCoerceBTC (protoPayBase);
                    protoPay->transaction = BRTransactionCopy (cryptoTransferAsBTC (transfer));
                    protoPay->payment     = BRPaymentProtocolPaymentNew (merchantData,
                                                                         merchantDataLen,
                                                                         &protoPay->transaction, 1,
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
        default:
            assert (0);
            break;
    }
    
    return protoPayBase;
}

extern uint8_t *
cryptoPaymentProtocolPaymentEncodeBTC (BRCryptoPaymentProtocolPayment protoPayBase,
                                       size_t *encodedLen) {
    BRCryptoPaymentProtocolPaymentBTC protoPay = cryptoPaymentProtocolPaymentCoerceBTC(protoPayBase);
    
    uint8_t * encoded = NULL;

    assert (NULL != encodedLen);
    switch (protoPayBase->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY: {
            size_t transactionBufLen = BRTransactionSerialize (protoPay->transaction, NULL, 0);
            if (0 != transactionBufLen) {
                BRArrayOf(uint8_t) encodedArray;
                array_new (encodedArray, 512);
                array_add (encodedArray, '{');

                #define PP_JSON_CURRENCY_PRE    "\"currency\":\""
                #define PP_JSON_CURRENCY_PRE_SZ (sizeof(PP_JSON_CURRENCY_PRE) - 1)
                array_add_array (encodedArray, (uint8_t*) PP_JSON_CURRENCY_PRE, PP_JSON_CURRENCY_PRE_SZ);

                const char *currencyCode = cryptoCurrencyGetCode (protoPayBase->cryptoCurrency);
                size_t currencyCodeLen = strlen(currencyCode);
                for (size_t index = 0; index < currencyCodeLen; index++) {
                    array_add (encodedArray, toupper(currencyCode[index]));
                }

                #define PP_JSON_CURRENCY_PST    "\","
                #define PP_JSON_CURRENCY_PST_SZ (sizeof(PP_JSON_CURRENCY_PST) - 1)
                array_add_array (encodedArray, (uint8_t*) PP_JSON_CURRENCY_PST, PP_JSON_CURRENCY_PST_SZ);

                #define PP_JSON_TXNS_PRE        "\"transactions\": [\""
                #define PP_JSON_TXNS_PRE_SZ     (sizeof(PP_JSON_TXNS_PRE) - 1)
                array_add_array (encodedArray, (uint8_t*) PP_JSON_TXNS_PRE, PP_JSON_TXNS_PRE_SZ);

                uint8_t *transactionBuf = malloc (transactionBufLen);
                BRTransactionSerialize (protoPay->transaction, transactionBuf, transactionBufLen);
                size_t transactionHexLen = 0;
                char *transactionHex = hexEncodeCreate (&transactionHexLen, transactionBuf, transactionBufLen);

                array_add_array (encodedArray, (uint8_t*) transactionHex, transactionHexLen - 1);

                free (transactionHex);
                free (transactionBuf);

                #define PP_JSON_TXNS_PST        "\"]"
                #define PP_JSON_TXNS_PST_SZ     (sizeof(PP_JSON_TXNS_PST) - 1)
                array_add_array (encodedArray, (uint8_t*) PP_JSON_TXNS_PST, PP_JSON_TXNS_PST_SZ);

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
            *encodedLen = BRPaymentProtocolPaymentSerialize(protoPay->payment,
                                                            NULL,
                                                            0);
            if (0 != *encodedLen) {
                encoded = malloc(*encodedLen);
                BRPaymentProtocolPaymentSerialize(protoPay->payment,
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

extern void
cryptoPaymentProtocolPaymentReleaseBTC (BRCryptoPaymentProtocolPayment protoPayBase) {
    BRCryptoPaymentProtocolPaymentBTC protoPay = cryptoPaymentProtocolPaymentCoerceBTC(protoPayBase);
    
    switch (protoPayBase->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY: {
            BRTransactionFree (protoPay->transaction);
            break;
        }
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRTransactionFree (protoPay->transaction);
            BRPaymentProtocolPaymentFree (protoPay->payment);
            break;
        }
        default: {
            break;
        }
    }
}

// MARK: - Support

static BRArrayOf(BRTxOutput)
cryptoPaymentProtocolRequestGetOutputsAsBTC (BRCryptoPaymentProtocolRequest protoReqBase) {
    BRCryptoPaymentProtocolRequestBTC protoReq = cryptoPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    BRArrayOf(BRTxOutput) outputs = NULL;
    switch (protoReqBase->paymentProtocolType) {
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRPaymentProtocolRequest *request = protoReq->request;
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

// MARK: -

BRCryptoPaymentProtocolHandlers cryptoPaymentProtocolHandlersBTC = {
    cryptoPaymentProtocolRequestBitPayBuilderCreateBTC,
    cryptoPaymentProtocolRequestBitPayBuilderAddOutputBTC,
    cryptoPaymentProtocolRequestBitPayBuilderReleaseBTC,
    
    cryptoPaymentProtocolRequestValidateSupportedBTC,
    cryptoPaymentProtocolRequestCreateForBitPayBTC,
    cryptoPaymentProtocolRequestCreateForBip70BTC,
    cryptoPaymentProtocolRequestEstimateFeeBasisBTC,
    cryptoPaymentProtocolRequestCreateTransferBTC,

    cryptoPaymentProtocolRequestIsSecureBTC,
    cryptoPaymentProtocolRequestGetMemoBTC,
    cryptoPaymentProtocolRequestGetPaymentURLBTC,
    cryptoPaymentProtocolRequestGetTotalAmountBTC,
    cryptoPaymentProtocolRequestGetPrimaryTargetAddressBTC,
    cryptoPaymentProtocolRequestGetCommonNameBTC,
    cryptoPaymentProtocolRequestIsValidBTC,
    cryptoPaymentProtocolRequestReleaseBTC,
    
    cryptoPaymentProtocolPaymentCreateBTC,
    cryptoPaymentProtocolPaymentEncodeBTC,
    cryptoPaymentProtocolPaymentReleaseBTC
};
