/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.blockset.walletkit.nativex.library.WKNativeLibraryIndirect;
import com.blockset.walletkit.nativex.utility.SizeT;
import com.blockset.walletkit.nativex.utility.SizeTByReference;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedInts;
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

import java.util.ArrayList;
import java.util.List;

public class WKSystem extends PointerType {

    public static Optional<WKSystem> create (WKClient client,
                                             WKListener listener,
                                             WKAccount account,
                                             String path,
                                             boolean onMainnet) {

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkSystemCreate (
                        client.toByValue(),
                        listener.getPointer(),
                        account.getPointer(),
                        path,
                        onMainnet ? 1 : 0)
                )
                .transform(WKSystem::new);
    }

    public WKSystem() {
        super();
    }

    public WKSystem(Pointer address) {
        super(address);
    }

    @SuppressWarnings("unused")
    public boolean onMainnet () {
        Pointer thisPtr = this.getPointer();
        return WKBoolean.WK_TRUE ==
                WKNativeLibraryDirect.wkSystemOnMainnet(thisPtr);
    }

    @SuppressWarnings("unused")
    public boolean isReachable () {
        Pointer thisPtr = this.getPointer();
        return WKBoolean.WK_TRUE ==
                WKNativeLibraryDirect.wkSystemIsReachable(thisPtr);
    }

    public void setIsReachable (boolean reachable) {
        Pointer thisPtr = this.getPointer();
        WKNativeLibraryDirect.wkSystemSetReachable(thisPtr, reachable);
    }

    @SuppressWarnings("unused")
    public String getResolvedPath () {
        Pointer thisPtr = this.getPointer();
        return WKNativeLibraryDirect.wkSystemGetResolvedPath(thisPtr).getString(0, "UTF-8");
    }

    public WKSystemState getState () {
        Pointer thisPtr = this.getPointer();
        return WKSystemState.fromCore(
                WKNativeLibraryDirect.wkSystemGetState(thisPtr)
        );
    }

// MARK: - System Networks

    @SuppressWarnings("unused")
    public boolean hasNetwork (WKNetwork network) {
        Pointer thisPtr = this.getPointer();
        return WKBoolean.WK_TRUE ==
                WKNativeLibraryDirect.wkSystemHasNetwork(
                        thisPtr,
                        network.getPointer());
    }

    public List<WKNetwork> getNetworks () {
        List<WKNetwork> networks = new ArrayList<>();

        SizeTByReference count = new SizeTByReference();
        Pointer networksPtr = WKNativeLibraryDirect.wkSystemGetNetworks(this.getPointer(), count);
        if (null != networksPtr) {
            try {
                int networksSize = UnsignedInts.checkedCast(count.getValue().longValue());
                for (Pointer networkPtr : networksPtr.getPointerArray(0, networksSize)) {
                    networks.add(new WKNetwork(networkPtr));
                }
            } finally {
                Native.free(Pointer.nativeValue(networksPtr));
            }
        }

        return networks;
    }

    @SuppressWarnings("unused")
    public Optional<WKNetwork> getNetworkAt (int index) {
        Pointer thisPtr = this.getPointer();
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkSystemGetNetworkAt(
                        thisPtr,
                        new SizeT(index))
        )
                .transform(WKNetwork::new);
    }

    @SuppressWarnings("unused")
    public Optional<WKNetwork> getNetworkForUIDS (String uids) {
        Pointer thisPtr = this.getPointer();
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkSystemGetNetworkForUids(
                        thisPtr,
                        uids)
        )
                .transform(WKNetwork::new);
    }

    public UnsignedLong getNetworksCount() {
        Pointer thisPtr = this.getPointer();
        return UnsignedLong.fromLongBits(
                WKNativeLibraryDirect.wkSystemGetNetworksCount(
                        thisPtr
                ).longValue());
    }


// MARK: - System Wallet Managers

    public boolean hasManager (WKWalletManager manager) {
        Pointer thisPtr = this.getPointer();
        return WKBoolean.WK_TRUE ==
                WKNativeLibraryDirect.wkSystemHasWalletManager(
                        thisPtr,
                        manager.getPointer());
    }

    public List<WKWalletManager> getManagers () {
        List<WKWalletManager> managers = new ArrayList<>();

        SizeTByReference count = new SizeTByReference();
        Pointer managersPtr = WKNativeLibraryDirect.wkSystemGetWalletManagers(this.getPointer(), count);
        if (null != managersPtr) {
            try {
                int managersSize = UnsignedInts.checkedCast(count.getValue().longValue());
                for (Pointer managerPtr : managersPtr.getPointerArray(0, managersSize)) {
                    managers.add(new WKWalletManager(managerPtr));
                }
            } finally {
                Native.free(Pointer.nativeValue(managersPtr));
            }
        }

        return managers;
    }

    @SuppressWarnings("unused")
    public Optional<WKWalletManager> getManagerAt(int index) {
        Pointer thisPtr = this.getPointer();
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkSystemGetWalletManagerAt(
                        thisPtr,
                        new SizeT(index))
        )
                .transform(WKWalletManager::new);
    }

    public UnsignedLong getManagersCount() {
        Pointer thisPtr = this.getPointer();
        return UnsignedLong.fromLongBits(
                WKNativeLibraryDirect.wkSystemGetWalletManagersCount(
                        thisPtr
                ).longValue());
    }

    @SuppressWarnings("unused")
    public Optional<WKWalletManager> getManagerForNetwork(WKNetwork network) {
        Pointer thisPtr = this.getPointer();
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkSystemGetWalletManagerByNetwork(
                        thisPtr,
                        network.getPointer())
        )
                .transform(WKWalletManager::new);
    }

    public Optional<WKWalletManager> createManager(WKSystem system,
                                                   WKNetwork network,
                                                   WKSyncMode mode,
                                                   WKAddressScheme scheme,
                                                   List<WKCurrency> currencies) {
        Pointer thisPtr = this.getPointer();

        int currenciesCount = currencies.size();
        WKCurrency[] currenciesRefs = new WKCurrency[currenciesCount];
        for (int i = 0; i < currenciesCount; i++) currenciesRefs[i] = currencies.get(i);

        return Optional.fromNullable(
                WKNativeLibraryIndirect.wkSystemCreateWalletManager(
                        thisPtr,
                        network.getPointer(),
                        mode.toCore(),
                        scheme.toCore(),
                        currenciesRefs,
                        new SizeT(currenciesCount))
        ).transform(WKWalletManager::new);
    }

    public void start () {
        Pointer thisPtr = this.getPointer();
        WKNativeLibraryDirect.wkSystemStart(thisPtr);
    }

    public void stop () {
        Pointer thisPtr = this.getPointer();
        WKNativeLibraryDirect.wkSystemStop(thisPtr);
    }

    public void connect () {
        Pointer thisPtr = this.getPointer();
        WKNativeLibraryDirect.wkSystemConnect(thisPtr);
    }

    public void disconnect () {
        Pointer thisPtr = this.getPointer();
        WKNativeLibraryDirect.wkSystemDisconnect(thisPtr);
    }

    public void announceCurrencies(List<WKClientCurrencyBundle> bundles) {
        int bundlesCount = bundles.size();
        WKClientCurrencyBundle[] bundlesArr = bundles.toArray(new WKClientCurrencyBundle[bundlesCount]);

        WKNativeLibraryIndirect.wkClientAnnounceCurrencies(
                this.getPointer(),
                bundlesArr,
                new SizeT(bundlesCount));
    }

    public WKSystem take() {
        Pointer thisPtr = this.getPointer();

        return new WKSystem(WKNativeLibraryDirect.wkSystemTake(thisPtr));
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkSystemGive(thisPtr);
    }

}
