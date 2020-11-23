package com.breadwallet.corenative.crypto;

import com.breadwallet.corenative.CryptoLibraryDirect;
import com.breadwallet.corenative.CryptoLibraryIndirect;
import com.breadwallet.corenative.utility.SizeT;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

import java.util.ArrayList;
import java.util.List;


public class BRCryptoSystem extends PointerType {

    public static Optional<BRCryptoSystem> create (BRCryptoClient client,
                                                   BRCryptoListener listener,
                                                   BRCryptoAccount account,
                                                   String path,
                                                   boolean onMainnet) {

        return Optional.fromNullable(
                CryptoLibraryDirect.cryptoSystemCreate (
                        client.toByValue(),
                        listener.getPointer(),
                        account.getPointer(),
                        path,
                        onMainnet ? 1 : 0)
                )
                .transform(BRCryptoSystem::new);
    }

    public BRCryptoSystem() {
        super();
    }

    public BRCryptoSystem(Pointer address) {
        super(address);
    }

    public boolean onMainnet () {
        Pointer thisPtr = this.getPointer();
        return BRCryptoBoolean.CRYPTO_TRUE ==
                CryptoLibraryDirect.cryptoSystemOnMainnet(thisPtr);
    }

    public boolean isReachable () {
        Pointer thisPtr = this.getPointer();
        return BRCryptoBoolean.CRYPTO_TRUE ==
                CryptoLibraryDirect.cryptoSystemIsReachable(thisPtr);
    }

    public String getResolvedPath () {
        Pointer thisPtr = this.getPointer();
        return CryptoLibraryDirect.cryptoSystemGetResolvedPath(thisPtr).getString(0, "UTF-8");
    }

    public BRCryptoSystemState getState () {
        Pointer thisPtr = this.getPointer();
        return BRCryptoSystemState.fromCore(
                CryptoLibraryDirect.cryptoSystemGetState(thisPtr)
        );
    }

// MARK: - System Networks

    public boolean hasNetwork (BRCryptoNetwork network) {
        Pointer thisPtr = this.getPointer();
        return BRCryptoBoolean.CRYPTO_TRUE ==
                CryptoLibraryDirect.cryptoSystemHasNetwork(
                        thisPtr,
                        network.getPointer());
    }

    public List<BRCryptoNetwork> getNetworks () {
        Pointer thisPtr = this.getPointer();
        return new ArrayList<>();
    }

    public Optional<BRCryptoNetwork> getNetworkAt (int index) {
        Pointer thisPtr = this.getPointer();
        return Optional.fromNullable(
                CryptoLibraryDirect.cryptoSystemGetNetworkAt(
                        thisPtr,
                        new SizeT(index))
        )
                .transform(BRCryptoNetwork::new);
    }

    public Optional<BRCryptoNetwork> getNetworkForUIDS (String uids) {
        Pointer thisPtr = this.getPointer();
        return Optional.fromNullable(
                CryptoLibraryDirect.cryptoSystemGetNetworkForUids(
                        thisPtr,
                        uids)
        )
                .transform(BRCryptoNetwork::new);
    }

    public UnsignedLong getNetworksCount() {
        Pointer thisPtr = this.getPointer();
        return UnsignedLong.fromLongBits(
                CryptoLibraryDirect.cryptoSystemGetNetworksCount(
                        thisPtr
                ).longValue());
    }


// MARK: - System Wallet Managers

    public boolean hasManager (BRCryptoWalletManager manager) {
        Pointer thisPtr = this.getPointer();
        return BRCryptoBoolean.CRYPTO_TRUE ==
                CryptoLibraryDirect.cryptoSystemHasWalletManager(
                        thisPtr,
                        manager.getPointer());
    }

    public List<BRCryptoWalletManager> getManagers () {
        Pointer thisPtr = this.getPointer();
        return new ArrayList<>();
    }

    public Optional<BRCryptoWalletManager> getManagerAt(int index) {
        Pointer thisPtr = this.getPointer();
        return Optional.fromNullable(
                CryptoLibraryDirect.cryptoSystemGetWalletManagerAt(
                        thisPtr,
                        new SizeT(index))
        )
                .transform(BRCryptoWalletManager::new);
    }

    public UnsignedLong getManagersCount() {
        Pointer thisPtr = this.getPointer();
        return UnsignedLong.fromLongBits(
                CryptoLibraryDirect.cryptoSystemGetWalletManagersCount(
                        thisPtr
                ).longValue());
    }

    public Optional<BRCryptoWalletManager> getManagerForNetwork(BRCryptoNetwork network) {
        Pointer thisPtr = this.getPointer();
        return Optional.fromNullable(
                CryptoLibraryDirect.cryptoSystemGetWalletManagerByNetwork(
                        thisPtr,
                        network.getPointer())
        )
                .transform(BRCryptoWalletManager::new);
    }

    public Optional<BRCryptoWalletManager> createManager(BRCryptoSystem system,
                                                         BRCryptoNetwork network,
                                                         BRCryptoSyncMode mode,
                                                         BRCryptoAddressScheme scheme,
                                                         List<BRCryptoCurrency> currencies) {
        Pointer thisPtr = this.getPointer();

        int currenciesCount = currencies.size();
        BRCryptoCurrency[] currenciesRefs = new BRCryptoCurrency[currenciesCount];
        for (int i = 0; i < currenciesCount; i++) currenciesRefs[i] = currencies.get(i);

        return Optional.fromNullable(
                CryptoLibraryIndirect.cryptoSystemCreateWalletManager(
                        thisPtr,
                        network.getPointer(),
                        mode.toCore(),
                        scheme.toCore(),
                        new SizeT(currenciesCount),
                        currenciesRefs)
        ).transform(BRCryptoWalletManager::new);
    }

    public void start () {
        Pointer thisPtr = this.getPointer();
        CryptoLibraryDirect.cryptoSystemStart(thisPtr);
    }

    public void stop () {
        Pointer thisPtr = this.getPointer();
        CryptoLibraryDirect.cryptoSystemStop(thisPtr);
    }

    public void connect () {
        Pointer thisPtr = this.getPointer();
        CryptoLibraryDirect.cryptoSystemConnect(thisPtr);
    }

    public void disconnect () {
        Pointer thisPtr = this.getPointer();
        CryptoLibraryDirect.cryptoSystemDisconnect(thisPtr);
    }

    public BRCryptoSystem take() {
        Pointer thisPtr = this.getPointer();

        return new BRCryptoSystem(CryptoLibraryDirect.cryptoSystemTake(thisPtr));
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        CryptoLibraryDirect.cryptoSystemGive(thisPtr);
    }

}
