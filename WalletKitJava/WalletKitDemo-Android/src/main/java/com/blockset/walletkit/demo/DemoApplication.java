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

import java.io.File;
import java.nio.charset.StandardCharsets;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.logging.Level;
import java.util.logging.Logger;

import okhttp3.OkHttpClient;

import static com.google.common.base.Preconditions.checkState;

public class DemoApplication extends Application {

    public static final String EXTRA_BLOCKSETACCESS_TOKEN = "com.blockset.walletkit.demo.blocksetaccesstoken";
    public static final String EXTRA_BLOCKSETURL_TOKEN = "com.blockset.walletkit.demo.blockseturl";
    public static final String EXTRA_PAPERKEY_TOKEN = "com.blockset.walletkit.demo.paperkey";
    public static final String EXTRA_IS_MAINNET_TOKEN = "com.blockset.walletkit.demo.isMainnet";
    public static final String EXTRA_TIMESTAMP_TOKEN = "com.blockset.walletkit.demo.timestamp";

    private static final Logger Log = Logger.getLogger(DemoApplication.class.getName());

    private static final String EXTRA_MODE = "MODE";
    private static final String EXTRA_WIPE = "WIPE";

    private static final boolean DEFAULT_IS_MAINNET = true;
    private static final boolean DEFAULT_WIPE = true;

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
    private Hashtable<String, WalletManagerMode> currencyCodesToMode;
    private Set<String> registerCurrencyCodes;
    private AtomicBoolean runOnce = new AtomicBoolean(false);

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
        StrictMode.enableDefaults();
    }

    private void initFromLaunchIntent(Intent intent) {
        if (!runOnce.getAndSet(true)) {
            Logging.initialize(Level.FINE);

            if (intent.hasExtra(EXTRA_PAPERKEY_TOKEN)) {

                boolean isMainnet = intent.getBooleanExtra(EXTRA_IS_MAINNET_TOKEN, DEFAULT_IS_MAINNET);
                String paperKeyString = intent.getStringExtra(EXTRA_PAPERKEY_TOKEN);
                Api.initialize(ApiProvider.getInstance());


                String timestamp = intent.getStringExtra(EXTRA_TIMESTAMP_TOKEN);
                Date accountTs = new Date();
                try {
                    accountTs = new SimpleDateFormat("yyyy-MM-dd").parse(timestamp);
                } catch (ParseException e) {
                    e.printStackTrace();
                }

                boolean wipe = intent.getBooleanExtra(EXTRA_WIPE, DEFAULT_WIPE);

                systemExecutor = Executors.newSingleThreadScheduledExecutor();

                storageFile = new File(getFilesDir(), "core");
                if (wipe) System.wipeAll(storageFile.getAbsolutePath(), Collections.emptyList());
                if (!storageFile.exists()) checkState(storageFile.mkdirs());

                currencyCodesToMode = new Hashtable<String, WalletManagerMode>() {
                    {   put("btc", WalletManagerMode.API_ONLY);
                        put("bch", WalletManagerMode.API_ONLY);
                        put("bsv", WalletManagerMode.API_ONLY);
                        put("ltc", WalletManagerMode.P2P_ONLY);
                        put("doge", WalletManagerMode.P2P_ONLY);
                        put("eth", WalletManagerMode.API_ONLY);
                        put("xrp", WalletManagerMode.API_ONLY);
                        put("hbar", WalletManagerMode.API_ONLY);
                        put("xtz", WalletManagerMode.API_ONLY);
                        put("xlm", WalletManagerMode.API_ONLY);
                    }
                };

                String blocksetToken = intent.getStringExtra(EXTRA_BLOCKSETACCESS_TOKEN);
                String blocksetBaseURL = intent.getStringExtra(EXTRA_BLOCKSETURL_TOKEN);
                blockchainDb = BlocksetSystemClient.createForTest(new OkHttpClient(),
                        blocksetToken,
                        blocksetBaseURL);

                initializeSystemWithAccount(paperKeyString, isMainnet, accountTs);

                connectivityReceiver = new ConnectivityBroadcastReceiver();
                registerReceiver(connectivityReceiver, new IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION));
            }
        }
    }

    public static void setAccount(
            String  identifier,
            String  paperKeyString,
            boolean isMainnet,
            Date    timestamp   ) {
        checkState(null != instance && instance.runOnce.get());

        Log.log(Level.INFO, String.format("Switch account to '%s'", paperKeyString));
        instance.initializeSystemWithAccount(paperKeyString, isMainnet, timestamp);
    }

    private void initializeSystemWithAccount(
            String   paperKeyString,
            boolean  isMainnet,
            Date     timestamp       ) {

        paperKey = paperKeyString.getBytes(StandardCharsets.UTF_8);

        // Store locally to permit wipe
        this.isMainnet = isMainnet;

        Log.log(Level.FINE, String.format("Account PaperKey:  %s", paperKeyString));
        Log.log(Level.FINE, String.format("Account Timestamp: %s", timestamp));
        Log.log(Level.FINE, String.format("StoragePath:       %s", storageFile.getAbsolutePath()));
        Log.log(Level.FINE, String.format("Mainnet:           %s", isMainnet));

        String uids = UUID.randomUUID().toString();
        account = Account.createFromPhrase(paperKey,
                                           timestamp,
                                           uids).get();


        registerCurrencyCodes = isMainnet ? new HashSet(Arrays.asList(/* "zla", "adt" */)) :
                                            new HashSet(Arrays.asList("brd", "tst"));

        systemListener = new DispatchingSystemListener();
        systemListener.addSystemListener(new DemoSystemListener(currencyCodesToMode,
                                                                registerCurrencyCodes,
                                                                isMainnet));

        wipeSystemImpl();

        System.wipeAll(storageFile.getAbsolutePath(), Collections.singletonList(system));
    }

    private void wipeSystemImpl() {
        Log.log(Level.FINE, "Wiping");

        // Wipe the current system.
        if (system != null)
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
}
