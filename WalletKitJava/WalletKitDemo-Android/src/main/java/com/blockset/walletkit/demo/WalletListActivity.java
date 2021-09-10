/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.demo;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import androidx.recyclerview.widget.SortedList;
import androidx.recyclerview.widget.SortedListAdapterCallback;
import androidx.appcompat.widget.Toolbar;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.view.ViewGroup.LayoutParams;
import android.widget.TextView;

import com.blockset.walletkit.Currency;
import com.blockset.walletkit.Network;
import com.blockset.walletkit.System;
import com.blockset.walletkit.Wallet;
import com.blockset.walletkit.WalletManager;
import com.blockset.walletkit.WalletManagerState;
import com.blockset.walletkit.events.system.DefaultSystemListener;
import com.blockset.walletkit.events.wallet.DefaultWalletEventVisitor;
import com.blockset.walletkit.events.wallet.WalletBalanceUpdatedEvent;
import com.blockset.walletkit.events.wallet.WalletChangedEvent;
import com.blockset.walletkit.events.wallet.WalletCreatedEvent;
import com.blockset.walletkit.events.wallet.WalletDeletedEvent;
import com.blockset.walletkit.events.wallet.WalletEvent;
import com.blockset.walletkit.events.walletmanager.DefaultWalletManagerEventVisitor;
import com.blockset.walletkit.events.walletmanager.WalletManagerEvent;
import com.blockset.walletkit.events.walletmanager.WalletManagerSyncProgressEvent;
import com.blockset.walletkit.events.walletmanager.WalletManagerSyncStartedEvent;
import com.blockset.walletkit.events.walletmanager.WalletManagerSyncStoppedEvent;
import com.blockset.walletkit.utility.AccountSpecification;
import com.blockset.walletkit.utility.TestConfiguration;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.datatype.guava.GuavaModule;
import com.google.common.base.Supplier;
import com.google.common.base.Suppliers;
import com.google.common.collect.Lists;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class WalletListActivity extends AppCompatActivity implements DefaultSystemListener {

    private static final int WALLET_CHUNK_SIZE = 10;

    private Adapter walletsAdapter;
    private TestConfiguration testConfiguration;
    private List<AccountSpecification> accounts;
    private int accountIdx = -1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_wallet_list);

        getTestConfiguration();

        // Blockset access
        getIntent().putExtra(DemoApplication.EXTRA_BLOCKSETACCESS_TOKEN,
                             this.testConfiguration.getBlocksetAccess().getToken());
        getIntent().putExtra(DemoApplication.EXTRA_BLOCKSETURL_TOKEN,
                             this.testConfiguration.getBlocksetAccess().getBaseURL());

        setInitialAccount();
        DemoApplication.initialize(this);
        if (configOk()) {

            setActivityTitle();
            RecyclerView walletsView = findViewById(R.id.wallet_recycler_view);

            walletsView.addItemDecoration(new DividerItemDecoration(getApplicationContext(), DividerItemDecoration.VERTICAL));

            RecyclerView.LayoutManager walletsLayoutManager = new LinearLayoutManager(this);
            walletsView.setLayoutManager(walletsLayoutManager);

            walletsAdapter = new Adapter((wallet) -> TransferListActivity.start(this, wallet));
            walletsView.setAdapter(walletsAdapter);

            Toolbar toolbar = findViewById(R.id.toolbar_view);
            setSupportActionBar(toolbar);
        } else {

            LinearLayout topView = findViewById(R.id.wallet_list_layout);
            topView.removeAllViews();
            TextView discord = new TextView(this);
            discord.setText("Invalid or missing configuration JSON");
            discord.setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));
            topView.addView (discord);
        }
    }

    private void setActivityTitle() {

        setTitle(accounts.get(accountIdx).getNetwork() + " "
                 + getString(R.string.wallets_list_title)
                 + " ("
                 + accounts.get(accountIdx).getIdentifier()
                 + ")");
    }

    private boolean configOk() {
        return accountIdx != -1;
    }


    /* Initial account selection is the first account provided
     * in the config, assuming the config was valid
     */
    private void setInitialAccount() {

        Intent intent = getIntent();

       if (accounts != null && accounts.size() > 0) {

            // We have a valid account
            accountIdx = 0;

            ObjectMapper mapper = new ObjectMapper();
            mapper.registerModule(new GuavaModule());
            String acctSer;
            try {
                acctSer = mapper.writeValueAsString(accounts.get(accountIdx));
            } catch (JsonProcessingException processingException) {
                throw new RuntimeException (processingException);
            }

            intent.putExtra(DemoApplication.EXTRA_ACCOUNT_SPEC, acctSer);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        if (configOk()) {
            DemoApplication.getDispatchingSystemListener().addSystemListener(this);
            loadWallets();
        }
    }

    @Override
    protected void onPause() {
        if (configOk()) {
            DemoApplication.getDispatchingSystemListener().removeSystemListener(this);
        }

        super.onPause();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_wallet_list, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.action_choose_account:
                showChooseAccount();
                return true;
            case R.id.action_connect:
                connect();
                return true;
            case R.id.action_disconnect:
                disconnect();
                return true;
            case R.id.action_sync:
                sync();
                return true;
            case R.id.action_wipe:
                wipe();
                return true;
            case R.id.action_update_fees:
                updateFees();
                return true;
            case R.id.action_add_wallet:
                showAddWalletMenu();
                return true;

        }
        return false;
    }

    @Override
    public void handleManagerEvent(System system, WalletManager manager, WalletManagerEvent event) {
        ApplicationExecutors.runOnUiExecutor(() -> event.accept(new DefaultWalletManagerEventVisitor<Void>() {
            @Override
            public Void visit(WalletManagerSyncStartedEvent event) {
                updateWallets(manager);
                return null;
            }

            @Nullable
            @Override
            public Void visit(WalletManagerSyncProgressEvent event) {
                updateWalletsForSync(manager, event.getPercentComplete());
                return null;
            }

            @Override
            public Void visit(WalletManagerSyncStoppedEvent event) {
                updateWallets(manager);
                return null;
            }
        }));
    }

    @Override
    public void handleWalletEvent(System system, WalletManager manager, Wallet wallet, WalletEvent event) {
        ApplicationExecutors.runOnUiExecutor(() -> event.accept(new DefaultWalletEventVisitor<Void>() {
            @Override
            public Void visit(WalletBalanceUpdatedEvent event) {
                updateWallet(wallet);
                return null;
            }

            @Override
            public Void visit(WalletChangedEvent event) {
                updateWallet(wallet);
                return null;
            }

            @Override
            public Void visit(WalletCreatedEvent event) {
                addWallet(wallet);
                return null;
            }

            @Override
            public Void visit(WalletDeletedEvent event) {
                removeWallet(wallet);
                return null;
            }
        }));
    }

    private void loadWallets() {
        ApplicationExecutors.runOnUiExecutor(() -> {
            List<? extends Wallet> wallets = DemoApplication.getSystem().getWallets();
            List<WalletViewModel> viewModels = WalletViewModel.create(wallets);
            Collections.sort(viewModels, WalletViewModel::compare);
            runOnUiThread(walletsAdapter::clear);
            for (List<WalletViewModel> viewModelsChunk: Lists.partition(viewModels, WALLET_CHUNK_SIZE)) {
                runOnUiThread(() -> walletsAdapter.add(viewModelsChunk));
            }
        });
    }

    private void addWallet(Wallet wallet) {
        ApplicationExecutors.runOnUiExecutor(() -> {
            WalletViewModel vm = WalletViewModel.create(wallet);
            runOnUiThread(() -> walletsAdapter.add(vm));
        });
    }

    private void removeWallet(Wallet wallet) {
        ApplicationExecutors.runOnUiExecutor(() -> {
            WalletViewModel vm = WalletViewModel.create(wallet);
            runOnUiThread(() -> walletsAdapter.remove(vm));
        });
    }

    private void updateWallet(Wallet wallet) {
        ApplicationExecutors.runOnUiExecutor(() -> {
            WalletViewModel vm = WalletViewModel.create(wallet);
            runOnUiThread(() -> walletsAdapter.update(vm));
        });
    }

    private void updateWallets(WalletManager manager) {
        ApplicationExecutors.runOnUiExecutor(() -> {
            List<? extends Wallet> wallets = manager.getWallets();
            List<WalletViewModel> vms = WalletViewModel.create(wallets);
            runOnUiThread(() -> walletsAdapter.update(vms));
        });
    }

    private void updateWalletsForSync(WalletManager manager, float percentComplete) {
        ApplicationExecutors.runOnUiExecutor(() -> {
            List<? extends Wallet> wallets = manager.getWallets();
            List<WalletViewModel> vms = WalletViewModel.create(wallets, percentComplete);
            runOnUiThread(() -> walletsAdapter.update(vms));
        });
    }

    private void connect() {
        ApplicationExecutors.runOnBlockingExecutor(() -> DemoApplication.getSystem().resume());
    }

    private void disconnect() {
        ApplicationExecutors.runOnBlockingExecutor(() -> DemoApplication.getSystem().pause());
    }

    private void sync() {
        ApplicationExecutors.runOnBlockingExecutor(() -> {
            for (WalletManager wm : DemoApplication.getSystem().getWalletManagers()) {
                wm.sync();
            }
        });
    }

    private void updateFees() {
        ApplicationExecutors.runOnBlockingExecutor(() -> DemoApplication.getSystem().updateNetworkFees(null));
    }

    private void wipe() {
        ApplicationExecutors.runOnBlockingExecutor(() -> {
            DemoApplication.wipeSystem();
            runOnUiThread(this::recreate);
        });
    }

    private void showChooseAccount() {
        ApplicationExecutors.runOnUiExecutor(() -> {

            String[] accountTexts = new String[accounts.size()];
            for (int i = 0; i < accounts.size(); i++) {
                accountTexts[i] = accounts.get(i).getIdentifier();
            }

            int selectedItem = 0;
            runOnUiThread(() -> new AlertDialog.Builder(this)
                    .setTitle("Select Account")
                    .setNegativeButton("Cancel", (d,w) -> {})
                    .setPositiveButton("Ok", (d,w) -> {

                        // We can assume the index is within valid bounds
                        // as it comes from selection of the choice initialized
                        // with range of account strings
                        int selected = ((AlertDialog)d).getListView().getCheckedItemPosition();
                        AccountSpecification acc = accounts.get(selected);
                        if (!accounts.get(accountIdx).getIdentifier().equals(acc.getIdentifier())) {
                            accountIdx = selected;
                            setActivityTitle();
                            DemoApplication.setAccount(accounts.get(accountIdx));

                            onResume();
                        }

                        // Switch system account here
                        d.dismiss();
                    })
                    .setSingleChoiceItems(accountTexts, accountIdx,null)
                    .show());
        });
    }

    private void showAddWalletMenu() {
        ApplicationExecutors.runOnUiExecutor(() -> {
            Set<Currency> missingSet = new HashSet<>();

            List<? extends WalletManager> managers = DemoApplication.getSystem().getWalletManagers();
            for (WalletManager manager: managers) {
                // get all currencies for existing wallet managers
                missingSet.addAll(manager.getNetwork().getCurrencies());

                for (Wallet wallet: manager.getWallets()) {
                    // remove the currencies for existing wallets
                    missingSet.remove(wallet.getCurrency());
                }
            }

            // sort by currency code
            List<Currency> missingList = new ArrayList<>(missingSet);
            Collections.sort(missingList, (o1, o2) -> o1.getCode().compareTo(o2.getCode()));

            // all currencies have wallets; ignore
            if (missingList.isEmpty()) {
                runOnUiThread(() -> new AlertDialog.Builder(this)
                        .setTitle("Add Wallet")
                        .setMessage("All currencies added and accounted for!")
                        .setCancelable(false)
                        .setNeutralButton("Ok", (d, w) -> { })
                        .show());

                // extract the currencies and their descriptions
            } else {
                String[] itemTexts = new String[missingList.size()];
                Currency[] itemCurrencies = new Currency[missingList.size()];
                for (int i = 0; i < missingList.size(); i++) {
                    itemCurrencies[i] = missingList.get(i);
                    itemTexts[i] = itemCurrencies[i].getCode();
                }

                runOnUiThread(() -> new AlertDialog.Builder(this)
                        .setTitle("Add Wallet")
                        .setSingleChoiceItems(itemTexts,
                                -1,
                                (d, w) -> {
                                    for (WalletManager manager : managers) {
                                        if (manager.getNetwork().hasCurrency(itemCurrencies[w])) {
                                            manager.registerWalletFor(itemCurrencies[w]);
                                            break;
                                        }
                                    }
                                    d.dismiss();
                                })
                        .show());
            }
        });
    }

    private interface OnItemClickListener<T> {
        void onItemClick(T item);
    }

    private static class WalletViewModel {

        static int compare(WalletViewModel vm1, WalletViewModel vm2) {
            if (vm1.wallet.equals(vm2.wallet)) return 0;

            int managerCompare = vm1.walletManagerName().compareTo(vm2.walletManagerName());
            if (0 != managerCompare) return managerCompare;

            return vm1.walletName().compareTo(vm2.walletName());
        }

        static boolean areContentsTheSame(WalletViewModel vm1, WalletViewModel vm2) {
            return vm1.isSyncing() == vm2.isSyncing() &&
                    vm1.currencyText().equals(vm2.currencyText()) &&
                    vm1.balanceText().equals(vm2.balanceText()) &&
                    vm1.syncText().equals(vm2.syncText());
        }

        static boolean areItemsTheSame(WalletViewModel vm1, WalletViewModel vm2) {
            return vm1.wallet.equals(vm2.wallet);
        }

        static List<WalletViewModel> create(List<? extends Wallet> wallets) {
            List<WalletViewModel> vms = new ArrayList<>(wallets.size());
            for (Wallet w: wallets) {
                vms.add(create(w));
            }
            return vms;
        }

        static List<WalletViewModel> create(List<? extends Wallet> wallets, float percentComplete) {
            List<WalletViewModel> vms = new ArrayList<>(wallets.size());
            for (Wallet w: wallets) {
                vms.add(create(w, percentComplete));
            }
            return vms;
        }

        static WalletViewModel create(Wallet wallet) {
            return create(wallet, 0);
        }

        static WalletViewModel create(Wallet wallet, float percentComplete) {
            return new WalletViewModel(wallet, percentComplete);
        }

        final Wallet wallet;
        private final float percentComplete;

        final Supplier<String> walletName;
        final Supplier<String> walletManagerName;
        final Supplier<WalletManagerState> walletManagerState;

        final Supplier<String> currencyText;
        final Supplier<String> balanceText;
        final Supplier<String> syncText;

        WalletViewModel(Wallet wallet, float percentComplete) {
            this.wallet = wallet;
            this.percentComplete = percentComplete;

            this.walletName = Suppliers.memoize(wallet::getName);

            Supplier<WalletManager> walletManagerSupplier = Suppliers.memoize(wallet::getWalletManager);
            Supplier<Network> networkSupplier = Suppliers.memoize(() -> walletManagerSupplier.get().getNetwork());
            this.walletManagerState = Suppliers.memoize(() -> walletManagerSupplier.get().getState());
            this.walletManagerName = Suppliers.memoize(() -> networkSupplier.get().getName());

            this.balanceText = Suppliers.memoize(() -> wallet.getBalance().toString());
            this.currencyText = Suppliers.memoize(() -> String.format("%s (%s)", walletName(), walletManagerName()));
            this.syncText = Suppliers.memoize(() -> String.format("Syncing at %.2f%%", percentComplete()));
        }

        String walletName() {
            return walletName.get();
        }

        String walletManagerName() {
            return walletManagerName.get();
        }

        boolean isSyncing() {
            return 0 != percentComplete && walletManagerState.get().equals(WalletManagerState.SYNCING());
        }

        float percentComplete() {
            return percentComplete;
        }

        String currencyText() {
            return currencyText.get();
        }

        String balanceText() {
            return balanceText.get();
        }

        String syncText() {
            return syncText.get();
        }
    }

    private static class WalletViewModelSortedListAdapterCallback extends SortedListAdapterCallback<WalletViewModel> {

        WalletViewModelSortedListAdapterCallback(Adapter adapter) {
            super(adapter);
        }

        @Override
        public int compare(WalletViewModel t1, WalletViewModel t2) {
            return WalletViewModel.compare(t1, t2);
        }

        @Override
        public boolean areContentsTheSame(WalletViewModel t1, WalletViewModel t2) {
            return WalletViewModel.areContentsTheSame(t1, t2);
        }

        @Override
        public boolean areItemsTheSame(WalletViewModel t1, WalletViewModel t2) {
            return WalletViewModel.areItemsTheSame(t1, t2);
        }
    }

    private static class Adapter extends RecyclerView.Adapter<ViewHolder> {

        final OnItemClickListener<Wallet> listener;
        final SortedList<WalletViewModel> viewModels;

        Adapter(OnItemClickListener<Wallet> listener) {
            this.listener = listener;
            this.viewModels = new SortedList<>(WalletViewModel.class, new WalletViewModelSortedListAdapterCallback(this));
        }

        @NonNull
        @Override
        public ViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
            View v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.layout_wallet_item, viewGroup, false);
            return new ViewHolder(v);
        }

        @Override
        public void onBindViewHolder(@NonNull ViewHolder vh, int i) {
            WalletViewModel vm = viewModels.get(i);

            vh.itemView.setOnClickListener(v -> listener.onItemClick(vm.wallet));
            vh.currencyView.setText(vm.currencyText());
            vh.symbolView.setText(vm.balanceText());
            vh.syncView.setText(vm.isSyncing() ? vm.syncText() : "");
            vh.syncView.setVisibility(vm.isSyncing() ? View.VISIBLE : View.GONE);
        }

        @Override
        public int getItemCount() {
            return viewModels.size();
        }

        void clear() {
            viewModels.clear();
        }

        void add(List<WalletViewModel> wallets) {
            viewModels.addAll(wallets);
        }

        void add(WalletViewModel wallet) {
            viewModels.add(wallet);
        }

        void remove(WalletViewModel wallet) {
            viewModels.remove(wallet);
        }

        void update(WalletViewModel wallet) {
            viewModels.add(wallet);
        }

        void update(List<WalletViewModel> wallets) {
            viewModels.addAll(wallets);
        }
    }

    private static class ViewHolder extends RecyclerView.ViewHolder {

        TextView currencyView;
        TextView symbolView;
        TextView syncView;

        ViewHolder(@NonNull View view) {
            super(view);

            currencyView = view.findViewById(R.id.item_currency);
            symbolView = view.findViewById(R.id.item_symbol);
            syncView = view.findViewById(R.id.item_sync_status);
        }
    }

    public void getTestConfiguration() {
        try (final InputStream inputStream = getAssets().open("WalletKitTestsConfig.json");
             final BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream))) {
            this.testConfiguration = TestConfiguration.loadFrom(reader);
            this.accounts = testConfiguration.getAccountSpecifications();
        }
        catch (IOException e) {
            throw new RuntimeException (e);
        }
    }
}
