/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.demo;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.os.StrictMode;

import com.blockset.walletkit.brd.ApiProvider;
import com.blockset.walletkit.Account;
import com.blockset.walletkit.Api;
import com.blockset.walletkit.DispatchingSystemListener;
import com.blockset.walletkit.WalletManagerMode;
import com.blockset.walletkit.SystemClient;
import com.blockset.walletkit.brd.systemclient.BlocksetSystemClient;
import com.blockset.walletkit.System;
import com.blockset.walletkit.utility.TestConfiguration;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.logging.Level;
import java.util.logging.Logger;

import okhttp3.OkHttpClient;

import static com.google.common.base.Preconditions.checkState;

public class DemoApplication extends Application {

    private static final Logger Log = Logger.getLogger(DemoApplication.class.getName());

    private static final String EXTRA_TIMESTAMP = "TIMESTAMP";
    private static final String EXTRA_PAPER_KEY = "PAPER_KEY";
    private static final String EXTRA_MODE = "MODE";
    private static final String EXTRA_WIPE = "WIPE";
    private static final String EXTRA_IS_MAINNET = "IS_MAINNET";

    private static final boolean DEFAULT_IS_MAINNET = true;
    private static final boolean DEFAULT_WIPE = true;
    private static final long DEFAULT_TIMESTAMP = 0;
    private static final String DEFAULT_PAPER_KEY = "boring head harsh green empty clip fatal typical found crane dinner timber";
    private static final WalletManagerMode DEFAULT_MODE = WalletManagerMode.API_ONLY;

    private static DemoApplication instance;

    private System system;
    private Account account;
    private DispatchingSystemListener systemListener;
    private ConnectivityBroadcastReceiver connectivityReceiver;
    private ScheduledExecutorService systemExecutor;
    private boolean isMainnet;
    private SystemClient blockchainDb;
    private byte[] paperKey;
    private File storageFile;

    private AtomicBoolean runOnce = new AtomicBoolean(false);

    private TestConfiguration testConfiguration;

    public static void initialize(Activity launchingActivity) {
        instance.initFromLaunchIntent(launchingActivity.getIntent());
    }

    public static Context getContext() {
        return instance.getApplicationContext();
    }

    public static System getSystem() {
        checkState(null != instance && instance.runOnce.get());

        return instance.system;
    }

    public static byte[] getPaperKey() {
        checkState(null != instance && instance.runOnce.get());

        return instance.paperKey;
    }

    public static DispatchingSystemListener getDispatchingSystemListener() {
        checkState(null != instance && instance.runOnce.get());

        return instance.systemListener;
    }

    public static void wipeSystem() {
        checkState(null != instance && instance.runOnce.get());

        instance.wipeSystemImpl();
    }

    @Override
    public void onCreate() {
        super.onCreate();
        instance = this;
        this.testConfiguration = getTestConfiguration();
        StrictMode.enableDefaults();
    }

    private void initFromLaunchIntent(Intent intent) {
        if (!runOnce.getAndSet(true)) {
            Logging.initialize(Level.FINE);

            Api.initialize(ApiProvider.getInstance());

            String paperKeyString = (intent.hasExtra(EXTRA_PAPER_KEY) ? intent.getStringExtra(EXTRA_PAPER_KEY) : DEFAULT_PAPER_KEY);
            paperKey = paperKeyString.getBytes(StandardCharsets.UTF_8);

            isMainnet = intent.getBooleanExtra(EXTRA_IS_MAINNET, DEFAULT_IS_MAINNET);

            long timestamp = intent.getLongExtra(EXTRA_TIMESTAMP, DEFAULT_TIMESTAMP);
            WalletManagerMode mode = intent.hasExtra(EXTRA_MODE) ? WalletManagerMode.valueOf(intent.getStringExtra(EXTRA_MODE)) : DEFAULT_MODE;
            boolean wipe = intent.getBooleanExtra(EXTRA_WIPE, DEFAULT_WIPE);

            systemExecutor = Executors.newSingleThreadScheduledExecutor();

            storageFile = new File(getFilesDir(), "core");
            if (wipe) System.wipeAll(storageFile.getAbsolutePath(), Collections.emptyList());
            if (!storageFile.exists()) checkState(storageFile.mkdirs());

            Log.log(Level.FINE, String.format("Account PaperKey:  %s", paperKeyString));
            Log.log(Level.FINE, String.format("Account Timestamp: %s", timestamp));
            Log.log(Level.FINE, String.format("StoragePath:       %s", storageFile.getAbsolutePath()));
            Log.log(Level.FINE, String.format("Mainnet:           %s", isMainnet));

            List<String> currencyCodesNeeded = Arrays.asList(
                    "btc",
                    "bch",
                    "bsv",
                    "eth",
                    "xrp",
                    "hbar",
                    "xtz",
                    "xlm",
                    "ignore - end-of-array, w/o comma"
            );
            systemListener = new DispatchingSystemListener();
            systemListener.addSystemListener(new DemoSystemListener(mode, isMainnet, currencyCodesNeeded));

            String uids = UUID.randomUUID().toString();
            account = Account.createFromPhrase(paperKey, new Date(TimeUnit.SECONDS.toMillis(timestamp)), uids).get();

            blockchainDb = BlocksetSystemClient.createForTest (new OkHttpClient(),
                    testConfiguration.getBlocksetAccess().getToken(),
                    testConfiguration.getBlocksetAccess().getBaseURL());
            system = System.create(systemExecutor, systemListener, account,
                    isMainnet, storageFile.getAbsolutePath(), blockchainDb);
            system.configure();

            System.wipeAll(storageFile.getAbsolutePath(), Collections.singletonList(system));

            connectivityReceiver = new ConnectivityBroadcastReceiver();
            registerReceiver(connectivityReceiver , new IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION));
        }
    }

    private void wipeSystemImpl() {
        Log.log(Level.FINE, "Wiping");

        // Wipe the current system.
        System.wipe(system);

        // Create a new system
        system = System.create(
                systemExecutor,
                systemListener,
                account,
                isMainnet,
                storageFile.getAbsolutePath(),
                blockchainDb);

        // Passing empty list... it is a demo app...
        system.configure();
    }

    public TestConfiguration getTestConfiguration() {
        try (final InputStream inputStream = getAssets().open("WalletKitTestsConfig.json");
             final BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream))) {
            return TestConfiguration.loadFrom(reader);
        }
        catch (IOException e) {
            throw new RuntimeException (e);
        }
    }
}
