/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corenative;

import com.breadwallet.corenative.crypto.BRCryptoAddressScheme;
import com.breadwallet.corenative.crypto.BRCryptoClient;
import com.breadwallet.corenative.crypto.BRCryptoClientCurrencyBundle;
import com.breadwallet.corenative.crypto.BRCryptoClientCurrencyDenominationBundle;
import com.breadwallet.corenative.crypto.BRCryptoClientTransactionBundle;
import com.breadwallet.corenative.crypto.BRCryptoClientTransferBundle;
import com.breadwallet.corenative.crypto.BRCryptoCurrency;
import com.breadwallet.corenative.crypto.BRCryptoNetworkFee;
import com.breadwallet.corenative.crypto.BRCryptoSyncMode;
import com.breadwallet.corenative.crypto.BRCryptoTransferAttribute;
import com.breadwallet.corenative.utility.SizeT;
import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;
import com.sun.jna.ptr.IntByReference;

public final class CryptoLibraryIndirect {

    // JNA does NOT support passing zero length arrays using the indirect interface method; as a result, check
    // all arrays being passed as arguments for lengths and NULL appropriately

    private static final LibraryInterface INSTANCE = Native.load(CryptoLibrary.LIBRARY_NAME, LibraryInterface.class);

    // Can this be migrated to CryptoLibraryDirect? Well, not easily. The JNA library explicitly mentions
    // it doesn't support arrays of pointers in direct mapping mode. That said, it has an example of how
    // this can be done (see: com.sun.jna.StringArray).
    public static void cryptoNetworkSetNetworkFees(Pointer network, BRCryptoNetworkFee[] fees, SizeT count) {
        fees = fees.length == 0 ? null : fees;
        INSTANCE.cryptoNetworkSetNetworkFees(network, fees, count);
    }

    public static Pointer cryptoWalletCreateTransfer(Pointer wallet, Pointer target, Pointer amount, Pointer feeBasis, SizeT attributesCount, BRCryptoTransferAttribute[] attributes) {
        attributes = attributesCount.intValue() == 0 ? null : attributes;
        return INSTANCE.cryptoWalletCreateTransfer(wallet, target, amount, feeBasis, attributesCount, attributes);
    }

    public static int cryptoWalletValidateTransferAttributes(Pointer wallet, SizeT attributesCount, BRCryptoTransferAttribute[] attributes, IntByReference validates) {
        attributes = attributesCount.intValue() == 0 ? null : attributes;
        return INSTANCE.cryptoWalletValidateTransferAttributes(wallet, attributesCount, attributes, validates);
    }

    public static void cwmAnnounceEstimateTransactionFee(Pointer cwm,
                                                         Pointer callbackState,
                                                         int success,
                                                         long costUnits,
                                                         SizeT attributesCount,
                                                         String[] attributeKeys,
                                                         String[] attributeVals) {
        attributeKeys = attributesCount.intValue() == 0 ? null : attributeKeys;
        attributeVals = attributesCount.intValue() == 0 ? null : attributeVals;

        INSTANCE.cwmAnnounceEstimateTransactionFee(cwm, callbackState, success,
                costUnits,
                attributesCount,
                attributeKeys,
                attributeVals);
    }

    public static Pointer cryptoClientTransferBundleCreate(int status,
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
        return INSTANCE.cryptoClientTransferBundleCreate(status,
                uids, hash, identifier, sourceAddr, targetAddr,
                amount, currency, fee,
                blockTimestamp, blockHeight, blockConfirmations, blockTransactionIndex, blockHash,
                attributesCount, attributeKeys, attributeVals);
    }

    public static void cwmAnnounceTransactions(Pointer cwm, Pointer callbackState, int success, BRCryptoClientTransactionBundle[] bundles, SizeT bundlesCount) {
        INSTANCE.cwmAnnounceTransactions(cwm, callbackState, success,
                (0 == bundlesCount.intValue() ? null : bundles),
                bundlesCount);
    }

    public static void cwmAnnounceTransfers(Pointer cwm, Pointer callbackState, int success, BRCryptoClientTransferBundle[] bundles, SizeT bundlesCount) {
        INSTANCE.cwmAnnounceTransfers(cwm, callbackState, success,
                (0 == bundlesCount.intValue() ? null : bundles),
                bundlesCount);
    }

    public static Pointer cryptoClientCurrencyBundleCreate(String id,
                                                           String name,
                                                           String code,
                                                           String type,
                                                           String blockchainId,
                                                           String address,
                                                           boolean verified,
                                                           SizeT denominationsCount,
                                                           BRCryptoClientCurrencyDenominationBundle[] denominations) {
        return INSTANCE.cryptoClientCurrencyBundleCreate(
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

    public static void cwmAnnounceCurrencies(Pointer system, BRCryptoClientCurrencyBundle[] bundles, SizeT bundlesCount) {
        INSTANCE.cwmAnnounceCurrencies(system,
                (0 == bundles.length ? null : bundles),
                bundlesCount);
    }

    public static void cryptoWalletManagerEstimateFeeBasis(Pointer cwm,
                                                           Pointer wid,
                                                           Pointer cookie,
                                                           Pointer target,
                                                           Pointer amount,
                                                           Pointer fee,
                                                           SizeT attributesCount,
                                                           BRCryptoTransferAttribute[] attributes) {
        attributes = attributes.length == 0 ? null : attributes;
        INSTANCE.cryptoWalletManagerEstimateFeeBasis(
                cwm,
                wid,
                cookie,
                target,
                amount,
                fee,
                attributesCount,
                attributes);
    }

    public static Pointer cryptoSystemCreateWalletManager(Pointer system,
                                                          Pointer network,
                                                          int mode,
                                                          int scheme,
                                                          BRCryptoCurrency[] currencies,
                                                          SizeT currenciesCount) {
        return INSTANCE.cryptoSystemCreateWalletManager(
                system,
                network,
                mode,
                scheme,
                (0 == currencies.length ? null : currencies),
                currenciesCount);
    }

    public interface LibraryInterface extends Library {

        // crypto/BRCryptoNetwork.h
        void cryptoNetworkSetNetworkFees(Pointer network, BRCryptoNetworkFee[] fees, SizeT count);

        // crypto/BRCryptoWallet.h
        Pointer cryptoWalletCreateTransfer(Pointer wallet, Pointer target, Pointer amount, Pointer feeBasis, SizeT attributesCount, BRCryptoTransferAttribute[] attributes);

        int cryptoWalletValidateTransferAttributes(Pointer wallet, SizeT countOfAttributes, BRCryptoTransferAttribute[] attributes, IntByReference validates);

        Pointer cryptoClientTransferBundleCreate(int status,
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

        void cwmAnnounceTransactions(Pointer cwm, Pointer callbackState, int success, BRCryptoClientTransactionBundle[] bundles, SizeT bundlesCount);
        void cwmAnnounceTransfers(Pointer cwm, Pointer callbackState, int success, BRCryptoClientTransferBundle[] bundles, SizeT bundlesCount);

        Pointer cryptoClientCurrencyBundleCreate(String id,
                                                 String name,
                                                 String code,
                                                 String type,
                                                 String blockchainId,
                                                 String address,
                                                 boolean verified,
                                                 SizeT denominationsCount,
                                                 BRCryptoClientCurrencyDenominationBundle[] denominations);

        void cwmAnnounceCurrencies (Pointer system, BRCryptoClientCurrencyBundle[] bundles, SizeT bundlesCount);

        void cwmAnnounceEstimateTransactionFee(Pointer cwm,
                                               Pointer callbackState,
                                               int success,
                                               long costUnits,
                                               SizeT attributesCount,
                                               String[] attributeKeys,
                                               String[] attributeVals);

        // crypto/BRCryptoWalletManager.h
        void cryptoWalletManagerEstimateFeeBasis (Pointer cwm,
                                                  Pointer wallet,
                                                  Pointer cookie,
                                                  Pointer target,
                                                  Pointer amount,
                                                  Pointer fee,
                                                  SizeT attributesCount,
                                                  BRCryptoTransferAttribute[] attributes);

        // crypto/BRCryptoSystem.h
        Pointer cryptoSystemCreateWalletManager(Pointer system,
                                                Pointer network,
                                                int mode,
                                                int scheme,
                                                BRCryptoCurrency[] currencies,
                                                SizeT currenciesCount);
    }
}
