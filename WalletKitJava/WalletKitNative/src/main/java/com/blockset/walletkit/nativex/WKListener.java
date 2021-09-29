/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.blockset.walletkit.nativex.utility.Cookie;
import com.sun.jna.Callback;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKListener extends PointerType {

    //
    // Implementation Detail
    //
    public interface BRCryptoListenerSystemEvent extends Callback {
        void callback(Pointer context,
                      Pointer system,
                      WKSystemEvent.ByValue event);
    }

    public interface BRCryptoListenerNetworkEvent extends Callback {
        void callback(Pointer context,
                      Pointer network,
                      WKNetworkEvent.ByValue event);
    }

    public interface BRCryptoListenerWalletManagerEvent extends Callback {
        void callback(Pointer context,
                      Pointer manager,
                      WKWalletManagerEvent.ByValue event);
    }

    public interface BRCryptoListenerWalletEvent extends Callback {
        void callback(Pointer context,
                      Pointer manager,
                      Pointer wallet,
                      WKWalletEvent event);
    }

    public interface BRCryptoListenerTransferEvent extends Callback {
        void callback(Pointer context,
                      Pointer manager,
                      Pointer wallet,
                      Pointer transfer,
                      WKTransferEvent.ByValue event);
    }

    //
    // Client Interface
    //

    public interface SystemEventCallback extends BRCryptoListenerSystemEvent {
        void handle(Cookie context,
                /* OwnershipGiven */ WKSystem system,
                /* OwnershipGiven */ WKSystemEvent.ByValue event);

        @Override
        default void callback(Pointer context,
                              Pointer system,
                              WKSystemEvent.ByValue event) {
            handle(new Cookie(context),
                    new WKSystem(system),
                    event);
        }
    }

    public interface NetworkEventCallback extends BRCryptoListenerNetworkEvent {
        void handle(Cookie context,
                /* OwnershipGiven */ WKNetwork network,
                /* OwnershipGiven */ WKNetworkEvent.ByValue event);

        @Override
        default void callback(Pointer context,
                              Pointer network,
                              WKNetworkEvent.ByValue event) {
            handle(new Cookie(context),
                    new WKNetwork(network),
                    event);
        }
    }

    public interface WalletManagerEventCallback extends BRCryptoListenerWalletManagerEvent {
        void handle(Cookie context,
                /* OwnershipGiven */ WKWalletManager manager,
                /* OwnershipGiven */ WKWalletManagerEvent.ByValue event);

        @Override
        default void callback(Pointer context,
                              Pointer manager,
                              WKWalletManagerEvent.ByValue event) {
            handle(new Cookie(context),
                    new WKWalletManager(manager),
                    event);
        }
    }

    public interface WalletEventCallback extends BRCryptoListenerWalletEvent {
        void handle(Cookie context,
                /* OwnershipGiven */ WKWalletManager manager,
                /* OwnershipGiven */ WKWallet wallet,
                /* OwnershipGiven */ WKWalletEvent event);

        @Override
        default void callback(Pointer context,
                              Pointer manager,
                              Pointer wallet,
                              WKWalletEvent event) {
            handle(new Cookie(context),
                    new WKWalletManager(manager),
                    new WKWallet(wallet),
                    event);
        }
    }

    public interface TransferEventCallback extends BRCryptoListenerTransferEvent {
        void handle(Cookie context,
                /* OwnershipGiven */ WKWalletManager manager,
                /* OwnershipGiven */ WKWallet wallet,
                /* OwnershipGiven */ WKTransfer transfer,
                /* OwnershipGiven */ WKTransferEvent.ByValue event);

        @Override
        default void callback(Pointer context,
                              Pointer manager,
                              Pointer wallet,
                              Pointer transfer,
                              WKTransferEvent.ByValue event) {
            handle(new Cookie(context),
                    new WKWalletManager(manager),
                    new WKWallet(wallet),
                    new WKTransfer(transfer),
                    event);
        }
    }

    //
    // Listener Pointer
    //

    public WKListener() {
        super();
    }

    public WKListener(Pointer pointer) {
        super(pointer);
    }

    public static WKListener create(Cookie context,
                                    SystemEventCallback systemEventCallback,
                                    NetworkEventCallback networkEventCallback,
                                    WalletManagerEventCallback walletManagerEventCallback,
                                    WalletEventCallback walletEventCallback,
                                    TransferEventCallback transferEventCallback) {
        return new WKListener(
                WKNativeLibraryDirect.wkListenerCreate(
                        context.getPointer(),
                        systemEventCallback,
                        networkEventCallback,
                        walletManagerEventCallback,
                        walletEventCallback,
                        transferEventCallback));
    }

    public WKListener take() {
        Pointer thisPtr = this.getPointer();

        return new WKListener(WKNativeLibraryDirect.wkListenerTake(thisPtr));
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkListenerGive(thisPtr);
    }

}
