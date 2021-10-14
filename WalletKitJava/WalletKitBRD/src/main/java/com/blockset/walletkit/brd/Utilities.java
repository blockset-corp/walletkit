/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import static com.blockset.walletkit.TransferSubmitError.Type.CLIENT_BAD_REQUEST;
import static com.blockset.walletkit.TransferSubmitError.Type.CLIENT_BAD_RESPONSE;
import static com.blockset.walletkit.TransferSubmitError.Type.CLIENT_PERMISSION;
import static com.blockset.walletkit.TransferSubmitError.Type.CLIENT_RESOURCE;
import static com.blockset.walletkit.TransferSubmitError.Type.CLIENT_UNAVAILABLE;
import static com.blockset.walletkit.TransferSubmitError.Type.DUPLICATE;
import static com.blockset.walletkit.TransferSubmitError.Type.INSUFFICIENT_BALANCE;
import static com.blockset.walletkit.TransferSubmitError.Type.INSUFFICIENT_FEE;
import static com.blockset.walletkit.TransferSubmitError.Type.INSUFFICIENT_NETWORK_COST_UNIT;
import static com.blockset.walletkit.TransferSubmitError.Type.INSUFFICIENT_NETWORK_FEE;
import static com.blockset.walletkit.TransferSubmitError.Type.NONCE_INVALID;
import static com.blockset.walletkit.TransferSubmitError.Type.NONCE_TOO_LOW;
import static com.blockset.walletkit.TransferSubmitError.Type.SIGNATURE;
import static com.blockset.walletkit.TransferSubmitError.Type.TRANSACTION;
import static com.blockset.walletkit.TransferSubmitError.Type.TRANSACTION_EXPIRED;
import static com.blockset.walletkit.TransferSubmitError.Type.UNKNOWN;

import com.blockset.walletkit.AddressScheme;
import com.blockset.walletkit.NetworkType;
import com.blockset.walletkit.PaymentProtocolRequestType;
import com.blockset.walletkit.SystemState;
import com.blockset.walletkit.TransferConfirmation;
import com.blockset.walletkit.TransferDirection;
import com.blockset.walletkit.TransferIncludeStatus;
import com.blockset.walletkit.TransferState;
import com.blockset.walletkit.TransferSubmitError;
import com.blockset.walletkit.WalletManagerDisconnectReason;
import com.blockset.walletkit.WalletManagerMode;
import com.blockset.walletkit.WalletManagerState;
import com.blockset.walletkit.WalletManagerSyncDepth;
import com.blockset.walletkit.WalletManagerSyncStoppedReason;
import com.blockset.walletkit.WalletState;
import com.blockset.walletkit.errors.FeeEstimationError;
import com.blockset.walletkit.errors.FeeEstimationServiceFailureError;
import com.blockset.walletkit.errors.FeeEstimationServiceUnavailableError;
import com.blockset.walletkit.errors.PaymentProtocolCertificateMissingError;
import com.blockset.walletkit.errors.PaymentProtocolCertificateNotTrustedError;
import com.blockset.walletkit.errors.PaymentProtocolError;
import com.blockset.walletkit.errors.PaymentProtocolRequestExpiredError;
import com.blockset.walletkit.errors.PaymentProtocolSignatureTypeUnsupportedError;
import com.blockset.walletkit.errors.PaymentProtocolSignatureVerificationFailedError;
import com.google.common.base.Optional;
import com.blockset.walletkit.errors.SystemClientError;
import com.blockset.walletkit.errors.SystemClientSubmitError;
import com.blockset.walletkit.nativex.WKAddressScheme;
import com.blockset.walletkit.nativex.WKClientError;
import com.blockset.walletkit.nativex.WKFeeBasis;
import com.blockset.walletkit.nativex.WKNetworkType;
import com.blockset.walletkit.nativex.WKPaymentProtocolError;
import com.blockset.walletkit.nativex.WKPaymentProtocolType;
import com.blockset.walletkit.nativex.WKStatus;
import com.blockset.walletkit.nativex.WKSyncDepth;
import com.blockset.walletkit.nativex.WKSyncMode;
import com.blockset.walletkit.nativex.WKSyncStoppedReason;
import com.blockset.walletkit.nativex.WKSystemState;
import com.blockset.walletkit.nativex.WKTransferAttributeValidationError;
import com.blockset.walletkit.nativex.WKTransferDirection;
import com.blockset.walletkit.nativex.WKTransferIncludeStatus;
import com.blockset.walletkit.nativex.WKTransferState;
import com.blockset.walletkit.nativex.WKTransferSubmitError;
import com.blockset.walletkit.nativex.WKWalletManagerState;
import com.blockset.walletkit.nativex.WKWalletState;
import com.google.common.primitives.UnsignedLong;

import java.util.Date;
import java.util.concurrent.TimeUnit;

/* package */
final class Utilities {

    /* package */
    static SystemState systemStateFromCrypto (WKSystemState state) {
        switch (state) {
            case CREATED: return SystemState.CREATED();
            case DELETED: return SystemState.DELETED();
            default: throw new IllegalArgumentException("Unsupported state");
        }
    }

    /* package */
    static WKSyncMode walletManagerModeToCrypto(WalletManagerMode mode) {
        switch (mode) {
            case API_ONLY: return WKSyncMode.API_ONLY;
            case API_WITH_P2P_SUBMIT: return WKSyncMode.API_WITH_P2P_SEND;
            case P2P_ONLY: return WKSyncMode.P2P_ONLY;
            case P2P_WITH_API_SYNC: return WKSyncMode.P2P_WITH_API_SYNC;
            default: throw new IllegalArgumentException("Unsupported mode");
        }
    }

    /* package */
    static WalletManagerMode walletManagerModeFromCrypto(WKSyncMode mode) {
        switch (mode) {
            case API_ONLY: return WalletManagerMode.API_ONLY;
            case API_WITH_P2P_SEND: return WalletManagerMode.API_WITH_P2P_SUBMIT;
            case P2P_ONLY: return WalletManagerMode.P2P_ONLY;
            case P2P_WITH_API_SYNC: return WalletManagerMode.P2P_WITH_API_SYNC;
            default: throw new IllegalArgumentException("Unsupported mode");
        }
    }

    /* package */
    static WalletManagerState walletManagerStateFromCrypto(WKWalletManagerState state) {
        switch (state.type()) {
            case CREATED: return WalletManagerState.CREATED();
            case DELETED: return WalletManagerState.DELETED();
            case CONNECTED: return WalletManagerState.CONNECTED();
            case SYNCING: return WalletManagerState.SYNCING();
            case DISCONNECTED:
                switch (state.u.disconnected.reason.type()) {
                    case REQUESTED: return WalletManagerState.DISCONNECTED(
                            WalletManagerDisconnectReason.REQUESTED()
                    );
                    case UNKNOWN: return WalletManagerState.DISCONNECTED(
                            WalletManagerDisconnectReason.UNKNOWN()
                    );
                    case POSIX: return WalletManagerState.DISCONNECTED(
                            WalletManagerDisconnectReason.POSIX(
                                    state.u.disconnected.reason.u.posix.errnum,
                                    state.u.disconnected.reason.getMessage().orNull()
                            )
                    );
                    default: throw new IllegalArgumentException("Unsupported reason");
                }
            default: throw new IllegalArgumentException("Unsupported state");
        }
    }

    /* package */
    static WalletManagerSyncStoppedReason walletManagerSyncStoppedReasonFromCrypto(WKSyncStoppedReason reason) {
        switch (reason.type()) {
            case COMPLETE: return WalletManagerSyncStoppedReason.COMPLETE();
            case REQUESTED: return WalletManagerSyncStoppedReason.REQUESTED();
            case UNKNOWN: return WalletManagerSyncStoppedReason.UNKNOWN();
            case POSIX: return WalletManagerSyncStoppedReason.POSIX(
                    reason.u.posix.errnum,
                    reason.getMessage().orNull()
            );
            default: throw new IllegalArgumentException("Unsupported reason");
        }
    }

    /* package */
    static WalletState walletStateFromCrypto(WKWalletState state) {
        switch (state) {
            case CREATED: return WalletState.CREATED;
            case DELETED: return WalletState.DELETED;
            default: throw new IllegalArgumentException("Unsupported state");
        }
    }

    /* package */
    static TransferDirection transferDirectionFromCrypto(WKTransferDirection direction) {
        switch (direction) {
            case RECEIVED: return TransferDirection.RECEIVED;
            case SENT: return TransferDirection.SENT;
            case RECOVERED: return TransferDirection.RECOVERED;
            default: throw new IllegalArgumentException("Unsupported direction");
        }
    }

    static TransferIncludeStatus.Type transferIncludeStatusTypeFromCrypto (WKTransferIncludeStatus.Type type) {
        switch (type) {
            case SUCCESS:          return TransferIncludeStatus.Type.SUCCESS;
            case FAILURE_INSUFFICIENT_NETWORK_COST_UNIT: return TransferIncludeStatus.Type.INSUFFICIENT_NETWORK_CORE_UNIT;
            case FAILURE_REVERTED: return TransferIncludeStatus.Type.REVERTED;
            case FAILURE_UNKNOWN:  return TransferIncludeStatus.Type.UNKNOWN;
            default: throw new IllegalArgumentException("Unsupported type");
        }
    }

    static TransferSubmitError.Type transferSubmitErrorTypeFromCrypto (WKTransferSubmitError.Type type) {
        switch (type) {
            case ACCOUNT:              return TransferSubmitError.Type.ACCOUNT;
            case SIGNATURE:            return TransferSubmitError.Type.SIGNATURE;
            case DUPLICATE:            return TransferSubmitError.Type.DUPLICATE;
            case INSUFFICIENT_BALANCE: return TransferSubmitError.Type.INSUFFICIENT_BALANCE;
            case INSUFFICIENT_NETWORK_FEE:       return TransferSubmitError.Type.INSUFFICIENT_NETWORK_FEE;
            case INSUFFICIENT_NETWORK_COST_UNIT: return TransferSubmitError.Type.INSUFFICIENT_NETWORK_COST_UNIT;
            case INSUFFICIENT_FEE:     return TransferSubmitError.Type.INSUFFICIENT_FEE;
            case NONCE_TOO_LOW:        return TransferSubmitError.Type.NONCE_TOO_LOW;
            case NONCE_INVALID:        return TransferSubmitError.Type.NONCE_INVALID;
            case TRANSACTION_EXPIRED:  return TransferSubmitError.Type.TRANSACTION_EXPIRED;
            case TRANSACTION:          return TransferSubmitError.Type.TRANSACTION;
            case UNKNOWN:              return TransferSubmitError.Type.UNKNOWN;
            case CLIENT_BAD_REQUEST:   return TransferSubmitError.Type.CLIENT_BAD_REQUEST;
            case CLIENT_PERMISSION:    return TransferSubmitError.Type.CLIENT_PERMISSION;
            case CLIENT_RESOURCE:      return TransferSubmitError.Type.CLIENT_RESOURCE;
            case CLIENT_BAD_RESPONSE:  return TransferSubmitError.Type.CLIENT_BAD_RESPONSE;
            case CLIENT_UNAVAILABLE:   return TransferSubmitError.Type.CLIENT_UNAVAILABLE;
            default: throw new IllegalArgumentException("Unsupported type");
        }
    }
    /* package */
    static TransferState transferStateFromCrypto(WKTransferState state) {
        switch (state.type()) {
            case CREATED:   return TransferState.CREATED();
            case DELETED:   return TransferState.DELETED();
            case SIGNED:    return TransferState.SIGNED();
            case SUBMITTED: return TransferState.SUBMITTED();
            case INCLUDED: {
                WKTransferState.Included included = state.included();
                WKTransferIncludeStatus status = included.status;

                return TransferState.INCLUDED(
                    new TransferConfirmation(
                            included.blockNumber,
                            included.transactionIndex,
                            included.blockTimestamp,
                            Optional.fromNullable(included.feeBasis)
                                    .transform(WKFeeBasis::take)
                                    .transform(TransferFeeBasis::create)
                                    .transform(TransferFeeBasis::getFee),
                            new TransferIncludeStatus (transferIncludeStatusTypeFromCrypto (status.getType()), status.getDetails())));
            }
            case ERRORED: {
                WKTransferSubmitError error = state.errored();
                return TransferState.FAILED(
                        new TransferSubmitError(
                                transferSubmitErrorTypeFromCrypto(error.getType()),
                                error.getDetails()));
            }
            default: throw new IllegalArgumentException("Unsupported state");
        }
    }

    static TransferAttribute.Error transferAttributeErrorFromCrypto(WKTransferAttributeValidationError error) {
        switch (error) {
            case REQUIRED_BUT_NOT_PROVIDED:
                return TransferAttribute.Error.REQUIRED_BUT_NOT_PROVIDED;
            case MISMATCHED_TYPE:
                return TransferAttribute.Error.MISMATCHED_TYPE;
            case RELATIONSHIP_INCONSISTENCY:
                return TransferAttribute.Error.RELATIONSHIP_INCONSISTENCY;
            default: throw new IllegalArgumentException(("Unsupported TransferAttribute.Error"));
            }
    }

    private static final SystemClientError.Visitor<WKClientError> systemClientErrorVisitor =
            new SystemClientError.Visitor<WKClientError>() {

                @Override
                public WKClientError visit(SystemClientError.BadRequest error) {
                    return new WKClientError(WKClientError.Type.BAD_REQUEST, error.details);
                }

                @Override
                public WKClientError visit(SystemClientError.Permission error) {
                    return new WKClientError(WKClientError.Type.PERMISSION, null);
                }

                @Override
                public WKClientError visit(SystemClientError.Resource error) {
                    return new WKClientError(WKClientError.Type.RESOURCE, null);
                }

                @Override
                public WKClientError visit(SystemClientError.Submission error) {
                    return systemClientSubmitErrorToCrypto(error.error);
                }

                @Override
                public WKClientError visit(SystemClientError.BadResponse error) {
                    return new WKClientError(WKClientError.Type.BAD_RESPONSE, error.details);
                }

                @Override
                public WKClientError visit(SystemClientError.Unavailable error) {
                    return new WKClientError(WKClientError.Type.UNAVAILABLE, null);
                }
            };

    static WKClientError systemClientErrorToCrypto(SystemClientError error) {
        return error.accept(systemClientErrorVisitor);
    }

    private static final SystemClientSubmitError.Visitor<WKClientError> systemClientSubmitErrorVisitor =
            new SystemClientSubmitError.Visitor<WKClientError>() {
                @Override
                public WKClientError visit(SystemClientSubmitError.Access error) {
                    return new WKClientError(WKTransferSubmitError.Type.UNKNOWN, error.details);
                }

                @Override
                public WKClientError visit(SystemClientSubmitError.Account error) {
                    return new WKClientError(WKTransferSubmitError.Type.ACCOUNT, error.details);
                }

                @Override
                public WKClientError visit(SystemClientSubmitError.Signature error) {
                    return new WKClientError(WKTransferSubmitError.Type.SIGNATURE, error.details);
                }

                @Override
                public WKClientError visit(SystemClientSubmitError.InsufficientBalance error) {
                    return new WKClientError(WKTransferSubmitError.Type.INSUFFICIENT_BALANCE, error.details);
                }

                @Override
                public WKClientError visit(SystemClientSubmitError.InsufficientNetworkFee error) {
                    return new WKClientError(WKTransferSubmitError.Type.INSUFFICIENT_NETWORK_FEE, error.details);
                }

                @Override
                public WKClientError visit(SystemClientSubmitError.InsufficientNetworkCostUnit error) {
                    return new WKClientError(WKTransferSubmitError.Type.INSUFFICIENT_NETWORK_COST_UNIT, error.details);
                }

                @Override
                public WKClientError visit(SystemClientSubmitError.InsufficientFee error) {
                    return new WKClientError(WKTransferSubmitError.Type.INSUFFICIENT_FEE, error.details);
                }

                @Override
                public WKClientError visit(SystemClientSubmitError.NonceTooLow error) {
                    return new WKClientError(WKTransferSubmitError.Type.NONCE_TOO_LOW, error.details);
                }

                @Override
                public WKClientError visit(SystemClientSubmitError.NonceInvalid error) {
                    return new WKClientError(WKTransferSubmitError.Type.NONCE_INVALID, error.details);
                }

                @Override
                public WKClientError visit(SystemClientSubmitError.TransactionDuplicate error) {
                    return new WKClientError(WKTransferSubmitError.Type.DUPLICATE, error.details);
                }

                @Override
                public WKClientError visit(SystemClientSubmitError.TransactionExpired error) {
                    return new WKClientError(WKTransferSubmitError.Type.TRANSACTION_EXPIRED, error.details);
                }

                @Override
                public WKClientError visit(SystemClientSubmitError.Transaction error) {
                    return new WKClientError(WKTransferSubmitError.Type.TRANSACTION, error.details);
                }

                @Override
                public WKClientError visit(SystemClientSubmitError.Unknown error) {
                    return new WKClientError(WKTransferSubmitError.Type.UNKNOWN, error.details);
                }
            };

    static WKClientError systemClientSubmitErrorToCrypto (SystemClientSubmitError error) {
        return error.accept(systemClientSubmitErrorVisitor);
    }

    /* package */
    static WKNetworkType networkTypeToCrypto(NetworkType type) {
        switch (type) {
            case BTC: return WKNetworkType.BTC;
            case BCH: return WKNetworkType.BCH;
            case BSV: return WKNetworkType.BSV;
            case LTC: return WKNetworkType.LTC;
            case DOGE:return WKNetworkType.DOGE;
            case ETH: return WKNetworkType.ETH;
            case XRP: return WKNetworkType.XRP;
            case HBAR:return WKNetworkType.HBAR;
            case XTZ: return WKNetworkType.XTZ;
            case XLM: return WKNetworkType.XLM;
            /* case __SYMBOL__: return WKNetworkType.__SYMBOL__; */
            default: throw new IllegalArgumentException("Unsupported type");
        }
    }

    /* package */
    static NetworkType networkTypeFromCrypto(WKNetworkType type) {
        switch (type) {
            case BTC: return NetworkType.BTC;
            case BCH: return NetworkType.BCH;
            case BSV: return NetworkType.BSV;
            case LTC: return NetworkType.LTC;
            case DOGE:return NetworkType.DOGE;
            case ETH: return NetworkType.ETH;
            case XRP: return NetworkType.XRP;
            case HBAR:return NetworkType.HBAR;
            case XTZ: return NetworkType.XTZ;
            case XLM: return NetworkType.XLM;
            /* case __SYMBOL__: return NetworkType.__SYMBOL__; */
            default: throw new IllegalArgumentException("Unsupported type");
        }
    }

    /* package */
    static WKAddressScheme addressSchemeToCrypto(AddressScheme scheme) {
        switch (scheme) {
            case BTC_LEGACY: return WKAddressScheme.CRYPTO_ADDRESS_SCHEME_BTC_LEGACY;
            case BTC_SEGWIT: return WKAddressScheme.CRYPTO_ADDRESS_SCHEME_BTC_SEGWIT;
            case NATIVE:     return WKAddressScheme.CRYPTO_ADDRESS_SCHEME_NATIVE;
            default: throw new IllegalArgumentException("Unsupported scheme");
        }
    }

    /* package */
    static AddressScheme addressSchemeFromCrypto(WKAddressScheme scheme) {
        switch (scheme) {
            case CRYPTO_ADDRESS_SCHEME_BTC_LEGACY: return AddressScheme.BTC_LEGACY;
            case CRYPTO_ADDRESS_SCHEME_BTC_SEGWIT: return AddressScheme.BTC_SEGWIT;
            case CRYPTO_ADDRESS_SCHEME_NATIVE:     return AddressScheme.NATIVE;
            default: throw new IllegalArgumentException("Unsupported scheme");
        }
    }

    /* package */
    static FeeEstimationError feeEstimationErrorFromStatus(WKStatus status) {
        if (status == WKStatus.NODE_NOT_CONNECTED) {
            return new FeeEstimationServiceUnavailableError();
        }
        return new FeeEstimationServiceFailureError();
    }

    /* package */
    static Optional<PaymentProtocolError> paymentProtocolErrorFromCrypto(WKPaymentProtocolError error) {
        switch (error) {
            case NONE: return Optional.absent();
            case CERT_MISSING: return Optional.of(new PaymentProtocolCertificateMissingError());
            case CERT_NOT_TRUSTED: return Optional.of(new PaymentProtocolCertificateNotTrustedError());
            case EXPIRED: return Optional.of(new PaymentProtocolRequestExpiredError());
            case SIGNATURE_TYPE_NOT_SUPPORTED: return Optional.of(new PaymentProtocolSignatureTypeUnsupportedError());
            case SIGNATURE_VERIFICATION_FAILED: return Optional.of(new PaymentProtocolSignatureVerificationFailedError());
            default: throw new IllegalArgumentException("Unsupported error");
        }
    }

    /* package */
    static PaymentProtocolRequestType paymentProtocolRequestTypeFromCrypto(WKPaymentProtocolType type) {
        switch (type) {
            case BIP70: return PaymentProtocolRequestType.BIP70;
            case BITPAY: return PaymentProtocolRequestType.BITPAY;
            default: throw new IllegalArgumentException("Unsupported type");
        }
    }

    /* package */
    static WKSyncDepth syncDepthToCrypto(WalletManagerSyncDepth depth) {
        switch (depth) {
            case FROM_LAST_CONFIRMED_SEND: return WKSyncDepth.FROM_LAST_CONFIRMED_SEND;
            case FROM_LAST_TRUSTED_BLOCK:  return WKSyncDepth.FROM_LAST_TRUSTED_BLOCK;
            case FROM_CREATION:            return WKSyncDepth.FROM_CREATION;
            default: throw new IllegalArgumentException("Unsupported depth");
        }
    }

    /* package */
    static WalletManagerSyncDepth syncDepthFromCrypto(WKSyncDepth depth) {
        switch (depth) {
            case FROM_LAST_CONFIRMED_SEND: return WalletManagerSyncDepth.FROM_LAST_CONFIRMED_SEND;
            case FROM_LAST_TRUSTED_BLOCK:  return WalletManagerSyncDepth.FROM_LAST_TRUSTED_BLOCK;
            case FROM_CREATION:            return WalletManagerSyncDepth.FROM_CREATION;
            default: throw new IllegalArgumentException("Unsupported depth");
        }
    }

    /* package */
    static UnsignedLong dateAsUnixTimestamp(Date date) {
        long timestamp = TimeUnit.MILLISECONDS.toSeconds(date.getTime());
        return timestamp > 0 ? UnsignedLong.valueOf(timestamp) : UnsignedLong.ZERO;
    }
}
