/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 10/18/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corenative;

import com.breadwallet.corenative.crypto.BRCryptoClient;
import com.breadwallet.corenative.crypto.BRCryptoListener;
import com.breadwallet.corenative.crypto.BRCryptoPayProtReqBitPayAndBip70Callbacks;
import com.breadwallet.corenative.crypto.BRCryptoTransferState;
import com.breadwallet.corenative.crypto.BRCryptoWalletManagerState;
import com.breadwallet.corenative.crypto.BRCryptoWalletMigratorStatus;
import com.breadwallet.corenative.crypto.BRCryptoWalletManagerDisconnectReason;
import com.breadwallet.corenative.crypto.BRCryptoSyncStoppedReason;
import com.breadwallet.corenative.crypto.BRCryptoTransferSubmitError;
import com.breadwallet.corenative.support.BRCryptoSecret;
import com.breadwallet.corenative.utility.SizeT;
import com.breadwallet.corenative.utility.SizeTByReference;
import com.sun.jna.Callback;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.StringArray;
import com.sun.jna.ptr.IntByReference;

import java.nio.ByteBuffer;

public final class CryptoLibraryDirect {

    // The goal with this class is to remove any type values other than Java or JNA primitives. Each
    // type used outside of those parameters results in a performance hit when calling into the native
    // function.

    //
    // Crypto Core
    //

    // crypto/BRCryptoAccount.h
    public static native Pointer cryptoAccountCreate(ByteBuffer phrase, long timestamp, String uids);
    public static native Pointer cryptoAccountCreateFromSerialization(byte[] serialization, SizeT serializationLength, String uids);
    public static native long cryptoAccountGetTimestamp(Pointer account);
    public static native Pointer cryptoAccountGetUids(Pointer account);
    public static native Pointer cryptoAccountGetFileSystemIdentifier(Pointer account);
    public static native Pointer cryptoAccountSerialize(Pointer account, SizeTByReference count);
    public static native int cryptoAccountValidateSerialization(Pointer account, byte[] serialization, SizeT count);
    public static native int cryptoNetworkIsAccountInitialized (Pointer network, Pointer account);
    public static native Pointer cryptoNetworkGetAccountInitializationData (Pointer network, Pointer account, SizeTByReference bytesCount);
    public static native void cryptoNetworkInitializeAccount (Pointer network, Pointer account, byte[] bytes, SizeT bytesCount);
    public static native int cryptoAccountValidateWordsList(SizeT count);
    public static native Pointer cryptoAccountGeneratePaperKey(StringArray words);
    public static native int cryptoAccountValidatePaperKey(ByteBuffer phraseBuffer, StringArray wordsArray);
    public static native void cryptoAccountGive(Pointer obj);

    // crypto/BRCryptoAddress.h
    public static native Pointer cryptoNetworkCreateAddress(Pointer pointer, String address);
    public static native Pointer cryptoAddressAsString(Pointer address);
    public static native int cryptoAddressIsIdentical(Pointer a1, Pointer a2);
    public static native void cryptoAddressGive(Pointer obj);

    // crypto/BRCryptoAmount.h
    public static native Pointer cryptoAmountCreateDouble(double value, Pointer unit);
    public static native Pointer cryptoAmountCreateInteger(long value, Pointer unit);
    public static native Pointer cryptoAmountCreateString(String value, int isNegative, Pointer unit);
    public static native Pointer cryptoAmountGetCurrency(Pointer amount);
    public static native Pointer cryptoAmountGetUnit(Pointer amount);
    public static native int cryptoAmountHasCurrency(Pointer amount, Pointer currency);
    public static native int cryptoAmountIsNegative(Pointer amount);
    public static native int cryptoAmountIsZero(Pointer amount);
    public static native int cryptoAmountIsCompatible(Pointer a1, Pointer a2);
    public static native int cryptoAmountCompare(Pointer a1, Pointer a2);
    public static native Pointer cryptoAmountAdd(Pointer a1, Pointer a2);
    public static native Pointer cryptoAmountSub(Pointer a1, Pointer a2);
    public static native Pointer cryptoAmountNegate(Pointer amount);
    public static native Pointer cryptoAmountConvertToUnit(Pointer amount, Pointer unit);
    public static native double cryptoAmountGetDouble(Pointer amount, Pointer unit, IntByReference overflow);
    public static native Pointer cryptoAmountGetStringPrefaced (Pointer amount, int base, String preface);
    public static native void cryptoAmountGive(Pointer obj);

    // crypto/BRCryptoCurrency.h
    public static native Pointer cryptoCurrencyGetUids(Pointer currency);
    public static native Pointer cryptoCurrencyGetName(Pointer currency);
    public static native Pointer cryptoCurrencyGetCode(Pointer currency);
    public static native Pointer cryptoCurrencyGetType(Pointer currency);
    public static native Pointer cryptoCurrencyGetIssuer(Pointer currency);
    public static native int cryptoCurrencyIsIdentical(Pointer c1, Pointer c2);
    public static native void cryptoCurrencyGive(Pointer obj);

    // crypto/BRCryptoFeeBasis.h
    public static native Pointer cryptoFeeBasisGetPricePerCostFactor (Pointer feeBasis);
    public static native double cryptoFeeBasisGetCostFactor (Pointer feeBasis);
    public static native Pointer cryptoFeeBasisGetFee (Pointer feeBasis);
    public static native int cryptoFeeBasisIsEqual(Pointer f1, Pointer f2);
    public static native void cryptoFeeBasisGive(Pointer obj);

    // crypto/BRCryptoHash.h
    public static native int cryptoHashEqual(Pointer h1, Pointer h2);
    public static native Pointer cryptoHashEncodeString(Pointer hash);
    public static native int cryptoHashGetHashValue(Pointer hash);
    public static native void cryptoHashGive(Pointer obj);

    // crypto/BRCryptoKey.h
    public static native int cryptoKeyIsProtectedPrivate(ByteBuffer keyBuffer);
    public static native Pointer cryptoKeyCreateFromPhraseWithWords(ByteBuffer phraseBuffer, StringArray wordsArray);
    public static native Pointer cryptoKeyCreateFromStringPrivate(ByteBuffer stringBuffer);
    public static native Pointer cryptoKeyCreateFromStringProtectedPrivate(ByteBuffer stringBuffer, ByteBuffer phraseBuffer);
    public static native Pointer cryptoKeyCreateFromStringPublic(ByteBuffer stringBuffer);
    public static native Pointer cryptoKeyCreateForPigeon(Pointer key, byte[] nonce, SizeT nonceCount);
    public static native Pointer cryptoKeyCreateForBIP32ApiAuth(ByteBuffer phraseBuffer, StringArray wordsArray);
    public static native Pointer cryptoKeyCreateForBIP32BitID(ByteBuffer phraseBuffer, int index, String uri, StringArray wordsArray);
    public static native Pointer cryptoKeyCreateFromSecret(BRCryptoSecret.ByValue secret);
    public static native void cryptoKeyProvidePublicKey(Pointer key, int useCompressed, int compressed);
    public static native int cryptoKeyHasSecret(Pointer key);
    public static native int cryptoKeyPublicMatch(Pointer key, Pointer other);
    public static native int cryptoKeySecretMatch(Pointer key, Pointer other);
    public static native Pointer cryptoKeyEncodePrivate(Pointer key);
    public static native Pointer cryptoKeyEncodePublic(Pointer key);
    public static native BRCryptoSecret.ByValue cryptoKeyGetSecret(Pointer key);
    public static native void cryptoKeyGive(Pointer key);

    // crypto/BRCryptoNetwork.h
    public static native Pointer cryptoNetworkGetUids(Pointer network);
    public static native Pointer cryptoNetworkGetName(Pointer network);
    public static native int cryptoNetworkIsMainnet(Pointer network);
    public static native Pointer cryptoNetworkGetCurrency(Pointer network);
    public static native Pointer cryptoNetworkGetUnitAsDefault(Pointer network, Pointer currency);
    public static native Pointer cryptoNetworkGetUnitAsBase(Pointer network, Pointer currency);
    public static native long cryptoNetworkGetHeight(Pointer network);
    public static native Pointer cryptoNetworkGetVerifiedBlockHash (Pointer network);
    public static native void cryptoNetworkSetVerifiedBlockHash (Pointer network, Pointer verifiedBlockHash);
    public static native void cryptoNetworkSetVerifiedBlockHashAsString (Pointer network, String verifiedBlockHashString);
    public static native int cryptoNetworkGetConfirmationsUntilFinal(Pointer network);
    public static native void cryptoNetworkSetConfirmationsUntilFinal(Pointer network, int confirmationsUntilFinal);
    public static native SizeT cryptoNetworkGetCurrencyCount(Pointer network);
    public static native Pointer cryptoNetworkGetCurrencyAt(Pointer network, SizeT index);
    public static native int cryptoNetworkHasCurrency(Pointer network, Pointer currency);
    public static native SizeT cryptoNetworkGetUnitCount(Pointer network, Pointer currency);
    public static native Pointer cryptoNetworkGetUnitAt(Pointer network, Pointer currency, SizeT index);
    // public static native void cryptoNetworkSetNetworkFees(Pointer network, BRCryptoNetworkFee[] fees, SizeT count);
    public static native Pointer cryptoNetworkGetNetworkFees(Pointer network, SizeTByReference count);
    public static native Pointer cryptoNetworkTake(Pointer obj);
    public static native void cryptoNetworkGive(Pointer obj);
    public static native int cryptoNetworkGetType(Pointer obj);
    public static native int cryptoNetworkGetDefaultAddressScheme(Pointer network);
    public static native Pointer cryptoNetworkGetSupportedAddressSchemes(Pointer network, SizeTByReference count);
    public static native int cryptoNetworkSupportsAddressScheme(Pointer network, int scheme);
    public static native int cryptoNetworkGetDefaultSyncMode(Pointer network);
    public static native Pointer cryptoNetworkGetSupportedSyncModes(Pointer network, SizeTByReference count);
    public static native int cryptoNetworkSupportsSyncMode(Pointer network, int mode);
    public static native int cryptoNetworkRequiresMigration(Pointer network);

    public static native Pointer cryptoNetworkInstallBuiltins(SizeTByReference count);
    public static native Pointer cryptoNetworkFindBuiltin(String uids);

    // crypto/BRCryptoNetwork.h (BRCryptoNetworkFee)
    public static native long cryptoNetworkFeeGetConfirmationTimeInMilliseconds(Pointer fee);
    public static native Pointer cryptoNetworkFeeGetPricePerCostFactor(Pointer fee);
    public static native int cryptoNetworkFeeEqual(Pointer fee, Pointer other);
    public static native void cryptoNetworkFeeGive(Pointer obj);

    // crypto/BRCryptoNetwork.h (BRCryptoPeer)
    public static native Pointer cryptoPeerCreate(Pointer network, String address, short port, String publicKey);
    public static native Pointer cryptoPeerGetNetwork(Pointer peer);
    public static native Pointer cryptoPeerGetAddress(Pointer peer);
    public static native Pointer cryptoPeerGetPublicKey(Pointer peer);
    public static native short cryptoPeerGetPort(Pointer peer);
    public static native int cryptoPeerIsIdentical(Pointer peer, Pointer other);
    public static native void cryptoPeerGive(Pointer peer);

    // crypto/BRCryptoPayment.h (BRCryptoPaymentProtocolRequestBitPayBuilder)
    public static native Pointer cryptoPaymentProtocolRequestBitPayBuilderCreate(Pointer network,
                                                                                 Pointer currency,
                                                                                 BRCryptoPayProtReqBitPayAndBip70Callbacks.ByValue callbacks,
                                                                                 String name,
                                                                                 long time,
                                                                                 long expires,
                                                                                 double feePerByte,
                                                                                 String memo,
                                                                                 String paymentUrl,
                                                                                 byte[] merchantData,
                                                                                 SizeT merchantDataLen);
    public static native void cryptoPaymentProtocolRequestBitPayBuilderAddOutput(Pointer builder, String address, long amount);
    public static native Pointer cryptoPaymentProtocolRequestBitPayBuilderBuild(Pointer builder);
    public static native void cryptoPaymentProtocolRequestBitPayBuilderGive(Pointer builder);

    // crypto/BRCryptoPayment.h (BRCryptoPaymentProtocolRequest)
    public static native int cryptoPaymentProtocolRequestValidateSupported(int type,
                                                                           Pointer network,
                                                                           Pointer currency,
                                                                           Pointer wallet);
    public static native Pointer cryptoPaymentProtocolRequestCreateForBip70(Pointer network,
                                                                            Pointer currency,
                                                                            BRCryptoPayProtReqBitPayAndBip70Callbacks.ByValue callbacks,
                                                                            byte[] serialization,
                                                                            SizeT serializationLen);
    public static native int cryptoPaymentProtocolRequestGetType(Pointer request);
    public static native int cryptoPaymentProtocolRequestIsSecure(Pointer request);
    public static native Pointer cryptoPaymentProtocolRequestGetMemo(Pointer request);
    public static native Pointer cryptoPaymentProtocolRequestGetPaymentURL(Pointer request);
    public static native Pointer cryptoPaymentProtocolRequestGetTotalAmount(Pointer request);
    public static native Pointer cryptoPaymentProtocolRequestGetRequiredNetworkFee (Pointer request);
    public static native Pointer cryptoPaymentProtocolRequestGetPrimaryTargetAddress(Pointer request);
    public static native Pointer cryptoPaymentProtocolRequestGetCommonName(Pointer request);
    public static native int cryptoPaymentProtocolRequestIsValid(Pointer request);
    public static native void cryptoPaymentProtocolRequestGive(Pointer request);

    // crypto/BRCryptoPayment.h (BRCryptoPaymentProtocolPayment)
    public static native Pointer cryptoPaymentProtocolPaymentCreate(Pointer request, Pointer transfer, Pointer refundAddress);
    public static native Pointer cryptoPaymentProtocolPaymentEncode(Pointer payment, SizeTByReference encodedLength);
    public static native void cryptoPaymentProtocolPaymentGive(Pointer payment);

    // crypto/BRCryptoPayment.h (BRCryptoPaymentProtocolPaymentACK)
    public static native Pointer cryptoPaymentProtocolPaymentACKCreateForBip70(byte[] serialization, SizeT serializationLen);
    public static native Pointer cryptoPaymentProtocolPaymentACKGetMemo(Pointer ack);
    public static native void cryptoPaymentProtocolPaymentACKGive(Pointer ack);

    // crypto/BRCryptoPrivate.h (BRCryptoCurrency)
    public static native Pointer cryptoCurrencyCreate(String uids, String name, String code, String type, String issuer);

    // crypto/BRCryptoPrivate.h (BRCryptoNetworkFee)
    public static native Pointer cryptoNetworkFeeCreate(long timeInternalInMilliseconds, Pointer pricePerCostFactor, Pointer pricePerCostFactorUnit);

    // crypto/BRCryptoPrivate.h (BRCryptoNetwork)
    public static native void cryptoNetworkSetHeight(Pointer network, long height);
    public static native void cryptoNetworkSetCurrency(Pointer network, Pointer currency);
    public static native void cryptoNetworkAddCurrency(Pointer network, Pointer currency, Pointer baseUnit, Pointer defaultUnit);
    public static native void cryptoNetworkAddCurrencyUnit(Pointer network, Pointer currency, Pointer unit);
    public static native void cryptoNetworkAddNetworkFee(Pointer network, Pointer networkFee);

    // crypto/BRCryptoPrivate.h (BRCryptoUnit)
    public static native Pointer cryptoUnitCreateAsBase(Pointer currency, String uids, String name, String symbol);
    public static native Pointer cryptoUnitCreate(Pointer currency, String uids, String name, String symbol, Pointer base, byte decimals);

    // crypto/BRCryptoTransfer.h
    public static native Pointer cryptoTransferGetSourceAddress(Pointer transfer);
    public static native Pointer cryptoTransferGetTargetAddress(Pointer transfer);
    public static native Pointer cryptoTransferGetAmount(Pointer transfer);
    public static native Pointer cryptoTransferGetAmountDirected(Pointer transfer);
    public static native int cryptoTransferGetDirection(Pointer transfer);
    public static native BRCryptoTransferState.ByValue cryptoTransferGetState(Pointer transfer);
    public static native Pointer cryptoTransferGetHash(Pointer transfer);
    public static native Pointer cryptoTransferGetUnitForAmount (Pointer transfer);
    public static native Pointer cryptoTransferGetUnitForFee (Pointer transfer);
    public static native Pointer cryptoTransferGetEstimatedFeeBasis (Pointer transfer);
    public static native Pointer cryptoTransferGetConfirmedFeeBasis (Pointer transfer);

    public static native SizeT cryptoTransferGetAttributeCount(Pointer transfer);
    public static native Pointer cryptoTransferGetAttributeAt(Pointer transfer, SizeT index);

    public static native int cryptoTransferEqual(Pointer transfer, Pointer other);
    public static native Pointer cryptoTransferTake(Pointer obj);
    public static native void cryptoTransferGive(Pointer obj);

    public static native Pointer cryptoTransferSubmitErrorGetMessage(BRCryptoTransferSubmitError error);

    public static native Pointer cryptoTransferAttributeCopy(Pointer attribute);
    public static native Pointer cryptoTransferAttributeGetKey(Pointer attribute);
    public static native Pointer cryptoTransferAttributeGetValue(Pointer attribute);
    public static native void cryptoTransferAttributeSetValue(Pointer attribute, String value);
    public static native int cryptoTransferAttributeIsRequired(Pointer attribute);
    public static native void cryptoTransferAttributeGive(Pointer attribute);


    // crypto/BRCryptoUnit.h
    public static native Pointer cryptoUnitGetUids(Pointer unit);
    public static native Pointer cryptoUnitGetName(Pointer unit);
    public static native Pointer cryptoUnitGetSymbol(Pointer unit);
    public static native Pointer cryptoUnitGetCurrency(Pointer unit);
    public static native int cryptoUnitHasCurrency(Pointer unit, Pointer currency);
    public static native Pointer cryptoUnitGetBaseUnit(Pointer unit);
    public static native byte cryptoUnitGetBaseDecimalOffset(Pointer unit);
    public static native int cryptoUnitIsCompatible(Pointer u1, Pointer u2);
    public static native int cryptoUnitIsIdentical(Pointer u1, Pointer u2);
    public static native void cryptoUnitGive(Pointer obj);

    // crypto/BRCryptoWallet.h
    public static native int cryptoWalletGetState(Pointer wallet);
    public static native Pointer cryptoWalletGetBalance(Pointer wallet);
    public static native Pointer cryptoWalletGetBalanceMaximum(Pointer wallet);
    public static native Pointer cryptoWalletGetBalanceMinimum(Pointer wallet);
    public static native Pointer cryptoWalletGetTransfers(Pointer wallet, SizeTByReference count);
    public static native int cryptoWalletHasTransfer(Pointer wallet, Pointer transfer);
    public static native Pointer cryptoWalletGetAddress(Pointer wallet, int addressScheme);
    public static native int cryptoWalletHasAddress(Pointer wallet, Pointer address);
    public static native Pointer cryptoWalletGetUnit(Pointer wallet);
    public static native Pointer cryptoWalletGetUnitForFee(Pointer wallet);
    public static native Pointer cryptoWalletGetCurrency(Pointer wallet);
    // INDIRECT: public static native Pointer cryptoWalletCreateTransfer(Pointer wallet, Pointer target, Pointer amount, Pointer feeBasis, SizeT attributesCount, Pointer arrayOfAttributes);
    public static native Pointer cryptoWalletCreateTransferForPaymentProtocolRequest(Pointer wallet, Pointer request, Pointer feeBasis);

    public static native SizeT cryptoWalletGetTransferAttributeCount(Pointer wallet, Pointer target);
    public static native Pointer cryptoWalletGetTransferAttributeAt(Pointer wallet, Pointer target, SizeT index);
    public static native int cryptoWalletValidateTransferAttribute(Pointer wallet, Pointer attribute, IntByReference validates);
    // INDIRECT: public static native int cryptoWalletValidateTransferAttributes(Pointer wallet, SizeT countOfAttributes, Pointer arrayOfAttributes, IntByReference validates);


    public static native Pointer cryptoWalletTake(Pointer wallet);
    public static native void cryptoWalletGive(Pointer obj);

    // crypto/BRCryptoWalletManager.h
    public static native Pointer cryptoWalletManagerWipe(Pointer network, String path);
    public static native Pointer cryptoWalletManagerCreate(Pointer listener,
                                                           BRCryptoClient.ByValue client,
                                                           Pointer account,
                                                           Pointer network,
                                                           int mode,
                                                           int addressScheme,
                                                           String path);
    public static native Pointer cryptoWalletManagerGetNetwork(Pointer cwm);
    public static native Pointer cryptoWalletManagerGetAccount(Pointer cwm);
    public static native int cryptoWalletManagerGetMode(Pointer cwm);
    public static native void cryptoWalletManagerSetMode(Pointer cwm, int mode);
    public static native BRCryptoWalletManagerState.ByValue cryptoWalletManagerGetState(Pointer cwm);
    public static native int cryptoWalletManagerGetAddressScheme (Pointer cwm);
    public static native void cryptoWalletManagerSetAddressScheme (Pointer cwm, int scheme);
    public static native Pointer cryptoWalletManagerGetPath(Pointer cwm);
    public static native void cryptoWalletManagerSetNetworkReachable(Pointer cwm, int isNetworkReachable);
    public static native Pointer cryptoWalletManagerGetWallet(Pointer cwm);
    public static native Pointer cryptoWalletManagerGetWallets(Pointer cwm, SizeTByReference count);
    public static native int cryptoWalletManagerHasWallet(Pointer cwm, Pointer wallet);
    public static native Pointer cryptoWalletManagerCreateWallet(Pointer cwm, Pointer currency);
    public static native void cryptoWalletManagerConnect(Pointer cwm, Pointer peer);
    public static native void cryptoWalletManagerDisconnect(Pointer cwm);
    public static native void cryptoWalletManagerSync(Pointer cwm);
    public static native void cryptoWalletManagerSyncToDepth(Pointer cwm, int depth);
    public static native void cryptoWalletManagerStop(Pointer cwm);
    public static native int cryptoWalletManagerSign(Pointer cwm, Pointer wid, Pointer tid, ByteBuffer paperKey);
    public static native void cryptoWalletManagerSubmit(Pointer cwm, Pointer wid, Pointer tid, ByteBuffer paperKey);
    public static native void cryptoWalletManagerSubmitForKey(Pointer cwm, Pointer wid, Pointer tid, Pointer key);
    public static native void cryptoWalletManagerSubmitSigned(Pointer cwm, Pointer wid, Pointer tid);
    public static native Pointer cryptoWalletManagerEstimateLimit(Pointer cwm, Pointer wid, int asMaximum, Pointer target, Pointer fee, IntByReference needEstimate, IntByReference isZeroIfInsuffientFunds);
    public static native void cryptoWalletManagerEstimateFeeBasis(Pointer cwm, Pointer wid, Pointer cookie, Pointer target, Pointer amount, Pointer fee);
    public static native void cryptoWalletManagerEstimateFeeBasisForWalletSweep(Pointer sweeper, Pointer cwm, Pointer wid, Pointer cookie, Pointer fee);
    public static native void cryptoWalletManagerEstimateFeeBasisForPaymentProtocolRequest(Pointer cwm, Pointer wid, Pointer cookie, Pointer request, Pointer fee);
    public static native Pointer cryptoWalletManagerTake(Pointer cwm);
    public static native void cryptoWalletManagerGive(Pointer cwm);

    public static native Pointer cryptoWalletManagerDisconnectReasonGetMessage(BRCryptoWalletManagerDisconnectReason reason);

    // crypto/BRCryptoSync.h
    public static native Pointer cryptoSyncStoppedReasonGetMessage(BRCryptoSyncStoppedReason reason);

    // crypto/BRCryptoWalletManager.h (BRCryptoWalletSweeper)
    public static native int cryptoWalletManagerWalletSweeperValidateSupported(Pointer cwm, Pointer wallet, Pointer key);
    public static native Pointer cryptoWalletManagerCreateWalletSweeper(Pointer cwm, Pointer wallet, Pointer key);
    public static native Pointer cryptoWalletSweeperGetKey(Pointer sweeper);
    public static native Pointer cryptoWalletSweeperGetBalance(Pointer sweeper);
    public static native Pointer cryptoWalletSweeperGetAddress(Pointer sweeper);
    public static native int cryptoWalletSweeperAddTransactionFromBundle(Pointer sweeper, byte[] transaction, SizeT transactionLen);//TODO:SWEEP use transaction bundle
    public static native int cryptoWalletSweeperValidate(Pointer sweeper);
    public static native void cryptoWalletSweeperRelease(Pointer sweeper);
    public static native Pointer cryptoWalletSweeperCreateTransferForWalletSweep(Pointer sweeper, Pointer walletManager, Pointer wallet, Pointer feeBasis);


    // crypto/BRCryptoClient.h
    public static native Pointer cryptoClientTransactionBundleCreate (int status,
                                                                      byte[] transaction,
                                                                      SizeT transactionLength,
                                                                      long timestamp,
                                                                      long blockHeight);
    // See 'Indirect': void cryptoClientTransferBundleCreate (int status, ...)

    public static native void cwmAnnounceBlockNumber(Pointer cwm, Pointer callbackState, boolean success, long blockNumber, String verifiedBlockHash);
    public static native void cwmAnnounceSubmitTransfer(Pointer cwm, Pointer callbackState, boolean success);

    //
    // Crypto Primitives
    //

    // crypto/BRCryptoCipher.h
    public static native Pointer cryptoCipherCreateForAESECB(byte[] key, SizeT keyLen);
    public static native Pointer cryptoCipherCreateForChacha20Poly1305(Pointer key, byte[] nonce12, SizeT nonce12Len, byte[] ad, SizeT adLen);
    public static native Pointer cryptoCipherCreateForPigeon(Pointer privKey, Pointer pubKey, byte[] nonce12, SizeT nonce12Len);
    public static native SizeT cryptoCipherEncryptLength(Pointer cipher, byte[] src, SizeT srcLen);
    public static native int cryptoCipherEncrypt(Pointer cipher, byte[] dst, SizeT dstLen, byte[] src, SizeT srcLen);
    public static native SizeT cryptoCipherDecryptLength(Pointer cipher, byte[] src, SizeT srcLen);
    public static native int cryptoCipherDecrypt(Pointer cipher, byte[] dst, SizeT dstLen, byte[] src, SizeT srcLen);
    public static native int cryptoCipherMigrateBRCoreKeyCiphertext(Pointer cipher, byte[] dst, SizeT dstLen, byte[] src, SizeT srcLen);
    public static native void cryptoCipherGive(Pointer cipher);

    // crypto/BRCryptoCoder.h
    public static native Pointer cryptoCoderCreate(int type);
    public static native SizeT cryptoCoderEncodeLength(Pointer coder, byte[] src, SizeT srcLen);
    public static native int cryptoCoderEncode(Pointer coder, byte[] dst, SizeT dstLen, byte[] src, SizeT srcLen);
    public static native SizeT cryptoCoderDecodeLength(Pointer coder, byte[] src);
    public static native int cryptoCoderDecode(Pointer coder, byte[] dst, SizeT dstLen, byte[] src);
    public static native void cryptoCoderGive(Pointer coder);

    // crypto/BRCryptoHasher.h
    public static native Pointer cryptoHasherCreate(int type);
    public static native SizeT cryptoHasherLength(Pointer hasher);
    public static native int cryptoHasherHash(Pointer hasher, byte[] dst, SizeT dstLen, byte[] src, SizeT srcLen);
    public static native void cryptoHasherGive(Pointer hasher);

    // crypto/BRCryptoSigner.h
    public static native Pointer cryptoSignerCreate(int type);
    public static native SizeT cryptoSignerSignLength(Pointer signer, Pointer key, byte[] digest, SizeT digestlen);
    public static native int cryptoSignerSign(Pointer signer, Pointer key, byte[] signature, SizeT signatureLen, byte[] digest, SizeT digestLen);
    public static native Pointer cryptoSignerRecover(Pointer signer, byte[] digest, SizeT digestLen, byte[] signature, SizeT signatureLen);
    public static native void cryptoSignerGive(Pointer signer);

    // crypto/BRCryptoListener.h
    public static native Pointer cryptoListenerCreate (Pointer context, Callback systemCB, Callback networkCB, Callback managerCB, Callback walletCB, Callback transferCB);

    // crypto/BRCryptoSystem.h
    public static native Pointer cryptoSystemCreate(BRCryptoClient.ByValue client,
                                                    Pointer listener,
                                                    Pointer account,
                                                    String path,
                                                    int onMainnet);

    public static native int cryptoSystemGetState (Pointer system);
    public static native int cryptoSystemOnMainnet (Pointer system);
    public static native int cryptoSystemIsReachable (Pointer system);
    public static native Pointer cryptoSystemGetResolvedPath (Pointer system);

    public static native int cryptoSystemHasNetwork (Pointer system, Pointer network);
    //extern BRCryptoNetwork *cryptoSystemGetNetworks (Pointer system, size_t *count);
    public static native Pointer cryptoSystemGetNetworkAt (Pointer system, SizeT index);
    public static native Pointer cryptoSystemGetNetworkForUids (Pointer system, String uids);
    public static native SizeT   cryptoSystemGetNetworksCount (Pointer system);

    public static native int cryptoSystemHasWalletManager (Pointer system, Pointer manager);
    //extern BRCryptoWalletManager *cryptoSystemGetWalletManagers (Pointer system, size_t *count);
    public static native Pointer cryptoSystemGetWalletManagerAt (Pointer system, SizeT index);
    public static native Pointer cryptoSystemGetWalletManagerByNetwork (Pointer system, Pointer network);
    public static native SizeT cryptoSystemGetWalletManagersCount (Pointer system);
    // See 'Indirect': Pointer cryptoSystemCreateWalletManager (Pointer system, ...);

    public static native void cryptoSystemStart (Pointer system);
    public static native void cryptoSystemStop (Pointer system);
    public static native void cryptoSystemConnect (Pointer system);
    public static native void cryptoSystemDisconnect (Pointer system);
    public static native Pointer cryptoSystemTake(Pointer obj);
    public static native void cryptoSystemGive(Pointer obj);

    static {
        Native.register(CryptoLibraryDirect.class, CryptoLibrary.LIBRARY);
    }

    private CryptoLibraryDirect() {}
}
