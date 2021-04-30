/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryIndirect;
import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.blockset.walletkit.nativex.utility.SizeT;
import com.blockset.walletkit.nativex.utility.SizeTByReference;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedInteger;
import com.google.common.primitives.UnsignedInts;
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

import java.util.ArrayList;
import java.util.List;

public class WKNetwork extends PointerType {

    public static Optional<WKNetwork> findBuiltin (String uids) {
        Pointer builtin = WKNativeLibraryDirect.wkNetworkFindBuiltin (uids, uids.endsWith("mainnet") ? 1 : 0);
        return (null == builtin
                ? Optional.absent()
                : Optional.of (new WKNetwork(builtin)));
    }

    public static List<WKNetwork> installBuiltins () {
        List<WKNetwork> builtins = new ArrayList<>();
        SizeTByReference count = new SizeTByReference();
        Pointer builtinsPtr = WKNativeLibraryDirect.wkNetworkInstallBuiltins(count);

        if (null != builtinsPtr) {
            try {
                int builtinsSize = UnsignedInts.checkedCast(count.getValue().longValue());
                for (Pointer builtinPtr : builtinsPtr.getPointerArray(0, builtinsSize)) {
                    builtins.add(new WKNetwork(builtinPtr));
                }

            } finally {
                Native.free(Pointer.nativeValue(builtinsPtr));
            }
        }
        return builtins;
    }

    public WKNetwork() {
        super();
    }

    public WKNetwork(Pointer address) {
        super(address);
    }

    public WKNetworkType getCanonicalType () {
        return WKNetworkType.fromCore(
                WKNativeLibraryDirect.wkNetworkGetType(
                        this.getPointer())
        );
    }

    public WKCurrency getCurrency() {
        Pointer thisPtr = this.getPointer();

        return new WKCurrency(
                WKNativeLibraryDirect.wkNetworkGetCurrency(
                        thisPtr
                )
        );
    }
    public boolean hasCurrency(WKCurrency currency) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkNetworkHasCurrency(thisPtr, currency.getPointer());
    }

    public UnsignedLong getCurrencyCount() {
        Pointer thisPtr = this.getPointer();

        return UnsignedLong.fromLongBits(WKNativeLibraryDirect.wkNetworkGetCurrencyCount(thisPtr).longValue());
    }

    public WKCurrency getCurrency(UnsignedLong index) {
        Pointer thisPtr = this.getPointer();

        return new WKCurrency(
                WKNativeLibraryDirect.wkNetworkGetCurrencyAt(
                        thisPtr,
                        new SizeT(index.longValue())
                )
        );
    }

    public List<WKNetworkFee> getFees() {
        Pointer thisPtr = this.getPointer();

        List<WKNetworkFee> fees = new ArrayList<>();
        SizeTByReference count = new SizeTByReference();
        Pointer feesPtr = WKNativeLibraryDirect.wkNetworkGetNetworkFees(thisPtr, count);
        if (null != feesPtr) {
            try {
                int feesSize = UnsignedInts.checkedCast(count.getValue().longValue());
                for (Pointer feePtr: feesPtr.getPointerArray(0, feesSize)) {
                    fees.add(new WKNetworkFee(feePtr));
                }

            } finally {
                Native.free(Pointer.nativeValue(feesPtr));
            }
        }
        return fees;
    }

    public void setFees(List<WKNetworkFee> fees) {
        Pointer thisPtr = this.getPointer();

        WKNetworkFee[] cryptoFees = new WKNetworkFee[fees.size()];
        for (int i = 0; i < fees.size(); i++) cryptoFees[i] = fees.get(i);

        WKNativeLibraryIndirect.wkNetworkSetNetworkFees(thisPtr, cryptoFees, new SizeT(cryptoFees.length));
    }

    public String getUids() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkNetworkGetUids(thisPtr).getString(0, "UTF-8");
    }

    public boolean isMainnet() {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkNetworkIsMainnet(thisPtr);
    }

    public UnsignedLong getHeight() {
        Pointer thisPtr = this.getPointer();

        return UnsignedLong.fromLongBits(WKNativeLibraryDirect.wkNetworkGetHeight(thisPtr));
    }

    public void setHeight(UnsignedLong height) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkNetworkSetHeight(thisPtr, height.longValue());
    }

    public Optional<WKHash> getVerifiedBlockHash() {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkNetworkGetVerifiedBlockHash(
                        this.getPointer())
        ).transform(WKHash::new);
    }

    public void setVerifiedBlockHashAsString(String hash) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkNetworkSetVerifiedBlockHashAsString(thisPtr, hash);
    }

    public UnsignedInteger getConfirmationsUntilFinal() {
        Pointer thisPtr = this.getPointer();

        return UnsignedInteger.fromIntBits(WKNativeLibraryDirect.wkNetworkGetConfirmationsUntilFinal(thisPtr));
    }

    public void setConfirmationsUntilFinal(UnsignedInteger confirmationsUntilFinal) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkNetworkSetConfirmationsUntilFinal(thisPtr, confirmationsUntilFinal.intValue());
    }

    public String getName() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkNetworkGetName(thisPtr).getString(0, "UTF-8");
    }

    public void addFee(WKNetworkFee networkFee) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkNetworkAddNetworkFee(
                thisPtr,
                networkFee.getPointer()
        );
    }

    public void addCurrency(WKCurrency currency, WKUnit baseUnit, WKUnit defaultUnit) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkNetworkAddCurrency(
                thisPtr,
                currency.getPointer(),
                baseUnit.getPointer(),
                defaultUnit.getPointer()
        );
    }

    public void addCurrencyUnit(WKCurrency currency, WKUnit unit) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkNetworkAddCurrencyUnit(
                thisPtr,
                currency.getPointer(),
                unit.getPointer()
        );
    }

    public Optional<WKUnit> getUnitAsBase(WKCurrency currency) {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkNetworkGetUnitAsBase(
                        thisPtr,
                        currency.getPointer()
                )
        ).transform(WKUnit::new);
    }

    public Optional<WKUnit> getUnitAsDefault(WKCurrency currency) {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkNetworkGetUnitAsDefault(
                        thisPtr,
                        currency.getPointer()
                )
        ).transform(WKUnit::new);
    }

    public UnsignedLong getUnitCount(WKCurrency currency) {
        Pointer thisPtr = this.getPointer();

        return UnsignedLong.fromLongBits(
                WKNativeLibraryDirect.wkNetworkGetUnitCount(
                        thisPtr,
                        currency.getPointer()
                ).longValue()
        );
    }

    public Optional<WKUnit> getUnitAt(WKCurrency currency, UnsignedLong index) {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkNetworkGetUnitAt(
                        thisPtr,
                        currency.getPointer(),
                        new SizeT(index.longValue())
                )
        ).transform(WKUnit::new);
    }

    public WKAddressScheme getDefaultAddressScheme() {
        return WKAddressScheme.fromCore(
                WKNativeLibraryDirect.wkNetworkGetDefaultAddressScheme(
                        this.getPointer())
        );
    }

    public List<WKAddressScheme> getSupportedAddressSchemes() {
        Pointer thisPtr = this.getPointer();

        List<WKAddressScheme> schemes = new ArrayList<>();
        SizeTByReference count = new SizeTByReference();
        Pointer schemesPtr = WKNativeLibraryDirect.wkNetworkGetSupportedAddressSchemes(thisPtr, count);
        if (null != schemesPtr) {
            try {
                int schemesSize = UnsignedInts.checkedCast(count.getValue().longValue());
                for (int schemeInt: schemesPtr.getIntArray(0, schemesSize)) {
                    schemes.add(WKAddressScheme.fromCore(schemeInt));
                }
            } finally {
                Native.free(Pointer.nativeValue(schemesPtr));
            }
        }
        return schemes;
    }

    public boolean supportsAddressScheme(WKAddressScheme addressScheme) {
        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkNetworkSupportsAddressScheme (
                this.getPointer(),
                addressScheme.toCore()
        );
    }

    public WKSyncMode getDefaultSyncMode() {
        return WKSyncMode.fromCore(
                WKNativeLibraryDirect.wkNetworkGetDefaultSyncMode(
                        this.getPointer())
        );
    }

    public List<WKSyncMode> getSupportedSyncModes() {
       Pointer thisPtr = this.getPointer();

        List<WKSyncMode> modes = new ArrayList<>();
        SizeTByReference count = new SizeTByReference();
        Pointer modesPtr = WKNativeLibraryDirect.wkNetworkGetSupportedSyncModes(thisPtr, count);
        if (null != modesPtr) {
            try {
                int modesSize = UnsignedInts.checkedCast(count.getValue().longValue());
                for (int modeInt: modesPtr.getIntArray(0, modesSize)) {
                    modes.add(WKSyncMode.fromCore(modeInt));
                }
            } finally {
                Native.free(Pointer.nativeValue(modesPtr));
            }
        }
        return modes;
    }

    public boolean supportsSyncMode(WKSyncMode mode) {
        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkNetworkSupportsSyncMode(
                this.getPointer(),
                mode.toCore()
        );
    }

    public boolean requiresMigration () {
        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkNetworkRequiresMigration(
                this.getPointer()
        );
    }

    public WKNetwork take() {
        Pointer thisPtr = this.getPointer();

        return new WKNetwork(WKNativeLibraryDirect.wkNetworkTake(thisPtr));
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkNetworkGive(thisPtr);
    }
}
