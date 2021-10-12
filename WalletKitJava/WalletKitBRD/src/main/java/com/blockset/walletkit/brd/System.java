/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import com.blockset.walletkit.Key;
import com.blockset.walletkit.brd.systemclient.BlocksetTransaction;
import com.blockset.walletkit.nativex.WKFeeBasis;
import com.blockset.walletkit.nativex.cleaner.ReferenceCleaner;
import com.blockset.walletkit.nativex.WKClient;
import com.blockset.walletkit.nativex.WKClientCallbackState;
import com.blockset.walletkit.nativex.WKClientCurrencyBundle;
import com.blockset.walletkit.nativex.WKClientCurrencyDenominationBundle;
import com.blockset.walletkit.nativex.WKClientTransactionBundle;
import com.blockset.walletkit.nativex.WKClientTransferBundle;
import com.blockset.walletkit.nativex.WKCurrency;
import com.blockset.walletkit.nativex.WKListener;
import com.blockset.walletkit.nativex.WKNetwork;
import com.blockset.walletkit.nativex.WKNetworkEvent;
import com.blockset.walletkit.nativex.WKStatus;
import com.blockset.walletkit.nativex.WKSystem;
import com.blockset.walletkit.nativex.WKSystemEvent;
import com.blockset.walletkit.nativex.WKTransfer;
import com.blockset.walletkit.nativex.WKTransferEvent;
import com.blockset.walletkit.nativex.WKTransferStateType;
import com.blockset.walletkit.nativex.WKWallet;
import com.blockset.walletkit.nativex.WKWalletEvent;
import com.blockset.walletkit.nativex.WKWalletManager;
import com.blockset.walletkit.nativex.WKWalletManagerEvent;
import com.blockset.walletkit.nativex.support.WKConstants;
import com.blockset.walletkit.nativex.utility.Cookie;
import com.blockset.walletkit.AddressScheme;
import com.blockset.walletkit.NetworkType;
import com.blockset.walletkit.SystemState;
import com.blockset.walletkit.TransferState;
import com.blockset.walletkit.WalletManagerMode;
import com.blockset.walletkit.WalletManagerState;
import com.blockset.walletkit.WalletManagerSyncDepth;
import com.blockset.walletkit.WalletManagerSyncStoppedReason;
import com.blockset.walletkit.WalletState;
import com.blockset.walletkit.SystemClient;
import com.blockset.walletkit.errors.QueryError;
import com.blockset.walletkit.errors.QueryNoDataError;
import com.blockset.walletkit.SystemClient.Blockchain;
import com.blockset.walletkit.SystemClient.BlockchainFee;
import com.blockset.walletkit.SystemClient.CurrencyDenomination;
import com.blockset.walletkit.SystemClient.HederaAccount;
import com.blockset.walletkit.SystemClient.Transaction;
import com.blockset.walletkit.SystemClient.TransactionFee;
import com.blockset.walletkit.SystemClient.TransactionIdentifier;
import com.blockset.walletkit.errors.AccountInitializationAlreadyInitializedError;
import com.blockset.walletkit.errors.AccountInitializationCantCreateError;
import com.blockset.walletkit.errors.AccountInitializationError;
import com.blockset.walletkit.errors.AccountInitializationMultipleHederaAccountsError;
import com.blockset.walletkit.errors.AccountInitializationQueryError;
import com.blockset.walletkit.errors.CurrencyUpdateCurrenciesUnavailableError;
import com.blockset.walletkit.errors.CurrencyUpdateError;
import com.blockset.walletkit.errors.FeeEstimationError;
import com.blockset.walletkit.errors.NetworkFeeUpdateError;
import com.blockset.walletkit.errors.NetworkFeeUpdateFeesUnavailableError;
import com.blockset.walletkit.events.network.NetworkEvent;
import com.blockset.walletkit.events.system.SystemChangedEvent;
import com.blockset.walletkit.events.system.SystemCreatedEvent;
import com.blockset.walletkit.events.system.SystemDeletedEvent;
import com.blockset.walletkit.events.system.SystemDiscoveredNetworksEvent;
import com.blockset.walletkit.events.system.SystemEvent;
import com.blockset.walletkit.events.system.SystemListener;
import com.blockset.walletkit.events.system.SystemManagerAddedEvent;
import com.blockset.walletkit.events.system.SystemNetworkAddedEvent;
import com.blockset.walletkit.events.transfer.TransferEvent;
import com.blockset.walletkit.events.transfer.TransferChangedEvent;
import com.blockset.walletkit.events.transfer.TransferCreatedEvent;
import com.blockset.walletkit.events.transfer.TransferDeletedEvent;
import com.blockset.walletkit.events.wallet.WalletBalanceUpdatedEvent;
import com.blockset.walletkit.events.wallet.WalletChangedEvent;
import com.blockset.walletkit.events.wallet.WalletCreatedEvent;
import com.blockset.walletkit.events.wallet.WalletDeletedEvent;
import com.blockset.walletkit.events.wallet.WalletEvent;
import com.blockset.walletkit.events.wallet.WalletFeeBasisUpdatedEvent;
import com.blockset.walletkit.events.wallet.WalletTransferAddedEvent;
import com.blockset.walletkit.events.wallet.WalletTransferChangedEvent;
import com.blockset.walletkit.events.wallet.WalletTransferDeletedEvent;
import com.blockset.walletkit.events.wallet.WalletTransferSubmittedEvent;
import com.blockset.walletkit.events.walletmanager.WalletManagerBlockUpdatedEvent;
import com.blockset.walletkit.events.walletmanager.WalletManagerChangedEvent;
import com.blockset.walletkit.events.walletmanager.WalletManagerCreatedEvent;
import com.blockset.walletkit.events.walletmanager.WalletManagerDeletedEvent;
import com.blockset.walletkit.events.walletmanager.WalletManagerEvent;
import com.blockset.walletkit.events.walletmanager.WalletManagerSyncProgressEvent;
import com.blockset.walletkit.events.walletmanager.WalletManagerSyncRecommendedEvent;
import com.blockset.walletkit.events.walletmanager.WalletManagerSyncStartedEvent;
import com.blockset.walletkit.events.walletmanager.WalletManagerSyncStoppedEvent;
import com.blockset.walletkit.events.walletmanager.WalletManagerWalletAddedEvent;
import com.blockset.walletkit.events.walletmanager.WalletManagerWalletChangedEvent;
import com.blockset.walletkit.events.walletmanager.WalletManagerWalletDeletedEvent;
import com.blockset.walletkit.brd.systemclient.BlocksetAmount;
import com.blockset.walletkit.brd.systemclient.BlocksetCurrency;
import com.blockset.walletkit.brd.systemclient.BlocksetTransfer;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.base.Function;
import com.google.common.base.Optional;
import com.google.common.collect.Collections2;
import com.google.common.primitives.UnsignedInteger;
import com.google.common.primitives.UnsignedLong;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.logging.Level;
import java.util.logging.Logger;

import static com.google.common.base.Preconditions.checkState;

import androidx.annotation.Nullable;

/* package */
final class System implements com.blockset.walletkit.System {

    private static final Logger Log = Logger.getLogger(System.class.getName());

    /// A index to globally identify systems.
    private static final AtomicInteger SYSTEM_IDS = new AtomicInteger(0);

    /// A dictionary mapping an index to a system.
    private static final Map<Cookie, System> SYSTEMS_ACTIVE = new ConcurrentHashMap<>();

    /// An array of removed systems.  This is a workaround for systems that have been destroyed.
    /// We do not correctly handle 'release' and thus C-level memory issues are introduced; rather
    /// than solving those memory issues now, we'll avoid 'release' by holding a reference.
    private static final List<System> SYSTEMS_INACTIVE = Collections.synchronizedList(new ArrayList<>());

    /// If true, save removed system in the above array. Set to `false` for debugging 'release'.
    private static final boolean SYSTEMS_INACTIVE_RETAIN = true;

    // Create a dedicated executor to pump CWM events as quickly as possible
    private static final Executor EXECUTOR_LISTENER = Executors.newSingleThreadExecutor();

    // Create a dedicated executor to pump CWM callbacks. This is a separate executor
    // than the one used to handle events as they *really* need to be pumped as fast as possible.
    private static final Executor EXECUTOR_CLIENT = Executors.newSingleThreadExecutor();

    //
    // Keep a static reference to the callbacks so that they are never GC'ed
    //

    private static final WKListener.SystemEventCallback CWM_LISTENER_SYSTEM_CALLBACK = System::systemEventCallback;
    private static final WKListener.NetworkEventCallback CWM_LISTENER_NETWORK_CALLBACK = System::networkEventCallback;
    private static final WKListener.WalletManagerEventCallback CWM_LISTENER_WALLET_MANAGER_CALLBACK = System::walletManagerEventCallback;
    private static final WKListener.WalletEventCallback CWM_LISTENER_WALLET_CALLBACK = System::walletEventCallback;
    private static final WKListener.TransferEventCallback CWM_LISTENER_TRANSFER_CALLBACK = System::transferEventCallback;

    private static final boolean DEFAULT_IS_NETWORK_REACHABLE = true;

    private static boolean ensurePath(String storagePath) {
        File storageFile = new File(storagePath);
        return ((storageFile.exists() || storageFile.mkdirs())
                && storageFile.isDirectory()
                && storageFile.canWrite());
    }

    /* package */
    static System create(ScheduledExecutorService executor,
                         SystemListener listener,
                         com.blockset.walletkit.Account account,
                         boolean isMainnet,
                         String storagePath,
                         SystemClient query) {
        Account cryptoAccount = Account.from(account);

        storagePath = storagePath + (storagePath.endsWith(File.separator) ? "" : File.separator) + cryptoAccount.getFilesystemIdentifier();
        checkState(ensurePath(storagePath));

        Cookie context = new Cookie(SYSTEM_IDS.incrementAndGet());

        WKListener cwmListener = WKListener.create(
                context,
                CWM_LISTENER_SYSTEM_CALLBACK,
                CWM_LISTENER_NETWORK_CALLBACK,
                CWM_LISTENER_WALLET_MANAGER_CALLBACK,
                CWM_LISTENER_WALLET_CALLBACK,
                CWM_LISTENER_TRANSFER_CALLBACK);

        WKClient cwmClient = new WKClient(
                context,
                System::getBlockNumber,
                System::getTransactions,
                System::getTransfers,
                System::submitTransaction,
                System::estimateTransactionFee);

        System system = new System(executor,
                listener,
                cryptoAccount,
                isMainnet,
                storagePath,
                query,
                context,
                cwmListener,
                cwmClient);
        ReferenceCleaner.register(system, system.core::give);

        SYSTEMS_ACTIVE.put(context, system);

        system.core.start();

        return system;
    }

    /* package */
    static Optional<SystemClient.Currency> asBDBCurrency(String uids,
                                                         String name,
                                                         String code,
                                                         String type,
                                                         UnsignedInteger decimals) {
        int index = uids.indexOf(':');
        if (index == -1) return Optional.absent();

        type = type.toLowerCase(Locale.ROOT);
        if (!"erc20".equals(type) && !"native".equals(type)) return Optional.absent();

        code = code.toLowerCase(Locale.ROOT);
        String blockchainId = uids.substring(0, index);
        String address = uids.substring(index);

        // TODO: SystemClient inscrutability question?
        return Optional.of(
                BlocksetCurrency.create(
                        uids,
                        name,
                        code,
                        type,
                        blockchainId,
                        address.equals("__native__") ? null : address,
                        Boolean.valueOf(true),
                        Blockchains.makeCurrencyDemominationsErc20(code, decimals)
                )
        );
    }

    /* package */
    static Optional<byte[]> migrateBRCoreKeyCiphertext(Key key,
                                                       byte[] nonce12,
                                                       byte[] authenticatedData,
                                                       byte[] ciphertext) {
        return Cipher.migrateBRCoreKeyCiphertext(key, nonce12, authenticatedData, ciphertext);
    }

    /* package */
    static void wipe(com.blockset.walletkit.System system) {
        // Safe the path to the persistent storage
        String storagePath = system.getPath();

        // Destroy the system.
        destroy(system);

        // Clear out persistent storage
        deleteRecursively(storagePath);
    }

    /* package */
    static void wipeAll(String storagePath, List<com.blockset.walletkit.System> exemptSystems) {
        Set<String> exemptSystemPath = new HashSet<>();
        for (com.blockset.walletkit.System sys: exemptSystems) {
            exemptSystemPath.add(sys.getPath());
        }

        File storageFile = new File(storagePath);
        File[] childFiles = storageFile.listFiles();
        if (null != childFiles) {
            for (File child : childFiles) {
                if (!exemptSystemPath.contains(child.getAbsolutePath())) {
                    deleteRecursively(child);
                }
            }
        }
    }

    private static void destroy(com.blockset.walletkit.System system) {
        System sys = System.from(system);
        // Stop all callbacks.  This might be inconsistent with 'deleted' events.
        SYSTEMS_ACTIVE.remove(sys.context);

        // Disconnect all wallet managers
        sys.pause();

        // Stop
        sys.stopAll();

        // Register the system as inactive
        if (SYSTEMS_INACTIVE_RETAIN) {
            SYSTEMS_INACTIVE.add(sys);
        }
    }

    private static void deleteRecursively (String toDeletePath) {
        deleteRecursively(new File(toDeletePath));
    }

    private static void deleteRecursively (File toDelete) {
        if (toDelete.isDirectory()) {
            for (File child : toDelete.listFiles()) {
                deleteRecursively(child);
            }
        }

        if (toDelete.exists() && !toDelete.delete()) {
            Log.log(Level.SEVERE, "Failed to delete " + toDelete.getAbsolutePath());
        }
    }

    /** Extraction: Allows system and related classes to be incrementally resolved
     *              by creating Java objects directly, using the native WalletKit
     *              objects directly, and without recourse to querying WalletKit
     *              for related structures.
     *
     *              Extraction does not take ownership of any of the provided
     *              core objects.
     */
    private final static class Extraction {

        // Valid Extraction object implies system is valid
        System          system;

        // Following objects are 'add-ons' utilitizing provided
        // WKCore objects + the known system, and may or may not
        // be available depending upon how much information was
        // provided at Extraction creation time
        WalletManager   manager;
        Wallet          wallet;
        Transfer        transfer;

        Extraction (System system) {
            this.system = system;
        }

        /** A Function to create Extraction from System
         *
         */
        private static final Function<System, Extraction> toSystemExtraction =
            new Function<System, Extraction>() {
                @Override
                public Extraction apply(final System system) {
                    return new Extraction(system);
                }
            };

        /** Working with system 'context', create an extraction involving the System.
         *
         * @param context The system context
         * @return An Optional containing the System if system is found.
         */
        static Optional<Extraction> extract(Cookie context) {
            Optional<Extraction> extraction = Optional.fromNullable(SYSTEMS_ACTIVE.get(context)).transform(toSystemExtraction);
            if (!extraction.isPresent()) {
                Log.log(Level.SEVERE, "Extraction missed system");
            }
            return extraction;
        }

        /** Create an Extraction involving system 'context' and the core wallet manager object.
         *
         * @param context The system context
         * @param coreWalletManager The WalletKit manager object
         * @return An Optional containing System and WalletManager if the optional is present.
         */
        static Optional<Extraction> extract(Cookie            context,
                                            WKWalletManager   coreWalletManager   ) {
            return extract(context).transform((e)-> {
                e.manager = WalletManager.create(coreWalletManager,
                                                 true,
                                                 e.system,
                                                 e.system.callbackCoordinator);


                if (e.manager == null) {
                    Log.log(Level.SEVERE, "Extraction missed wallet manager");
                }
                return e;
            });
        }

        /** Create an Extraction involving system 'context', core wallet manager and core
         *  wallet objects.
         *
         * @param context The system context
         * @param coreWalletManager The WalletKit manager object
         * @param coreWallet The WalletKit wallet object
         * @return An Optional containing System, WalletManager and Wallet if the optional is present.
         */
        static Optional<Extraction> extract(Cookie            context,
                                            WKWalletManager   coreWalletManager,
                                            WKWallet          coreWallet          ) {
            return extract(context,
                           coreWalletManager).transform((e)-> {
                if (e.manager != null) {
                    e.wallet = e.manager.walletByCoreOrCreate(coreWallet, true);
                    if (e.wallet == null) {
                        Log.log(Level.SEVERE, "Extraction missed wallet");
                    }
                }
                return e;
            });
        }

        /** Create an Extraction involving all of system 'context', core wallet manager, core
         *  wallet, and core transfer.
         *
         * @param context The system context
         * @param coreWalletManager The WalletKit manager object
         * @param coreWallet The WalletKit wallet object
         * @param coreTransfer The WalletKit transfer object
         * @return An Optional containing System, WalletManager, Wallet and Transfer
         *         if the optional is present.
         */
        static Optional<Extraction> extract(Cookie            context,
                                            WKWalletManager   coreWalletManager,
                                            WKWallet          coreWallet,
                                            WKTransfer        coreTransfer) {

            return extract(context,
                           coreWalletManager,
                           coreWallet).transform((e)-> {
                if (e.manager != null && e.wallet != null) {
                    e.transfer = e.wallet.transferByCoreOrCreate(coreTransfer, true);
                    if (e.transfer == null) {
                        Log.log(Level.SEVERE, "Extraction missed transfer");
                    }
                }
                return e;  });
        }


    }

    private static System from(com.blockset.walletkit.System system) {
        if (system == null) {
            return null;
        }

        if (system instanceof System) {
            return (System) system;
        }

        throw new IllegalArgumentException("Unsupported system instance");
    }

    private final WKSystem core;
    private final ExecutorService executor;
    private final SystemListener listener;
    private final SystemCallbackCoordinator callbackCoordinator;
    private final Account account;
    private final boolean isMainnet;
    private final String storagePath;
    private final SystemClient query;
    private final Cookie context;
    private final WKListener cwmListener;
    private final WKClient cwmClient;

    private System(ScheduledExecutorService executor,
                   SystemListener listener,
                   Account account,
                   boolean isMainnet,
                   String storagePath,
                   SystemClient query,
                   Cookie context,
                   WKListener cwmListener,
                   WKClient cwmClient) {
        this.executor = executor;
        this.listener = listener;
        this.callbackCoordinator = new SystemCallbackCoordinator(executor);
        this.account = account;
        this.isMainnet = isMainnet;
        this.storagePath = storagePath;
        this.query = query;
        this.context = context;
        this.cwmListener = cwmListener;
        this.cwmClient = cwmClient;

        this.core = WKSystem.create(
                this.cwmClient,
                this.cwmListener,
                this.account.getCoreBRCryptoAccount(),
                storagePath,
                isMainnet).get();
    }

    @Override
    public void configure() {
        Log.log(Level.FINE, "Configure");
        updateNetworkFees(null);
        updateCurrencies(null);

//        NetworkDiscovery.discoverNetworks(query, isMainnet, getNetworks(), appCurrencies, new NetworkDiscovery.Callback() {
//            @Override
//            public void discovered(Network network) {
//                announceNetworkEvent(network, new NetworkCreatedEvent());
//                announceSystemEvent(new SystemNetworkAddedEvent(network));
//            }
//
//            @Override
//            public void updated(Network network) {
//                announceNetworkEvent(network, new NetworkUpdatedEvent());
//            }
//
//            @Override
//            public void complete(List<Network> networks) {
//                announceSystemEvent(new SystemDiscoveredNetworksEvent(networks));
//            }
//        });
    }

    /* package */
    Network networkBy(WKNetwork coreNetwork) {
        for (Network n: getNetworks()) {
            if (n.getCoreBRCryptoNetwork().equals(coreNetwork))
                return n;
        }
        return null;
    }

    @Override
    public boolean createWalletManager(com.blockset.walletkit.Network network,
                                       WalletManagerMode mode,
                                       AddressScheme scheme,
                                       Set<com.blockset.walletkit.Currency> currencies) {
        checkState(network.supportsWalletManagerMode(mode));
        checkState(network.supportsAddressScheme(scheme));

        List<WKCurrency> currenciesList = new ArrayList<>();
        for (com.blockset.walletkit.Currency currency : currencies)
            currenciesList.add(Currency.from(currency).getCoreBRCryptoCurrency());

        return core.createManager(core,
                        Network.from(network).getCoreBRCryptoNetwork(),
                        Utilities.walletManagerModeToCrypto(mode),
                        Utilities.addressSchemeToCrypto(scheme),
                        currenciesList)
                .isPresent();
    }

    @Override
    public void wipe(com.blockset.walletkit.Network network) {
        boolean found = false;
        for (WalletManager walletManager: getWalletManagers()) {
            if (walletManager.getNetwork().equals(network)) {
                found = true;
                break;
            }
        }

        // Racy - but if there is no wallet manager for `network`... then
        if (!found) {
            WalletManager.wipe(Network.from(network), storagePath);
        }
    }

    @Override
    public void resume () {
        Log.log(Level.FINE, "Resume");

        updateNetworkFees(null);
        updateCurrencies(null);

        for (WalletManager manager : getWalletManagers()) {
            manager.connect(null);
        }
    }

    @Override
    public void pause() {
        Log.log(Level.FINE, "Pause");
        for (WalletManager manager : getWalletManagers()) {
            manager.disconnect();
        }
        query.cancelAll();
    }

    @Override
    public void subscribe(String subscriptionToken) {
        // TODO(fix): Implement this!
    }

    @Override
    public void updateNetworkFees(@Nullable CompletionHandler<List<com.blockset.walletkit.Network>, NetworkFeeUpdateError> handler) {
        query.getBlockchains(isMainnet, new CompletionHandler<List<Blockchain>, QueryError>() {
            @Override
            public void handleData(List<Blockchain> blockchainModels) {
                Map<String, Network> networksByUuid = new HashMap<>();
                for (Network network: getNetworks()) networksByUuid.put(network.getUids(), network);

                List<com.blockset.walletkit.Network> networks = new ArrayList<>();
                for (Blockchain blockChainModel: blockchainModels) {
                    Network network = networksByUuid.get(blockChainModel.getId());
                    if (null == network) continue;

                    // We always have a feeUnit for network
                    Optional<Unit> maybeFeeUnitBase = network.baseUnitFor(network.getCurrency());
                    checkState(maybeFeeUnitBase.isPresent());

                    Optional<Unit> maybeFeeUnitDefault = network.defaultUnitFor(network.getCurrency());
                    checkState(maybeFeeUnitDefault.isPresent());

                    // Set the blockHeight
                    UnsignedLong blockHeight = blockChainModel.getBlockHeight().orNull();
                    if (null != blockHeight)
                        network.setHeight(blockHeight);

                    // Set the verifiedBlockHash
                    String verifiedBlockHash = blockChainModel.getVerifiedBlockHash().orNull();
                    if (null != verifiedBlockHash)
                        network.setVerifiedBlockHashAsString(verifiedBlockHash);;

                    List<NetworkFee> fees = new ArrayList<>();
                    for (BlockchainFee feeEstimate: blockChainModel.getFeeEstimates()) {
                        // Well, quietly ignore a fee if we can't parse the amount.
                        Optional<Amount> maybeFeeAmount =
                                Amount.create(feeEstimate.getAmount(), false, maybeFeeUnitBase.get())
                                        .transform(a -> a.convert(maybeFeeUnitDefault.get()).or(a));
                        if (!maybeFeeAmount.isPresent()) continue;

                        fees.add(NetworkFee.create(feeEstimate.getConfirmationTimeInMilliseconds(), maybeFeeAmount.get()));
                    }

                    // The fees are unlikely to change; but we'll announce feesUpdated anyways.
                    network.setFees(fees);
                    networks.add(network);
                }

                if (null != handler) handler.handleData(networks);
            }

            @Override
            public void handleError(QueryError error) {
                // On an error, just skip out; we'll query again later, presumably
                if (null != handler) handler.handleError(new NetworkFeeUpdateFeesUnavailableError());
            }
        });
    }

    @Override
    public <T extends com.blockset.walletkit.Network> void updateCurrencies(@Nullable CompletionHandler<List<T>, CurrencyUpdateError> handler) {
        query.getCurrencies(null, isMainnet, new CompletionHandler<List<SystemClient.Currency>, QueryError>() {
            @Override
            public void handleData(List<SystemClient.Currency> currencyModels) {
                List<WKClientCurrencyBundle> bundles = new ArrayList<>();

                for (SystemClient.Currency currencyModel : currencyModels) {
                    List<WKClientCurrencyDenominationBundle> denominationBundles = new ArrayList<>();
                    for (CurrencyDenomination currencyDenomination : currencyModel.getDenominations())
                        denominationBundles.add(
                                WKClientCurrencyDenominationBundle.create(
                                        currencyDenomination.getName(),
                                        currencyDenomination.getCode(),
                                        currencyDenomination.getSymbol(),
                                        currencyDenomination.getDecimals()));

                    bundles.add(WKClientCurrencyBundle.create(
                            currencyModel.getId(),
                            currencyModel.getName(),
                            currencyModel.getCode(),
                            currencyModel.getType(),
                            currencyModel.getBlockchainId(),
                            currencyModel.getAddress().isPresent() ? currencyModel.getAddress().get() : null,
                            currencyModel.getVerified(),
                            denominationBundles));
                }

                getCoreBRCryptoSystem().announceCurrencies(bundles);
                for (WKClientCurrencyBundle bundle : bundles) bundle.release();

                if (null != handler) {
                    handler.handleData((List<T>) getNetworks());
                }
            }

            @Override
            public void handleError(QueryError error) {
                if (null != handler)
                    handler.handleError(new CurrencyUpdateCurrenciesUnavailableError());
            }
        });
    }

    @Override
    public void setNetworkReachable(boolean isNetworkReachable) {
        core.setIsReachable(isNetworkReachable);
    }

    @Override
    public Account getAccount() {
        return account;
    }

    @Override
    public String getPath() {
        return storagePath;
    }

    @Override
    public List<Wallet> getWallets() {
        List<Wallet> wallets = new ArrayList<>();
        for (WalletManager manager: getWalletManagers()) {
            wallets.addAll(manager.getWallets());
        }
        return wallets;
    }

    private void stopAll() {
        for (WalletManager manager: getWalletManagers()) {
            manager.stop();
        }
    }

    // Network management

    private UnsignedLong getNetworksCount () {
        return core.getNetworksCount();
    }

    @Override
    public List<? extends Network> getNetworks() {
        List<Network> networks = new ArrayList<>();
        for (WKNetwork coreNetwork: core.getNetworks())
            networks.add (Network.create(coreNetwork, false));
        return networks;
    }

    private Optional<Network> getNetwork(WKNetwork coreNetwork) {
        return (core.hasNetwork(coreNetwork)
                ? Optional.of (Network.create(coreNetwork, true))
                : Optional.absent());
    }

    // WalletManager management

    private UnsignedLong getWalletManagersCount () {
        return core.getManagersCount();
    }

    @Override
    public List<WalletManager> getWalletManagers() {
        List<WalletManager> managers = new ArrayList<>();
        for (WKWalletManager coreManager: core.getManagers())
            managers.add(createWalletManager(coreManager, false));
        return managers;
    }

    private Optional<WalletManager> getWalletManager(WKWalletManager coreManager) {
        return (core.hasManager(coreManager)
                ? Optional.of (createWalletManager(coreManager, true))
                : Optional.absent());
    }

    private WalletManager createWalletManager(WKWalletManager coreWalletManager, boolean needTake) {
        return WalletManager.create(
                coreWalletManager,
                needTake,
                this,
                callbackCoordinator);
    }

    // Miscellaneous

    /* package */
    SystemClient getSystemClient() {
        return query;
    }

    /* package */
    WKSystem getCoreBRCryptoSystem() {
        return core;
    }

    // Event announcements

    private void announceSystemEvent(SystemEvent event) {
        executor.submit(() -> listener.handleSystemEvent(this, event));
    }

    private void announceNetworkEvent(Network network, NetworkEvent event) {
        executor.submit(() -> listener.handleNetworkEvent(this, network, event));
    }

    private void announceWalletManagerEvent(WalletManager walletManager, WalletManagerEvent event) {
        executor.submit(() -> listener.handleManagerEvent(this, walletManager, event));
    }

    private void announceWalletEvent(WalletManager walletManager, Wallet wallet, WalletEvent event) {
        executor.submit(() -> listener.handleWalletEvent(this, walletManager, wallet, event));
    }

    private void announceTransferEvent(WalletManager walletManager, Wallet wallet, Transfer transfer, TransferEvent event) {
        executor.submit(() -> listener.handleTransferEvent(this, walletManager, wallet, transfer, event));
    }

    //
    // WalletManager Events
    //

    private static void systemEventCallback(Cookie context,
            /* OwnershipGiven */ WKSystem coreSystem,
            /* OwnershipGiven */ WKSystemEvent event) {
        EXECUTOR_LISTENER.execute(() -> {

            try {

                Optional<Extraction> optExtraction = Extraction.extract(context);
                if (!optExtraction.isPresent()) {
                    Log.log(Level.SEVERE,
                         String.format("%s: missed within extraction", event.type().toString()));
                    return;
                }

                System system = optExtraction.get().system;

                SystemEvent sysEvent = null;

                switch (event.type()) {

                    case CREATED:

                        Log.log(Level.FINE, "SystemCreated");
                        sysEvent = new SystemCreatedEvent();
                        break;

                    case CHANGED:

                        SystemState oldState = Utilities.systemStateFromCrypto(event.u.state.oldState());
                        SystemState newState = Utilities.systemStateFromCrypto(event.u.state.newState());
                        Log.log(Level.FINE, String.format("SystemChanged (%s -> %s)", oldState, newState));
                        sysEvent = new SystemChangedEvent(oldState, newState);
                        break;

                    case DELETED:

                        Log.log(Level.FINE, "System Deleted");
                        sysEvent = new SystemDeletedEvent();
                        break;

                    case NETWORK_ADDED:

                        Log.log(Level.FINE, "System Network Added");
                        Network network = Network.create(event.u.network, true);
                        sysEvent = new SystemNetworkAddedEvent(network);
                        break;

                    case MANAGER_ADDED:

                        Log.log(Level.FINE, "System WalletManager Added");
                        WalletManager manager = WalletManager.create(event.u.walletManager,
                                                                     true,
                                                                     system,
                                                                     system.callbackCoordinator);
                        sysEvent = new SystemManagerAddedEvent(manager);
                        break;

                    case DISCOVERED_NETWORKS:

                        Log.log(Level.FINE, "System Discovered Networks");
                        sysEvent = new SystemDiscoveredNetworksEvent(system.getNetworks());
                        break;

                    default:
                        Log.log(Level.SEVERE,
                                String.format("Untreated System Event %s",
                                              event.type().toString()));
                        break;
                }

                if (sysEvent != null)
                    system.announceSystemEvent(sysEvent);

            } finally {
                coreSystem.give();
                switch (event.type()) {
                    case CHANGED:
                        break;
                    case MANAGER_ADDED:
                    case MANAGER_CHANGED:
                    case MANAGER_DELETED:
                        if (null != event.u.walletManager) event.u.walletManager.give();
                        break;
                    case NETWORK_ADDED:
                    case NETWORK_CHANGED:
                    case NETWORK_DELETED:
                        if (null != event.u.network) event.u.network.give();
                        break;
                    default:
                        break;
                }
            }
        });
    }

    private static void networkEventCallback(Cookie context,
            /* OwnershipGiven */ WKNetwork coreNetwork,
            /* OwnershipGiven */ WKNetworkEvent event) {
        EXECUTOR_LISTENER.execute(() -> {
            try {
                // Nothing
            } finally {
                coreNetwork.give();
                // Nothing to give in `event`
            }
        });
    }

    private static void walletManagerEventCallback(Cookie context,
            /* OwnershipGiven */ WKWalletManager coreWalletManager,
            /* OwnershipGiven */ WKWalletManagerEvent event) {
        EXECUTOR_LISTENER.execute(() -> {

            try {

                Optional<Extraction> optExtraction = Extraction.extract(context, coreWalletManager);
                if (!optExtraction.isPresent()) {
                    Log.log(Level.SEVERE,
                            String.format("%s: missed within extraction",
                                    event.type().toString()));
                    return;
                }

                System        system  = optExtraction.get().system;
                WalletManager manager = optExtraction.get().manager;

                Wallet wallet = null;

                WalletManagerEvent mgrEvent = null;

                switch (event.type()) {

                    case CREATED:

                        Log.log(Level.FINE, "WalletManagerCreated");
                        mgrEvent = new WalletManagerCreatedEvent();
                        break;

                    case CHANGED:

                        WalletManagerState oldState = Utilities.walletManagerStateFromCrypto(event.u.state.oldValue);
                        WalletManagerState newState = Utilities.walletManagerStateFromCrypto(event.u.state.newValue);
                        Log.log(Level.FINE, String.format("WalletManagerChanged (%s -> %s)", oldState, newState));
                        mgrEvent = new WalletManagerChangedEvent(oldState, newState);
                        break;

                    case DELETED:

                        Log.log(Level.FINE, "WalletManagerDeleted");
                        mgrEvent = new WalletManagerDeletedEvent();
                        break;

                    case WALLET_ADDED:

                        Log.log(Level.FINE, "WalletManagerWalletAdded");
                        wallet = manager.walletBy(event.u.wallet);
                        if (null == wallet) {
                            Log.log(Level.SEVERE, "WalletManagerWalletAdded: Extraction missed wallet");
                            return;
                        }
                        mgrEvent = new WalletManagerWalletAddedEvent(wallet);
                        break;

                    case WALLET_CHANGED:

                        Log.log(Level.FINE, "WalletManagerWalletChanged");
                        wallet = manager.walletBy(event.u.wallet);
                        if (null == wallet) {
                            Log.log(Level.SEVERE, "WalletManagerWalletChanged: Extraction missed wallet");
                            return;
                        }
                        mgrEvent = new WalletManagerWalletChangedEvent(wallet);
                        break;

                    case WALLET_DELETED:

                        Log.log(Level.FINE, "WalletManagerWalletDeleted");
                        wallet = manager.walletBy(event.u.wallet);
                        if (null == wallet) {
                            Log.log(Level.SEVERE, "WalletManagerWalletDeleted: Extraction missed wallet");
                            return;
                        }
                        mgrEvent = new WalletManagerWalletDeletedEvent(wallet);
                        break;

                    case SYNC_STARTED:

                        Log.log(Level.FINE, "WalletManagerSyncStarted");
                        mgrEvent = new WalletManagerSyncStartedEvent();
                        break;

                    case SYNC_CONTINUES:

                        float percent = event.u.syncContinues.percentComplete;
                        Date timestamp = 0 == event.u.syncContinues.timestamp ? null : new Date(TimeUnit.SECONDS.toMillis(event.u.syncContinues.timestamp));
                        Log.log(Level.FINE, String.format("WalletManagerSyncProgress (%s)", percent));
                        mgrEvent = new WalletManagerSyncProgressEvent(percent, timestamp);
                        break;

                    case SYNC_STOPPED:

                        WalletManagerSyncStoppedReason reason = Utilities.walletManagerSyncStoppedReasonFromCrypto(event.u.syncStopped.reason);
                        Log.log(Level.FINE, String.format("WalletManagerSyncStopped: (%s)", reason));
                        mgrEvent = new WalletManagerSyncStoppedEvent(reason);
                        break;

                    case SYNC_RECOMMENDED:

                        WalletManagerSyncDepth depth = Utilities.syncDepthFromCrypto(event.u.syncRecommended.depth());
                        Log.log(Level.FINE, String.format("WalletManagerSyncRecommended: (%s)", depth));
                        mgrEvent = new WalletManagerSyncRecommendedEvent(depth);
                        break;


                    case BLOCK_HEIGHT_UPDATED:

                        UnsignedLong blockHeight = UnsignedLong.fromLongBits(event.u.blockHeight);
                        Log.log(Level.FINE, String.format("WalletManagerBlockHeightUpdated (%s)", blockHeight));
                        mgrEvent = new WalletManagerBlockUpdatedEvent(blockHeight);
                        break;

                    default:
                        Log.log(Level.SEVERE,
                                String.format("Untreated Wallet Manager Event %s",
                                        event.type().toString()));
                        break;
                }

                if (mgrEvent != null)
                    system.announceWalletManagerEvent(manager, mgrEvent);


            } finally {
                coreWalletManager.give();
                switch (event.type()) {
                    case CHANGED:
                        break;

                    case WALLET_ADDED:
                    case WALLET_CHANGED:
                    case WALLET_DELETED:
                        if (null != event.u.wallet) event.u.wallet.give();
                        break;
                    default:
                        break;
                }
            }
        });
    }

    //
    // Wallet Events
    //

    private static void walletEventCallback(Cookie context,
            /* OwnershipGiven */ WKWalletManager coreWalletManager,
            /* OwnershipGiven */ WKWallet coreWallet,
            /* OwnershipGiven */ WKWalletEvent coreEvent) {
        EXECUTOR_LISTENER.execute(() -> {

            try {

                Optional<Extraction> optExtraction = Extraction.extract(context, coreWalletManager, coreWallet);
                if (!optExtraction.isPresent()) {
                    Log.log(Level.SEVERE,
                            String.format("%s: missed within extraction",
                                    coreEvent.type().toString()));
                    return;
                }

                System        system  = optExtraction.get().system;
                WalletManager manager = optExtraction.get().manager;
                Wallet        wallet  = optExtraction.get().wallet;

                Transfer         transfer = null;
                TransferFeeBasis feeBasis = null;

                WalletEvent walletEvent = null;

                switch (coreEvent.type()) {
                    case CREATED:

                        Log.log(Level.FINE, "WalletCreated");
                        walletEvent = new WalletCreatedEvent();
                        break;

                    case CHANGED:

                        WKWalletEvent.States states = coreEvent.states();
                        WalletState oldState = Utilities.walletStateFromCrypto(states.oldState);
                        WalletState newState = Utilities.walletStateFromCrypto(states.newState);
                        Log.log(Level.FINE, String.format("WalletChanged (%s -> %s)",
                                                          oldState, newState));
                        walletEvent = new WalletChangedEvent(oldState, newState);
                        break;

                    case DELETED:

                        Log.log(Level.FINE, "WalletDeleted");
                        walletEvent = new WalletDeletedEvent();
                        break;

                    case TRANSFER_ADDED:

                        Log.log(Level.FINE, "WalletTransferAdded");
                        if (null == coreEvent.transfer()) {
                            Log.log(Level.SEVERE, "WalletTransferAdded: Extraction missed transfer");
                            return;
                        }
                        transfer = Transfer.create(coreEvent.transfer(), wallet, true);
                        walletEvent = new WalletTransferAddedEvent(transfer);
                        break;

                    case TRANSFER_CHANGED:

                        Log.log(Level.FINE, "WalletTransferChanged");
                        if (null == coreEvent.transfer()) {
                            Log.log(Level.SEVERE, "WalletTransferChanged: Extraction missed transfer");
                            return;
                        }
                        transfer = Transfer.create(coreEvent.transfer(), wallet, true);
                        walletEvent = new WalletTransferChangedEvent(transfer);
                        break;

                    case TRANSFER_SUBMITTED:

                        Log.log(Level.FINE, "WalletTransferSubmitted");
                        if (null == coreEvent.transferSubmit()) {
                            Log.log(Level.SEVERE, "WalletTransferSubmitted: Extraction missed transfer");
                            return;
                        }
                        transfer = Transfer.create(coreEvent.transferSubmit(), wallet, true);
                        walletEvent = new WalletTransferSubmittedEvent(transfer);
                        break;

                    case TRANSFER_DELETED:

                        Log.log(Level.FINE, "WalletTransferDeleted");
                        if (null == coreEvent.transfer()) {
                            Log.log(Level.SEVERE, "WalletTransferDeleted: Extraction missed transfer");
                            return;
                        }
                        transfer = Transfer.create(coreEvent.transfer(), wallet, true);
                        walletEvent = new WalletTransferDeletedEvent(transfer);
                        break;

                    case BALANCE_UPDATED:

                        Amount amount = Amount.create(coreEvent.balance().take());
                        Log.log(Level.FINE, String.format("WalletBalanceUpdated: %s", amount));
                        walletEvent = new WalletBalanceUpdatedEvent(amount);
                        break;

                    case FEE_BASIS_UPDATED:

                        WKFeeBasis coreFeeBasis = coreEvent.feeBasisUpdate().take();

                        feeBasis = TransferFeeBasis.create(coreFeeBasis);
                        Log.log(Level.FINE, String.format("WalletFeeBasisUpdate: %s", feeBasis));
                        walletEvent = new WalletFeeBasisUpdatedEvent(feeBasis);
                        break;

                    case FEE_BASIS_ESTIMATED:

                        // estimate.basis has ownership taken here.  It is not given with `coreEvent.give()`
                        WKWalletEvent.FeeBasisEstimate estimate = coreEvent.feeBasisEstimate();
                        Log.log(Level.FINE, String.format("WalletFeeBasisEstimated (%s)",
                                                          estimate.status));

                        if (estimate.status == WKStatus.SUCCESS) {
                            feeBasis = TransferFeeBasis.create(estimate.basis.take());
                        }

                        // Giving `estimate.basis` to release ownership
                        estimate.basis.give();

                        Cookie opCookie = new Cookie(estimate.cookie);
                        if (estimate.status == WKStatus.SUCCESS) {
                            Log.log(Level.FINE, String.format("WalletFeeBasisEstimated: %s", feeBasis));
                            system.callbackCoordinator.completeFeeBasisEstimateHandlerWithSuccess(opCookie, feeBasis);

                        } else {
                            FeeEstimationError error = Utilities.feeEstimationErrorFromStatus(estimate.status);

                            Log.log(Level.FINE, String.format("WalletFeeBasisEstimated: %s", error));
                            system.callbackCoordinator.completeFeeBasisEstimateHandlerWithError(opCookie, error);
                        }

                        // walletEvent is `null`, by design.
                        break;

                    default:
                        Log.log(Level.SEVERE,
                                String.format("Untreated Wallet Event %s",
                                              coreEvent.toString()));
                        break;
                }

                if (walletEvent != null)
                    system.announceWalletEvent(manager, wallet, walletEvent);

            } finally {
                coreEvent.give();
                coreWallet.give();
                coreWalletManager.give();
            }
        });
    }

    //
    // Transfer Events
    //

    private static void transferEventCallback(Cookie context,
            /* OwnershipGiven */ WKWalletManager coreWalletManager,
            /* OwnershipGiven */ WKWallet coreWallet,
            /* OwnershipGiven */ WKTransfer coreTransfer,
            /* OwnershipGiven */ WKTransferEvent event) {
        EXECUTOR_LISTENER.execute(() -> {

            try {

                Optional<Extraction> optExtraction = Extraction.extract(context, coreWalletManager, coreWallet, coreTransfer);
                if (!optExtraction.isPresent()) {
                    Log.log(Level.SEVERE,
                            String.format("%s: missed within extraction",
                                    event.type().toString()));
                    return;
                }

                System        system   = optExtraction.get().system;
                WalletManager manager  = optExtraction.get().manager;
                Wallet        wallet   = optExtraction.get().wallet;
                Transfer      transfer = optExtraction.get().transfer;

                TransferEvent transferEvent = null;

                 switch (event.type()) {
                    case CREATED:

                        Log.log(Level.FINE, "TransferCreated");
                        transferEvent = new TransferCreatedEvent();
                        break;

                    case CHANGED:

                        TransferState oldState = Utilities.transferStateFromCrypto(event.u.state.oldState);
                        TransferState newState = Utilities.transferStateFromCrypto(event.u.state.newState);
                        Log.log(Level.FINE, String.format("TransferChanged (%s -> %s)", oldState, newState));
                        transferEvent = new TransferChangedEvent(oldState, newState);
                        break;

                    case DELETED:

                        Log.log(Level.FINE, "TransferDeleted");
                        transferEvent = new TransferDeletedEvent();
                        break;

                    default:
                        Log.log(Level.SEVERE,
                                String.format("Untreated Transfer Event %s",
                                               event.type().toString()));
                        break;

                }

                system.announceTransferEvent(manager, wallet, transfer, transferEvent);

            } finally {
                coreTransfer.give();
                coreWallet.give();
                coreWalletManager.give();
                switch (event.type()) {
                    case CHANGED:
                        event.u.state.oldState.give();
                        event.u.state.newState.give();;
                        break;
                    default:
                        break;
                }
            }
        });
    }

    // BTC client

    private static void getBlockNumber(Cookie context, WKWalletManager coreWalletManager, WKClientCallbackState callbackState) {
        EXECUTOR_CLIENT.execute(() -> {

            Extraction extract = null;

            try {
                Log.log(Level.FINE, "BRCryptoCWMGetBlockNumberCallback");

                Optional<Extraction> optExtraction = Extraction.extract(context, coreWalletManager);
                if (!optExtraction.isPresent()) {
                    throw new IllegalStateException("BRCryptoCWMGetBlockNumberCallback: missing extraction");
                }

                System        system   = optExtraction.get().system;
                WalletManager manager  = optExtraction.get().manager;

                system.query.getBlockchain(manager.getNetwork().getUids(),
                        new CompletionHandler<Blockchain, QueryError>() {

                            @Override
                            public void handleData(Blockchain blockchain) {
                                Optional<UnsignedLong> maybeBlockHeight = blockchain.getBlockHeight();
                                Optional<String> maybeVerifiedBlockHash = blockchain.getVerifiedBlockHash();
                                if (maybeBlockHeight.isPresent() && maybeVerifiedBlockHash.isPresent()) {
                                    UnsignedLong blockchainHeight = maybeBlockHeight.get();
                                    String verifiedBlockHash = maybeVerifiedBlockHash.get();
                                    Log.log(Level.FINE, String.format("BRCryptoCWMGetBlockNumberCallback: succeeded (%s, %s)", blockchainHeight, verifiedBlockHash));
                                    manager.getCoreBRCryptoWalletManager().announceGetBlockNumber(callbackState,
                                            true,
                                            blockchainHeight,
                                            verifiedBlockHash);
                                } else {
                                    Log.log(Level.SEVERE, "BRCryptoCWMGetBlockNumberCallback: failed with missing block height");
                                    manager.getCoreBRCryptoWalletManager().announceGetBlockNumber(callbackState,
                                            false,
                                            UnsignedLong.ZERO,
                                            "");
                                }
                            }

                            @Override
                            public void handleError(QueryError error) {
                                Log.log(Level.SEVERE, "BRCryptoCWMGetBlockNumberCallback: failed", error);
                                manager.getCoreBRCryptoWalletManager().announceGetBlockNumber(callbackState,
                                        false,
                                        UnsignedLong.ZERO,
                                        "");
                            }
                        });

            } catch (RuntimeException e) {
                Log.log(Level.SEVERE, e.getMessage());
                coreWalletManager.announceGetBlockNumber(callbackState, false, UnsignedLong.ZERO, "");
            } finally {
                coreWalletManager.give();
            }
        });
    }

    private static WKTransferStateType getTransferStatus (String apiStatus) {
        switch (apiStatus) {
            case "confirmed":
                return WKTransferStateType.INCLUDED;
            case "submitted":
            case "reverted":
                return WKTransferStateType.SUBMITTED;
            case "failed":
            case "rejected":
                return WKTransferStateType.ERRORED;
            default:
                // throw new IllegalArgumentException("Unexpected API Status of " + apiStatus);
                return WKTransferStateType.DELETED;
        }
    }

    private static List<String> canonicalAddresses(List<String> addresses, NetworkType networkType) {
        switch (networkType) {
            case ETH:
                List<String> canonicalAddresses = new ArrayList<>(addresses.size());
                for (String address : addresses) {
                    canonicalAddresses.add(address.toLowerCase());
                }
                return canonicalAddresses;

            default:
                return addresses;
        }
    }

    protected static Optional<WKClientTransactionBundle> makeTransactionBundle(Transaction transaction) {
        Optional<byte[]> optRaw = transaction.getRaw();
        if (!optRaw.isPresent()) {
            Log.log(Level.SEVERE, "BRCryptoCWMGetTransactionsCallback completing with missing raw bytes");
            return Optional.absent();
        }
        UnsignedLong blockHeight =
                transaction.getBlockHeight().or(WKConstants.BLOCK_HEIGHT_UNBOUND);
        UnsignedLong timestamp =
                transaction.getTimestamp().transform(Utilities::dateAsUnixTimestamp).or(UnsignedLong.ZERO);

        WKTransferStateType status = getTransferStatus(transaction.getStatus());

        if (status != WKTransferStateType.DELETED) {
            Log.log(Level.FINE,"BRCryptoCWMGetTransactionsCallback announcing " + transaction.getId());
        } else {
            Log.log(Level.SEVERE,"BRCryptoCWMGetTransactionsCallback received an unknown status, completing with failure");
            return Optional.absent();
        }

        return Optional.of(WKClientTransactionBundle.create(
                status,
                optRaw.get(),
                timestamp,
                blockHeight));
    }

    private static void canonicalizeTransactions (List<Transaction> transactions) {
        // Sort descending (reverse order) by {BlockHeight, Index}
        Collections.sort (transactions, BlocksetTransaction.blockHeightAndIndexComparator.reversed());

        // Remove duplicates
        HashSet<String> uids = new HashSet<>();
        transactions.removeIf(t -> !uids.add (t.getId()));

        // Sort ascending
        Collections.reverse(transactions);
    }

     private static void getTransactions(Cookie context, WKWalletManager coreWalletManager, WKClientCallbackState callbackState,
                                         List<String> addresses, long begBlockNumber, long endBlockNumber) {
        EXECUTOR_CLIENT.execute(() -> {

            try {

                Optional<Extraction> optExtraction = Extraction.extract(context, coreWalletManager);
                if (!optExtraction.isPresent()) {
                    throw new IllegalStateException("BRCryptoCWMGetTransactionsCallback: missing extraction");
                }

                System        system   = optExtraction.get().system;
                WalletManager manager  = optExtraction.get().manager;

                UnsignedLong begBlockNumberUnsigned = UnsignedLong.fromLongBits(begBlockNumber);
                UnsignedLong endBlockNumberUnsigned = UnsignedLong.fromLongBits(endBlockNumber);

                Log.log(Level.FINE, String.format("BRCryptoCWMGetTransactionsCallback (%s -> %s)",
                        begBlockNumberUnsigned,
                        endBlockNumberUnsigned));

                final List<String> canonicalAddresses = canonicalAddresses(addresses, manager.getNetwork().getType());

                system.query.getTransactions(manager.getNetwork().getUids(),
                        canonicalAddresses,
                        begBlockNumberUnsigned.equals(WKConstants.BLOCK_HEIGHT_UNBOUND) ? null : begBlockNumberUnsigned,
                        endBlockNumberUnsigned.equals(WKConstants.BLOCK_HEIGHT_UNBOUND) ? null : endBlockNumberUnsigned,
                        true,
                        false,
                        false,
                        null,
                        new CompletionHandler<List<Transaction>, QueryError>() {
                            @Override
                            public void handleData(List<Transaction> transactions) {
                                boolean success = false;
                                Log.log(Level.FINE, "BRCryptoCWMGetTransactionsCallback received transactions");

                                // Sort and filter `transactions` - will be ascending, duplicate free.
                                canonicalizeTransactions(transactions);

                                List<WKClientTransactionBundle> bundles = new ArrayList<>();
                                for (Transaction transaction : transactions) {
                                    makeTransactionBundle(transaction)
                                            .transform(b -> { bundles.add (b); return true; });
                                }
                                manager.getCoreBRCryptoWalletManager().announceTransactions(callbackState,
                                        true,
                                        bundles);

                                success = true;
                                Log.log(Level.FINE, "BRCryptoCWMGetTransactionsCallback: complete");
                            }

                            @Override
                            public void handleError(QueryError error) {
                                Log.log(Level.SEVERE, "BRCryptoCWMGetTransactionsCallback received an error, completing with failure: ", error);
                                manager.getCoreBRCryptoWalletManager().announceTransactions(callbackState,
                                        false,
                                        new ArrayList<>());

                            }
                        });

            } catch (RuntimeException e) {
                Log.log(Level.SEVERE, e.getMessage());
                coreWalletManager.announceTransactions(callbackState, false, new ArrayList<>());
            } finally {
                coreWalletManager.give();
            }
        });
     }

    protected static List<WKClientTransferBundle> makeTransferBundles (Transaction transaction, List<String> addresses) {
        List<WKClientTransferBundle> result = new ArrayList<>();

        UnsignedLong blockHeight    = transaction.getBlockHeight().or(WKConstants.BLOCK_HEIGHT_UNBOUND);
        UnsignedLong blockTimestamp = transaction.getTimestamp().transform(Utilities::dateAsUnixTimestamp).or(UnsignedLong.ZERO);
        UnsignedLong blockConfirmations = transaction.getConfirmations().or(UnsignedLong.ZERO);
        UnsignedLong blockTransactionIndex = transaction.getIndex().or(UnsignedLong.ZERO);
        String blockHash = transaction.getHash();

        WKTransferStateType status = getTransferStatus (transaction.getStatus());

        for (ObjectPair<SystemClient.Transfer, String> o : System.mergeTransfers(transaction, addresses)) {
            Log.log(Level.FINE, "BRCryptoCWMGetTransfersCallback  announcing " + o.o1.getId());

            // Merge Transfer 'meta' into Transaction' meta; duplicates from Transfer
            Map<String,String> meta = new HashMap<>(transaction.getMetaData());
            meta.putAll(o.o1.getMetaData());

            result.add (WKClientTransferBundle.create(
                    status,
                    transaction.getHash(),
                    transaction.getIdentifier(),
                    o.o1.getId(),
                    o.o1.getSource().orNull(),
                    o.o1.getTarget().orNull(),
                    o.o1.getAmount().getAmount(),
                    o.o1.getAmount().getCurrency(),
                    o.o2, // fee
                    o.o1.getIndex(),
                    blockTimestamp,
                    blockHeight,
                    blockConfirmations,
                    blockTransactionIndex,
                    blockHash,
                    meta));
        }

        return result;
    }

    private static void getTransfers(Cookie context, WKWalletManager coreWalletManager, WKClientCallbackState callbackState,
                                     List<String> addresses, long begBlockNumber, long endBlockNumber) {
        EXECUTOR_CLIENT.execute(() -> {

            try {

                Optional<Extraction> optExtraction = Extraction.extract(context, coreWalletManager);
                if (!optExtraction.isPresent()) {
                    throw new IllegalStateException("BRCryptoCWMGetTransfersCallback : missing extraction");
                }

                System        system   = optExtraction.get().system;
                WalletManager manager  = optExtraction.get().manager;

                UnsignedLong begBlockNumberUnsigned = UnsignedLong.fromLongBits(begBlockNumber);
                UnsignedLong endBlockNumberUnsigned = UnsignedLong.fromLongBits(endBlockNumber);

                Log.log(Level.FINE, String.format("BRCryptoCWMGetTransfersCallback (%s -> %s)", begBlockNumberUnsigned, endBlockNumberUnsigned));

                final List<String> canonicalAddresses = canonicalAddresses(addresses, manager.getNetwork().getType());

                system.query.getTransactions(
                        manager.getNetwork().getUids(),
                        canonicalAddresses,
                        begBlockNumberUnsigned.equals(WKConstants.BLOCK_HEIGHT_UNBOUND) ? null : begBlockNumberUnsigned,
                        endBlockNumberUnsigned.equals(WKConstants.BLOCK_HEIGHT_UNBOUND) ? null : endBlockNumberUnsigned,
                        false,
                        false,
                        true,
                        null,
                        new CompletionHandler<List<Transaction>, QueryError>() {
                            @Override
                            public void handleData(List<Transaction> transactions) {
                                boolean success = false;
                                Log.log(Level.FINE, "BRCryptoCWMGetTransfersCallback received transfers");

                                // Sort and filter `transactions` - will be ascending, duplicate free.
                                canonicalizeTransactions(transactions);

                                List<WKClientTransferBundle> bundles = new ArrayList<>();

                                for (Transaction transaction : transactions) {
                                    bundles.addAll(makeTransferBundles(transaction, canonicalAddresses));
                                }

                                manager.getCoreBRCryptoWalletManager().announceTransfers(callbackState,
                                        true,
                                        bundles);

                                success = true;
                                Log.log(Level.FINE, "BRCryptoCWMGetTransfersCallback : complete");
                            }

                            @Override
                            public void handleError(QueryError error) {
                                Log.log(Level.SEVERE, "BRCryptoCWMGetTransfersCallback  received an error, completing with failure: ", error);
                                manager.getCoreBRCryptoWalletManager().announceTransfers(callbackState,
                                        false,
                                        new ArrayList<>());
                            }
                        });

            } catch (RuntimeException e) {
                Log.log(Level.SEVERE, e.getMessage());
                coreWalletManager.announceTransfers(callbackState, false, new ArrayList<>());
            } finally {
                coreWalletManager.give();
            }
        });
    }

    private static void submitTransaction(Cookie context, WKWalletManager coreWalletManager, WKClientCallbackState callbackState,
                                          String identifier,
                                          byte[] transaction) {
        EXECUTOR_CLIENT.execute(() -> {

            try {
                Log.log(Level.FINE, "BRCryptoCWMSubmitTransactionCallback");

                Optional<Extraction> optExtraction = Extraction.extract(context,
                                                                        coreWalletManager);
                if (!optExtraction.isPresent()) {
                    throw new IllegalStateException("BRCryptoCWMSubmitTransactionCallback: missing extraction");
                }

                System        system   = optExtraction.get().system;
                WalletManager manager  = optExtraction.get().manager;

                system.query.createTransaction(manager.getNetwork().getUids(), transaction, identifier,
                        new CompletionHandler<TransactionIdentifier, QueryError>() {
                            @Override
                            public void handleData(TransactionIdentifier tid) {
                                Log.log(Level.FINE, "BRCryptoCWMSubmitTransactionCallback: succeeded");
                                manager.getCoreBRCryptoWalletManager().announceSubmitTransfer(callbackState,
                                        tid.getIdentifier(),
                                        tid.getHash().orNull(),
                                        true);
                            }

                            @Override
                            public void handleError(QueryError error) {
                                Log.log(Level.SEVERE, "BRCryptoCWMSubmitTransactionCallback: failed", error);
                                manager.getCoreBRCryptoWalletManager().announceSubmitTransfer(callbackState,
                                        null,
                                        null,
                                        false);
                            }
                        });


            } catch (RuntimeException e) {
                Log.log(Level.SEVERE, e.getMessage());
                coreWalletManager.announceSubmitTransfer(callbackState, null, null, false);
            } finally {
                coreWalletManager.give();
            }
        });
    }

    private static void estimateTransactionFee(Cookie context, WKWalletManager coreWalletManager, WKClientCallbackState callbackState,
                                               byte[] transaction) {
        EXECUTOR_CLIENT.execute(() -> {

            try {
                Log.log(Level.FINE, "BRCryptoCWMEstimateTransactionFeeCallback");

                Optional<Extraction> optExtraction = Extraction.extract(context,
                                                                        coreWalletManager);
                if (!optExtraction.isPresent()) {
                    throw new IllegalStateException("BRCryptoCWMEstimateTransactionFeeCallback: missing extraction");
                }

                System        system   = optExtraction.get().system;
                WalletManager manager  = optExtraction.get().manager;

                system.query.estimateTransactionFee(manager.getNetwork().getUids(),
                        transaction,
                        new CompletionHandler<TransactionFee, QueryError>() {
                            @Override
                            public void handleData(TransactionFee fee) {
                                Log.log(Level.FINE, "BRCryptoCWMEstimateTransactionFeeCallback: succeeded");
                                manager.getCoreBRCryptoWalletManager().announceEstimateTransactionFee(callbackState,
                                        true,
                                        fee.getCostUnits(),
                                        fee.getProperties());
                            }

                            @Override
                            public void handleError(QueryError error) {
                                Log.log(Level.SEVERE, "BRCryptoCWMEstimateTransactionFeeCallback: failed ", error);
                                manager.getCoreBRCryptoWalletManager().announceEstimateTransactionFee(callbackState,
                                        false,
                                        UnsignedLong.ZERO,
                                        new LinkedHashMap<>());
                            }
                        });
            } catch (RuntimeException e) {
                Log.log(Level.SEVERE, e.getMessage());
                coreWalletManager.announceEstimateTransactionFee(callbackState, false, UnsignedLong.ZERO, new LinkedHashMap<>());
            } finally {
                coreWalletManager.give();
            }
        });
    }

    private static class ObjectPair<T1, T2> {
        final T1 o1;
        final T2 o2;

        ObjectPair (T1 o1, T2 o2) {
            this.o1 = o1;
            this.o2 = o2;
        }
    }

    private static List<ObjectPair<SystemClient.Transfer, String>> mergeTransfers(Transaction transaction, List<String> addresses) {
        List<SystemClient.Transfer> transfers;
        List<SystemClient.Transfer> transfersWithFee;
        List<SystemClient.Transfer> transfersWithoutFee;
        List<ObjectPair<SystemClient.Transfer, String>> transfersMerged;
        SystemClient.Transfer transferWithFee;

        // Only consider transfers w/ `address`
        transfers = new ArrayList<>(Collections2.filter(transaction.getTransfers(),
                t -> addresses.contains(t.getSource().orNull()) ||
                        addresses.contains(t.getTarget().orNull())));

        // Note for later: all transfers have a unique id

        transfersWithFee = new ArrayList<>(Collections2.filter(transfers, t -> "__fee__".equals(t.getTarget().orNull())));
        transfersWithoutFee = new ArrayList<>(Collections2.filter(transfers, t -> !"__fee__".equals(t.getTarget().orNull())));

        // Get the transferWithFee if we have one
        checkState(transfersWithFee.size() <= 1);
        transferWithFee = transfersWithFee.isEmpty() ? null : transfersWithFee.get(0);

        transfersMerged = new ArrayList<>(transfers.size());

        // There is no "__fee__" entry
        if (transferWithFee == null) {
            // Announce transfers with no fee
            for (SystemClient.Transfer transfer: transfers) {
                transfersMerged.add(new ObjectPair<>(transfer, null));
            }

        // There is a single "__fee__" entry, due to `checkState(transfersWithFee.size() <= 1)` above
        } else {
            // We may or may not have a non-fee transfer matching `transferWithFee`.  We
            // may or may not have more than one non-fee transfers matching `transferWithFee`

            // Find the first of the non-fee transfers matching `transferWithFee` that also matches
            // the amount's currency.
            SystemClient.Transfer transferMatchingFee = null;
            for (SystemClient.Transfer transfer: transfersWithoutFee) {
                if (transferWithFee.getTransactionId().equals(transfer.getTransactionId()) &&
                    transferWithFee.getSource().equals(transfer.getSource()) &&
                    transferWithFee.getAmount().getCurrency().equals(transfer.getAmount().getCurrency())) {
                    transferMatchingFee = transfer;
                    break;
                }
            }

            // If there is still no `transferWithFee`, find the first w/o matching the amount's currency
            if (null == transferMatchingFee)
                for (SystemClient.Transfer transfer : transfersWithoutFee) {
                    if (transferWithFee.getTransactionId().equals(transfer.getTransactionId()) &&
                            transferWithFee.getSource().equals(transfer.getSource())) {
                        transferMatchingFee = transfer;
                        break;
                    }
                }

            // We must have a transferMatchingFee; if we don't add one
            transfers = new ArrayList<>(transfersWithoutFee);
            if (null == transferMatchingFee) {
                transfers.add(
                        BlocksetTransfer.create(
                                transferWithFee.getId(),
                                transferWithFee.getBlockchainId(),
                                transferWithFee.getIndex(),
                                BlocksetAmount.create(transferWithFee.getAmount().getCurrency(), "0"),
                                transferWithFee.getMetaData(),
                                transferWithFee.getSource().orNull(),
                                "unknown",
                                transferWithFee.getTransactionId().or("0"),
                                transferWithFee.getAcknowledgements().orNull())
                );
            }

            // Hold the Id for the transfer that we'll add a fee to.
            String transferForFeeId = transferMatchingFee != null ? transferMatchingFee.getId() : transferWithFee.getId();

            // Announce transfers adding the fee to the `transferforFeeId`
            for (SystemClient.Transfer transfer: transfers) {
                String fee = transfer.getId().equals(transferForFeeId) ? transferWithFee.getAmount().getAmount() : null;

                transfersMerged.add(new ObjectPair<>(transfer, fee));
            }
        }

        return transfersMerged;
    }

    @Override
    public boolean accountIsInitialized(com.blockset.walletkit.Account account, com.blockset.walletkit.Network network) {
        return account.isInitialized(network);
    }

    @Override
    public void accountInitialize(com.blockset.walletkit.Account account,
                                  com.blockset.walletkit.Network network,
                                  boolean create,
                                  CompletionHandler<byte[], AccountInitializationError> handler) {
        EXECUTOR_CLIENT.execute(() -> {
            if (accountIsInitialized(account, network)) {
                accountInitializeReportError(new AccountInitializationAlreadyInitializedError(), handler);
                return;
            }

            switch (network.getType()) {
                case HBAR:
                    Optional<String> publicKey = Optional.fromNullable(account.getInitializationData(network))
                            .transform((data) -> Coder.createForAlgorithm(com.blockset.walletkit.Coder.Algorithm.HEX).encode(data))
                            .get();

                    if (!publicKey.isPresent()) {
                        accountInitializeReportError(new AccountInitializationQueryError(new QueryNoDataError()), handler);
                        return;
                    }

                    Log.log(Level.INFO, String.format ("HBAR accountInitialize: publicKey: %s", publicKey.get()));

                    // We'll recursively reference this 'hederaHandler' - put it in a 'final box' so
                    // that the compiler permits references w/o 'perhaps not initialized' errors.
                    final HederaAccountCompletionHandler[] hederaHandlerBox = new HederaAccountCompletionHandler[1];
                    hederaHandlerBox[0] = new HederaAccountCompletionHandler() {
                        @Override
                        public void handleData(List<HederaAccount> accounts) {
                            switch (accounts.size()) {
                                case 0:
                                    if (!hederaHandlerBox[0].create) {
                                        accountInitializeReportError(new AccountInitializationCantCreateError(), handler);
                                    } else {
                                        // Create the account; but only try once.
                                        hederaHandlerBox[0].create = false;
                                        query.createHederaAccount(network.getUids(), publicKey.get(), hederaHandlerBox[0]);
                                    }
                                    break;

                                case 1:
                                    Log.log(Level.INFO, String.format("HBAR accountInitialize: Hedera AccountId: %s, Balance: %s",
                                            accounts.get(0).getId(),
                                            accounts.get(0).getBalance()));

                                    Optional<byte[]> serialization = accountInitializeUsingHedera(account, network, accounts.get(0));
                                    if (serialization.isPresent()) {
                                        accountInitializeReportSuccess(serialization.get(), handler);
                                    } else {
                                        accountInitializeReportError(new AccountInitializationQueryError(new QueryNoDataError()), handler);
                                    }
                                    break;

                                default:
                                    accountInitializeReportError(new AccountInitializationMultipleHederaAccountsError(accounts), handler);
                                    break;
                            }

                        }

                        @Override
                        public void handleError(QueryError error) {
                            accountInitializeReportError(new AccountInitializationQueryError(error), handler);
                        }
                    };

                    hederaHandlerBox[0].create = create;
                    query.getHederaAccount(network.getUids(), publicKey.get(), hederaHandlerBox[0]);
                    break;

                default:
                    checkState(false);
                    break;
            }
        });
    }

    @Override
    public Optional<byte[]> accountInitializeUsingData(com.blockset.walletkit.Account account, com.blockset.walletkit.Network network, byte[] data) {
        return Optional.fromNullable (account.initialize (network, data));
    }

    @Override
    public Optional<byte[]> accountInitializeUsingHedera(com.blockset.walletkit.Account account, com.blockset.walletkit.Network network, HederaAccount hedera) {
        return Optional. fromNullable (account.initialize (network, hedera.getId().getBytes()));
    }

    private void accountInitializeReportError(AccountInitializationError error,
                                              CompletionHandler<byte[], AccountInitializationError> handler) {
        handler.handleError(error);
    }

    private void accountInitializeReportSuccess(byte[] data,
                                                CompletionHandler<byte[], AccountInitializationError> handler) {
        handler.handleData(data);
    }

    private abstract class HederaAccountCompletionHandler implements CompletionHandler<List<HederaAccount>, QueryError> {
        boolean create = true;
    }
}
