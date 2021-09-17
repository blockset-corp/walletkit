/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 10/18/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex.library;

import com.blockset.walletkit.nativex.WKClient;
import com.blockset.walletkit.nativex.WKPayProtReqBitPayAndBip70Callbacks;
import com.blockset.walletkit.nativex.WKSyncStoppedReason;
import com.blockset.walletkit.nativex.WKWalletManager;
import com.blockset.walletkit.nativex.WKWalletManagerState;
import com.blockset.walletkit.nativex.WKWalletManagerDisconnectReason;
import com.blockset.walletkit.nativex.WKTransferSubmitError;
import com.blockset.walletkit.nativex.support.*;
import com.blockset.walletkit.nativex.utility.*;
import com.sun.jna.Callback;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.StringArray;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.LongByReference;
import com.sun.jna.ptr.PointerByReference;

import java.nio.ByteBuffer;

public final class WKNativeLibraryDirect {

    // The goal with this class is to remove any type values other than Java or JNA primitives. Each
    // type used outside of those parameters results in a performance hit when calling into the native
    // function.

    //
    // Crypto Core
    //
    public static native void wkMemoryFreeExtern(Pointer memory);

    // include/WKAccount.h
    public static native Pointer wkAccountCreate(ByteBuffer phrase, long /* BRCryptoTimestamp */ timestamp, String uids);
    public static native Pointer wkAccountCreateFromSerialization(byte[] serialization, SizeT serializationLength, String uids);
    public static native long wkAccountGetTimestamp(Pointer account);
    public static native Pointer wkAccountGetUids(Pointer account);
    public static native Pointer wkAccountGetFileSystemIdentifier(Pointer account);
    public static native Pointer wkAccountSerialize(Pointer account, SizeTByReference count);
    public static native int wkAccountValidateSerialization(Pointer account, byte[] serialization, SizeT count);
    public static native int wkAccountValidateWordsList(SizeT count);
    public static native Pointer wkAccountGeneratePaperKey(StringArray words);
    public static native int wkAccountValidatePaperKey(ByteBuffer phraseBuffer, StringArray wordsArray);
    public static native void wkAccountGive(Pointer obj);

    // include/WKAddress.h
    public static native Pointer wkAddressAsString(Pointer address);
    public static native int wkAddressIsIdentical(Pointer a1, Pointer a2);
    public static native void wkAddressGive(Pointer obj);

    // include/WKAmount.h
    public static native Pointer wkAmountCreateDouble(double value, Pointer unit);
    public static native Pointer wkAmountCreateInteger(long value, Pointer unit);
    public static native Pointer wkAmountCreateString(String value, int isNegative, Pointer unit);
    public static native Pointer wkAmountGetCurrency(Pointer amount);
    public static native Pointer wkAmountGetUnit(Pointer amount);
    public static native int wkAmountHasCurrency(Pointer amount, Pointer currency);
    public static native int wkAmountIsNegative(Pointer amount);
    public static native int wkAmountIsZero(Pointer amount);
    public static native int wkAmountIsCompatible(Pointer a1, Pointer a2);
    public static native int wkAmountCompare(Pointer a1, Pointer a2);
    public static native Pointer wkAmountAdd(Pointer a1, Pointer a2);
    public static native Pointer wkAmountSub(Pointer a1, Pointer a2);
    public static native Pointer wkAmountNegate(Pointer amount);
    public static native Pointer wkAmountConvertToUnit(Pointer amount, Pointer unit);
    public static native double wkAmountGetDouble(Pointer amount, Pointer unit, IntByReference overflow);
    public static native Pointer wkAmountGetStringPrefaced (Pointer amount, int base, String preface);
    public static native Pointer wkAmountTake(Pointer obj);
    public static native void wkAmountGive(Pointer obj);

    // include/WKCurrency.h
    public static native Pointer wkCurrencyGetUids(Pointer currency);
    public static native Pointer wkCurrencyGetName(Pointer currency);
    public static native Pointer wkCurrencyGetCode(Pointer currency);
    public static native Pointer wkCurrencyGetType(Pointer currency);
    public static native Pointer wkCurrencyGetIssuer(Pointer currency);
    public static native int wkCurrencyIsIdentical(Pointer c1, Pointer c2);
    public static native void wkCurrencyGive(Pointer obj);

    // include/WKFeeBasis.h
    public static native Pointer wkFeeBasisGetPricePerCostFactor (Pointer feeBasis);
    public static native double wkFeeBasisGetCostFactor (Pointer feeBasis);
    public static native Pointer wkFeeBasisGetFee (Pointer feeBasis);
    public static native int wkFeeBasisIsEqual(Pointer f1, Pointer f2);
    public static native Pointer wkFeeBasisTake(Pointer obj);
    public static native void wkFeeBasisGive(Pointer obj);

    // include/WKHash.h
    public static native int wkHashEqual(Pointer h1, Pointer h2);
    public static native Pointer wkHashEncodeString(Pointer hash);
    public static native int wkHashGetHashValue(Pointer hash);
    public static native void wkHashGive(Pointer obj);

    // include/WKKey.h
    public static native int wkKeyIsProtectedPrivate(ByteBuffer keyBuffer);
    public static native Pointer wkKeyCreateFromPhraseWithWords(ByteBuffer phraseBuffer, StringArray wordsArray);
    public static native Pointer wkKeyCreateFromStringPrivate(ByteBuffer stringBuffer);
    public static native Pointer wkKeyCreateFromStringProtectedPrivate(ByteBuffer stringBuffer, ByteBuffer phraseBuffer);
    public static native Pointer wkKeyCreateFromStringPublic(ByteBuffer stringBuffer);
    public static native Pointer wkKeyCreateForPigeon(Pointer key, byte[] nonce, SizeT nonceCount);
    public static native Pointer wkKeyCreateForBIP32ApiAuth(ByteBuffer phraseBuffer, StringArray wordsArray);
    public static native Pointer wkKeyCreateForBIP32BitID(ByteBuffer phraseBuffer, int index, String uri, StringArray wordsArray);
    public static native Pointer wkKeyCreateFromSecret(WKSecret.ByValue secret);
    public static native void wkKeyProvidePublicKey(Pointer key, int useCompressed, int compressed);
    public static native int wkKeyHasSecret(Pointer key);
    public static native int wkKeyPublicMatch(Pointer key, Pointer other);
    public static native int wkKeySecretMatch(Pointer key, Pointer other);
    public static native Pointer wkKeyEncodePrivate(Pointer key);
    public static native Pointer wkKeyEncodePublic(Pointer key);
    public static native WKSecret.ByValue wkKeyGetSecret(Pointer key);
    public static native void wkKeyGive(Pointer key);

    // include/WKNetwork.h (WKNetwork)
    public static native Pointer wkNetworkGetUids(Pointer network);
    public static native Pointer wkNetworkGetName(Pointer network);
    public static native int wkNetworkIsMainnet(Pointer network);
    public static native Pointer wkNetworkGetCurrency(Pointer network);
    public static native Pointer wkNetworkGetUnitAsDefault(Pointer network, Pointer currency);
    public static native Pointer wkNetworkGetUnitAsBase(Pointer network, Pointer currency);
    public static native long wkNetworkGetHeight(Pointer network);
    public static native Pointer wkNetworkGetVerifiedBlockHash (Pointer network);
    public static native void wkNetworkSetVerifiedBlockHash (Pointer network, Pointer verifiedBlockHash);
    public static native void wkNetworkSetVerifiedBlockHashAsString (Pointer network, String verifiedBlockHashString);
    public static native int wkNetworkGetConfirmationsUntilFinal(Pointer network);
    public static native void wkNetworkSetConfirmationsUntilFinal(Pointer network, int confirmationsUntilFinal);
    public static native SizeT wkNetworkGetCurrencyCount(Pointer network);
    public static native Pointer wkNetworkGetCurrencyAt(Pointer network, SizeT index);
    public static native int wkNetworkHasCurrency(Pointer network, Pointer currency);
    public static native SizeT wkNetworkGetUnitCount(Pointer network, Pointer currency);
    public static native Pointer wkNetworkGetUnitAt(Pointer network, Pointer currency, SizeT index);
    // public static native void wkNetworkSetNetworkFees(Pointer network, BRCryptoNetworkFee[] fees, SizeT count);
    public static native Pointer wkNetworkGetNetworkFees(Pointer network, SizeTByReference count);
    public static native Pointer wkNetworkTake(Pointer obj);
    public static native void wkNetworkGive(Pointer obj);
    public static native int wkNetworkGetType(Pointer obj);
    public static native int wkNetworkGetDefaultAddressScheme(Pointer network);
    public static native Pointer wkNetworkGetSupportedAddressSchemes(Pointer network, SizeTByReference count);
    public static native Pointer wkNetworkCreateAddress(Pointer pointer, String address);
    public static native int wkNetworkSupportsAddressScheme(Pointer network, int scheme);
    public static native int wkNetworkGetDefaultSyncMode(Pointer network);
    public static native Pointer wkNetworkGetSupportedSyncModes(Pointer network, SizeTByReference count);
    public static native int wkNetworkSupportsSyncMode(Pointer network, int mode);
    public static native int wkNetworkRequiresMigration(Pointer network);
    public static native Pointer wkNetworkFindBuiltin(String uids, int isMainnet);
    public static native int wkNetworkIsAccountInitialized (Pointer network, Pointer account);
    public static native Pointer wkNetworkGetAccountInitializationData (Pointer network, Pointer account, SizeTByReference bytesCount);
    public static native void wkNetworkInitializeAccount (Pointer network, Pointer account, byte[] bytes, SizeT bytesCount);
    public static native void wkNetworkSetHeight(Pointer network, long height);
    public static native void wkNetworkAddCurrency(Pointer network, Pointer currency, Pointer baseUnit, Pointer defaultUnit);
    public static native void wkNetworkAddCurrencyUnit(Pointer network, Pointer currency, Pointer unit);
    public static native void wkNetworkAddNetworkFee(Pointer network, Pointer networkFee);

    // src/walletkit/WKNetworkP.h
    public static native Pointer wkNetworkInstallBuiltins(SizeTByReference count);

    // include/WKNetwork.h (WKNetworkFee)
    public static native long wkNetworkFeeGetConfirmationTimeInMilliseconds(Pointer fee);
    public static native Pointer wkNetworkFeeGetPricePerCostFactor(Pointer fee);
    public static native int wkNetworkFeeEqual(Pointer fee, Pointer other);
    public static native void wkNetworkFeeGive(Pointer obj);
    public static native Pointer wkNetworkFeeCreate(long timeInternalInMilliseconds, Pointer pricePerCostFactor, Pointer pricePerCostFactorUnit);

    // include/WKPeer.h (WKPeer)
    public static native Pointer wkPeerCreate(Pointer network, String address, short port, String publicKey);
    public static native Pointer wkPeerGetNetwork(Pointer peer);
    public static native Pointer wkPeerGetAddress(Pointer peer);
    public static native Pointer wkPeerGetPublicKey(Pointer peer);
    public static native short wkPeerGetPort(Pointer peer);
    public static native int wkPeerIsIdentical(Pointer peer, Pointer other);
    public static native void wkPeerGive(Pointer peer);

    // include/WKPayment.h (WKPaymentProtocolRequestBitPayBuilder)
    public static native Pointer wkPaymentProtocolRequestBitPayBuilderCreate(Pointer network,
                                                                             Pointer currency,
                                                                             WKPayProtReqBitPayAndBip70Callbacks.ByValue callbacks,
                                                                             String name,
                                                                             long time,
                                                                             long expires,
                                                                             double feePerByte,
                                                                             String memo,
                                                                             String paymentUrl,
                                                                             byte[] merchantData,
                                                                             SizeT merchantDataLen);
    public static native void wkPaymentProtocolRequestBitPayBuilderAddOutput(Pointer builder, String address, long amount);
    public static native Pointer wkPaymentProtocolRequestBitPayBuilderBuild(Pointer builder);
    public static native void wkPaymentProtocolRequestBitPayBuilderGive(Pointer builder);

    // include/WKPayment.h (WKPaymentProtocolRequest)
    public static native int wkPaymentProtocolRequestValidateSupported(int type,
                                                                       Pointer network,
                                                                       Pointer currency,
                                                                       Pointer wallet);
    public static native Pointer wkPaymentProtocolRequestCreateForBip70(Pointer network,
                                                                        Pointer currency,
                                                                        WKPayProtReqBitPayAndBip70Callbacks.ByValue callbacks,
                                                                        byte[] serialization,
                                                                        SizeT serializationLen);
    public static native int wkPaymentProtocolRequestGetType(Pointer request);
    public static native int wkPaymentProtocolRequestIsSecure(Pointer request);
    public static native Pointer wkPaymentProtocolRequestGetMemo(Pointer request);
    public static native Pointer wkPaymentProtocolRequestGetPaymentURL(Pointer request);
    public static native Pointer wkPaymentProtocolRequestGetTotalAmount(Pointer request);
    public static native Pointer wkPaymentProtocolRequestGetRequiredNetworkFee (Pointer request);
    public static native Pointer wkPaymentProtocolRequestGetPrimaryTargetAddress(Pointer request);
    public static native Pointer wkPaymentProtocolRequestGetCommonName(Pointer request);
    public static native int wkPaymentProtocolRequestIsValid(Pointer request);
    public static native void wkPaymentProtocolRequestGive(Pointer request);

    // include/WKPayment.h (WKPaymentProtocolPayment)
    public static native Pointer wkPaymentProtocolPaymentCreate(Pointer request, Pointer transfer, Pointer refundAddress);
    public static native Pointer wkPaymentProtocolPaymentEncode(Pointer payment, SizeTByReference encodedLength);
    public static native void wkPaymentProtocolPaymentGive(Pointer payment);

    // include/WKPayment.h (WKPaymentProtocolPaymentACK)
    public static native Pointer wkPaymentProtocolPaymentACKCreateForBip70(byte[] serialization, SizeT serializationLen);
    public static native Pointer wkPaymentProtocolPaymentACKGetMemo(Pointer ack);
    public static native void wkPaymentProtocolPaymentACKGive(Pointer ack);

    // include/WKCurrency.h (WKCurrency)
    public static native Pointer wkCurrencyCreate(String uids, String name, String code, String type, String issuer);

    // include/WKTransfer.h (WKTransfer)
    public static native Pointer wkTransferGetSourceAddress(Pointer transfer);
    public static native Pointer wkTransferGetTargetAddress(Pointer transfer);
    public static native Pointer wkTransferGetAmount(Pointer transfer);
    public static native Pointer wkTransferGetAmountDirected(Pointer transfer);
    public static native int wkTransferGetDirection(Pointer transfer);
    public static native Pointer wkTransferGetState(Pointer transfer);
    public static native Pointer wkTransferGetIdentifier(Pointer transfer);
    public static native Pointer wkTransferGetHash(Pointer transfer);
    public static native Pointer wkTransferGetUnitForAmount (Pointer transfer);
    public static native Pointer wkTransferGetUnitForFee (Pointer transfer);
    public static native Pointer wkTransferGetEstimatedFeeBasis (Pointer transfer);
    public static native Pointer wkTransferGetConfirmedFeeBasis (Pointer transfer);
    public static native SizeT wkTransferGetAttributeCount(Pointer transfer);
    public static native Pointer wkTransferGetAttributeAt(Pointer transfer, SizeT index);
    public static native int wkTransferEqual(Pointer transfer, Pointer other);
    public static native Pointer wkTransferTake(Pointer obj);
    public static native void wkTransferGive(Pointer obj);

    // include/WKTransfer.h (WKTransferState)
    public static native int wkTransferStateGetType(Pointer state);
    public static native int wkTransferStateExtractIncluded(Pointer state, LongByReference blockNumber, LongByReference blockTimestamp, LongByReference transactionIndex, PointerByReference feeBasis, IntByReference success, PointerByReference error);
    public static native int wkTransferStateExtractError(Pointer state, WKTransferSubmitError.ByValue error);
    public static native Pointer wkTransferStateTake(Pointer state);
    public static native void wkTransferStateGive(Pointer state);

    // include/WKTransfer.h (WKTransferSubmitError)
    public static native Pointer wkTransferSubmitErrorGetMessage(WKTransferSubmitError error);

    // include/WKTransfer.h (WKTransferAttribute)
    public static native Pointer wkTransferAttributeCopy(Pointer attribute);
    public static native Pointer wkTransferAttributeGetKey(Pointer attribute);
    public static native Pointer wkTransferAttributeGetValue(Pointer attribute);
    public static native void wkTransferAttributeSetValue(Pointer attribute, String value);
    public static native int wkTransferAttributeIsRequired(Pointer attribute);
    public static native void wkTransferAttributeGive(Pointer attribute);

    // include/WKUnit.h (WKUnit)
    public static native Pointer wkUnitCreateAsBase(Pointer currency, String uids, String name, String symbol);
    public static native Pointer wkUnitCreate(Pointer currency, String uids, String name, String symbol, Pointer base, byte decimals);
    public static native Pointer wkUnitGetUids(Pointer unit);
    public static native Pointer wkUnitGetName(Pointer unit);
    public static native Pointer wkUnitGetSymbol(Pointer unit);
    public static native Pointer wkUnitGetCurrency(Pointer unit);
    public static native int wkUnitHasCurrency(Pointer unit, Pointer currency);
    public static native Pointer wkUnitGetBaseUnit(Pointer unit);
    public static native byte wkUnitGetBaseDecimalOffset(Pointer unit);
    public static native int wkUnitIsCompatible(Pointer u1, Pointer u2);
    public static native int wkUnitIsIdentical(Pointer u1, Pointer u2);
    public static native void wkUnitGive(Pointer obj);

    // include/event/WKWallet.h (WKWalletEvent)
    public static native int wkWalletEventGetType(Pointer event);
    public static native int wkWalletEventExtractState(Pointer event, IntByReference oldState, IntByReference newState);
    public static native int wkWalletEventExtractTransfer(Pointer event, PointerByReference transfer);
    public static native int wkWalletEventExtractTransferSubmit(Pointer event, PointerByReference transfer);
    public static native int wkWalletEventExtractBalanceUpdate(Pointer event, PointerByReference balance);
    public static native int wkWalletEventExtractFeeBasisUpdate(Pointer event, PointerByReference feeBasis);
    public static native int wkWalletEventExtractFeeBasisEstimate(Pointer event, IntByReference status, PointerByReference cookie, PointerByReference feeBasis);
    public static native Pointer wkWalletEventTake(Pointer event);
    public static native void wkWalletEventGive(Pointer event);

    // include/WKWallet.h (WKWallet)
    public static native int wkWalletGetState(Pointer wallet);
    public static native Pointer wkWalletGetBalance(Pointer wallet);
    public static native Pointer wkWalletGetBalanceMaximum(Pointer wallet);
    public static native Pointer wkWalletGetBalanceMinimum(Pointer wallet);
    public static native Pointer wkWalletGetTransfers(Pointer wallet, SizeTByReference count);
    public static native int wkWalletHasTransfer(Pointer wallet, Pointer transfer);
    public static native Pointer wkWalletGetAddress(Pointer wallet, int addressScheme);
    public static native int wkWalletHasAddress(Pointer wallet, Pointer address);
    public static native Pointer wkWalletGetUnit(Pointer wallet);
    public static native Pointer wkWalletGetUnitForFee(Pointer wallet);
    public static native Pointer wkWalletGetCurrency(Pointer wallet);
    // INDIRECT: public static native Pointer wkWalletCreateTransfer(Pointer wallet, Pointer target, Pointer amount, Pointer feeBasis, SizeT attributesCount, Pointer arrayOfAttributes);
    public static native Pointer wkWalletCreateTransferForPaymentProtocolRequest(Pointer wallet, Pointer request, Pointer feeBasis);
    public static native SizeT wkWalletGetTransferAttributeCount(Pointer wallet, Pointer target);
    public static native Pointer wkWalletGetTransferAttributeAt(Pointer wallet, Pointer target, SizeT index);
    public static native int wkWalletValidateTransferAttribute(Pointer wallet, Pointer attribute, IntByReference validates);
    // INDIRECT: public static native int wkWalletValidateTransferAttributes(Pointer wallet, SizeT countOfAttributes, Pointer arrayOfAttributes, IntByReference validates);
    public static native Pointer wkWalletTake(Pointer wallet);
    public static native void wkWalletGive(Pointer obj);

    // include/WKExportablePaperWallet.h (WKNetwork)
    public static native int wkExportablePaperWalletValidateSupported(Pointer network, Pointer currency);
    public static native Pointer wkExportablePaperWalletCreate(Pointer network, Pointer currency);

    // include/WKExportablePaperWallet.h (WKExportablePaperWallet)
    public static native void wkExportablePaperWalletRelease(Pointer paperWallet);
    public static native Pointer wkExportablePaperWalletGetKey(Pointer paperWallet);
    public static native Pointer wkExportablePaperWalletGetAddress (Pointer paperWallet);

    // include/WKWalletConnector.h (WKWalletConnector)
    public static native Pointer wkWalletConnectorCreate(Pointer manager);
    public static native void wkWalletConnectorRelease(Pointer connector);
    public static native Pointer wkWalletConnectorCreateStandardMessage(Pointer connector, byte[] msg, SizeT messageLength, SizeTByReference standardMessageLength, IntByReference err);
    public static native Pointer wkWalletConnectorGetDigest(Pointer connector, byte[] msg, SizeT msgLen, SizeTByReference digestLength, IntByReference err);
    public static native Pointer wkWalletConnectorSignData(Pointer connector, byte[] data, SizeT dataLength, Pointer key, SizeTByReference signatureLength, IntByReference err);
    public static native Pointer wkWalletConnectorRecoverKey(Pointer connector, byte[] digest, SizeT digestLength, byte[] signature, SizeT signatureLength, IntByReference err);
    public static native Pointer wkWalletConnectorCreateTransactionFromArguments(Pointer connector, StringArray keysArray, StringArray valuesArray, SizeT arrayCount, SizeTByReference serializationLength, IntByReference err);
    public static native Pointer wkWalletConnectorCreateTransactionFromSerialization(Pointer connector, byte[] data, SizeT dataLength, SizeTByReference serializationLength, IntByReference isSigned, IntByReference err);
    public static native Pointer wkWalletConnectorSignTransactionData(Pointer connector, byte[] transactionData, SizeT dataLength, Pointer key, SizeTByReference signedDataLength, IntByReference err);

    // include/WKWalletManager.h (WKWalletManager)
    public static native Pointer wkWalletManagerWipe(Pointer network, String path);
    public static native Pointer wkWalletManagerCreate(WKWalletManager.Listener.ByValue listener,
                                                       WKClient.ByValue client,
                                                       Pointer account,
                                                       Pointer network,
                                                       int mode,
                                                       int addressScheme,
                                                       String path);
    public static native Pointer wkWalletManagerGetNetwork(Pointer cwm);
    public static native Pointer wkWalletManagerGetAccount(Pointer cwm);
    public static native int wkWalletManagerGetMode(Pointer cwm);
    public static native void wkWalletManagerSetMode(Pointer cwm, int mode);
    public static native WKWalletManagerState.ByValue wkWalletManagerGetState(Pointer cwm);
    public static native int wkWalletManagerGetAddressScheme (Pointer cwm);
    public static native void wkWalletManagerSetAddressScheme (Pointer cwm, int scheme);
    public static native Pointer wkWalletManagerGetPath(Pointer cwm);
    public static native void wkWalletManagerSetNetworkReachable(Pointer cwm, int isNetworkReachable);
    public static native Pointer wkWalletManagerGetWallet(Pointer cwm);
    public static native Pointer wkWalletManagerGetWallets(Pointer cwm, SizeTByReference count);
    public static native int wkWalletManagerHasWallet(Pointer cwm, Pointer wallet);
    public static native Pointer wkWalletManagerCreateWallet(Pointer cwm, Pointer currency);
    public static native void wkWalletManagerConnect(Pointer cwm, Pointer peer);
    public static native void wkWalletManagerDisconnect(Pointer cwm);
    public static native void wkWalletManagerSync(Pointer cwm);
    public static native void wkWalletManagerSyncToDepth(Pointer cwm, int depth);
    public static native void wkWalletManagerStop(Pointer cwm);
    public static native int wkWalletManagerSign(Pointer cwm, Pointer wid, Pointer tid, ByteBuffer paperKey);
    public static native void wkWalletManagerSubmit(Pointer cwm, Pointer wid, Pointer tid, ByteBuffer paperKey);
    public static native void wkWalletManagerSubmitForKey(Pointer cwm, Pointer wid, Pointer tid, Pointer key);
    public static native void wkWalletManagerSubmitSigned(Pointer cwm, Pointer wid, Pointer tid);
    public static native Pointer wkWalletManagerEstimateLimit(Pointer cwm, Pointer wid, int asMaximum, Pointer target, Pointer fee, IntByReference needEstimate, IntByReference isZeroIfInsuffientFunds);
    // INDIRECT: public static native void wkWalletManagerEstimateFeeBasis(Pointer cwm, Pointer wid, Pointer cookie, Pointer target, Pointer amount, Pointer fee);
    public static native void wkWalletManagerEstimateFeeBasisForWalletSweep(Pointer sweeper, Pointer cwm, Pointer wid, Pointer cookie, Pointer fee);
    public static native void wkWalletManagerEstimateFeeBasisForPaymentProtocolRequest(Pointer cwm, Pointer wid, Pointer cookie, Pointer request, Pointer fee);
    public static native Pointer wkWalletManagerTake(Pointer cwm);
    public static native void wkWalletManagerGive(Pointer cwm);

    // include/WKWalletManager.h (WKWalletManagerDisconnectReason)
    public static native Pointer wkWalletManagerDisconnectReasonGetMessage(WKWalletManagerDisconnectReason reason);

    // include/WKWalletManager.h (WKWalletSweeper)
    public static native int wkWalletManagerWalletSweeperValidateSupported(Pointer cwm, Pointer wallet, Pointer key);
    public static native Pointer wkWalletManagerCreateWalletSweeper(Pointer cwm, Pointer wallet, Pointer key);
    public static native Pointer wkWalletSweeperGetKey(Pointer sweeper);
    public static native Pointer wkWalletSweeperGetBalance(Pointer sweeper);
    public static native Pointer wkWalletSweeperGetAddress(Pointer sweeper);
    public static native int wkWalletSweeperAddTransactionFromBundle(Pointer sweeper, Pointer bundle);
    public static native int wkWalletSweeperValidate(Pointer sweeper);
    public static native void wkWalletSweeperRelease(Pointer sweeper);
    public static native Pointer wkWalletSweeperCreateTransferForWalletSweep(Pointer sweeper, Pointer walletManager, Pointer wallet, Pointer feeBasis);

    // include/WKSync.h (WKSyncStoppedReason)
    public static native Pointer wkSyncStoppedReasonGetMessage(WKSyncStoppedReason reason);


    // include/WKClient.h
    public static native Pointer wkClientTransactionBundleCreate (int status,
                                                                  byte[] transaction,
                                                                  SizeT transactionLength,
                                                                  long timestamp,
                                                                  long blockHeight);
    public static native void wkClientTransactionBundleRelease (Pointer bundle);

    // See 'Indirect': void wkClientTransferBundleCreate (int status, ...)

    public static native Pointer wkClientCurrencyDenominationBundleCreate (String name, String code, String symbol, int decimals);

    // See 'Indirect':
    public static native void wkClientCurrencyBundleRelease (Pointer currencyBundle);

    public static native void wkClientAnnounceBlockNumber(Pointer cwm, Pointer callbackState, int success, long blockNumber, String verifiedBlockHash);
    public static native void wkClientAnnounceSubmitTransfer(Pointer cwm, Pointer callbackState, String identifier, String hash, int success);

    //
    // Crypto Primitives
    //

    // include/WKCipher.h (WKCipher)
    public static native Pointer wkCipherCreateForAESECB(byte[] key, SizeT keyLen);
    public static native Pointer wkCipherCreateForChacha20Poly1305(Pointer key, byte[] nonce12, SizeT nonce12Len, byte[] ad, SizeT adLen);
    public static native Pointer wkCipherCreateForPigeon(Pointer privKey, Pointer pubKey, byte[] nonce12, SizeT nonce12Len);
    public static native SizeT wkCipherEncryptLength(Pointer cipher, byte[] src, SizeT srcLen);
    public static native int wkCipherEncrypt(Pointer cipher, byte[] dst, SizeT dstLen, byte[] src, SizeT srcLen);
    public static native SizeT wkCipherDecryptLength(Pointer cipher, byte[] src, SizeT srcLen);
    public static native int wkCipherDecrypt(Pointer cipher, byte[] dst, SizeT dstLen, byte[] src, SizeT srcLen);
    public static native int wkCipherMigrateBRCoreKeyCiphertext(Pointer cipher, byte[] dst, SizeT dstLen, byte[] src, SizeT srcLen);
    public static native void wkCipherGive(Pointer cipher);

    // include/WKCoder.h (WKCoder)
    public static native Pointer wkCoderCreate(int type);
    public static native SizeT wkCoderEncodeLength(Pointer coder, byte[] src, SizeT srcLen);
    public static native int wkCoderEncode(Pointer coder, byte[] dst, SizeT dstLen, byte[] src, SizeT srcLen);
    public static native SizeT wkCoderDecodeLength(Pointer coder, byte[] src);
    public static native int wkCoderDecode(Pointer coder, byte[] dst, SizeT dstLen, byte[] src);
    public static native void wkCoderGive(Pointer coder);

    // include/WKHasher.h (WKHasher)
    public static native Pointer wkHasherCreate(int type);
    public static native SizeT wkHasherLength(Pointer hasher);
    public static native int wkHasherHash(Pointer hasher, byte[] dst, SizeT dstLen, byte[] src, SizeT srcLen);
    public static native void wkHasherGive(Pointer hasher);

    // include/WKSigner.h (WKSigner)
    public static native Pointer wkSignerCreate(int type);
    public static native SizeT wkSignerSignLength(Pointer signer, Pointer key, byte[] digest, SizeT digestlen);
    public static native int wkSignerSign(Pointer signer, Pointer key, byte[] signature, SizeT signatureLen, byte[] digest, SizeT digestLen);
    public static native Pointer wkSignerRecover(Pointer signer, byte[] digest, SizeT digestLen, byte[] signature, SizeT signatureLen);
    public static native void wkSignerGive(Pointer signer);

    // include/WKListener.h (WKListener)
    public static native Pointer wkListenerCreate (Pointer context, Callback systemCB, Callback networkCB, Callback managerCB, Callback walletCB, Callback transferCB);
    public static native Pointer wkListenerTake(Pointer listener);
    public static native void wkListenerGive(Pointer listener);

    // include/WKSystem.h
    public static native Pointer wkSystemCreate(WKClient.ByValue client,
                                                Pointer listener,
                                                Pointer account,
                                                String path,
                                                int onMainnet);

    public static native int wkSystemGetState (Pointer system);
    public static native int wkSystemOnMainnet (Pointer system);
    public static native int wkSystemIsReachable (Pointer system);
    public static native void wkSystemSetReachable (Pointer system, boolean reachable);
    public static native Pointer wkSystemGetResolvedPath (Pointer system);

    public static native int wkSystemHasNetwork (Pointer system, Pointer network);
    public static native Pointer wkSystemGetNetworks(Pointer system, SizeTByReference count);
    public static native Pointer wkSystemGetNetworkAt (Pointer system, SizeT index);
    public static native Pointer wkSystemGetNetworkForUids (Pointer system, String uids);
    public static native SizeT   wkSystemGetNetworksCount (Pointer system);

    public static native int wkSystemHasWalletManager (Pointer system, Pointer manager);
    public static native Pointer wkSystemGetWalletManagers(Pointer system, SizeTByReference count);
    public static native Pointer wkSystemGetWalletManagerAt (Pointer system, SizeT index);
    public static native Pointer wkSystemGetWalletManagerByNetwork (Pointer system, Pointer network);
    public static native SizeT wkSystemGetWalletManagersCount (Pointer system);
    // See 'Indirect': Pointer wkSystemCreateWalletManager (Pointer system, ...);

    public static native void wkSystemStart (Pointer system);
    public static native void wkSystemStop (Pointer system);
    public static native void wkSystemConnect (Pointer system);
    public static native void wkSystemDisconnect (Pointer system);
    public static native Pointer wkSystemTake(Pointer obj);
    public static native void wkSystemGive(Pointer obj);

    static {
        Native.register(WKNativeLibraryDirect.class, WKNativeLibrary.LIBRARY);
    }

    private WKNativeLibraryDirect() {}
}
