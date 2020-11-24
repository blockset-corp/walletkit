/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corenative;

import com.breadwallet.corenative.crypto.BRCryptoAddressScheme;
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
        attributes = attributes.length == 0 ? null : attributes;
        return INSTANCE.cryptoWalletCreateTransfer(wallet, target, amount, feeBasis, attributesCount, attributes);
    }

    public static int cryptoWalletValidateTransferAttributes(Pointer wallet, SizeT countOfAttributes, BRCryptoTransferAttribute[] attributes, IntByReference validates) {
        attributes = attributes.length == 0 ? null : attributes;
        return INSTANCE.cryptoWalletValidateTransferAttributes(wallet, countOfAttributes, attributes, validates);
    }

    public static void cwmAnnounceEstimateTransactionFee(Pointer cwm,
                                                         Pointer callbackState,
                                                         boolean success,
                                                         String hash,
                                                         long costUnits,
                                                         SizeT attributesCount,
                                                         String[] attributeKeys,
                                                         String[] attributeVals) {
        attributeKeys = attributeKeys.length == 0 ? null : attributeKeys;
        attributeVals = attributeVals.length == 0 ? null : attributeVals;

        INSTANCE.cwmAnnounceEstimateTransactionFee(cwm, callbackState, success,
                hash,
                costUnits,
                attributesCount,
                attributeKeys,
                attributeVals);
    }

    public static Pointer cryptoClientTransferBundleCreate(int status,
                                                           String hash,
                                                           String uids,
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
        attributeKeys = attributeKeys.length == 0 ? null : attributeKeys;
        attributeVals = attributeVals.length == 0 ? null : attributeVals;
        return INSTANCE.cryptoClientTransferBundleCreate(status,
                hash, uids, sourceAddr, targetAddr,
                amount, currency, fee,
                blockTimestamp, blockHeight, blockConfirmations, blockTransactionIndex, blockHash,
                attributesCount, attributeKeys, attributeVals);
    }

    public static void cwmAnnounceTransactions(Pointer cwm, Pointer callbackState, boolean success, BRCryptoClientTransactionBundle[] bundles, SizeT bundlesCount) {
        INSTANCE.cwmAnnounceTransactions(cwm, callbackState, success,
                bundles,
                bundlesCount);
    }

    public static void cwmAnnounceTransfers(Pointer cwm, Pointer callbackState, boolean success, BRCryptoClientTransferBundle[] bundles, SizeT bundlesCount) {
        INSTANCE.cwmAnnounceTransfers(cwm, callbackState, success,
                bundles,
                bundlesCount);
    }

    public static Pointer cryptoSystemCreateWalletManager(Pointer system,
                                                          Pointer network,
                                                          int mode,
                                                          int scheme,
                                                          SizeT currenciesCount,
                                                          BRCryptoCurrency[] currencies) {
        return INSTANCE.cryptoSystemCreateWalletManager(system, network, mode, scheme, currenciesCount, currencies);
    }

    public interface LibraryInterface extends Library {

        // crypto/BRCryptoNetwork.h
        void cryptoNetworkSetNetworkFees(Pointer network, BRCryptoNetworkFee[] fees, SizeT count);

        // crypto/BRCryptoWallet.h
        Pointer cryptoWalletCreateTransfer(Pointer wallet, Pointer target, Pointer amount, Pointer feeBasis, SizeT attributesCount, BRCryptoTransferAttribute[] attributes);

        int cryptoWalletValidateTransferAttributes(Pointer wallet, SizeT countOfAttributes, BRCryptoTransferAttribute[] attributes, IntByReference validates);

        Pointer cryptoClientTransferBundleCreate(int status,
                                                 String hash,
                                                 String uids,
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

        void cwmAnnounceTransactions(Pointer cwm, Pointer callbackState, boolean success, BRCryptoClientTransactionBundle[] bundles, SizeT bundlesCount);
        void cwmAnnounceTransfers(Pointer cwm, Pointer callbackState, boolean success, BRCryptoClientTransferBundle[] bundles, SizeT bundlesCount);

        void cwmAnnounceEstimateTransactionFee(Pointer cwm,
                                               Pointer callbackState,
                                               boolean success,
                                               String hash,
                                               long costUnits,
                                               SizeT attributesCount,
                                               String[] attributeKeys,
                                               String[] attributeVals);

        // crypto/BRCryptoSystem.h
        Pointer cryptoSystemCreateWalletManager(Pointer system,
                                                Pointer network,
                                                int mode,
                                                int scheme,
                                                SizeT currenciesCount,
                                                BRCryptoCurrency[] currencies);
    }
}
