/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.blockset.walletkit.nativex.library.WKNativeLibraryIndirect;
import com.blockset.walletkit.nativex.utility.Cookie;
import com.blockset.walletkit.nativex.utility.SizeT;
import com.blockset.walletkit.nativex.utility.SizeTByReference;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedInts;
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Memory;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;
import com.sun.jna.Structure;
import com.sun.jna.ptr.IntByReference;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

import javax.annotation.Nullable;

public class WKWalletManager extends PointerType {

    public static void wipe(WKNetwork network, String path) {
        WKNativeLibraryDirect.wkWalletManagerWipe(network.getPointer(), path);
    }

    public static Optional<WKWalletManager> create(WKSystem system,
                                                   WKListener listener,
                                                   WKClient client,
                                                   WKAccount account,
                                                   WKNetwork network,
                                                   WKSyncMode mode,
                                                   WKAddressScheme scheme,
                                                   String path) {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkWalletManagerCreate(
                        new Listener (listener, system).toByValue(),
                        client.toByValue(),
                        account.getPointer(),
                        network.getPointer(),
                        mode.toCore(),
                        scheme.toCore(),
                        path
                )
        ).transform(WKWalletManager::new);
    }

    public WKWalletManager() {
        super();
    }

    public WKWalletManager(Pointer address) {
        super(address);
    }

    public WKAccount getAccount() {
        Pointer thisPtr = this.getPointer();

        return new WKAccount(WKNativeLibraryDirect.wkWalletManagerGetAccount(thisPtr));
    }

    public WKNetwork getNetwork() {
        Pointer thisPtr = this.getPointer();

        return new WKNetwork(WKNativeLibraryDirect.wkWalletManagerGetNetwork(thisPtr));
    }

    public WKWallet getWallet() {
        Pointer thisPtr = this.getPointer();

        return new WKWallet(WKNativeLibraryDirect.wkWalletManagerGetWallet(thisPtr));
    }


    public List<WKWallet> getWallets() {
        Pointer thisPtr = this.getPointer();

        List<WKWallet> wallets = new ArrayList<>();
        SizeTByReference count = new SizeTByReference();
        Pointer walletsPtr = WKNativeLibraryDirect.wkWalletManagerGetWallets(thisPtr, count);
        if (null != walletsPtr) {
            try {
                int walletsSize = UnsignedInts.checkedCast(count.getValue().longValue());
                for (Pointer walletPtr : walletsPtr.getPointerArray(0, walletsSize)) {
                    wallets.add(new WKWallet(walletPtr));
                }

            } finally {
                Native.free(Pointer.nativeValue(walletsPtr));
            }
        }
        return wallets;
    }

    public boolean containsWallet(WKWallet wallet) {
        Pointer thisPtr = this.getPointer();

        return  WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkWalletManagerHasWallet(thisPtr, wallet.getPointer());
    }

    public Optional<WKWallet> registerWallet(WKCurrency currency) {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkWalletManagerCreateWallet(
                        thisPtr,
                        currency.getPointer()
                )
        ).transform(WKWallet::new);
    }

    public void setNetworkReachable(boolean isNetworkReachable) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkWalletManagerSetNetworkReachable(
                thisPtr,
                isNetworkReachable ? WKBoolean.WK_TRUE : WKBoolean.WK_FALSE
        );
    }

    public WKSyncMode getMode() {
        Pointer thisPtr = this.getPointer();

        return WKSyncMode.fromCore(WKNativeLibraryDirect.wkWalletManagerGetMode(thisPtr));
    }

    public void setMode(WKSyncMode mode) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkWalletManagerSetMode(thisPtr, mode.toCore());
    }

    public String getPath() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkWalletManagerGetPath(thisPtr).getString(0, "UTF-8");
    }

    public WKWalletManagerState getState() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkWalletManagerGetState(thisPtr);
    }

    public WKAddressScheme getAddressScheme() {
        Pointer thisPtr = this.getPointer();

        return WKAddressScheme.fromCore(WKNativeLibraryDirect.wkWalletManagerGetAddressScheme(thisPtr));
    }

    public void setAddressScheme(WKAddressScheme scheme) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkWalletManagerSetAddressScheme(thisPtr, scheme.toCore());
    }

    public void connect(@Nullable WKPeer peer) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkWalletManagerConnect(thisPtr, peer == null ? null : peer.getPointer());
    }

    public void disconnect() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkWalletManagerDisconnect(thisPtr);
    }

    public void sync() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkWalletManagerSync(thisPtr);
    }

    public void stop() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkWalletManagerStop(thisPtr);
    }

    public void syncToDepth(WKSyncDepth depth) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkWalletManagerSyncToDepth(thisPtr, depth.toCore());
    }

    public boolean sign(WKWallet wallet, WKTransfer transfer, byte[] phraseUtf8) {
        Pointer thisPtr = this.getPointer();

        // ensure string is null terminated
        phraseUtf8 = Arrays.copyOf(phraseUtf8, phraseUtf8.length + 1);
        try {
            Memory phraseMemory = new Memory(phraseUtf8.length);
            try {
                phraseMemory.write(0, phraseUtf8, 0, phraseUtf8.length);
                ByteBuffer phraseBuffer = phraseMemory.getByteBuffer(0, phraseUtf8.length);

                int success = WKNativeLibraryDirect.wkWalletManagerSign(thisPtr, wallet.getPointer(), transfer.getPointer(), phraseBuffer);
                return WKBoolean.WK_TRUE == success;
            } finally {
                phraseMemory.clear();
            }
        } finally {
            // clear out our copy; caller responsible for original array
            Arrays.fill(phraseUtf8, (byte) 0);
        }
    }

    public void submit(WKWallet wallet, WKTransfer transfer, byte[] phraseUtf8) {
        Pointer thisPtr = this.getPointer();

        // ensure string is null terminated
        phraseUtf8 = Arrays.copyOf(phraseUtf8, phraseUtf8.length + 1);
        try {
            Memory phraseMemory = new Memory(phraseUtf8.length);
            try {
                phraseMemory.write(0, phraseUtf8, 0, phraseUtf8.length);
                ByteBuffer phraseBuffer = phraseMemory.getByteBuffer(0, phraseUtf8.length);

                WKNativeLibraryDirect.wkWalletManagerSubmit(thisPtr, wallet.getPointer(), transfer.getPointer(), phraseBuffer);
            } finally {
                phraseMemory.clear();
            }
        } finally {
            // clear out our copy; caller responsible for original array
            Arrays.fill(phraseUtf8, (byte) 0);
        }
    }

    public void submit(WKWallet wallet, WKTransfer transfer, WKKey key) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkWalletManagerSubmitForKey(thisPtr, wallet.getPointer(), transfer.getPointer(), key.getPointer());
    }

    public void submit(WKWallet wallet, WKTransfer transfer) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkWalletManagerSubmitSigned(thisPtr, wallet.getPointer(), transfer.getPointer());
    }

    public static class EstimateLimitResult {

        public @Nullable
        WKAmount amount;
        public boolean needFeeEstimate;
        public boolean isZeroIfInsuffientFunds;

        EstimateLimitResult(@Nullable WKAmount amount, boolean needFeeEstimate, boolean isZeroIfInsuffientFunds) {
            this.amount = amount;
            this.needFeeEstimate = needFeeEstimate;
            this.isZeroIfInsuffientFunds = isZeroIfInsuffientFunds;
        }
    }

    public EstimateLimitResult estimateLimit(WKWallet wallet, boolean asMaximum, WKAddress coreAddress, WKNetworkFee coreFee) {
        IntByReference needFeeEstimateRef = new IntByReference(WKBoolean.WK_FALSE);
        IntByReference isZeroIfInsuffientFundsRef = new IntByReference(WKBoolean.WK_FALSE);

        Optional<WKAmount> maybeAmount = Optional.fromNullable(WKNativeLibraryDirect.wkWalletManagerEstimateLimit(
                this.getPointer(),
                wallet.getPointer(),
                asMaximum ? WKBoolean.WK_TRUE : WKBoolean.WK_FALSE,
                coreAddress.getPointer(),
                coreFee.getPointer(),
                needFeeEstimateRef,
                isZeroIfInsuffientFundsRef
        )).transform(WKAmount::new);

        return new EstimateLimitResult(
                maybeAmount.orNull(),
                needFeeEstimateRef.getValue() == WKBoolean.WK_TRUE,
                isZeroIfInsuffientFundsRef.getValue() == WKBoolean.WK_TRUE
        );
    }

    public void estimateFeeBasis(WKWallet wallet, Cookie cookie,
                                 WKAddress target, WKAmount amount, WKNetworkFee fee, List<WKTransferAttribute> attributes) {
        Pointer thisPtr = this.getPointer();

        int attributesCount  = attributes.size();
        WKTransferAttribute[] attributeRefs = new WKTransferAttribute[attributesCount];
        for (int i = 0; i < attributesCount; i++) attributeRefs[i] = attributes.get(i);

        WKNativeLibraryIndirect.wkWalletManagerEstimateFeeBasis(
                thisPtr,
                wallet.getPointer(),
                cookie.getPointer(),
                target.getPointer(),
                amount.getPointer(),
                fee.getPointer(),
                new SizeT(attributesCount),
                attributeRefs);
    }

    public void estimateFeeBasisForWalletSweep(WKWallet wallet, Cookie cookie,
                                               WKWalletSweeper sweeper, WKNetworkFee fee) {
        WKNativeLibraryDirect.wkWalletManagerEstimateFeeBasisForWalletSweep(
                sweeper.getPointer(),
                this.getPointer(),
                wallet.getPointer(),
                cookie.getPointer(),
                fee.getPointer());
    }

    public void estimateFeeBasisForPaymentProtocolRequest(WKWallet wallet, Cookie cookie,
                                                          WKPaymentProtocolRequest request, WKNetworkFee fee) {
        WKNativeLibraryDirect.wkWalletManagerEstimateFeeBasisForPaymentProtocolRequest(
                this.getPointer(),
                wallet.getPointer(),
                cookie.getPointer(),
                request.getPointer(),
                fee.getPointer());
    }

    public void announceGetBlockNumberSuccess(WKClientCallbackState callbackState, UnsignedLong blockNumber, String verifiedBlockHash) {
        WKNativeLibraryDirect.wkClientAnnounceBlockNumberSuccess (
                this.getPointer(),
                callbackState.getPointer(),
                blockNumber.longValue(),
                verifiedBlockHash);
    }

    public void announceGetBlockNumberFailure(WKClientCallbackState callbackState, WKClientError error) {
        WKNativeLibraryDirect.wkClientAnnounceBlockNumberFailure (
                this.getPointer(),
                callbackState.getPointer(),
                error.getPointer());
    }

    public void announceTransactionsSuccess(WKClientCallbackState callbackState, List<WKClientTransactionBundle> bundles) {
        int bundlesCount = bundles.size();
        WKClientTransactionBundle[] bundlesArr = bundles.toArray(new WKClientTransactionBundle[bundlesCount]);

        WKNativeLibraryIndirect.wkClientAnnounceTransactionsSuccess(
                this.getPointer(),
                callbackState.getPointer(),
                bundlesArr,
                new SizeT(bundlesCount));
    }

    public void announceTransactionsFailure(WKClientCallbackState callbackState, WKClientError error) {

        WKNativeLibraryIndirect.wkClientAnnounceTransactionsFailure(
                this.getPointer(),
                callbackState.getPointer(),
                error.getPointer());
     }


    public void announceTransfersSuccess(WKClientCallbackState callbackState, List<WKClientTransferBundle> bundles) {
        int bundlesCount = bundles.size();
        WKClientTransferBundle[] bundlesArr = bundles.toArray(new WKClientTransferBundle[bundlesCount]);

        WKNativeLibraryIndirect.wkClientAnnounceTransfersSuccess(
                this.getPointer(),
                callbackState.getPointer(),
                bundlesArr,
                new SizeT(bundlesCount));
    }

    public void announceTransfersFailure(WKClientCallbackState callbackState, WKClientError error) {

        WKNativeLibraryIndirect.wkClientAnnounceTransfersFailure(
                this.getPointer(),
                callbackState.getPointer(),
                error.getPointer());
    }

    public void announceSubmitTransferSuccess(WKClientCallbackState callbackState, String identifier, String hash) {
        WKNativeLibraryDirect.wkClientAnnounceSubmitTransferSuccess (
                this.getPointer(),
                callbackState.getPointer(),
                identifier,
                hash);
    }

    public void announceSubmitTransferFailure(WKClientCallbackState callbackState, WKClientError error) {
        WKNativeLibraryDirect.wkClientAnnounceSubmitTransferFailure (
                this.getPointer(),
                callbackState.getPointer(),
                error.getPointer());
    }

    public void announceEstimateTransactionFeeSuccess(WKClientCallbackState callbackState, UnsignedLong costUnits, Map<String, String> meta) {
        int metaCount     = (null == meta ? 0    : meta.size());
        String[] metaKeys = (null == meta ? null : meta.keySet().toArray(new String[metaCount]));
        String[] metaVals = (null == meta ? null : meta.values().toArray(new String[metaCount]));

        WKNativeLibraryIndirect.wkClientAnnounceEstimateTransactionFeeSuccess(
                this.getPointer(),
                callbackState.getPointer(),
                costUnits.longValue(),
                new SizeT(metaCount),
                metaKeys,
                metaVals);
    }

    public void announceEstimateTransactionFeeFailure(WKClientCallbackState callbackState, WKClientError error) {
        WKNativeLibraryIndirect.wkClientAnnounceEstimateTransactionFeeFailure(
                this.getPointer(),
                callbackState.getPointer(),
                error.getPointer());
    }

    public WKWalletManager take() {
        Pointer thisPtr = this.getPointer();

        return new WKWalletManager(WKNativeLibraryDirect.wkWalletManagerTake(thisPtr));
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkWalletManagerGive(thisPtr);
    }

    public static class Listener extends Structure {
        public WKListener listener;
        public WKSystem system;

        public Listener() { super(); }
        public Listener(Pointer pointer) { super(pointer); }

        public Listener (WKListener listener,
                         WKSystem system) {
            super();
            this.listener = listener;
            this.system   = system;
        }

        @Override
        protected List<String> getFieldOrder() {
            return Arrays.asList(
                    "listener",
                    "system"
            );
        }

        public ByValue toByValue() {
            ByValue other = new ByValue();

            other.listener = this.listener;
            other.system = this.system;

            return other;
        }

        public static class ByValue extends Listener implements Structure.ByValue {}
        }
}
