/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corenative.crypto;

import com.breadwallet.corenative.utility.Cookie;
import com.breadwallet.corenative.utility.SizeT;
import com.google.common.primitives.UnsignedInts;
import com.sun.jna.Callback;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;


public class BRCryptoClient extends Structure {

    //
    // Implementation Detail
    //

    public interface BRCryptoClientGetBlockNumberCallback extends Callback {
        void callback(Pointer context,
                      Pointer manager,
                      Pointer callbackState);
    }

    public interface BRCryptoClientGetTransactionsCallback extends Callback {
        void callback(Pointer context,
                      Pointer manager,
                      Pointer callbackState,
                      Pointer addrs,
                      SizeT addrCount,
                      long begBlockNumber,
                      long endBlockNumber);
    }

    public interface BRCryptoClientGetTransfersCallback extends Callback {
        void callback(Pointer context,
                      Pointer manager,
                      Pointer callbackState,
                      Pointer addrs,
                      SizeT addrCount,
                      long begBlockNumber,
                      long endBlockNumber);
    }

    public interface BRCryptoClientSubmitTransactionCallback extends Callback {
        void callback(Pointer context,
                      Pointer manager,
                      Pointer callbackState,
                      Pointer tx,
                      SizeT txLength);
    }

    public interface BRCryptoClientEstimateTransactionFeeCallback extends Callback {
        void callback(Pointer context,
                      Pointer manager,
                      Pointer callbackState,
                      Pointer tx,
                      SizeT txLength);
    }

    //
    // Client Interface
    //

     public interface GetBlockNumberCallback extends BRCryptoClientGetBlockNumberCallback {
        void handle(Cookie context,
                    BRCryptoWalletManager manager,
                    BRCryptoClientCallbackState callbackState);

        @Override
        default void callback(Pointer context,
                              Pointer manager,
                              Pointer callbackState) {
            handle(
                    new Cookie(context),
                    new BRCryptoWalletManager(manager),
                    new BRCryptoClientCallbackState(callbackState)
            );
        }
    }

    public interface GetTransactionsCallback extends BRCryptoClientGetTransactionsCallback {
        void handle(Cookie context,
                    BRCryptoWalletManager manager,
                    BRCryptoClientCallbackState callbackState,
                    List<String> addresses,
                    long begBlockNumber,
                    long endBlockNumber);

        @Override
        default void callback(Pointer context,
                              Pointer manager,
                              Pointer callbackState,
                              Pointer addrs,
                              SizeT addrCount,
                              long begBlockNumber,
                              long endBlockNumber) {
            int addressesCount = UnsignedInts.checkedCast(addrCount.longValue());
            String[] addressesArray = addrs.getStringArray(0, addressesCount, "UTF-8");
            List<String> addressesList = Arrays.asList(addressesArray);

            handle(
                    new Cookie(context),
                    new BRCryptoWalletManager(manager),
                    new BRCryptoClientCallbackState(callbackState),
                    addressesList,
                    begBlockNumber,
                    endBlockNumber
            );
        }
    }

    public interface GetTransfersCallback extends BRCryptoClientGetTransfersCallback {
        void handle(Cookie context,
                    BRCryptoWalletManager manager,
                    BRCryptoClientCallbackState callbackState,
                    List<String> addresses,
                    long begBlockNumber,
                    long endBlockNumber);

        @Override
        default void callback(Pointer context,
                              Pointer manager,
                              Pointer callbackState,
                              Pointer addrs,
                              SizeT addrCount,
                              long begBlockNumber,
                              long endBlockNumber) {
            int addressesCount = UnsignedInts.checkedCast(addrCount.longValue());
            String[] addressesArray = addrs.getStringArray(0, addressesCount, "UTF-8");
            List<String> addressesList = Arrays.asList(addressesArray);

            handle(
                    new Cookie(context),
                    new BRCryptoWalletManager(manager),
                    new BRCryptoClientCallbackState(callbackState),
                    addressesList,
                    begBlockNumber,
                    endBlockNumber
            );
        }
    }

    public interface SubmitTransactionCallback extends BRCryptoClientSubmitTransactionCallback {
        void handle(Cookie context,
                    BRCryptoWalletManager manager,
                    BRCryptoClientCallbackState callbackState,
                    byte[] transaction);

        @Override
        default void callback(Pointer context,
                              Pointer manager,
                              Pointer callbackState,
                              Pointer tx,
                              SizeT txLength) {
            handle(
                    new Cookie(context),
                    new BRCryptoWalletManager(manager),
                    new BRCryptoClientCallbackState(callbackState),
                    tx.getByteArray(0, UnsignedInts.checkedCast(txLength.longValue()))
            );
        }
    }

    public interface EstimateTransactionFeeCallback extends BRCryptoClientEstimateTransactionFeeCallback {
        void handle(Cookie context,
                    BRCryptoWalletManager manager,
                    BRCryptoClientCallbackState callbackState,
                    byte[] transaction);

        @Override
        default void callback(Pointer context,
                              Pointer manager,
                              Pointer callbackState,
                              Pointer tx,
                              SizeT txLength) {
            handle(
                    new Cookie(context),
                    new BRCryptoWalletManager(manager),
                    new BRCryptoClientCallbackState(callbackState),
                    tx.getByteArray(0, UnsignedInts.checkedCast(txLength.longValue()))
            );
        }
    }

    //
    // Client Struct
    //

    public Pointer context;

    public BRCryptoClientGetBlockNumberCallback funcGetBlockNumber;
    public BRCryptoClientGetTransactionsCallback funcGetTransactions;
    public BRCryptoClientGetTransfersCallback funcGetTransfers;
    public BRCryptoClientSubmitTransactionCallback funcSubmitTransaction;
    public BRCryptoClientEstimateTransactionFeeCallback funcEstimateTransactionFee;

    public BRCryptoClient() {
        super();
    }

    public BRCryptoClient(Pointer pointer) {
        super(pointer);
    }

    public BRCryptoClient(Cookie context,
                          GetBlockNumberCallback funcGetBlockNumber,
                          GetTransactionsCallback funcGetTransactions,
                          GetTransfersCallback funcGetTransfers,
                          SubmitTransactionCallback funcSubmitTransaction,
                          EstimateTransactionFeeCallback funcEstimateTransactionFee) {
        super();
        this.context = context.getPointer();
        this.funcGetBlockNumber = funcGetBlockNumber;
        this.funcGetTransactions = funcGetTransactions;
        this.funcGetTransfers = funcGetTransfers;
        this.funcSubmitTransaction = funcSubmitTransaction;
        this.funcEstimateTransactionFee = funcEstimateTransactionFee;
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList(
                "context",
                "funcGetBlockNumber",
                "funcGetTransactions",
                "funcGetTransfers",
                "funcSubmitTransaction",
                "funcEstimateTransactionFee"
        );
    }

    public ByValue toByValue() {
        ByValue other = new ByValue();

        other.context = this.context;
        other.funcGetBlockNumber = this.funcGetBlockNumber;
        other.funcGetTransactions = this.funcGetTransactions;
        other.funcGetTransfers = this.funcGetTransfers;
        other.funcSubmitTransaction = this.funcSubmitTransaction;
        other.funcEstimateTransactionFee = this.funcEstimateTransactionFee;

        return other;
    }

    public static class ByReference extends BRCryptoClient implements Structure.ByReference {
    }

    public static class ByValue extends BRCryptoClient implements Structure.ByValue {
    }
}
