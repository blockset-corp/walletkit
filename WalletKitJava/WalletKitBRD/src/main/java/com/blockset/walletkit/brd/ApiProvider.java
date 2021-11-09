/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import com.blockset.walletkit.Api;
import com.blockset.walletkit.Network;
import com.blockset.walletkit.PaymentProtocolRequestType;
import com.blockset.walletkit.Unit;
import com.blockset.walletkit.Wallet;
import com.blockset.walletkit.SystemClient;
import com.blockset.walletkit.events.system.SystemListener;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedInteger;

import java.util.Date;
import java.util.List;
import java.util.concurrent.ScheduledExecutorService;

public final class ApiProvider implements Api.Provider {

    private static final ApiProvider INSTANCE = new ApiProvider();

    public static ApiProvider getInstance() {
        return INSTANCE;
    }

    private static final Api.AccountProvider accountProvider = new Api.AccountProvider() {
        @Override
        public Optional<com.blockset.walletkit.Account> createFromPhrase(byte[] phraseUtf8, Date timestamp, String uids, boolean isMainnet) {
            return Account.createFromPhrase(phraseUtf8, timestamp, uids, isMainnet).transform(a -> a);
        }

        @Override
        public Optional<com.blockset.walletkit.Account> createFromSerialization(byte[] serialization, String uids) {
            return Account.createFromSerialization(serialization, uids).transform(a -> a);
        }

        @Override
        public byte[] generatePhrase(List<String> words) {
            return Account.generatePhrase(words);
        }

        @Override
        public boolean validatePhrase(byte[] phraseUtf8, List<String> words) {
            return Account.validatePhrase(phraseUtf8, words);
        }
    };

    private static final Api.AddressProvider addressProvider = new Api.AddressProvider() {
        @Override
        public Optional<com.blockset.walletkit.Address> create(String address, Network network) {
            return Address.create(address, network).transform(a -> a);
        }
    };

    private static final Api.AmountProvider amountProvider = new Api.AmountProvider() {
        @Override
        public com.blockset.walletkit.Amount create(long value, Unit unit) {
            return Amount.create(value, unit);
        }

        @Override
        public com.blockset.walletkit.Amount create(double value, Unit unit) {
            return Amount.create(value, unit);
        }

        @Override
        public Optional<com.blockset.walletkit.Amount> create(String value, boolean isNegative, Unit unit) {
            return Amount.create(value, isNegative, unit).transform(a -> a);
        }
    };

    private static final Api.SystemProvider systemProvider = new Api.SystemProvider() {
        @Override
        public com.blockset.walletkit.System create(ScheduledExecutorService executor,
                                                    SystemListener listener,
                                                    com.blockset.walletkit.Account account,
                                                    boolean isMainnet,
                                                    String path,
                                                    SystemClient query) {
            return System.create(executor, listener, account, isMainnet, path, query);
        }

        @Override
        public Optional<SystemClient.Currency> asBDBCurrency(String uids, String name, String code, String type, UnsignedInteger decimals) {
            return System.asBDBCurrency(uids, name, code, type, decimals);
        }

        @Override
        public Optional<byte[]> migrateBRCoreKeyCiphertext(com.blockset.walletkit.Key key, byte[] nonce12,
                                                           byte[] authenticatedData, byte[] ciphertext) {
            return System.migrateBRCoreKeyCiphertext(key, nonce12, authenticatedData, ciphertext);
        }

        @Override
        public void wipe(com.blockset.walletkit.System system) {
            System.wipe(system);
        }

        @Override
        public void wipeAll(String path, List<com.blockset.walletkit.System> exemptSystems) {
            System.wipeAll(path, exemptSystems);
        }
    };

    private static final Api.PaymentProvider paymentProvider = new Api.PaymentProvider() {

        @Override
        public Optional<com.blockset.walletkit.PaymentProtocolRequest> createRequestForBip70(Wallet wallet, byte[] serialization) {
            return PaymentProtocolRequest.createForBip70(wallet, serialization).transform(r -> r);
        }

        @Override
        public boolean checkPaymentMethodSupported(Wallet                       wallet,
                                                   PaymentProtocolRequestType   protocolType) {
            return PaymentProtocolRequest.checkPaymentMethodSupported(wallet, protocolType);
        }

        @Override
        public Optional<com.blockset.walletkit.PaymentProtocolRequest> createRequestForBitPay(Wallet wallet, String json) {
            return PaymentProtocolRequest.createForBitPay(wallet, json).transform(r -> r);
        }

        @Override
        public Optional<com.blockset.walletkit.PaymentProtocolPaymentAck> createAckForBip70(byte[] serialization) {
            return PaymentProtocolPaymentAck.createForBip70(serialization).transform(t -> t);
        }

        @Override
        public Optional<com.blockset.walletkit.PaymentProtocolPaymentAck> createAckForBitPay(String json) {
            return PaymentProtocolPaymentAck.createForBitPay(json).transform(t -> t);
        }
    };

    private static final Api.CoderProvider coderProvider = new Api.CoderProvider() {
        @Override
        public Coder createCoderForAlgorithm(Coder.Algorithm algorithm) {
            return Coder.createForAlgorithm(algorithm);
        }
    };

    private static final Api.CipherProvider cipherProvider = new Api.CipherProvider() {
        @Override
        public Cipher createCipherForAesEcb(byte[] key) {
            return Cipher.createForAesEcb(key);
        }

        @Override
        public Cipher createCipherForChaCha20Poly1305(com.blockset.walletkit.Key key, byte[] nonce12, byte[] ad) {
            return Cipher.createForChaCha20Poly1305(key, nonce12, ad);
        }

        @Override
        public Cipher createCipherForPigeon(com.blockset.walletkit.Key privKey,
                                            com.blockset.walletkit.Key pubKey, byte[] nonce12) {
            return Cipher.createForPigeon(privKey, pubKey, nonce12);
        }
    };

    private static final Api.HasherProvider hasherProvider = new Api.HasherProvider() {
        @Override
        public Hasher createHasherForAlgorithm(Hasher.Algorithm algorithm) {
            return Hasher.createForAlgorithm(algorithm);
        }
    };

    private static final Api.KeyProvider keyProvider = new Api.KeyProvider() {
        @Override
        public void setDefaultWordList(List<String> wordList) {
            Key.setDefaultWordList(wordList);
        }

        @Override
        public List<String> getDefaultWordList() {
            return Key.getDefaultWordList();
        }

        @Override
        public boolean isProtectedPrivateKeyString(byte[] keyStringUtf8) {
            return Key.isProtectedPrivateKeyString(keyStringUtf8);
        }

        @Override
        public Optional<com.blockset.walletkit.Key> createFromPhrase(byte[] phraseUtf8, List<String> words) {
            return Key.createFromPhrase(phraseUtf8, words).transform(a -> a);
        }

        @Override
        public Optional<com.blockset.walletkit.Key> createFromPrivateKeyString(byte[] keyStringUtf8) {
            return Key.createFromPrivateKeyString(keyStringUtf8).transform(a -> a);
        }

        @Override
        public Optional<com.blockset.walletkit.Key> createFromPrivateKeyString(byte[] keyStringUtf8, byte[] passphraseUtf8) {
            return Key.createFromPrivateKeyString(keyStringUtf8, passphraseUtf8).transform(a -> a);
        }

        @Override
        public Optional<com.blockset.walletkit.Key> createFromPublicKeyString(byte[] keyStringUtf8) {
            return Key.createFromPublicKeyString(keyStringUtf8).transform(a -> a);
        }

        public Optional<com.blockset.walletkit.Key> createForPigeon(com.blockset.walletkit.Key key, byte[] nonce) {
            return Key.createForPigeon(key, nonce).transform(a -> a);
        }

        @Override
        public Optional<com.blockset.walletkit.Key> createForBIP32ApiAuth(byte[] phraseUtf8, List<String> words) {
            return Key.createForBIP32ApiAuth(phraseUtf8, words).transform(a -> a);
        }

        @Override
        public Optional<com.blockset.walletkit.Key> createForBIP32BitID(byte[] phraseUtf8, int index, String uri, List<String> words) {
            return Key.createForBIP32BitID(phraseUtf8, index, uri, words).transform(a -> a);
        }
    };

    private static final Api.SignerProvider signerProvider = new Api.SignerProvider() {
        @Override
        public Signer createSignerForAlgorithm(Signer.Algorithm algorithm) {
            return Signer.createForAlgorithm(algorithm);
        }
    };

    private ApiProvider() {

    }

    @Override
    public Api.AccountProvider accountProvider() {
        return accountProvider;
    }

    @Override
    public Api.AddressProvider addressProvider() {
        return addressProvider;
    }

    @Override
    public Api.AmountProvider amountProvider() {
        return amountProvider;
    }

    @Override
    public Api.SystemProvider systemProvider() {
        return systemProvider;
    }

    @Override
    public Api.PaymentProvider paymentProvider() {
        return paymentProvider;
    }

    @Override
    public Api.CoderProvider coderPrivider() {
        return coderProvider;
    }

    @Override
    public Api.CipherProvider cipherProvider() {
        return cipherProvider;
    }

    @Override
    public Api.HasherProvider hasherProvider() {
        return hasherProvider;
    }

    @Override
    public Api.KeyProvider keyProvider() {
        return keyProvider;
    }

    @Override
    public Api.SignerProvider signerProvider() {
        return signerProvider;
    }
}
