/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex.library;

import com.blockset.walletkit.nativex.WKClientCurrencyBundle;
import com.blockset.walletkit.nativex.WKClientCurrencyDenominationBundle;
import com.blockset.walletkit.nativex.WKClientError;
import com.blockset.walletkit.nativex.WKClientTransactionBundle;
import com.blockset.walletkit.nativex.WKClientTransferBundle;
import com.blockset.walletkit.nativex.WKCurrency;
import com.blockset.walletkit.nativex.WKNetworkFee;
import com.blockset.walletkit.nativex.WKTransferAttribute;
import com.blockset.walletkit.nativex.utility.*;
import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.IntByReference;

public final class WKNativeLibraryIndirect {

    // JNA does NOT support passing zero length arrays using the indirect interface method; as a result, check
    // all arrays being passed as arguments for lengths and NULL appropriately

    private static final LibraryInterface INSTANCE = Native.load(WKNativeLibrary.LIBRARY_NAME, LibraryInterface.class);

    // Can this be migrated to CryptoLibraryDirect? Well, not easily. The JNA library explicitly mentions
    // it doesn't support arrays of pointers in direct mapping mode. That said, it has an example of how
    // this can be done (see: com.sun.jna.StringArray).
    public static void wkNetworkSetNetworkFees(Pointer network, WKNetworkFee[] fees, SizeT count) {
        fees = fees.length == 0 ? null : fees;
        INSTANCE.wkNetworkSetNetworkFees(network, fees, count);
    }

    public static Pointer wkWalletCreateTransfer(Pointer wallet, Pointer target, Pointer amount, Pointer feeBasis, SizeT attributesCount, WKTransferAttribute[] attributes) {
        attributes = attributesCount.intValue() == 0 ? null : attributes;
        return INSTANCE.wkWalletCreateTransfer(wallet, target, amount, feeBasis, attributesCount, attributes);
    }

    public static int wkWalletValidateTransferAttributes(Pointer wallet, SizeT attributesCount, WKTransferAttribute[] attributes, IntByReference validates) {
        attributes = attributesCount.intValue() == 0 ? null : attributes;
        return INSTANCE.wkWalletValidateTransferAttributes(wallet, attributesCount, attributes, validates);
    }

    public static void wkClientAnnounceEstimateTransactionFeeSuccess(Pointer cwm,
                                                                     Pointer callbackState,
                                                                     long costUnits,
                                                                     SizeT attributesCount,
                                                                     String[] attributeKeys,
                                                                     String[] attributeVals) {
        attributeKeys = attributesCount.intValue() == 0 ? null : attributeKeys;
        attributeVals = attributesCount.intValue() == 0 ? null : attributeVals;

        INSTANCE.wkClientAnnounceEstimateTransactionFeeSuccess(cwm, callbackState,
                costUnits,
                attributesCount,
                attributeKeys,
                attributeVals);
    }

    public static void wkClientAnnounceEstimateTransactionFeeFailure(Pointer cwm,
                                                                     Pointer callbackState,
                                                                     Pointer error) {
        INSTANCE.wkClientAnnounceEstimateTransactionFeeFailure(cwm, callbackState, error);
    }


    public static Pointer wkClientTransferBundleCreate(int status,
                                                       String hash,
                                                       String identifier,
                                                       String uids,
                                                       String sourceAddr,
                                                       String targetAddr,
                                                       String amount,
                                                       String currency,
                                                       String fee,
                                                       long transferIndex,
                                                       long blockTimestamp,
                                                       long blockHeight,
                                                       long blockConfirmations,
                                                       long blockTransactionIndex,
                                                       String blockHash,
                                                       SizeT attributesCount,
                                                       String[] attributeKeys,
                                                       String[] attributeVals) {
        attributeKeys = attributesCount.intValue() == 0 ? null : attributeKeys;
        attributeVals = attributesCount.intValue() == 0 ? null : attributeVals;
        return INSTANCE.wkClientTransferBundleCreate(status,
                hash, identifier, uids, sourceAddr, targetAddr,
                amount, currency, fee,
                transferIndex, blockTimestamp, blockHeight, blockConfirmations, blockTransactionIndex, blockHash,
                attributesCount, attributeKeys, attributeVals);
    }

    public static void wkClientAnnounceTransactionsSuccess(Pointer cwm, Pointer callbackState, WKClientTransactionBundle[] bundles, SizeT bundlesCount) {
        INSTANCE.wkClientAnnounceTransactionsSuccess(cwm, callbackState,
                (0 == bundlesCount.intValue() ? null : bundles),
                bundlesCount);
    }

    public static void wkClientAnnounceTransactionsFailure(Pointer cwm, Pointer callbackState, Pointer error) {
        INSTANCE.wkClientAnnounceTransactionsFailure(cwm, callbackState, error);
    }

    public static void wkClientAnnounceTransfersSuccess(Pointer cwm, Pointer callbackState, WKClientTransferBundle[] bundles, SizeT bundlesCount) {
        INSTANCE.wkClientAnnounceTransfersSuccess(cwm, callbackState,
                (0 == bundlesCount.intValue() ? null : bundles),
                bundlesCount);
    }

    public static void wkClientAnnounceTransfersFailure(Pointer cwm, Pointer callbackState, Pointer error) {
        INSTANCE.wkClientAnnounceTransfersFailure(cwm, callbackState, error);
    }

    public static Pointer wkClientCurrencyBundleCreate(String id,
                                                       String name,
                                                       String code,
                                                       String type,
                                                       String blockchainId,
                                                       String address,
                                                       boolean verified,
                                                       SizeT denominationsCount,
                                                       WKClientCurrencyDenominationBundle[] denominations) {
        return INSTANCE.wkClientCurrencyBundleCreate(
                id,
                name,
                code,
                type,
                blockchainId,
                address,
                verified,
                denominationsCount,
                (0 == denominationsCount.intValue() ? null : denominations));
    }

    public static void wkClientAnnounceCurrenciesSuccess (Pointer system, WKClientCurrencyBundle[] bundles, SizeT bundlesCount) {
        INSTANCE.wkClientAnnounceCurrenciesSuccess(system,
                (0 == bundles.length ? null : bundles),
                bundlesCount);
    }

    public static void wkClientAnnounceCurrenciesFailure(Pointer system, Pointer error) {
        INSTANCE.wkClientAnnounceCurrenciesFailure(system, error);
    }

    public static void wkWalletManagerEstimateFeeBasis(Pointer cwm,
                                                       Pointer wid,
                                                       Pointer cookie,
                                                       Pointer target,
                                                       Pointer amount,
                                                       Pointer fee,
                                                       SizeT attributesCount,
                                                       WKTransferAttribute[] attributes) {
        attributes = attributes.length == 0 ? null : attributes;
        INSTANCE.wkWalletManagerEstimateFeeBasis(
                cwm,
                wid,
                cookie,
                target,
                amount,
                fee,
                attributesCount,
                attributes);
    }

    public static Pointer wkSystemCreateWalletManager(Pointer system,
                                                      Pointer network,
                                                      int mode,
                                                      int scheme,
                                                      WKCurrency[] currencies,
                                                      SizeT currenciesCount) {
        return INSTANCE.wkSystemCreateWalletManager(
                system,
                network,
                mode,
                scheme,
                (0 == currencies.length ? null : currencies),
                currenciesCount);
    }

    public interface LibraryInterface extends Library {

        // crypto/BRCryptoNetwork.h
        void wkNetworkSetNetworkFees(Pointer network, WKNetworkFee[] fees, SizeT count);

        // crypto/BRCryptoWallet.h
        Pointer wkWalletCreateTransfer(Pointer wallet, Pointer target, Pointer amount, Pointer feeBasis, SizeT attributesCount, WKTransferAttribute[] attributes);

        int wkWalletValidateTransferAttributes(Pointer wallet, SizeT countOfAttributes, WKTransferAttribute[] attributes, IntByReference validates);

        Pointer wkClientTransferBundleCreate(int status,
                                             String hash,
                                             String identifier,
                                             String uids,
                                             String sourceAddr,
                                             String targetAddr,
                                             String amount,
                                             String currency,
                                             String fee,
                                             long transferIndex,
                                             long blockTimestamp,
                                             long blockHeight,
                                             long blockConfirmations,
                                             long blockTransactionIndex,
                                             String blockHash,
                                             SizeT attributesCount,
                                             String[] attributeKeys,
                                             String[] attributeVals);

        void wkClientAnnounceTransactionsSuccess(Pointer cwm, Pointer callbackState, WKClientTransactionBundle[] bundles, SizeT bundlesCount);

        void wkClientAnnounceTransactionsFailure(Pointer cwm, Pointer callbackState, Pointer error);

        void wkClientAnnounceTransfersSuccess(Pointer cwm, Pointer callbackState, WKClientTransferBundle[] bundles, SizeT bundlesCount);

        void wkClientAnnounceTransfersFailure(Pointer cwm, Pointer callbackState, Pointer error);

        Pointer wkClientCurrencyBundleCreate(String id,
                                             String name,
                                             String code,
                                             String type,
                                             String blockchainId,
                                             String address,
                                             boolean verified,
                                             SizeT denominationsCount,
                                             WKClientCurrencyDenominationBundle[] denominations);

        void wkClientAnnounceCurrenciesSuccess(Pointer system, WKClientCurrencyBundle[] bundles, SizeT bundlesCount);

        void wkClientAnnounceCurrenciesFailure(Pointer system, Pointer error);

        void wkClientAnnounceEstimateTransactionFeeSuccess(Pointer cwm,
                                                           Pointer callbackState,
                                                           long costUnits,
                                                           SizeT attributesCount,
                                                           String[] attributeKeys,
                                                           String[] attributeVals);

        void wkClientAnnounceEstimateTransactionFeeFailure(Pointer cwm,
                                                           Pointer callbackState,
                                                           Pointer error);

        // crypto/BRCryptoWalletManager.h
        void wkWalletManagerEstimateFeeBasis(Pointer cwm,
                                             Pointer wallet,
                                             Pointer cookie,
                                             Pointer target,
                                             Pointer amount,
                                             Pointer fee,
                                             SizeT attributesCount,
                                             WKTransferAttribute[] attributes);

        // crypto/BRCryptoSystem.h
        Pointer wkSystemCreateWalletManager(Pointer system,
                                            Pointer network,
                                            int mode,
                                            int scheme,
                                            WKCurrency[] currencies,
                                            SizeT currenciesCount);
    }
}
