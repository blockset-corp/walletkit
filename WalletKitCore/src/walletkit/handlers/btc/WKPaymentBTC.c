//
//  WKPaymentBTC.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 5/26/20.
//  Based on implementation by Michael Carrara.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "WKBTC.h"
#include "walletkit/WKPaymentP.h"

#include <ctype.h>
#include <string.h>

#include "bcash/BRBCashAddr.h"
#include "bcash/BRBCashParams.h"
#include "support/BRArray.h"
#include "walletkit/WKAmountP.h"
#include "support/util/BRUtil.h"

// MARK: Forward Declaration

static BRArrayOf(BRBitcoinTxOutput)
wkPaymentProtocolRequestGetOutputsAsBTC (WKPaymentProtocolRequest protoReqBase);

// MARK: - BitPay Payment Protocol Request Builder

static WKPaymentProtocolRequestBitPayBuilderBTC
wkPaymentProtocolRequestBitPayBuilderCoerceBTC (WKPaymentProtocolRequestBitPayBuilder builder) {
    assert (WK_NETWORK_TYPE_BTC == builder->type);
    return (WKPaymentProtocolRequestBitPayBuilderBTC) builder;
}

static WKPaymentProtocolRequestBitPayBuilder
wkPaymentProtocolRequestBitPayBuilderCreateBTC (WKNetworkType chainType,
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
    WKPaymentProtocolRequestBitPayBuilder builderBase = wkPaymentProtocolRequestBitPayBuilderAllocAndInit (sizeof (struct WKPaymentProtocolRequestBitPayBuilderBTCRecord),
                                                                                                                     chainType,
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
    WKPaymentProtocolRequestBitPayBuilderBTC builder = wkPaymentProtocolRequestBitPayBuilderCoerceBTC (builderBase);
    
    array_new (builder->outputs, 10);

    return builderBase;
}

static void wkPaymentProtocolRequestBitPayBuilderAddOutputBTC (WKPaymentProtocolRequestBitPayBuilder builderBase,
                                                            const char *address,
                                                            uint64_t satoshis) {
    WKPaymentProtocolRequestBitPayBuilderBTC builder = wkPaymentProtocolRequestBitPayBuilderCoerceBTC (builderBase);
    
    if (satoshis) {
        const BRBitcoinChainParams * chainParams = wkNetworkAsBTC (builderBase->wkNetwork);

        // TODO: Use WK_NETWORK_TYPE_{BTC,BCH,BSV}
        if (btcChainParamsIsBitcoin (chainParams)) {
            if (BRAddressIsValid (chainParams->addrParams, address)) {
                BRBitcoinTxOutput output = {0};
                btcTxOutputSetAddress (&output, chainParams->addrParams, address);
                output.amount = satoshis;
                array_add (builder->outputs, output);
            }
        } else if (btcChainParamsIsBitcash (chainParams)) {
            char cashAddr[36];
            if (0 != bchAddrDecode (cashAddr, address) && !BRAddressIsValid(chainParams->addrParams, address)) {
                BRBitcoinTxOutput output = {0};
                btcTxOutputSetAddress (&output, chainParams->addrParams, cashAddr);
                output.amount = satoshis;
                array_add (builder->outputs, output);
            }
        } else {
            // No BitcoinSV
        }
    }
}

static void
wkPaymentProtocolRequestBitPayBuilderReleaseBTC (WKPaymentProtocolRequestBitPayBuilder builderBase) {
    WKPaymentProtocolRequestBitPayBuilderBTC builder = wkPaymentProtocolRequestBitPayBuilderCoerceBTC (builderBase);
    for (size_t index = 0; index < array_count (builder->outputs); index++ ) {
        const BRBitcoinChainParams * chainParams = wkNetworkAsBTC (builderBase->wkNetwork);
        btcTxOutputSetAddress (&builder->outputs[index], chainParams->addrParams, NULL);
    }
    array_free (builder->outputs);
}

// MARK: - Payment Protocol Request

static WKPaymentProtocolRequestBTC
wkPaymentProtocolRequestCoerceBTC (WKPaymentProtocolRequest protoReq) {
    assert (WK_NETWORK_TYPE_BTC == protoReq->chainType);
    return (WKPaymentProtocolRequestBTC) protoReq;
}

extern WKBoolean
wkPaymentProtocolRequestValidateSupportedBTC (WKPaymentProtocolType type) {
    return AS_WK_BOOLEAN (WK_PAYMENT_PROTOCOL_TYPE_BITPAY == type ||
                              WK_PAYMENT_PROTOCOL_TYPE_BIP70 == type);
}

extern WKPaymentProtocolRequest
wkPaymentProtocolRequestCreateForBitPayBTC (WKPaymentProtocolRequestBitPayBuilder builderBase) {
    WKPaymentProtocolRequestBitPayBuilderBTC builder = wkPaymentProtocolRequestBitPayBuilderCoerceBTC (builderBase);
    
    WKNetworkType type = wkNetworkGetType (builderBase->wkNetwork);
    
    WKPaymentProtocolRequest protoReqBase = NULL;

    if ((WK_NETWORK_TYPE_BTC == type) &&
        wkNetworkHasCurrency(builderBase->wkNetwork, builderBase->wkCurrency) &&
        0 != array_count (builder->outputs) && 0 != builder->outputs[0].amount && 0 != builder->outputs[0].scriptLen) {
        
        BRBitcoinPaymentProtocolDetails *details = btcPaymentProtocolDetailsNew (builderBase->network,
                                                                         builder->outputs,
                                                                         array_count (builder->outputs),
                                                                         builderBase->time,
                                                                         builderBase->expires,
                                                                         builderBase->memo,
                                                                         builderBase->paymentURL,
                                                                         builderBase->merchantData,
                                                                         builderBase->merchantDataLen);
        
        BRBitcoinPaymentProtocolRequest *request = btcPaymentProtocolRequestNew (1,
                                                                         "none",
                                                                         NULL,
                                                                         0,
                                                                         details,
                                                                         NULL,
                                                                         0);
        
        if (NULL != request) {
            WKNetworkFee requiredFee = NULL;
            if (0 != builderBase->feeCostFactor) {
                WKUnit feeUnit = wkNetworkGetUnitAsBase (builderBase->wkNetwork, builderBase->wkCurrency);
                WKAmount feeAmount = wkAmountCreateDouble (builderBase->feeCostFactor, feeUnit);
                requiredFee = wkNetworkFeeCreate (0, feeAmount, feeUnit);
                wkAmountGive (feeAmount);
                wkUnitGive (feeUnit);
            }
            
            protoReqBase = wkPaymentProtocolRequestAllocAndInit(sizeof (struct WKPaymentProtocolRequestBTCRecord),
                                                                    type,
                                                                    WK_PAYMENT_PROTOCOL_TYPE_BITPAY,
                                                                    builderBase->wkNetwork,
                                                                    builderBase->wkCurrency,
                                                                    requiredFee,
                                                                    builderBase->callbacks);
            wkNetworkFeeGive (requiredFee);
            
            WKPaymentProtocolRequestBTC protoReq = wkPaymentProtocolRequestCoerceBTC (protoReqBase);
            protoReq->request = request;
        } else {
            btcPaymentProtocolDetailsFree (details);
        }
    }
    
    return protoReqBase;
}

extern WKPaymentProtocolRequest
wkPaymentProtocolRequestCreateForBip70BTC (WKNetwork wkNetwork,
                                               WKCurrency wkCurrency,
                                               WKPayProtReqBip70Callbacks callbacks,
                                               uint8_t *serialization,
                                               size_t serializationLen) {
    WKNetworkType type = wkNetworkGetType (wkNetwork);
    
    WKPaymentProtocolRequest protoReqBase = NULL;
    
    if ((WK_NETWORK_TYPE_BTC == type) &&
        (wkNetworkHasCurrency(wkNetwork, wkCurrency))) {
        
        BRBitcoinPaymentProtocolRequest *request = btcPaymentProtocolRequestParse (serialization,
                                                                        serializationLen);
        if (NULL != request) {
            protoReqBase = wkPaymentProtocolRequestAllocAndInit(sizeof (struct WKPaymentProtocolRequestBTCRecord),
                                                                                               type,
                                                                                               WK_PAYMENT_PROTOCOL_TYPE_BIP70, wkNetwork,
                                                                                               wkCurrency,
                                                                                               NULL,
                                                                                               callbacks);
            
            WKPaymentProtocolRequestBTC protoReq = wkPaymentProtocolRequestCoerceBTC (protoReqBase);
            protoReq->request = request;
        }
    }
    
    return protoReqBase;
}

extern WKFeeBasis
wkPaymentProtocolRequestEstimateFeeBasisBTC (WKPaymentProtocolRequest protoReqBase,
                                                 WKWalletManager cwm,
                                                 WKWallet wallet,
                                                 WKCookie cookie,
                                                 WKNetworkFee networkFee) {
    BRBitcoinWallet *wid = wkWalletAsBTC (wallet);
    uint64_t btcFeePerKB = 1000 * wkNetworkFeeAsBTC (networkFee);
    uint64_t btcFee = 0;
    
    assert (WK_PAYMENT_PROTOCOL_TYPE_BITPAY == protoReqBase->paymentProtocolType ||
            WK_PAYMENT_PROTOCOL_TYPE_BIP70 == protoReqBase->paymentProtocolType);
    
    BRArrayOf(BRBitcoinTxOutput) outputs = wkPaymentProtocolRequestGetOutputsAsBTC (protoReqBase);
    if (NULL != outputs) {
        BRBitcoinTransaction *transaction = btcWalletCreateTxForOutputsWithFeePerKb (wid, btcFeePerKB, outputs, array_count (outputs));
        
        if (NULL != transaction) {
            btcFee = btcWalletFeeForTx(wid, transaction);
            btcTransactionFree(transaction);
        }
        array_free (outputs);
    }
    
    return wkFeeBasisCreateAsBTC (wallet->unitForFee, btcFee, btcFeePerKB, WK_FEE_BASIS_BTC_SIZE_UNKNOWN);
}

extern WKTransfer
wkPaymentProtocolRequestCreateTransferBTC (WKPaymentProtocolRequest protoReq,
                                               WKWallet wallet,
                                               WKFeeBasis estimatedFeeBasis) {
    WKTransfer transfer = NULL;
    
    WKUnit unit       = wkWalletGetUnit (wallet);
    WKUnit unitForFee = wkWalletGetUnitForFee (wallet);
    
    switch (wallet->type) {
        case WK_NETWORK_TYPE_BTC: {
            BRBitcoinWallet *wid = wkWalletAsBTC (wallet);
            
            switch (wkPaymentProtocolRequestGetType (protoReq)) {
                case WK_PAYMENT_PROTOCOL_TYPE_BITPAY:
                case WK_PAYMENT_PROTOCOL_TYPE_BIP70: {
                    BRArrayOf(BRBitcoinTxOutput) outputs = wkPaymentProtocolRequestGetOutputsAsBTC (protoReq);
                    if (NULL != outputs) {
                        uint64_t feePerKb = wkFeeBasisAsBTC (estimatedFeeBasis);
                        BRBitcoinTransaction *tid = btcWalletCreateTxForOutputsWithFeePerKb (wid, feePerKb, outputs, array_count (outputs));

                        transfer = NULL == tid ? NULL : wkTransferCreateAsBTC (wallet->listenerTransfer,
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
    
    wkUnitGive (unitForFee);
    wkUnitGive (unit);
    
    return transfer;
}

extern WKBoolean
wkPaymentProtocolRequestIsSecureBTC (WKPaymentProtocolRequest protoReqBase) {
    WKPaymentProtocolRequestBTC protoReq = wkPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    WKBoolean isSecure = WK_FALSE;
    switch (protoReqBase->paymentProtocolType) {
        case WK_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case WK_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRBitcoinPaymentProtocolRequest *request = protoReq->request;
            isSecure = AS_WK_BOOLEAN (NULL != request->pkiType && 0 != strcmp (request->pkiType, "none"));
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
wkPaymentProtocolRequestGetMemoBTC (WKPaymentProtocolRequest protoReqBase) {
    WKPaymentProtocolRequestBTC protoReq = wkPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    const char *memo = NULL;
    switch (protoReqBase->paymentProtocolType) {
        case WK_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case WK_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRBitcoinPaymentProtocolRequest *request = protoReq->request;
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
wkPaymentProtocolRequestGetPaymentURLBTC (WKPaymentProtocolRequest protoReqBase) {
    WKPaymentProtocolRequestBTC protoReq = wkPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    const char *paymentURL = NULL;
    switch (protoReqBase->paymentProtocolType) {
        case WK_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case WK_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRBitcoinPaymentProtocolRequest *request = protoReq->request;
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

extern WKAmount
wkPaymentProtocolRequestGetTotalAmountBTC (WKPaymentProtocolRequest protoReqBase) {
    WKPaymentProtocolRequestBTC protoReq = wkPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    WKAmount amount = NULL;
    switch (protoReqBase->paymentProtocolType) {
        case WK_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case WK_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRBitcoinPaymentProtocolRequest *request = protoReq->request;
            uint64_t satoshis = 0;
            for (size_t index = 0; index < request->details->outCount; index++) {
                BRBitcoinTxOutput *output = &request->details->outputs[index];
                satoshis += output->amount;
            }

            WKUnit baseUnit = wkNetworkGetUnitAsBase (protoReqBase->wkNetwork, protoReqBase->wkCurrency);
            amount = wkAmountCreate (baseUnit, WK_FALSE, uint256Create (satoshis));
            wkUnitGive (baseUnit);
            break;
        }
        default: {
            assert (0);
            break;
        }
    }
    return amount;
}

extern WKAddress
wkPaymentProtocolRequestGetPrimaryTargetAddressBTC (WKPaymentProtocolRequest protoReqBase) {
    WKPaymentProtocolRequestBTC protoReq = wkPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    WKAddress address = NULL;
    switch (protoReqBase->paymentProtocolType) {
        case WK_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case WK_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRBitcoinPaymentProtocolRequest *request = protoReq->request;
            if (0 != request->details->outCount) {
                const BRBitcoinChainParams * chainParams = wkNetworkAsBTC (protoReqBase->wkNetwork);

                BRBitcoinTxOutput *output = &request->details->outputs[0];
                size_t addressSize = btcTxOutputAddress (output, NULL, 0, chainParams->addrParams);
                char *addressString = malloc (addressSize);
                btcTxOutputAddress (output, addressString, addressSize, chainParams->addrParams);

                address = wkAddressCreateAsBTC (protoReqBase->chainType, BRAddressFill (chainParams->addrParams, addressString));
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
wkPaymentProtocolRequestGetCommonNameBTC (WKPaymentProtocolRequest protoReqBase) {
    WKPaymentProtocolRequestBTC protoReq = wkPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    char * name = NULL;
    switch (protoReqBase->paymentProtocolType) {
        case WK_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case WK_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRArrayOf(uint8_t *) certBytes;
            BRArrayOf(size_t) certLens;
            
            array_new (certBytes, 10);
            array_new (certLens, 10);
            
            size_t certLen, index = 0;
            while (( certLen = btcPaymentProtocolRequestCert (protoReq->request, NULL, 0, index)) > 0) {
                uint8_t *cert = malloc (certLen);
                btcPaymentProtocolRequestCert (protoReq->request, cert, certLen, index);
                
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

extern WKPaymentProtocolError
wkPaymentProtocolRequestIsValidBTC (WKPaymentProtocolRequest protoReqBase) {
    WKPaymentProtocolRequestBTC protoReq = wkPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    WKPaymentProtocolError error = WK_PAYMENT_PROTOCOL_ERROR_NONE;
    switch (protoReqBase->paymentProtocolType) {
        case WK_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case WK_PAYMENT_PROTOCOL_TYPE_BIP70: {
            uint8_t *digest = NULL;
            size_t digestLen = btcPaymentProtocolRequestDigest(protoReq->request, NULL, 0);
            if (digestLen) {
                digest = malloc (digestLen);
                btcPaymentProtocolRequestDigest(protoReq->request, digest, digestLen);
            }
            
            BRArrayOf(uint8_t *) certs;
            BRArrayOf(size_t) certLens;
            
            array_new (certs, 10);
            array_new (certLens, 10);
            
            size_t certLen, index = 0;
            while (( certLen = btcPaymentProtocolRequestCert (protoReq->request, NULL, 0, index)) > 0) {
                uint8_t *cert = malloc (certLen);
                btcPaymentProtocolRequestCert (protoReq->request, cert, certLen, index);
                
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
wkPaymentProtocolRequestGetMerchantDataBTC (WKPaymentProtocolRequest protoReqBase, size_t *merchantDataLen) {
    WKPaymentProtocolRequestBTC protoReq = wkPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    uint8_t *merchantData = NULL;
    switch (protoReqBase->paymentProtocolType) {
        case WK_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case WK_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRBitcoinPaymentProtocolRequest *request = protoReq->request;
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
wkPaymentProtocolRequestReleaseBTC (WKPaymentProtocolRequest protoReqBase) {
    WKPaymentProtocolRequestBTC protoReq = wkPaymentProtocolRequestCoerceBTC(protoReqBase);
    btcPaymentProtocolRequestFree (protoReq->request);
}

// MARK: - Payment Protocol Payment

static WKPaymentProtocolPaymentBTC
wkPaymentProtocolPaymentCoerceBTC (WKPaymentProtocolPayment payment) {
    assert (WK_NETWORK_TYPE_BTC == payment->chainType ||
            WK_NETWORK_TYPE_BCH == payment->chainType);
    return (WKPaymentProtocolPaymentBTC) payment;
}

extern WKPaymentProtocolPayment
wkPaymentProtocolPaymentCreateBTC (WKPaymentProtocolRequest protoReqBase,
                                       WKTransfer transfer,
                                       WKAddress refundAddress) {
    WKPaymentProtocolPayment protoPayBase = NULL;

    switch (wkPaymentProtocolRequestGetType (protoReqBase)) {
        case WK_PAYMENT_PROTOCOL_TYPE_BITPAY: {
            
            protoPayBase = wkPaymentProtocolPaymentAllocAndInit(sizeof (struct WKPaymentProtocolPaymentBTCRecord),
                                                                    protoReqBase);
            WKPaymentProtocolPaymentBTC protoPay = wkPaymentProtocolPaymentCoerceBTC (protoPayBase);
            protoPay->transaction = btcTransactionCopy (wkTransferAsBTC (transfer));
            protoPay->payment = NULL;
            break;
        }
        case WK_PAYMENT_PROTOCOL_TYPE_BIP70: {
            WKNetwork wkNetwork = wkNetworkTake (protoReqBase->wkNetwork);
            WKCurrency wkCurrency = wkCurrencyTake (protoReqBase->wkCurrency);
            
            WKAmount refundAmount = wkPaymentProtocolRequestGetTotalAmount (protoReqBase);
            WKUnit baseUnit = wkNetworkGetUnitAsBase (wkNetwork, wkCurrency);
            
            WKAmount baseAmount = wkAmountConvertToUnit (refundAmount, baseUnit);
            if (NULL != baseAmount) {
                WKBoolean overflow = WK_TRUE;
                uint64_t refundAmountInt = wkAmountGetIntegerRaw (baseAmount, &overflow);

                WKNetworkType refundType;
                BRAddress refundAddressBtc = wkAddressAsBTC (refundAddress, &refundType);
                
                const BRBitcoinChainParams * chainParams = wkNetworkAsBTC (wkNetwork);
                WKBoolean isBTC = AS_WK_BOOLEAN (btcChainParamsIsBitcoin (chainParams));
                
                if (WK_TRUE == isBTC && WK_NETWORK_TYPE_BTC == refundType && WK_FALSE == overflow) {
                    size_t merchantDataLen = 0;
                    const uint8_t *merchantData = wkPaymentProtocolRequestGetMerchantDataBTC (protoReqBase, &merchantDataLen);
                    
                    protoPayBase = wkPaymentProtocolPaymentAllocAndInit(sizeof (struct WKPaymentProtocolPaymentBTCRecord),
                                                                            protoReqBase);
                    WKPaymentProtocolPaymentBTC protoPay = wkPaymentProtocolPaymentCoerceBTC (protoPayBase);
                    protoPay->transaction = btcTransactionCopy (wkTransferAsBTC (transfer));
                    protoPay->payment     = btcPaymentProtocolPaymentNew (merchantData,
                                                                         merchantDataLen,
                                                                         &protoPay->transaction, 1,
                                                                         &refundAmountInt,
                                                                         chainParams->addrParams,
                                                                         &refundAddressBtc,
                                                                         1, NULL);
                }
                
                wkAmountGive (baseAmount);
            }
            
            wkUnitGive (baseUnit);
            wkAmountGive (refundAmount);

            wkCurrencyGive (wkCurrency);
            wkNetworkGive (wkNetwork);
            break;
        }
        default:
            assert (0);
            break;
    }
    
    return protoPayBase;
}

extern uint8_t *
wkPaymentProtocolPaymentEncodeBTC (WKPaymentProtocolPayment protoPayBase,
                                       size_t *encodedLen) {
    WKPaymentProtocolPaymentBTC protoPay = wkPaymentProtocolPaymentCoerceBTC(protoPayBase);
    
    uint8_t * encoded = NULL;

    assert (NULL != encodedLen);
    switch (protoPayBase->paymentProtocolType) {
        case WK_PAYMENT_PROTOCOL_TYPE_BITPAY: {
            size_t transactionBufLen = btcTransactionSerialize (protoPay->transaction, NULL, 0);
            if (0 != transactionBufLen) {
                BRArrayOf(uint8_t) encodedArray;
                array_new (encodedArray, 512);
                array_add (encodedArray, '{');

                #define PP_JSON_CURRENCY_PRE    "\"currency\":\""
                #define PP_JSON_CURRENCY_PRE_SZ (sizeof(PP_JSON_CURRENCY_PRE) - 1)
                array_add_array (encodedArray, (uint8_t*) PP_JSON_CURRENCY_PRE, PP_JSON_CURRENCY_PRE_SZ);

                const char *currencyCode = wkCurrencyGetCode (protoPayBase->wkCurrency);
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
                btcTransactionSerialize (protoPay->transaction, transactionBuf, transactionBufLen);
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
        case WK_PAYMENT_PROTOCOL_TYPE_BIP70: {
            *encodedLen = btcPaymentProtocolPaymentSerialize(protoPay->payment,
                                                            NULL,
                                                            0);
            if (0 != *encodedLen) {
                encoded = malloc(*encodedLen);
                btcPaymentProtocolPaymentSerialize(protoPay->payment,
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
wkPaymentProtocolPaymentReleaseBTC (WKPaymentProtocolPayment protoPayBase) {
    WKPaymentProtocolPaymentBTC protoPay = wkPaymentProtocolPaymentCoerceBTC(protoPayBase);
    
    switch (protoPayBase->paymentProtocolType) {
        case WK_PAYMENT_PROTOCOL_TYPE_BITPAY: {
            btcTransactionFree (protoPay->transaction);
            break;
        }
        case WK_PAYMENT_PROTOCOL_TYPE_BIP70: {
            btcTransactionFree (protoPay->transaction);
            btcPaymentProtocolPaymentFree (protoPay->payment);
            break;
        }
        default: {
            break;
        }
    }
}

// MARK: - Support

static BRArrayOf(BRBitcoinTxOutput)
wkPaymentProtocolRequestGetOutputsAsBTC (WKPaymentProtocolRequest protoReqBase) {
    WKPaymentProtocolRequestBTC protoReq = wkPaymentProtocolRequestCoerceBTC (protoReqBase);
    
    BRArrayOf(BRBitcoinTxOutput) outputs = NULL;
    switch (protoReqBase->paymentProtocolType) {
        case WK_PAYMENT_PROTOCOL_TYPE_BITPAY:
        case WK_PAYMENT_PROTOCOL_TYPE_BIP70: {
            BRBitcoinPaymentProtocolRequest *request = protoReq->request;
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

WKPaymentProtocolHandlers wkPaymentProtocolHandlersBTC = {
    wkPaymentProtocolRequestBitPayBuilderCreateBTC,
    wkPaymentProtocolRequestBitPayBuilderAddOutputBTC,
    wkPaymentProtocolRequestBitPayBuilderReleaseBTC,
    
    wkPaymentProtocolRequestValidateSupportedBTC,
    wkPaymentProtocolRequestCreateForBitPayBTC,
    wkPaymentProtocolRequestCreateForBip70BTC,
    wkPaymentProtocolRequestEstimateFeeBasisBTC,
    wkPaymentProtocolRequestCreateTransferBTC,

    wkPaymentProtocolRequestIsSecureBTC,
    wkPaymentProtocolRequestGetMemoBTC,
    wkPaymentProtocolRequestGetPaymentURLBTC,
    wkPaymentProtocolRequestGetTotalAmountBTC,
    wkPaymentProtocolRequestGetPrimaryTargetAddressBTC,
    wkPaymentProtocolRequestGetCommonNameBTC,
    wkPaymentProtocolRequestIsValidBTC,
    wkPaymentProtocolRequestReleaseBTC,
    
    wkPaymentProtocolPaymentCreateBTC,
    wkPaymentProtocolPaymentEncodeBTC,
    wkPaymentProtocolPaymentReleaseBTC
};
