/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corenative.crypto;

import com.breadwallet.corenative.CryptoLibraryDirect;
import com.breadwallet.corenative.utility.Cookie;
import com.sun.jna.Callback;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class BRCryptoListener extends PointerType {

    //
    // Implementation Detail
    //
    public interface BRCryptoListenerSystemEvent extends Callback {
        void callback(Pointer context,
                      Pointer system,
                      BRCryptoSystemEvent.ByValue event);
    }

    public interface BRCryptoListenerNetworkEvent extends Callback {
        void callback(Pointer context,
                      Pointer network,
                      BRCryptoNetworkEvent.ByValue event);
    }

    public interface BRCryptoListenerWalletManagerEvent extends Callback {
        void callback(Pointer context,
                      Pointer manager,
                      BRCryptoWalletManagerEvent.ByValue event);
    }

    public interface BRCryptoListenerWalletEvent extends Callback {
        void callback(Pointer context,
                      Pointer manager,
                      Pointer wallet,
                      BRCryptoWalletEvent.ByValue event);
    }

    public interface BRCryptoListenerTransferEvent extends Callback {
        void callback(Pointer context,
                      Pointer manager,
                      Pointer wallet,
                      Pointer transfer,
                      BRCryptoTransferEvent.ByValue event);
    }

    //
    // Client Interface
    //

    public interface SystemEventCallback extends BRCryptoListenerSystemEvent {
        void handle(Cookie context,
                    BRCryptoSystem system,
                    BRCryptoSystemEvent.ByValue event);

        @Override
        default void callback(Pointer context,
                              Pointer system,
                              BRCryptoSystemEvent.ByValue event) {
            handle(new Cookie(context),
                    new BRCryptoSystem(system),
                    event);
        }
    }

    public interface NetworkEventCallback extends BRCryptoListenerNetworkEvent {
        void handle(Cookie context,
                    BRCryptoNetwork network,
                    BRCryptoNetworkEvent.ByValue event);

        @Override
        default void callback(Pointer context,
                              Pointer network,
                              BRCryptoNetworkEvent.ByValue event) {
            handle(new Cookie(context),
                    new BRCryptoNetwork(network),
                    event);
        }
    }

    public interface WalletManagerEventCallback extends BRCryptoListenerWalletManagerEvent {
        void handle(Cookie context,
                    BRCryptoWalletManager manager,
                    BRCryptoWalletManagerEvent.ByValue event);

        @Override
        default void callback(Pointer context,
                              Pointer manager,
                              BRCryptoWalletManagerEvent.ByValue event) {
            handle(new Cookie(context),
                    new BRCryptoWalletManager(manager),
                    event);
        }
    }

    public interface WalletEventCallback extends BRCryptoListenerWalletEvent {
        void handle(Cookie context,
                    BRCryptoWalletManager manager,
                    BRCryptoWallet wallet,
                    BRCryptoWalletEvent.ByValue event);

        @Override
        default void callback(Pointer context,
                              Pointer manager,
                              Pointer wallet,
                              BRCryptoWalletEvent.ByValue event) {
            handle(new Cookie(context),
                    new BRCryptoWalletManager(manager),
                    new BRCryptoWallet(wallet),
                    event);
        }
    }

    public interface TransferEventCallback extends BRCryptoListenerTransferEvent {
        void handle(Cookie context,
                    BRCryptoWalletManager manager,
                    BRCryptoWallet wallet,
                    BRCryptoTransfer transfer,
                    BRCryptoTransferEvent.ByValue event);

        @Override
        default void callback(Pointer context,
                              Pointer manager,
                              Pointer wallet,
                              Pointer transfer,
                              BRCryptoTransferEvent.ByValue event) {
            handle(new Cookie(context),
                    new BRCryptoWalletManager(manager),
                    new BRCryptoWallet(wallet),
                    new BRCryptoTransfer(transfer),
                    event);
        }
    }

    //
    // Listener Pointer
    //

    public BRCryptoListener() {
        super();
    }

    public BRCryptoListener(Pointer pointer) {
        super(pointer);
    }

    public static BRCryptoListener create(Cookie context,
                                          SystemEventCallback systemEventCallback,
                                          NetworkEventCallback networkEventCallback,
                                          WalletManagerEventCallback walletManagerEventCallback,
                                          WalletEventCallback walletEventCallback,
                                          TransferEventCallback transferEventCallback) {
        return new BRCryptoListener(
                CryptoLibraryDirect.cryptoListenerCreate(
                        context.getPointer(),
                        systemEventCallback,
                        networkEventCallback,
                        walletManagerEventCallback,
                        walletEventCallback,
                        transferEventCallback));
    }

    public BRCryptoListener take() {
        Pointer thisPtr = this.getPointer();

        return new BRCryptoListener(CryptoLibraryDirect.cryptoListenerTake(thisPtr));
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        CryptoLibraryDirect.cryptoListenerGive(thisPtr);
    }

}
