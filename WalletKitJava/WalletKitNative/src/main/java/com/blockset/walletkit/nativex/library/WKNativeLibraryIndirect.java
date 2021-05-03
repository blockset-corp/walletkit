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

    public static void wkClientAnnounceEstimateTransactionFee(Pointer cwm,
                                                                  Pointer callbackState,
                                                                  int success,
                                                                  long costUnits,
                                                                  SizeT attributesCount,
                                                                  String[] attributeKeys,
                                                                  String[] attributeVals) {
        attributeKeys = attributesCount.intValue() == 0 ? null : attributeKeys;
        attributeVals = attributesCount.intValue() == 0 ? null : attributeVals;

        INSTANCE.wkClientAnnounceEstimateTransactionFee(cwm, callbackState, success,
                costUnits,
                attributesCount,
                attributeKeys,
                attributeVals);
    }

    public static Pointer wkClientTransferBundleCreate(int status,
                                                           String uids,
                                                           String hash,
                                                           String identifier,
                                                           String sourceAddr,
                                                           String targetAddr,
                                                           String amount,
                                                           String currency,
                                                           String fee,
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
                uids, hash, identifier, sourceAddr, targetAddr,
                amount, currency, fee,
                blockTimestamp, blockHeight, blockConfirmations, blockTransactionIndex, blockHash,
                attributesCount, attributeKeys, attributeVals);
    }

    public static void wkClientAnnounceTransactions(Pointer cwm, Pointer callbackState, int success, WKClientTransactionBundle[] bundles, SizeT bundlesCount) {
        INSTANCE.wkClientAnnounceTransactions(cwm, callbackState, success,
                (0 == bundlesCount.intValue() ? null : bundles),
                bundlesCount);
    }

    public static void wkClientAnnounceTransfers(Pointer cwm, Pointer callbackState, int success, WKClientTransferBundle[] bundles, SizeT bundlesCount) {
        INSTANCE.wkClientAnnounceTransfers(cwm, callbackState, success,
                (0 == bundlesCount.intValue() ? null : bundles),
                bundlesCount);
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

    public static void wkClientAnnounceCurrencies(Pointer system, WKClientCurrencyBundle[] bundles, SizeT bundlesCount) {
        INSTANCE.wkClientAnnounceCurrencies(system,
                (0 == bundles.length ? null : bundles),
                bundlesCount);
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
                                                 String uids,
                                                 String hash,
                                                 String identifier,
                                                 String sourceAddr,
                                                 String targetAddr,
                                                 String amount,
                                                 String currency,
                                                 String fee,
                                                 long blockTimestamp,
                                                 long blockHeight,
                                                 long blockConfirmations,
                                                 long blockTransactionIndex,
                                                 String blockHash,
                                                 SizeT attributesCount,
                                                 String[] attributeKeys,
                                                 String[] attributeVals);

        void wkClientAnnounceTransactions(Pointer cwm, Pointer callbackState, int success, WKClientTransactionBundle[] bundles, SizeT bundlesCount);
        void wkClientAnnounceTransfers(Pointer cwm, Pointer callbackState, int success, WKClientTransferBundle[] bundles, SizeT bundlesCount);

        Pointer wkClientCurrencyBundleCreate(String id,
                                                 String name,
                                                 String code,
                                                 String type,
                                                 String blockchainId,
                                                 String address,
                                                 boolean verified,
                                                 SizeT denominationsCount,
                                                 WKClientCurrencyDenominationBundle[] denominations);

        void wkClientAnnounceCurrencies (Pointer system, WKClientCurrencyBundle[] bundles, SizeT bundlesCount);

        void wkClientAnnounceEstimateTransactionFee(Pointer cwm,
                                                        Pointer callbackState,
                                                        int success,
                                                        long costUnits,
                                                        SizeT attributesCount,
                                                        String[] attributeKeys,
                                                        String[] attributeVals);

        // crypto/BRCryptoWalletManager.h
        void wkWalletManagerEstimateFeeBasis (Pointer cwm,
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
