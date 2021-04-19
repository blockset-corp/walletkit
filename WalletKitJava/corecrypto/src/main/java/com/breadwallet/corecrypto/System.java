/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corecrypto;

import android.support.annotation.Nullable;
import android.support.v4.util.ArrayMap;

import com.breadwallet.corenative.cleaner.ReferenceCleaner;
import com.breadwallet.corenative.crypto.BRCryptoClient;
import com.breadwallet.corenative.crypto.BRCryptoClientCallbackState;
import com.breadwallet.corenative.crypto.BRCryptoClientCurrencyBundle;
import com.breadwallet.corenative.crypto.BRCryptoClientCurrencyDenominationBundle;
import com.breadwallet.corenative.crypto.BRCryptoCurrency;
import com.breadwallet.corenative.crypto.BRCryptoListener;
import com.breadwallet.corenative.crypto.BRCryptoClientTransactionBundle;
import com.breadwallet.corenative.crypto.BRCryptoClientTransferBundle;
import com.breadwallet.corenative.crypto.BRCryptoNetwork;
import com.breadwallet.corenative.crypto.BRCryptoNetworkEvent;
import com.breadwallet.corenative.crypto.BRCryptoStatus;
import com.breadwallet.corenative.crypto.BRCryptoSystem;
import com.breadwallet.corenative.crypto.BRCryptoSystemEvent;
import com.breadwallet.corenative.crypto.BRCryptoTransfer;
import com.breadwallet.corenative.crypto.BRCryptoTransferEvent;
import com.breadwallet.corenative.crypto.BRCryptoTransferStateType;
import com.breadwallet.corenative.crypto.BRCryptoWallet;
import com.breadwallet.corenative.crypto.BRCryptoWalletEvent;
import com.breadwallet.corenative.crypto.BRCryptoWalletManager;
import com.breadwallet.corenative.crypto.BRCryptoWalletManagerEvent;
import com.breadwallet.corenative.support.BRConstants;
import com.breadwallet.corenative.utility.Cookie;
import com.breadwallet.crypto.AddressScheme;
import com.breadwallet.crypto.NetworkType;
import com.breadwallet.crypto.SystemState;
import com.breadwallet.crypto.TransferState;
import com.breadwallet.crypto.WalletManagerMode;
import com.breadwallet.crypto.WalletManagerState;
import com.breadwallet.crypto.WalletManagerSyncDepth;
import com.breadwallet.crypto.WalletManagerSyncStoppedReason;
import com.breadwallet.crypto.WalletState;
import com.breadwallet.crypto.blockchaindb.BlockchainDb;
import com.breadwallet.crypto.blockchaindb.errors.QueryError;
import com.breadwallet.crypto.blockchaindb.errors.QueryNoDataError;
import com.breadwallet.crypto.blockchaindb.models.bdb.Blockchain;
import com.breadwallet.crypto.blockchaindb.models.bdb.BlockchainFee;
import com.breadwallet.crypto.blockchaindb.models.bdb.CurrencyDenomination;
import com.breadwallet.crypto.blockchaindb.models.bdb.HederaAccount;
import com.breadwallet.crypto.blockchaindb.models.bdb.Transaction;
import com.breadwallet.crypto.blockchaindb.models.bdb.TransactionFee;
import com.breadwallet.crypto.blockchaindb.models.bdb.TransactionIdentifier;
import com.breadwallet.crypto.errors.AccountInitializationAlreadyInitializedError;
import com.breadwallet.crypto.errors.AccountInitializationCantCreateError;
import com.breadwallet.crypto.errors.AccountInitializationError;
import com.breadwallet.crypto.errors.AccountInitializationMultipleHederaAccountsError;
import com.breadwallet.crypto.errors.AccountInitializationQueryError;
import com.breadwallet.crypto.errors.CurrencyUpdateCurrenciesUnavailableError;
import com.breadwallet.crypto.errors.CurrencyUpdateError;
import com.breadwallet.crypto.errors.FeeEstimationError;
import com.breadwallet.crypto.errors.NetworkFeeUpdateError;
import com.breadwallet.crypto.errors.NetworkFeeUpdateFeesUnavailableError;
import com.breadwallet.crypto.events.network.NetworkEvent;
import com.breadwallet.crypto.events.system.SystemChangedEvent;
import com.breadwallet.crypto.events.system.SystemCreatedEvent;
import com.breadwallet.crypto.events.system.SystemDeletedEvent;
import com.breadwallet.crypto.events.system.SystemDiscoveredNetworksEvent;
import com.breadwallet.crypto.events.system.SystemEvent;
import com.breadwallet.crypto.events.system.SystemListener;
import com.breadwallet.crypto.events.system.SystemManagerAddedEvent;
import com.breadwallet.crypto.events.system.SystemNetworkAddedEvent;
import com.breadwallet.crypto.events.transfer.TranferEvent;
import com.breadwallet.crypto.events.transfer.TransferChangedEvent;
import com.breadwallet.crypto.events.transfer.TransferCreatedEvent;
import com.breadwallet.crypto.events.transfer.TransferDeletedEvent;
import com.breadwallet.crypto.events.wallet.WalletBalanceUpdatedEvent;
import com.breadwallet.crypto.events.wallet.WalletChangedEvent;
import com.breadwallet.crypto.events.wallet.WalletCreatedEvent;
import com.breadwallet.crypto.events.wallet.WalletDeletedEvent;
import com.breadwallet.crypto.events.wallet.WalletEvent;
import com.breadwallet.crypto.events.wallet.WalletFeeBasisUpdatedEvent;
import com.breadwallet.crypto.events.wallet.WalletTransferAddedEvent;
import com.breadwallet.crypto.events.wallet.WalletTransferChangedEvent;
import com.breadwallet.crypto.events.wallet.WalletTransferDeletedEvent;
import com.breadwallet.crypto.events.wallet.WalletTransferSubmittedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerBlockUpdatedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerChangedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerCreatedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerDeletedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerSyncProgressEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerSyncRecommendedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerSyncStartedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerSyncStoppedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerWalletAddedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerWalletChangedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerWalletDeletedEvent;
import com.breadwallet.crypto.utility.CompletionHandler;
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

import static com.breadwallet.corenative.crypto.BRCryptoTransferEventType.CRYPTO_TRANSFER_EVENT_CHANGED;
import static com.google.common.base.Preconditions.checkState;

/* package */
final class System implements com.breadwallet.crypto.System {

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

    private static final BRCryptoListener.SystemEventCallback CWM_LISTENER_SYSTEM_CALLBACK = System::systemEventCallback;
    private static final BRCryptoListener.NetworkEventCallback CWM_LISTENER_NETWORK_CALLBACK = System::networkEventCallback;
    private static final BRCryptoListener.WalletManagerEventCallback CWM_LISTENER_WALLET_MANAGER_CALLBACK = System::walletManagerEventCallback;
    private static final BRCryptoListener.WalletEventCallback CWM_LISTENER_WALLET_CALLBACK = System::walletEventCallback;
    private static final BRCryptoListener.TransferEventCallback CWM_LISTENER_TRANSFER_CALLBACK = System::transferEventCallback;

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
                         com.breadwallet.crypto.Account account,
                         boolean isMainnet,
                         String storagePath,
                         BlockchainDb query) {
        Account cryptoAccount = Account.from(account);

        storagePath = storagePath + (storagePath.endsWith(File.separator) ? "" : File.separator) + cryptoAccount.getFilesystemIdentifier();
        checkState(ensurePath(storagePath));

        Cookie context = new Cookie(SYSTEM_IDS.incrementAndGet());

        BRCryptoListener cwmListener = BRCryptoListener.create(
                context,
                CWM_LISTENER_SYSTEM_CALLBACK,
                CWM_LISTENER_NETWORK_CALLBACK,
                CWM_LISTENER_WALLET_MANAGER_CALLBACK,
                CWM_LISTENER_WALLET_CALLBACK,
                CWM_LISTENER_TRANSFER_CALLBACK);

        BRCryptoClient cwmClient = new BRCryptoClient(
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
    static Optional<com.breadwallet.crypto.blockchaindb.models.bdb.Currency> asBDBCurrency(String uids,
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

        return Optional.of(
                com.breadwallet.crypto.blockchaindb.models.bdb.Currency.create(
                        uids,
                        name,
                        code,
                        type,
                        blockchainId,
                        address.equals("__native__") ? null : address,
                        true,
                        Blockchains.makeCurrencyDemominationsErc20(code, decimals)
                )
        );
    }

    /* package */
    static Optional<byte[]> migrateBRCoreKeyCiphertext(com.breadwallet.crypto.Key key,
                                                       byte[] nonce12,
                                                       byte[] authenticatedData,
                                                       byte[] ciphertext) {
        return Cipher.migrateBRCoreKeyCiphertext(key, nonce12, authenticatedData, ciphertext);
    }

    /* package */
    static void wipe(com.breadwallet.crypto.System system) {
        // Safe the path to the persistent storage
        String storagePath = system.getPath();

        // Destroy the system.
        destroy(system);

        // Clear out persistent storage
        deleteRecursively(storagePath);
    }

    /* package */
    static void wipeAll(String storagePath, List<com.breadwallet.crypto.System> exemptSystems) {
        Set<String> exemptSystemPath = new HashSet<>();
        for (com.breadwallet.crypto.System sys: exemptSystems) {
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

    private static void destroy(com.breadwallet.crypto.System system) {
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

    private static Optional<System> getSystem(Cookie context) {
        return Optional.fromNullable(SYSTEMS_ACTIVE.get(context));
    }

    private static System from(com.breadwallet.crypto.System system) {
        if (system == null) {
            return null;
        }

        if (system instanceof System) {
            return (System) system;
        }

        throw new IllegalArgumentException("Unsupported system instance");
    }

    private final BRCryptoSystem core;
    private final ExecutorService executor;
    private final SystemListener listener;
    private final SystemCallbackCoordinator callbackCoordinator;
    private final Account account;
    private final boolean isMainnet;
    private final String storagePath;
    private final BlockchainDb query;
    private final Cookie context;
    private final BRCryptoListener cwmListener;
    private final BRCryptoClient cwmClient;

    private System(ScheduledExecutorService executor,
                   SystemListener listener,
                   Account account,
                   boolean isMainnet,
                   String storagePath,
                   BlockchainDb query,
                   Cookie context,
                   BRCryptoListener cwmListener,
                   BRCryptoClient cwmClient) {
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

        this.core = BRCryptoSystem.create(
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

    @Override
    public boolean createWalletManager(com.breadwallet.crypto.Network network,
                                       WalletManagerMode mode,
                                       AddressScheme scheme,
                                       Set<com.breadwallet.crypto.Currency> currencies) {
        checkState(network.supportsWalletManagerMode(mode));
        checkState(network.supportsAddressScheme(scheme));

        List<BRCryptoCurrency> currenciesList = new ArrayList<>();
        for (com.breadwallet.crypto.Currency currency : currencies)
            currenciesList.add(Currency.from(currency).getCoreBRCryptoCurrency());

        return core.createManager(core,
                        Network.from(network).getCoreBRCryptoNetwork(),
                        Utilities.walletManagerModeToCrypto(mode),
                        Utilities.addressSchemeToCrypto(scheme),
                        currenciesList)
                .isPresent();
    }

    @Override
    public void wipe(com.breadwallet.crypto.Network network) {
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
    public void updateNetworkFees(@Nullable CompletionHandler<List<com.breadwallet.crypto.Network>, NetworkFeeUpdateError> handler) {
        query.getBlockchains(isMainnet, new CompletionHandler<List<Blockchain>, QueryError>() {
            @Override
            public void handleData(List<Blockchain> blockchainModels) {
                Map<String, Network> networksByUuid = new HashMap<>();
                for (Network network: getNetworks()) networksByUuid.put(network.getUids(), network);

                List<com.breadwallet.crypto.Network> networks = new ArrayList<>();
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
    public <T extends com.breadwallet.crypto.Network> void updateCurrencies(@Nullable CompletionHandler<List<T>, CurrencyUpdateError> handler) {
        query.getCurrencies(isMainnet, new CompletionHandler<List<com.breadwallet.crypto.blockchaindb.models.bdb.Currency>, QueryError>() {
            @Override
            public void handleData(List<com.breadwallet.crypto.blockchaindb.models.bdb.Currency> currencyModels) {
                List<BRCryptoClientCurrencyBundle> bundles = new ArrayList<>();

                for (com.breadwallet.crypto.blockchaindb.models.bdb.Currency currencyModel : currencyModels) {
                    List<BRCryptoClientCurrencyDenominationBundle> denominationBundles = new ArrayList<>();
                    for (CurrencyDenomination currencyDenomination : currencyModel.getDenominations())
                        denominationBundles.add(
                                BRCryptoClientCurrencyDenominationBundle.create(
                                        currencyDenomination.getName(),
                                        currencyDenomination.getCode(),
                                        currencyDenomination.getSymbol(),
                                        currencyDenomination.getDecimals()));

                    bundles.add(BRCryptoClientCurrencyBundle.create(
                            currencyModel.getId(),
                            currencyModel.getName(),
                            currencyModel.getCode(),
                            currencyModel.getType(),
                            currencyModel.getBlockchainId(),
                            currencyModel.getAddressValue(),
                            currencyModel.getVerified(),
                            denominationBundles));
                }

                getCoreBRCryptoSystem().announceCurrencies(bundles);
                for (BRCryptoClientCurrencyBundle bundle : bundles) bundle.release();

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
        for (BRCryptoNetwork coreNetwork: core.getNetworks())
            networks.add (Network.create(coreNetwork));
        return networks;
    }

    private Optional<Network> getNetwork(BRCryptoNetwork coreNetwork) {
        return (core.hasNetwork(coreNetwork)
                ? Optional.of (createNetwork(coreNetwork, true))
                : Optional.absent());
    }

    private Network createNetwork (BRCryptoNetwork coreNetwork, boolean needTake) {
        return Network.create(needTake ? coreNetwork.take() : coreNetwork);
    }

    // WalletManager management

    private UnsignedLong getWalletManagersCount () {
        return core.getManagersCount();
    }

    @Override
    public List<WalletManager> getWalletManagers() {
        List<WalletManager> managers = new ArrayList<>();
        for (BRCryptoWalletManager coreManager: core.getManagers())
            managers.add(createWalletManager(coreManager, false));
        return managers;
    }

    private Optional<WalletManager> getWalletManager(BRCryptoWalletManager coreManager) {
        return (core.hasManager(coreManager)
                ? Optional.of (createWalletManager(coreManager, true))
                : Optional.absent());
    }

    private WalletManager createWalletManager(BRCryptoWalletManager coreWalletManager, boolean needTake) {
        return WalletManager.create(
                coreWalletManager,
                needTake,
                this,
                callbackCoordinator);
    }

    // Miscellaneous

    /* package */
    BlockchainDb getBlockchainDb() {
        return query;
    }

    /* package */
    BRCryptoSystem getCoreBRCryptoSystem() {
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

    private void announceTransferEvent(WalletManager walletManager, Wallet wallet, Transfer transfer, TranferEvent event) {
        executor.submit(() -> listener.handleTransferEvent(this, walletManager, wallet, transfer, event));
    }

    //
    // WalletManager Events
    //

    private static void systemEventCallback(Cookie context,
                                            BRCryptoSystem coreSystem,
                                            BRCryptoSystemEvent event) {
        EXECUTOR_LISTENER.execute(() -> {
            try {
                Log.log(Level.FINE, "SystemEventCallback");

                switch (event.type()) {
                    case CRYPTO_SYSTEM_EVENT_CREATED:
                        handleSystemCreated(context, coreSystem);
                        break;

                    case CRYPTO_SYSTEM_EVENT_CHANGED:
                        handleSystemChanged(context, coreSystem, event);
                        break;

                    case CRYPTO_SYSTEM_EVENT_DELETED:
                        handleSystemDeleted(context, coreSystem);
                        break;

                    case CRYPTO_SYSTEM_EVENT_NETWORK_ADDED:
                        handleSystemNetworkAdded(context, coreSystem, event);
                        break;

                    case CRYPTO_SYSTEM_EVENT_MANAGER_ADDED:
                        handleSystemManagerAdded(context, coreSystem, event);
                        break;

                    case CRYPTO_SYSTEM_EVENT_DISCOVERED_NETWORKS:
                        handleSystemDiscoveredNetworks(context, coreSystem);
                        break;
                }
            } finally {
                coreSystem.give();
            }
        });
    }

    private static void handleSystemCreated(Cookie context, BRCryptoSystem coreSystem) {
        Log.log(Level.FINE, "SystemCreated");

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            system.announceSystemEvent(new SystemCreatedEvent());
        } else {
            Log.log(Level.SEVERE, "SystemCreated: missed system");
        }
    }

    private static void handleSystemChanged(Cookie context, BRCryptoSystem coreSystem, BRCryptoSystemEvent event) {
        SystemState oldState = Utilities.systemStateFromCrypto(event.u.state.oldState());
        SystemState newState = Utilities.systemStateFromCrypto(event.u.state.newState());

        Log.log(Level.FINE, String.format("SystemChanged (%s -> %s)", oldState, newState));

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            system.announceSystemEvent(new SystemChangedEvent(oldState, newState));
        } else {
            Log.log(Level.SEVERE, "SystemChanged: missed system");
        }
    }

    private static void handleSystemDeleted(Cookie context, BRCryptoSystem coreSystem) {
        Log.log(Level.FINE, "System Deleted");

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            system.announceSystemEvent(new SystemDeletedEvent());

        } else {
            Log.log(Level.SEVERE, "SystemCreated: missed system");
        }

    }

    private static void handleSystemNetworkAdded(Cookie context, BRCryptoSystem coreSystem, BRCryptoSystemEvent event) {
        BRCryptoNetwork coreNetwork = event.u.network;

        Log.log(Level.FINE, "System Network Added");

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<Network> optional = system.getNetwork(coreNetwork);
            if (optional.isPresent()) {
                Network network = optional.get();
                system.announceSystemEvent(new SystemNetworkAddedEvent(network));
            } else {
                Log.log(Level.SEVERE, "SystemNetworkAdded: missed network");
            }
        } else {
            Log.log(Level.SEVERE, "SystemNetworkAdded: missed system");
        }
    }

    private static void handleSystemManagerAdded(Cookie context, BRCryptoSystem coreSystem, BRCryptoSystemEvent event) {
        BRCryptoWalletManager coreManagar = event.u.walletManager;

        Log.log(Level.FINE, "System WalletManager Added");

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<WalletManager> optional = system.getWalletManager(coreManagar);
            if (optional.isPresent()) {
                WalletManager manager = optional.get();
                system.announceSystemEvent(new SystemManagerAddedEvent(manager));
            } else {
                Log.log(Level.SEVERE, "SystemManagerAdded: missed manager");
            }
        } else {
            Log.log(Level.SEVERE, "SystemManagerAdded: missed system");
        }
    }


    private static void handleSystemDiscoveredNetworks(Cookie context, BRCryptoSystem coreSystem) {
        Log.log(Level.FINE, "System Discovered Networks");

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            system.announceSystemEvent(new SystemDiscoveredNetworksEvent(system.getNetworks()));

        } else {
            Log.log(Level.SEVERE, "SystemDiscoveredNetworks: missed system");
        }
    }

    private static void networkEventCallback(Cookie context,
                                             BRCryptoNetwork coreNetwork,
                                             BRCryptoNetworkEvent event) {
        EXECUTOR_LISTENER.execute(() -> {
            try {
                Log.log(Level.FINE, "NetworkEventCallback");

                switch (event.type()) {
                    case CRYPTO_NETWORK_EVENT_CREATED:
                        break;
                    case CRYPTO_NETWORK_EVENT_FEES_UPDATED:
                        break;
                    case CRYPTO_NETWORK_EVENT_CURRENCIES_UPDATED:
                        break;
                    case CRYPTO_NETWORK_EVENT_DELETED:
                        break;
                    }
            } finally {
                coreNetwork.give();
            }
        });
    }

    private static void walletManagerEventCallback(Cookie context,
                                                   BRCryptoWalletManager coreWalletManager,
                                                   BRCryptoWalletManagerEvent event) {
        EXECUTOR_LISTENER.execute(() -> {
            try {
                Log.log(Level.FINE, "WalletManagerEventCallback");

                switch (event.type()) {
                    case CRYPTO_WALLET_MANAGER_EVENT_CREATED: {
                        handleWalletManagerCreated(context, coreWalletManager);
                        break;
                    }
                    case CRYPTO_WALLET_MANAGER_EVENT_CHANGED: {
                        handleWalletManagerChanged(context, coreWalletManager, event);
                        break;
                    }
                    case CRYPTO_WALLET_MANAGER_EVENT_DELETED: {
                        handleWalletManagerDeleted(context, coreWalletManager);
                        break;
                    }
                    case CRYPTO_WALLET_MANAGER_EVENT_WALLET_ADDED: {
                        handleWalletManagerWalletAdded(context, coreWalletManager, event);
                        break;
                    }
                    case CRYPTO_WALLET_MANAGER_EVENT_WALLET_CHANGED: {
                        handleWalletManagerWalletChanged(context, coreWalletManager, event);
                        break;
                    }
                    case CRYPTO_WALLET_MANAGER_EVENT_WALLET_DELETED: {
                        handleWalletManagerWalletDeleted(context, coreWalletManager, event);
                        break;
                    }
                    case CRYPTO_WALLET_MANAGER_EVENT_SYNC_STARTED: {
                        handleWalletManagerSyncStarted(context, coreWalletManager);
                        break;
                    }
                    case CRYPTO_WALLET_MANAGER_EVENT_SYNC_CONTINUES: {
                        handleWalletManagerSyncProgress(context, coreWalletManager, event);
                        break;
                    }
                    case CRYPTO_WALLET_MANAGER_EVENT_SYNC_STOPPED: {
                        handleWalletManagerSyncStopped(context, coreWalletManager, event);
                        break;
                    }
                    case CRYPTO_WALLET_MANAGER_EVENT_SYNC_RECOMMENDED: {
                        handleWalletManagerSyncRecommended(context, coreWalletManager, event);
                        break;
                    }
                    case CRYPTO_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED: {
                        handleWalletManagerBlockHeightUpdated(context, coreWalletManager, event);
                        break;
                    }
                }
            } finally {
                coreWalletManager.give();
            }
        });
    }

    private static void handleWalletManagerCreated(Cookie context, BRCryptoWalletManager coreWalletManager) {
        Log.log(Level.FINE, "WalletManagerCreated");

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            WalletManager walletManager = system.createWalletManager (coreWalletManager, true);
            system.announceWalletManagerEvent(walletManager, new WalletManagerCreatedEvent());

        } else {
            Log.log(Level.SEVERE, "WalletManagerCreated: missed system");
        }
    }

    private static void handleWalletManagerChanged(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWalletManagerEvent event) {
        WalletManagerState oldState = Utilities.walletManagerStateFromCrypto(event.u.state.oldValue);
        WalletManagerState newState = Utilities.walletManagerStateFromCrypto(event.u.state.newValue);

        Log.log(Level.FINE, String.format("WalletManagerChanged (%s -> %s)", oldState, newState));

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
            if (optWalletManager.isPresent()) {
                WalletManager walletManager = optWalletManager.get();
                system.announceWalletManagerEvent(walletManager, new WalletManagerChangedEvent(oldState, newState));

            } else {
                Log.log(Level.SEVERE, "WalletManagerChanged: missed wallet manager");
            }

        } else {
            Log.log(Level.SEVERE, "WalletManagerChanged: missed system");
        }
    }

    private static void handleWalletManagerDeleted(Cookie context, BRCryptoWalletManager coreWalletManager) {
        Log.log(Level.FINE, "WalletManagerDeleted");

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
            if (optWalletManager.isPresent()) {
                WalletManager walletManager = optWalletManager.get();
                system.announceWalletManagerEvent(walletManager, new WalletManagerDeletedEvent());

            } else {
                Log.log(Level.SEVERE, "WalletManagerDeleted: missed wallet manager");
            }

        } else {
            Log.log(Level.SEVERE, "WalletManagerDeleted: missed system");
        }
    }

    private static void handleWalletManagerWalletAdded(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWalletManagerEvent event) {
        BRCryptoWallet coreWallet = event.u.wallet;
        try {
            Log.log(Level.FINE, "WalletManagerWalletAdded");

            Optional<System> optSystem = getSystem(context);
            if (optSystem.isPresent()) {
                System system = optSystem.get();

                Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
                if (optWalletManager.isPresent()) {
                    WalletManager walletManager = optWalletManager.get();

                    Optional<Wallet> optional = walletManager.getWallet(coreWallet);
                    if (optional.isPresent()) {
                        Wallet wallet = optional.get();
                        system.announceWalletManagerEvent(walletManager, new WalletManagerWalletAddedEvent(wallet));

                    } else {
                        Log.log(Level.SEVERE, "WalletManagerWalletAdded: missed wallet");
                    }

                } else {
                    Log.log(Level.SEVERE, "WalletManagerWalletAdded: missed wallet manager");
                }

            } else {
                Log.log(Level.SEVERE, "WalletManagerWalletAdded: missed system");
            }

        } finally {
            coreWallet.give();
        }
    }

    private static void handleWalletManagerWalletChanged(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWalletManagerEvent event) {
        BRCryptoWallet coreWallet = event.u.wallet;
        try {
            Log.log(Level.FINE, "WalletManagerWalletChanged");

            Optional<System> optSystem = getSystem(context);
            if (optSystem.isPresent()) {
                System system = optSystem.get();

                Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
                if (optWalletManager.isPresent()) {
                    WalletManager walletManager = optWalletManager.get();

                    Optional<Wallet> optional = walletManager.getWallet(coreWallet);
                    if (optional.isPresent()) {
                        Wallet wallet = optional.get();
                        system.announceWalletManagerEvent(walletManager, new WalletManagerWalletChangedEvent(wallet));

                    } else {
                        Log.log(Level.SEVERE, "WalletManagerWalletChanged: missed wallet");
                    }

                } else {
                    Log.log(Level.SEVERE, "WalletManagerWalletChanged: missed wallet manager");
                }

            } else {
                Log.log(Level.SEVERE, "WalletManagerWalletChanged: missed system");
            }

        } finally {
            coreWallet.give();
        }
    }

    private static void handleWalletManagerWalletDeleted(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWalletManagerEvent event) {
        BRCryptoWallet coreWallet = event.u.wallet;
        try {
            Log.log(Level.FINE, "WalletManagerWalletDeleted");

            Optional<System> optSystem = getSystem(context);
            if (optSystem.isPresent()) {
                System system = optSystem.get();

                Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
                if (optWalletManager.isPresent()) {
                    WalletManager walletManager = optWalletManager.get();

                    Optional<Wallet> optional = walletManager.getWallet(coreWallet);
                    if (optional.isPresent()) {
                        Wallet wallet = optional.get();
                        system.announceWalletManagerEvent(walletManager, new WalletManagerWalletDeletedEvent(wallet));

                    } else {
                        Log.log(Level.SEVERE, "WalletManagerWalletDeleted: missed wallet");
                    }

                } else {
                    Log.log(Level.SEVERE, "WalletManagerWalletDeleted: missed wallet manager");
                }

            } else {
                Log.log(Level.SEVERE, "WalletManagerWalletDeleted: missed system");
            }

        } finally {
            coreWallet.give();
        }
    }

    private static void handleWalletManagerSyncStarted(Cookie context, BRCryptoWalletManager coreWalletManager) {
        Log.log(Level.FINE, "WalletManagerSyncStarted");

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
            if (optWalletManager.isPresent()) {
                WalletManager walletManager = optWalletManager.get();
                system.announceWalletManagerEvent(walletManager, new WalletManagerSyncStartedEvent());

            } else {
                Log.log(Level.SEVERE, "WalletManagerSyncStarted: missed wallet manager");
            }

        } else {
            Log.log(Level.SEVERE, "WalletManagerSyncStarted: missed system");
        }
    }

    private static void handleWalletManagerSyncProgress(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWalletManagerEvent event) {
        float percent = event.u.syncContinues.percentComplete;
        Date timestamp = 0 == event.u.syncContinues.timestamp ? null : new Date(TimeUnit.SECONDS.toMillis(event.u.syncContinues.timestamp));

        Log.log(Level.FINE, String.format("WalletManagerSyncProgress (%s)", percent));

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
            if (optWalletManager.isPresent()) {
                WalletManager walletManager = optWalletManager.get();
                system.announceWalletManagerEvent(walletManager, new WalletManagerSyncProgressEvent(percent, timestamp));

            } else {
                Log.log(Level.SEVERE, "WalletManagerSyncProgress: missed wallet manager");
            }

        } else {
            Log.log(Level.SEVERE, "WalletManagerSyncProgress: missed system");
        }
    }

    private static void handleWalletManagerSyncStopped(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWalletManagerEvent event) {
        WalletManagerSyncStoppedReason reason = Utilities.walletManagerSyncStoppedReasonFromCrypto(event.u.syncStopped.reason);
        Log.log(Level.FINE, String.format("WalletManagerSyncStopped: (%s)", reason));

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
            if (optWalletManager.isPresent()) {
                WalletManager walletManager = optWalletManager.get();
                system.announceWalletManagerEvent(walletManager, new WalletManagerSyncStoppedEvent(reason));

            } else {
                Log.log(Level.SEVERE, "WalletManagerSyncStopped: missed wallet manager");
            }

        } else {
            Log.log(Level.SEVERE, "WalletManagerSyncStopped: missed system");
        }
    }

    private static void handleWalletManagerSyncRecommended(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWalletManagerEvent event) {
        WalletManagerSyncDepth depth = Utilities.syncDepthFromCrypto(event.u.syncRecommended.depth());
        Log.log(Level.FINE, String.format("WalletManagerSyncRecommended: (%s)", depth));

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
            if (optWalletManager.isPresent()) {
                WalletManager walletManager = optWalletManager.get();
                system.announceWalletManagerEvent(walletManager, new WalletManagerSyncRecommendedEvent(depth));

            } else {
                Log.log(Level.SEVERE, "WalletManagerSyncRecommended: missed wallet manager");
            }

        } else {
            Log.log(Level.SEVERE, "WalletManagerSyncRecommended: missed system");
        }
    }

    private static void handleWalletManagerBlockHeightUpdated(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWalletManagerEvent event) {
        UnsignedLong blockHeight = UnsignedLong.fromLongBits(event.u.blockHeight);

        Log.log(Level.FINE, String.format("WalletManagerBlockHeightUpdated (%s)", blockHeight));

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
            if (optWalletManager.isPresent()) {
                WalletManager walletManager = optWalletManager.get();
                system.announceWalletManagerEvent(walletManager, new WalletManagerBlockUpdatedEvent(blockHeight));

            } else {
                Log.log(Level.SEVERE, "WalletManagerBlockHeightUpdated: missed wallet manager");
            }

        } else {
            Log.log(Level.SEVERE, "WalletManagerBlockHeightUpdated: missed system");
        }
    }

    //
    // Wallet Events
    //

    private static void walletEventCallback(Cookie context,
                                            BRCryptoWalletManager coreWalletManager,
                                            BRCryptoWallet coreWallet,
                                            BRCryptoWalletEvent coreEvent) {
        EXECUTOR_LISTENER.execute(() -> {
            try {
                Log.log(Level.FINE, "WalletEventCallback");

                switch (coreEvent.type()) {
                    case CRYPTO_WALLET_EVENT_CREATED: {
                        handleWalletCreated(context, coreWalletManager, coreWallet);
                        break;
                    }
                    case CRYPTO_WALLET_EVENT_CHANGED: {
                        handleWalletChanged(context, coreWalletManager, coreWallet, coreEvent);
                        break;
                    }
                    case CRYPTO_WALLET_EVENT_DELETED: {
                        handleWalletDeleted(context, coreWalletManager, coreWallet);
                        break;
                    }
                    case CRYPTO_WALLET_EVENT_TRANSFER_ADDED: {
                        handleWalletTransferAdded(context, coreWalletManager, coreWallet, coreEvent);
                        break;
                    }
                    case CRYPTO_WALLET_EVENT_TRANSFER_CHANGED: {
                        handleWalletTransferChanged(context, coreWalletManager, coreWallet, coreEvent);
                        break;
                    }
                    case CRYPTO_WALLET_EVENT_TRANSFER_SUBMITTED: {
                        handleWalletTransferSubmitted(context, coreWalletManager, coreWallet, coreEvent);
                        break;
                    }
                    case CRYPTO_WALLET_EVENT_TRANSFER_DELETED: {
                        handleWalletTransferDeleted(context, coreWalletManager, coreWallet, coreEvent);
                        break;
                    }
                    case CRYPTO_WALLET_EVENT_BALANCE_UPDATED: {
                        handleWalletBalanceUpdated(context, coreWalletManager, coreWallet, coreEvent);
                        break;
                    }
                    case CRYPTO_WALLET_EVENT_FEE_BASIS_UPDATED: {
                        handleWalletFeeBasisUpdated(context, coreWalletManager, coreWallet, coreEvent);
                        break;
                    }
                    case CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED: {
                        handleWalletFeeBasisEstimated(context, coreEvent);
                        break;
                    }
                }
            } finally {
                coreEvent.give();
                coreWallet.give();
                coreWalletManager.give();
            }
        });
    }

    private static void handleWalletCreated(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWallet coreWallet) {
        Log.log(Level.FINE, "WalletCreated");

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
            if (optWalletManager.isPresent()) {
                WalletManager walletManager = optWalletManager.get();

                Wallet wallet = walletManager.createWallet(coreWallet);
                system.announceWalletEvent(walletManager, wallet, new WalletCreatedEvent());

            } else {
                Log.log(Level.SEVERE, "WalletCreated: missed wallet manager");
            }

        } else {
            Log.log(Level.SEVERE, "WalletCreated: missed system");
        }
    }

    private static void handleWalletChanged(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWallet coreWallet, BRCryptoWalletEvent event) {
        BRCryptoWalletEvent.States states = event.states();

        WalletState oldState = Utilities.walletStateFromCrypto(states.oldState);
        WalletState newState = Utilities.walletStateFromCrypto(states.newState);

        Log.log(Level.FINE, String.format("WalletChanged (%s -> %s)", oldState, newState));

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
            if (optWalletManager.isPresent()) {
                WalletManager walletManager = optWalletManager.get();

                Optional<Wallet> optWallet = walletManager.getWallet(coreWallet);
                if (optWallet.isPresent()) {
                    Wallet wallet = optWallet.get();
                    system.announceWalletEvent(walletManager, wallet, new WalletChangedEvent(oldState, newState));

                } else {
                    Log.log(Level.SEVERE, "WalletChanged: missed wallet");
                }

            } else {
                Log.log(Level.SEVERE, "WalletChanged: missed wallet manager");
            }

        } else {
            Log.log(Level.SEVERE, "WalletChanged: missed system");
        }
    }

    private static void handleWalletDeleted(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWallet coreWallet) {
        Log.log(Level.FINE, "WalletDeleted");

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
            if (optWalletManager.isPresent()) {
                WalletManager walletManager = optWalletManager.get();

                Optional<Wallet> optWallet = walletManager.getWallet(coreWallet);
                if (optWallet.isPresent()) {
                    Wallet wallet = optWallet.get();
                    system.announceWalletEvent(walletManager, wallet, new WalletDeletedEvent());

                } else {
                    Log.log(Level.SEVERE, "WalletDeleted: missed wallet");
                }

            } else {
                Log.log(Level.SEVERE, "WalletDeleted: missed wallet manager");
            }

        } else {
            Log.log(Level.SEVERE, "WalletDeleted: missed system");
        }
    }

    private static void handleWalletTransferAdded(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWallet coreWallet, BRCryptoWalletEvent event) {
        BRCryptoTransfer coreTransfer = event.transfer();
        try {
            Log.log(Level.FINE, "WalletTransferAdded");

            Optional<System> optSystem = getSystem(context);
            if (optSystem.isPresent()) {
                System system = optSystem.get();

                Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
                if (optWalletManager.isPresent()) {
                    WalletManager walletManager = optWalletManager.get();

                    Optional<Wallet> optWallet = walletManager.getWallet(coreWallet);
                    if (optWallet.isPresent()) {
                        Wallet wallet = optWallet.get();
                        Optional<Transfer> optional = wallet.getTransfer(coreTransfer);

                        if (optional.isPresent()) {
                            Transfer transfer = optional.get();
                            system.announceWalletEvent(walletManager, wallet, new WalletTransferAddedEvent(transfer));

                        } else {
                            Log.log(Level.SEVERE, "WalletTransferAdded: missed transfer");
                        }

                    } else {
                        Log.log(Level.SEVERE, "WalletTransferAdded: missed wallet");
                    }

                } else {
                    Log.log(Level.SEVERE, "WalletTransferAdded: missed wallet manager");
                }

            } else {
                Log.log(Level.SEVERE, "WalletTransferAdded: missed system");
            }
        } finally {
            coreTransfer.give();
        }
    }

    private static void handleWalletTransferChanged(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWallet coreWallet, BRCryptoWalletEvent event) {
        BRCryptoTransfer coreTransfer = event.transfer();
        try {
            Log.log(Level.FINE, "WalletTransferChanged");

            Optional<System> optSystem = getSystem(context);
            if (optSystem.isPresent()) {
                System system = optSystem.get();

                Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
                if (optWalletManager.isPresent()) {
                    WalletManager walletManager = optWalletManager.get();

                    Optional<Wallet> optWallet = walletManager.getWallet(coreWallet);
                    if (optWallet.isPresent()) {
                        Wallet wallet = optWallet.get();
                        Optional<Transfer> optional = wallet.getTransfer(coreTransfer);

                        if (optional.isPresent()) {
                            Transfer transfer = optional.get();
                            system.announceWalletEvent(walletManager, wallet, new WalletTransferChangedEvent(transfer));

                        } else {
                            Log.log(Level.SEVERE, "WalletTransferChanged: missed transfer");
                        }

                    } else {
                        Log.log(Level.SEVERE, "WalletTransferChanged: missed wallet");
                    }

                } else {
                    Log.log(Level.SEVERE, "WalletTransferChanged: missed wallet manager");
                }

            } else {
                Log.log(Level.SEVERE, "WalletTransferChanged: missed system");
            }
        } finally {
            coreTransfer.give();
        }
    }

    private static void handleWalletTransferSubmitted(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWallet coreWallet, BRCryptoWalletEvent event) {
        BRCryptoTransfer coreTransfer = event.transferSubmit();
        try {
            Log.log(Level.FINE, "WalletTransferSubmitted");

            Optional<System> optSystem = getSystem(context);
            if (optSystem.isPresent()) {
                System system = optSystem.get();

                Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
                if (optWalletManager.isPresent()) {
                    WalletManager walletManager = optWalletManager.get();

                    Optional<Wallet> optWallet = walletManager.getWallet(coreWallet);
                    if (optWallet.isPresent()) {
                        Wallet wallet = optWallet.get();
                        Optional<Transfer> optional = wallet.getTransfer(coreTransfer);

                        if (optional.isPresent()) {
                            Transfer transfer = optional.get();
                            system.announceWalletEvent(walletManager, wallet, new WalletTransferSubmittedEvent(transfer));

                        } else {
                            Log.log(Level.SEVERE, "WalletTransferSubmitted: missed transfer");
                        }

                    } else {
                        Log.log(Level.SEVERE, "WalletTransferSubmitted: missed wallet");
                    }

                } else {
                    Log.log(Level.SEVERE, "WalletTransferSubmitted: missed wallet manager");
                }

            } else {
                Log.log(Level.SEVERE, "WalletTransferSubmitted: missed system");
            }
        } finally {
            coreTransfer.give();
        }
    }

    private static void handleWalletTransferDeleted(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWallet coreWallet, BRCryptoWalletEvent event) {
        BRCryptoTransfer coreTransfer = event.transfer();
        try {
            Log.log(Level.FINE, "WalletTransferDeleted");

            Optional<System> optSystem = getSystem(context);
            if (optSystem.isPresent()) {
                System system = optSystem.get();

                Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
                if (optWalletManager.isPresent()) {
                    WalletManager walletManager = optWalletManager.get();

                    Optional<Wallet> optWallet = walletManager.getWallet(coreWallet);
                    if (optWallet.isPresent()) {
                        Wallet wallet = optWallet.get();
                        Optional<Transfer> optional = wallet.getTransfer(coreTransfer);

                        if (optional.isPresent()) {
                            Transfer transfer = optional.get();
                            system.announceWalletEvent(walletManager, wallet, new WalletTransferDeletedEvent(transfer));

                        } else {
                            Log.log(Level.SEVERE, "WalletTransferDeleted: missed transfer");
                        }

                    } else {
                        Log.log(Level.SEVERE, "WalletTransferDeleted: missed wallet");
                    }

                } else {
                    Log.log(Level.SEVERE, "WalletTransferDeleted: missed wallet manager");
                }

            } else {
                Log.log(Level.SEVERE, "WalletTransferDeleted: missed system");
            }
        } finally {
            coreTransfer.give();
        }
    }

    private static void handleWalletBalanceUpdated(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWallet coreWallet, BRCryptoWalletEvent event) {
        Log.log(Level.FINE, "WalletBalanceUpdated");

        Amount amount = Amount.create(event.balance());

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
            if (optWalletManager.isPresent()) {
                WalletManager walletManager = optWalletManager.get();

                Optional<Wallet> optWallet = walletManager.getWallet(coreWallet);
                if (optWallet.isPresent()) {
                    Wallet wallet = optWallet.get();

                    Log.log(Level.FINE, String.format("WalletBalanceUpdated: %s", amount));
                    system.announceWalletEvent(walletManager, wallet, new WalletBalanceUpdatedEvent(amount));

                } else {
                    Log.log(Level.SEVERE, "WalletBalanceUpdated: missed wallet");
                }

            } else {
                Log.log(Level.SEVERE, "WalletBalanceUpdated: missed wallet manager");
            }

        } else {
            Log.log(Level.SEVERE, "WalletBalanceUpdated: missed system");
        }
    }

    private static void handleWalletFeeBasisUpdated(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWallet coreWallet, BRCryptoWalletEvent event) {
        Log.log(Level.FINE, "WalletFeeBasisUpdate");

        TransferFeeBasis feeBasis = TransferFeeBasis.create(event.feeBasisUpdate());

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
            if (optWalletManager.isPresent()) {
                WalletManager walletManager = optWalletManager.get();

                Optional<Wallet> optWallet = walletManager.getWallet(coreWallet);
                if (optWallet.isPresent()) {
                    Wallet wallet = optWallet.get();

                    Log.log(Level.FINE, String.format("WalletFeeBasisUpdate: %s", feeBasis));
                    system.announceWalletEvent(walletManager, wallet, new WalletFeeBasisUpdatedEvent(feeBasis));

                } else {
                    Log.log(Level.SEVERE, "WalletFeeBasisUpdate: missed wallet");
                }

            } else {
                Log.log(Level.SEVERE, "WalletFeeBasisUpdate: missed wallet manager");
            }

        } else {
            Log.log(Level.SEVERE, "WalletFeeBasisUpdate: missed system");
        }
    }

    private static void handleWalletFeeBasisEstimated(Cookie context, BRCryptoWalletEvent event) {
        BRCryptoWalletEvent.FeeBasisEstimate estimate = event.feeBasisEstimate();
        // estimate.basis needs to be given

        Log.log(Level.FINE, String.format("WalletFeeBasisEstimated (%s)", estimate.status));

        boolean success = estimate.status == BRCryptoStatus.CRYPTO_SUCCESS;
        TransferFeeBasis feeBasis = success ? TransferFeeBasis.create(estimate.basis) : null;
        if (null == feeBasis) estimate.basis.give();

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();
            Cookie opCookie = new Cookie(estimate.cookie);

            if (success) {
                Log.log(Level.FINE, String.format("WalletFeeBasisEstimated: %s", feeBasis));
                system.callbackCoordinator.completeFeeBasisEstimateHandlerWithSuccess(opCookie, feeBasis);
            } else {
                FeeEstimationError error = Utilities.feeEstimationErrorFromStatus(estimate.status);
                Log.log(Level.FINE, String.format("WalletFeeBasisEstimated: %s", error));
                system.callbackCoordinator.completeFeeBasisEstimateHandlerWithError(opCookie, error);
            }

        } else {
            Log.log(Level.SEVERE, "WalletFeeBasisEstimated: missed system");
        }
    }

    //
    // Transfer Events
    //

    private static void transferEventCallback(Cookie context,
                                              BRCryptoWalletManager coreWalletManager,
                                              BRCryptoWallet coreWallet,
                                              BRCryptoTransfer coreTransfer,
                                              BRCryptoTransferEvent event) {
        EXECUTOR_LISTENER.execute(() -> {
            try {
                Log.log(Level.FINE, "TransferEventCallback");

                switch (event.type()) {
                    case CRYPTO_TRANSFER_EVENT_CREATED: {
                        handleTransferCreated(context, coreWalletManager, coreWallet, coreTransfer);
                        break;
                    }
                    case CRYPTO_TRANSFER_EVENT_CHANGED: {
                        handleTransferChanged(context, coreWalletManager, coreWallet, coreTransfer, event);
                        break;
                    }
                    case CRYPTO_TRANSFER_EVENT_DELETED: {
                        handleTransferDeleted(context, coreWalletManager, coreWallet, coreTransfer);
                        break;
                    }
                }
            } finally {
                if (CRYPTO_TRANSFER_EVENT_CHANGED == event.type()) {
                    event.u.state.oldState.give();
                    event.u.state.newState.give();
                }
                coreTransfer.give();
                coreWallet.give();
                coreWalletManager.give();
            }
        });
    }

    private static void handleTransferCreated(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWallet coreWallet, BRCryptoTransfer coreTransfer) {
        Log.log(Level.FINE, "TransferCreated");

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
            if (optWalletManager.isPresent()) {
                WalletManager walletManager = optWalletManager.get();

                Optional<Wallet> optWallet = walletManager.getWallet(coreWallet);
                if (optWallet.isPresent()) {
                    Wallet wallet = optWallet.get();

                    Transfer transfer = wallet.createTransfer(coreTransfer);
                    system.announceTransferEvent(walletManager, wallet, transfer, new TransferCreatedEvent());

                } else {
                    Log.log(Level.SEVERE, "TransferCreated: missed wallet");
                }

            } else {
                Log.log(Level.SEVERE, "TransferCreated: missed wallet manager");
            }

        } else {
            Log.log(Level.SEVERE, "TransferCreated: missed system");
        }
    }

    private static void handleTransferChanged(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWallet coreWallet, BRCryptoTransfer coreTransfer,
                                       BRCryptoTransferEvent event) {
        TransferState oldState = Utilities.transferStateFromCrypto(event.u.state.oldState);
        TransferState newState = Utilities.transferStateFromCrypto(event.u.state.newState);

        Log.log(Level.FINE, String.format("TransferChanged (%s -> %s)", oldState, newState));

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
            if (optWalletManager.isPresent()) {
                WalletManager walletManager = optWalletManager.get();

                Optional<Wallet> optWallet = walletManager.getWallet(coreWallet);
                if (optWallet.isPresent()) {
                    Wallet wallet = optWallet.get();

                    Optional<Transfer> optTransfer = wallet.getTransfer(coreTransfer);
                    if (optTransfer.isPresent()) {
                        Transfer transfer = optTransfer.get();

                        system.announceTransferEvent(walletManager, wallet, transfer, new TransferChangedEvent(oldState, newState));

                    } else {
                        Log.log(Level.SEVERE, "TransferChanged: missed transfer");
                    }

                } else {
                    Log.log(Level.SEVERE, "TransferChanged: missed wallet");
                }

            } else {
                Log.log(Level.SEVERE, "TransferChanged: missed wallet manager");
            }

        } else {
            Log.log(Level.SEVERE, "TransferChanged: missed system");
        }
    }

    private static void handleTransferDeleted(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoWallet coreWallet, BRCryptoTransfer coreTransfer) {
        Log.log(Level.FINE, "TransferDeleted");

        Optional<System> optSystem = getSystem(context);
        if (optSystem.isPresent()) {
            System system = optSystem.get();

            Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
            if (optWalletManager.isPresent()) {
                WalletManager walletManager = optWalletManager.get();

                Optional<Wallet> optWallet = walletManager.getWallet(coreWallet);
                if (optWallet.isPresent()) {
                    Wallet wallet = optWallet.get();

                    Optional<Transfer> optTransfer = wallet.getTransfer(coreTransfer);
                    if (optTransfer.isPresent()) {
                        Transfer transfer = optTransfer.get();
                        system.announceTransferEvent(walletManager, wallet, transfer, new TransferDeletedEvent());

                    } else {
                        Log.log(Level.SEVERE, "TransferDeleted: missed transfer");
                    }

                } else {
                    Log.log(Level.SEVERE, "TransferDeleted: missed wallet");
                }

            } else {
                Log.log(Level.SEVERE, "TransferDeleted: missed wallet manager");
            }

        } else {
            Log.log(Level.SEVERE, "TransferDeleted: missed system");
        }
    }

    // BTC client

    private static void getBlockNumber(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoClientCallbackState callbackState) {
        EXECUTOR_CLIENT.execute(() -> {
            try {
                Log.log(Level.FINE, "BRCryptoCWMGetBlockNumberCallback");

                Optional<System> optSystem = getSystem(context);
                if (optSystem.isPresent()) {
                    System system = optSystem.get();

                    Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
                    if (optWalletManager.isPresent()) {
                        WalletManager walletManager = optWalletManager.get();

                        system.query.getBlockchain(walletManager.getNetwork().getUids(), new CompletionHandler<Blockchain, QueryError>() {
                            @Override
                            public void handleData(Blockchain blockchain) {
                                Optional<UnsignedLong> maybeBlockHeight = blockchain.getBlockHeight();
                                Optional<String> maybeVerifiedBlockHash = blockchain.getVerifiedBlockHash();
                                if (maybeBlockHeight.isPresent() && maybeVerifiedBlockHash.isPresent()) {
                                    UnsignedLong blockchainHeight = maybeBlockHeight.get();
                                    String verifiedBlockHash = maybeVerifiedBlockHash.get();
                                    Log.log(Level.FINE, String.format("BRCryptoCWMGetBlockNumberCallback: succeeded (%s, %s)", blockchainHeight, verifiedBlockHash));
                                    walletManager.getCoreBRCryptoWalletManager().announceGetBlockNumber(callbackState, true, blockchainHeight, verifiedBlockHash);
                                } else {
                                    Log.log(Level.SEVERE, "BRCryptoCWMGetBlockNumberCallback: failed with missing block height");
                                    walletManager.getCoreBRCryptoWalletManager().announceGetBlockNumber(callbackState, false, UnsignedLong.ZERO, "");
                                }
                            }

                            @Override
                            public void handleError(QueryError error) {
                                Log.log(Level.SEVERE, "BRCryptoCWMGetBlockNumberCallback: failed", error);
                                walletManager.getCoreBRCryptoWalletManager().announceGetBlockNumber(callbackState, false, UnsignedLong.ZERO, "");
                            }
                        });
                    } else {
                        throw new IllegalStateException("BRCryptoCWMGetBlockNumberCallback: missing manager");
                    }

                } else {
                    throw new IllegalStateException("BRCryptoCWMGetBlockNumberCallback: missing system");
                }
            } catch (RuntimeException e) {
                Log.log(Level.SEVERE, e.getMessage());
                coreWalletManager.announceGetBlockNumber(callbackState, false, UnsignedLong.ZERO, "");
            } finally {
                coreWalletManager.give();
            }
        });
    }

    private static BRCryptoTransferStateType getTransferStatus (String apiStatus) {
        switch (apiStatus) {
            case "confirmed":
                return BRCryptoTransferStateType.CRYPTO_TRANSFER_STATE_INCLUDED;
            case "submitted":
            case "reverted":
                return BRCryptoTransferStateType.CRYPTO_TRANSFER_STATE_SUBMITTED;
            case "failed":
            case "rejected":
                return BRCryptoTransferStateType.CRYPTO_TRANSFER_STATE_ERRORED;
            default:
                // throw new IllegalArgumentException("Unexpected API Status of " + apiStatus);
                return BRCryptoTransferStateType.CRYPTO_TRANSFER_STATE_DELETED;
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

    private static Optional<BRCryptoClientTransactionBundle> makeTransactionBundle(Transaction transaction) {
        Optional<byte[]> optRaw = transaction.getRaw();
        if (!optRaw.isPresent()) {
            Log.log(Level.SEVERE, "BRCryptoCWMGetTransactionsCallback completing with missing raw bytes");
            return Optional.absent();
        }
        UnsignedLong blockHeight =
                transaction.getBlockHeight().or(BRConstants.BLOCK_HEIGHT_UNBOUND);
        UnsignedLong timestamp =
                transaction.getTimestamp().transform(Utilities::dateAsUnixTimestamp).or(UnsignedLong.ZERO);

        BRCryptoTransferStateType status = getTransferStatus(transaction.getStatus());

        if (status != BRCryptoTransferStateType.CRYPTO_TRANSFER_STATE_DELETED) {
            Log.log(Level.FINE,"BRCryptoCWMGetTransactionsCallback announcing " + transaction.getId());
        } else {
            Log.log(Level.SEVERE,"BRCryptoCWMGetTransactionsCallback received an unknown status, completing with failure");
            return Optional.absent();
        }

        return Optional.of(BRCryptoClientTransactionBundle.create(
                status,
                optRaw.get(),
                timestamp,
                blockHeight));
    }

     private static void getTransactions(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoClientCallbackState callbackState,
                                        List<String> addresses, long begBlockNumber, long endBlockNumber) {
        EXECUTOR_CLIENT.execute(() -> {
            try {
                UnsignedLong begBlockNumberUnsigned = UnsignedLong.fromLongBits(begBlockNumber);
                UnsignedLong endBlockNumberUnsigned = UnsignedLong.fromLongBits(endBlockNumber);

                Log.log(Level.FINE, String.format("BRCryptoCWMGetTransactionsCallback (%s -> %s)", begBlockNumberUnsigned, endBlockNumberUnsigned));

                Optional<System> optSystem = getSystem(context);
                if (optSystem.isPresent()) {
                    System system = optSystem.get();

                    Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
                    if (optWalletManager.isPresent()) {
                        WalletManager walletManager = optWalletManager.get();

                        final List<String> canonicalAddresses = canonicalAddresses(addresses, walletManager.getNetwork().getType());

                        system.query.getTransactions(walletManager.getNetwork().getUids(),
                                canonicalAddresses,
                                begBlockNumberUnsigned.equals(BRConstants.BLOCK_HEIGHT_UNBOUND) ? null : begBlockNumberUnsigned,
                                endBlockNumberUnsigned.equals(BRConstants.BLOCK_HEIGHT_UNBOUND) ? null : endBlockNumberUnsigned,
                                true,
                                false,
                                false,
                                new CompletionHandler<List<Transaction>, QueryError>() {
                                    @Override
                                    public void handleData(List<Transaction> transactions) {
                                        boolean success = false;
                                        Log.log(Level.FINE, "BRCryptoCWMGetTransactionsCallback received transactions");

                                        List<BRCryptoClientTransactionBundle> bundles = new ArrayList<>();
                                        for (Transaction transaction : transactions) {
                                            Optional<BRCryptoClientTransactionBundle> bundle = makeTransactionBundle(transaction);
                                            if (bundle.isPresent()) {
                                                bundles.add(bundle.get());
                                            }
                                        }
                                        walletManager.getCoreBRCryptoWalletManager().announceTransactions(callbackState, true, bundles);

                                        success = true;
                                        Log.log(Level.FINE, "BRCryptoCWMGetTransactionsCallback: complete");
                                    }

                                    @Override
                                    public void handleError(QueryError error) {
                                        Log.log(Level.SEVERE, "BRCryptoCWMGetTransactionsCallback received an error, completing with failure: ", error);
                                        walletManager.getCoreBRCryptoWalletManager().announceTransactions(callbackState, false, new ArrayList<>());

                                    }
                                });

                    } else {
                        throw new IllegalStateException("BRCryptoCWMGetTransactionsCallback: missing manager");
                    }

                } else {
                    throw new IllegalStateException("BRCryptoCWMGetTransactionsCallback: missing system");
                }
            } catch (RuntimeException e) {
                Log.log(Level.SEVERE, e.getMessage());
                coreWalletManager.announceTransactions(callbackState, false, new ArrayList<>());
            } finally {
                coreWalletManager.give();
            }
        });
    }

    private static List<BRCryptoClientTransferBundle> makeTransferBundles (Transaction transaction, List<String> addresses) {
        List<BRCryptoClientTransferBundle> result = new ArrayList<>();

        UnsignedLong blockHeight    = transaction.getBlockHeight().or(BRConstants.BLOCK_HEIGHT_UNBOUND);
        UnsignedLong blockTimestamp = transaction.getTimestamp().transform(Utilities::dateAsUnixTimestamp).or(UnsignedLong.ZERO);
        UnsignedLong blockConfirmations = transaction.getConfirmations().or(UnsignedLong.ZERO);
        UnsignedLong blockTransactionIndex = transaction.getIndex().or(UnsignedLong.ZERO);
        String blockHash = transaction.getHash();

        BRCryptoTransferStateType status = getTransferStatus (transaction.getStatus());

        for (ObjectPair<com.breadwallet.crypto.blockchaindb.models.bdb.Transfer, String> o : System.mergeTransfers(transaction, addresses)) {
            Log.log(Level.FINE, "BRCryptoCWMGetTransfersCallback  announcing " + o.o1.getId());

            // Merge Transfer 'meta' into Transaction' meta; duplicates from Transfer
            Map<String,String> meta = new HashMap<>(transaction.getMeta());
            meta.putAll(o.o1.getMeta());

            result.add (BRCryptoClientTransferBundle.create(
                    status,
                    o.o1.getId(),
                    transaction.getHash(),
                    transaction.getIdentifier(),
                    o.o1.getFromAddress().orNull(),
                    o.o1.getToAddress().orNull(),
                    o.o1.getAmount().getAmount(),
                    o.o1.getAmount().getCurrencyId(),
                    o.o2,
                    blockTimestamp,
                    blockHeight,
                    blockConfirmations,
                    blockTransactionIndex,
                    blockHash,
                    meta));
        }

        return result;
    }

    private static void getTransfers(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoClientCallbackState callbackState,
                                     List<String> addresses, long begBlockNumber, long endBlockNumber) {
        EXECUTOR_CLIENT.execute(() -> {
            try {
                UnsignedLong begBlockNumberUnsigned = UnsignedLong.fromLongBits(begBlockNumber);
                UnsignedLong endBlockNumberUnsigned = UnsignedLong.fromLongBits(endBlockNumber);

                Log.log(Level.FINE, String.format("BRCryptoCWMGetTransfersCallback (%s -> %s)", begBlockNumberUnsigned, endBlockNumberUnsigned));

                Optional<System> optSystem = getSystem(context);
                if (optSystem.isPresent()) {
                    System system = optSystem.get();

                    Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
                    if (optWalletManager.isPresent()) {
                        WalletManager walletManager = optWalletManager.get();

                        final List<String> canonicalAddresses = canonicalAddresses(addresses, walletManager.getNetwork().getType());

                        system.query.getTransactions(walletManager.getNetwork().getUids(), canonicalAddresses,
                                begBlockNumberUnsigned.equals(BRConstants.BLOCK_HEIGHT_UNBOUND) ? null : begBlockNumberUnsigned,
                                endBlockNumberUnsigned.equals(BRConstants.BLOCK_HEIGHT_UNBOUND) ? null : endBlockNumberUnsigned,
                                false,
                                false,
                                true,
                                new CompletionHandler<List<Transaction>, QueryError>() {
                                    @Override
                                    public void handleData(List<Transaction> transactions) {
                                        boolean success = false;
                                        Log.log(Level.FINE, "BRCryptoCWMGetTransfersCallback received transfers");

                                        List<BRCryptoClientTransferBundle> bundles = new ArrayList<>();

                                        try {
                                            for (Transaction transaction : transactions) {
                                                bundles.addAll(makeTransferBundles(transaction, canonicalAddresses));
                                             }

                                            success = true;
                                            Log.log(Level.FINE, "BRCryptoCWMGetTransfersCallback : complete");
                                        } finally {
                                            walletManager.getCoreBRCryptoWalletManager().announceTransfers(callbackState, true, bundles);
                                        }
                                    }

                                    @Override
                                    public void handleError(QueryError error) {
                                        Log.log(Level.SEVERE, "BRCryptoCWMGetTransfersCallback  received an error, completing with failure: ", error);
                                        walletManager.getCoreBRCryptoWalletManager().announceTransfers(callbackState, false, new ArrayList<>());
                                    }
                                });
                    } else {
                        throw new IllegalStateException("BRCryptoCWMGetTransfersCallback : missing manager");
                    }

                } else {
                    throw new IllegalStateException("BRCryptoCWMGetTransfersCallback : missing system");
                }
            } catch (RuntimeException e) {
                Log.log(Level.SEVERE, e.getMessage());
                coreWalletManager.announceTransfers(callbackState,false, new ArrayList<>());
            } finally {
                coreWalletManager.give();
            }
        });
    }

    private static void submitTransaction(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoClientCallbackState callbackState,
                                          String identifier,
                                          byte[] transaction) {
        EXECUTOR_CLIENT.execute(() -> {
            try {
                Log.log(Level.FINE, "BRCryptoCWMSubmitTransactionCallback");

                Optional<System> optSystem = getSystem(context);
                if (optSystem.isPresent()) {
                    System system = optSystem.get();

                    Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
                    if (optWalletManager.isPresent()) {
                        WalletManager walletManager = optWalletManager.get();

                        system.query.createTransaction(walletManager.getNetwork().getUids(), transaction, identifier,
                                new CompletionHandler<TransactionIdentifier, QueryError>() {
                                    @Override
                                    public void handleData(TransactionIdentifier tid) {
                                        Log.log(Level.FINE, "BRCryptoCWMSubmitTransactionCallback: succeeded");
                                        walletManager.getCoreBRCryptoWalletManager().announceSubmitTransfer(callbackState, tid.getIdentifier(), tid.getHash().orNull(), true);
                                    }

                                    @Override
                                    public void handleError(QueryError error) {
                                        Log.log(Level.SEVERE, "BRCryptoCWMSubmitTransactionCallback: failed", error);
                                        walletManager.getCoreBRCryptoWalletManager().announceSubmitTransfer(callbackState, null, null, false);
                                    }
                                });

                    } else {
                        throw new IllegalStateException("BRCryptoCWMSubmitTransactionCallback: missing manager");
                    }

                } else {
                    throw new IllegalStateException("BRCryptoCWMSubmitTransactionCallback: missing system");
                }
            } catch (RuntimeException e) {
                Log.log(Level.SEVERE, e.getMessage());
                coreWalletManager.announceSubmitTransfer(callbackState, null, null, false);
            } finally {
                coreWalletManager.give();
            }
        });
    }

    private static void estimateTransactionFee(Cookie context, BRCryptoWalletManager coreWalletManager, BRCryptoClientCallbackState callbackState,
                                               byte[] transaction) {
        EXECUTOR_CLIENT.execute(() -> {
            try {
                Log.log(Level.FINE, "BRCryptoCWMEstimateTransactionFeeCallback");

                Optional<System> optSystem = getSystem(context);
                if (optSystem.isPresent()) {
                    System system = optSystem.get();

                    Optional<WalletManager> optWalletManager = system.getWalletManager(coreWalletManager);
                    if (optWalletManager.isPresent()) {
                        WalletManager walletManager = optWalletManager.get();

                        system.query.estimateTransactionFee(walletManager.getNetwork().getUids(), transaction, new CompletionHandler<TransactionFee, QueryError>() {
                            @Override
                            public void handleData(TransactionFee fee) {
                                Log.log(Level.FINE, "BRCryptoCWMEstimateTransactionFeeCallback: succeeded");
                                walletManager.getCoreBRCryptoWalletManager().announceEstimateTransactionFee(callbackState, true, fee.getCostUnits(), fee.getMeta());
                            }

                            @Override
                            public void handleError(QueryError error) {
                                Log.log(Level.SEVERE, "BRCryptoCWMEstimateTransactionFeeCallback: failed", error);
                                walletManager.getCoreBRCryptoWalletManager().announceEstimateTransactionFee(callbackState, false, UnsignedLong.ZERO, new ArrayMap<>());
                            }
                        });
                    } else {
                        throw new IllegalStateException("BRCryptoCWMEstimateTransactionFeeCallback: missing manager");
                    }

                } else {
                    throw new IllegalStateException("BRCryptoCWMEstimateTransactionFeeCallback: missing system");
                }
            } catch (RuntimeException e) {
                Log.log(Level.SEVERE, e.getMessage());
                coreWalletManager.announceEstimateTransactionFee(callbackState, false, UnsignedLong.ZERO, new ArrayMap<>());
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

    private static List<ObjectPair<com.breadwallet.crypto.blockchaindb.models.bdb.Transfer, String>> mergeTransfers(Transaction transaction, List<String> addresses) {
        List<com.breadwallet.crypto.blockchaindb.models.bdb.Transfer> transfers;
        List<com.breadwallet.crypto.blockchaindb.models.bdb.Transfer> transfersWithFee;
        List<com.breadwallet.crypto.blockchaindb.models.bdb.Transfer> transfersWithoutFee;
        List<ObjectPair<com.breadwallet.crypto.blockchaindb.models.bdb.Transfer, String>> transfersMerged;
        com.breadwallet.crypto.blockchaindb.models.bdb.Transfer transferWithFee;

        // Only consider transfers w/ `address`
        transfers = new ArrayList<>(Collections2.filter(transaction.getTransfers(),
                t -> addresses.contains(t.getFromAddress().orNull()) ||
                        addresses.contains(t.getToAddress().orNull())));

        // Note for later: all transfers have a unique id

        transfersWithFee = new ArrayList<>(Collections2.filter(transfers, t -> "__fee__".equals(t.getToAddress().orNull())));
        transfersWithoutFee = new ArrayList<>(Collections2.filter(transfers, t -> !"__fee__".equals(t.getToAddress().orNull())));

        // Get the transferWithFee if we have one
        checkState(transfersWithFee.size() <= 1);
        transferWithFee = transfersWithFee.isEmpty() ? null : transfersWithFee.get(0);

        transfersMerged = new ArrayList<>(transfers.size());

        // There is no "__fee__" entry
        if (transferWithFee == null) {
            // Announce transfers with no fee
            for (com.breadwallet.crypto.blockchaindb.models.bdb.Transfer transfer: transfers) {
                transfersMerged.add(new ObjectPair<>(transfer, null));
            }

        // There is a single "__fee__" entry, due to `checkState(transfersWithFee.size() <= 1)` above
        } else {
            // We may or may not have a non-fee transfer matching `transferWithFee`.  We
            // may or may not have more than one non-fee transfers matching `transferWithFee`

            // Find the first of the non-fee transfers matching `transferWithFee` that also matches
            // the amount's currency.
            com.breadwallet.crypto.blockchaindb.models.bdb.Transfer transferMatchingFee = null;
            for (com.breadwallet.crypto.blockchaindb.models.bdb.Transfer transfer: transfersWithoutFee) {
                if (transferWithFee.getTransactionId().equals(transfer.getTransactionId()) &&
                    transferWithFee.getFromAddress().equals(transfer.getFromAddress()) &&
                    transferWithFee.getAmount().getCurrencyId().equals(transfer.getAmount().getCurrencyId())) {
                    transferMatchingFee = transfer;
                    break;
                }
            }

            // If there is still no `transferWithFee`, find the first w/o matching the amount's currency
            if (null == transferMatchingFee)
                for (com.breadwallet.crypto.blockchaindb.models.bdb.Transfer transfer : transfersWithoutFee) {
                    if (transferWithFee.getTransactionId().equals(transfer.getTransactionId()) &&
                            transferWithFee.getFromAddress().equals(transfer.getFromAddress())) {
                        transferMatchingFee = transfer;
                        break;
                    }
                }

            // We must have a transferMatchingFee; if we don't add one
            transfers = new ArrayList<>(transfersWithoutFee);
            if (null == transferMatchingFee) {
                transfers.add(
                        com.breadwallet.crypto.blockchaindb.models.bdb.Transfer.create(
                                transferWithFee.getId(),
                                transferWithFee.getBlockchainId(),
                                transferWithFee.getIndex(),
                                com.breadwallet.crypto.blockchaindb.models.bdb.Amount.create(transferWithFee.getAmount().getCurrencyId(), "0"),
                                transferWithFee.getMeta(),
                                transferWithFee.getFromAddress().orNull(),
                                "unknown",
                                transferWithFee.getTransactionId().or("0"),
                                transferWithFee.getAcknowledgements().orNull())
                );
            }

            // Hold the Id for the transfer that we'll add a fee to.
            String transferForFeeId = transferMatchingFee != null ? transferMatchingFee.getId() : transferWithFee.getId();

            // Announce transfers adding the fee to the `transferforFeeId`
            for (com.breadwallet.crypto.blockchaindb.models.bdb.Transfer transfer: transfers) {
                String fee = transfer.getId().equals(transferForFeeId) ? transferWithFee.getAmount().getAmount() : null;

                transfersMerged.add(new ObjectPair<>(transfer, fee));
            }
        }

        return transfersMerged;
    }

    @Override
    public boolean accountIsInitialized(com.breadwallet.crypto.Account account, com.breadwallet.crypto.Network network) {
        return account.isInitialized(network);
    }

    @Override
    public void accountInitialize(com.breadwallet.crypto.Account account,
                                  com.breadwallet.crypto.Network network,
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
                            .transform((data) -> Coder.createForAlgorithm(com.breadwallet.crypto.Coder.Algorithm.HEX).encode(data))
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
                                            accounts.get(0).getAccountId(),
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
    public Optional<byte[]> accountInitializeUsingData(com.breadwallet.crypto.Account account, com.breadwallet.crypto.Network network, byte[] data) {
        return Optional.fromNullable (account.initialize (network, data));
    }

    @Override
    public Optional<byte[]> accountInitializeUsingHedera(com.breadwallet.crypto.Account account, com.breadwallet.crypto.Network network, HederaAccount hedera) {
        return Optional.fromNullable (account.initialize (network, hedera.getAccountId().getBytes()));
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
