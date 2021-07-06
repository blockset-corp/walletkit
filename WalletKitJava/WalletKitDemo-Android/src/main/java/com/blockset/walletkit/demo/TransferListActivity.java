/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.demo;

import android.app.Activity;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.util.SortedList;
import android.support.v7.widget.DividerItemDecoration;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.support.v7.widget.util.SortedListAdapterCallback;
import android.text.Html;
import android.text.Spanned;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.blockset.walletkit.Network;
import com.blockset.walletkit.NetworkPeer;
import com.blockset.walletkit.NetworkType;
import com.blockset.walletkit.PaymentProtocolRequest;
import com.blockset.walletkit.PaymentProtocolRequestType;
import com.blockset.walletkit.System;
import com.blockset.walletkit.Transfer;
import com.blockset.walletkit.TransferConfirmation;
import com.blockset.walletkit.TransferState;
import com.blockset.walletkit.Wallet;
import com.blockset.walletkit.WalletManager;
import com.blockset.walletkit.WalletManagerSyncDepth;
import com.blockset.walletkit.WalletManagerMode;
import com.blockset.walletkit.events.system.DefaultSystemListener;
import com.blockset.walletkit.events.transfer.DefaultTransferEventVisitor;
import com.blockset.walletkit.events.transfer.TranferEvent;
import com.blockset.walletkit.events.transfer.TransferChangedEvent;
import com.blockset.walletkit.events.transfer.TransferCreatedEvent;
import com.blockset.walletkit.events.transfer.TransferDeletedEvent;
import com.google.common.base.Optional;
import com.google.common.base.Supplier;
import com.google.common.base.Suppliers;
import com.google.common.collect.Lists;
import com.google.common.primitives.UnsignedInteger;

import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class TransferListActivity extends AppCompatActivity implements DefaultSystemListener {

    private static final String EXTRA_WALLET_NAME = "com.blockset.walletkit.demo.TransferListActivity.EXTRA_WALLET_NAME";

    private static final int TRANSFER_CHUNK_SIZE = 10;

    public static void start(Activity callerActivity, Wallet wallet) {
        Intent intent = new Intent(callerActivity, TransferListActivity.class);
        intent.putExtra(EXTRA_WALLET_NAME, wallet.getName());
        callerActivity.startActivity(intent);
    }

    @Nullable
    private static Wallet getWallet(Intent intent) {
        String walletName = intent.getStringExtra(EXTRA_WALLET_NAME);
        for(Wallet wallet: DemoApplication.getSystem().getWallets()) {
            if (wallet.getName().equals(walletName)) {
                return wallet;
            }
        }
        return null;
    }

    private Wallet wallet;
    private boolean isBitcoin;
    private Adapter transferAdapter;
    private ClipboardManager clipboardManager;
    private ArrayList<PaymentProtocolRequestType> availablePaymentProtocols = new ArrayList();

    void setActionConditionalOnNetwork(Button button, List<NetworkType> netTypes, View.OnClickListener clicker) {
        if (netTypes.contains(wallet.getWalletManager().getNetwork().getType()))
            button.setOnClickListener(clicker);
        else
            button.setEnabled(false);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_transfer_list);

        DemoApplication.initialize(this);

        wallet = getWallet(getIntent());
        if (null == wallet) {
            finish();
            return;
        }

        Network net = wallet.getWalletManager().getNetwork();
        String currencyCode = wallet.getCurrency().getCode().toLowerCase();
        NetworkType networkType = net.getType();
        isBitcoin = NetworkType.BTC == networkType || NetworkType.BCH == networkType;
        clipboardManager = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);

        // Send currency must be contingent on available funds
        Button sendView = findViewById(R.id.send_view);
        if (wallet.getBalance().isZero())
            sendView.setEnabled(false);
        else
            sendView.setOnClickListener(v -> TransferCreateSendActivity.start(TransferListActivity.this, wallet));

        Button recvView = findViewById(R.id.receive_view);
        recvView.setOnClickListener(v -> copyReceiveAddress());

        // Payment is dependent on available payment protocols for this wallet.
        Button payView = findViewById(R.id.pay_view);
        for (PaymentProtocolRequestType pprt :PaymentProtocolRequestType.values()) {
            if (PaymentProtocolRequest.checkPaymentMethodSupported(wallet, pprt))
                availablePaymentProtocols.add(pprt);
        }
        if (availablePaymentProtocols.size() > 0)
            payView.setOnClickListener(v -> showPaymentMenu(TransferListActivity.this, wallet));
        else
            payView.setEnabled(false);

        Button sweepView = findViewById(R.id.sweep_view);
        setActionConditionalOnNetwork(sweepView, Arrays.asList(NetworkType.BTC, NetworkType.BCH), new View.OnClickListener()
        {
            @Override
            public void onClick(View v) {
                TransferCreateSweepActivity.start(TransferListActivity.this, wallet);
            }
        });

        Button delegateView = findViewById(R.id.delegate_view);
        setActionConditionalOnNetwork(delegateView, Arrays.asList(NetworkType.XTZ), new View.OnClickListener() {

            @Override
            public void onClick(View v) {

            }
        });

        Button exportablePaperView = findViewById(R.id.exportablepaper_view);
        setActionConditionalOnNetwork(exportablePaperView, Arrays.asList(NetworkType.BTC), new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                TransferCreateExportablePaperActivity.start(TransferListActivity.this, wallet);
            }
        });

        RecyclerView transfersView = findViewById(R.id.transfer_recycler_view);
        transfersView.addItemDecoration(new DividerItemDecoration(getApplicationContext(), DividerItemDecoration.VERTICAL));

        RecyclerView.LayoutManager transferLayoutManager = new LinearLayoutManager(this, LinearLayoutManager.VERTICAL, false);
        transfersView.setLayoutManager(transferLayoutManager);

        transferAdapter = new Adapter((transfer) -> TransferDetailsActivity.start(this, wallet, transfer));
        transfersView.setAdapter(transferAdapter);

        Toolbar toolbar = findViewById(R.id.toolbar_view);
        toolbar.setTitle(String.format("Wallet: %s", wallet.getName()));
        setSupportActionBar(toolbar);
    }

    @Override
    protected void onResume() {
        super.onResume();

        DemoApplication.getDispatchingSystemListener().addWalletListener(wallet, this);
        loadTransfers();
    }

    @Override
    protected void onPause() {
        DemoApplication.getDispatchingSystemListener().removeWalletListener(wallet, this);

        super.onPause();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_transfer_list, menu);
        menu.findItem(R.id.action_connect_with_peer).setVisible(isBitcoin);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.action_connect:
                connect();
                return true;
            case R.id.action_connect_with_peer:
                showConnectWithPeerMenu();
                return true;
            case R.id.action_disconnect:
                disconnect();
                return true;
            case R.id.action_sync:
                showSyncToDepthMenu();
                return true;
            case R.id.action_toggle_mode:
                showSelectModeMenu();
                return true;
        }
        return false;
    }

    @Override
    public void handleTransferEvent(System system, WalletManager manager, Wallet wallet, Transfer transfer, TranferEvent event) {
        event.accept(new DefaultTransferEventVisitor<Void>() {
            @Override
            public Void visit(TransferCreatedEvent event) {
                addTransfer(transfer);
                return null;
            }

            @Override
            public Void visit(TransferChangedEvent event) {
                updateTransfer(transfer);
                return null;
            }


            @Override
            public Void visit(TransferDeletedEvent event) {
                removeTransfer(transfer);
                return null;
            }
        });
    }

    private void loadTransfers() {
        ApplicationExecutors.runOnUiExecutor(() -> {
            List<? extends Transfer> transfers = wallet.getTransfers();
            List<TransferViewModel> viewModels = TransferViewModel.create(transfers);
            Collections.sort(viewModels, TransferViewModel::compare);

            runOnUiThread(transferAdapter::clear);
            for (List<TransferViewModel> viewModelsChunk: Lists.partition(viewModels, TRANSFER_CHUNK_SIZE)) {
                runOnUiThread(() -> transferAdapter.add(viewModelsChunk));
            }
        });
    }

    private void addTransfer(Transfer transfer) {
        ApplicationExecutors.runOnUiExecutor(() -> {
            TransferViewModel vm = TransferViewModel.create(transfer);
            runOnUiThread(() -> transferAdapter.add(vm));
        });
    }

    private void removeTransfer(Transfer transfer) {
        ApplicationExecutors.runOnUiExecutor(() -> {
            TransferViewModel vm = TransferViewModel.create(transfer);
            runOnUiThread(() -> transferAdapter.remove(vm));
        });
    }

    private void updateTransfer(Transfer transfer) {
        ApplicationExecutors.runOnUiExecutor(() -> {
            TransferViewModel vm = TransferViewModel.create(transfer);
            runOnUiThread(() -> transferAdapter.update(vm));
        });
    }

    private void connect() {
        ApplicationExecutors.runOnBlockingExecutor(() -> wallet.getWalletManager().connect(null));
    }

    private void disconnect() {
        ApplicationExecutors.runOnBlockingExecutor(() -> wallet.getWalletManager().disconnect());
    }

    private void showConnectWithPeerMenu() {
        runOnUiThread(() -> {
            View dialogView = getLayoutInflater().inflate(R.layout.layout_connect_with_peer, null);
            CharSequence address = ((EditText) dialogView.findViewById(R.id.address_view)).getText().toString();
            CharSequence port = ((EditText) dialogView.findViewById(R.id.port_view)).getText().toString();
            CharSequence pubkey = ((EditText) dialogView.findViewById(R.id.pubkey_view)).getText().toString();
            new AlertDialog.Builder(this)
                    .setTitle("Connect with Peer")
                    .setView(dialogView)
                    .setNegativeButton("Cancel", (d, w) -> {})
                    .setPositiveButton("Ok", (d, w) -> connectWithPeer(address, port, pubkey))
                    .show();
        });
    }

    private Optional<? extends NetworkPeer> createPeer(CharSequence addressSeq, CharSequence portSeq, CharSequence pubkeySeq) {
        // required
        String address = addressSeq.toString();
        if (address.isEmpty()) {
            return Optional.absent();
        }

        UnsignedInteger port;
        try {
            port = UnsignedInteger.valueOf(portSeq.toString());
        } catch (NumberFormatException e) {
            return Optional.absent();
        }

        // optional (null if empty)
        String pubkey = pubkeySeq.length() == 0 ? null : pubkeySeq.toString();
        return wallet.getWalletManager().getNetwork().createPeer(address, port, pubkey);
    }

    private void connectWithPeer(CharSequence addressSeq, CharSequence portSeq, CharSequence pubkeySeq) {
        Optional<? extends NetworkPeer> maybePeer = createPeer(addressSeq, portSeq, pubkeySeq);
        if (maybePeer.isPresent()) {
            ApplicationExecutors.runOnBlockingExecutor(() -> wallet.getWalletManager().connect(maybePeer.get()));
        } else {
            showError("Unable to create peer");
        }
    }

    private void showSyncToDepthMenu() {
        runOnUiThread(() -> new AlertDialog.Builder(this)
                .setTitle("Sync Depth")
                .setSingleChoiceItems(new String[]{"From Last Confirmed Send", "From Last Trusted Block", "From Creation"},
                        -1,
                        (d, w) -> {
                            ApplicationExecutors.runOnBlockingExecutor(() -> {
                                switch (w) {
                                    case 0:
                                        wallet.getWalletManager().syncToDepth(WalletManagerSyncDepth.FROM_LAST_CONFIRMED_SEND);
                                        break;
                                    case 1:
                                        wallet.getWalletManager().syncToDepth(WalletManagerSyncDepth.FROM_LAST_TRUSTED_BLOCK);
                                        break;
                                    default:
                                        wallet.getWalletManager().syncToDepth(WalletManagerSyncDepth.FROM_CREATION);
                                        break;
                                }
                            });
                            d.dismiss();
                        })
                .show());
    }

    private void showSelectModeMenu() {
        ApplicationExecutors.runOnUiExecutor(() -> {
            WalletManager wm = wallet.getWalletManager();
            System system = wm.getSystem();
            Network network = wm.getNetwork();
            List<WalletManagerMode> modes = network.getSupportedWalletManagerModes();

            String[] itemTexts = new String[modes.size()];
            WalletManagerMode[] itemModes = new WalletManagerMode[modes.size()];
            for (int i = 0; i < itemTexts.length; i++) {
                itemModes[i] = modes.get(i);
                itemTexts[i] = itemModes[i].toString();
            }

            runOnUiThread(() -> new AlertDialog.Builder(this)
                        .setTitle("Sync Mode")
                        .setSingleChoiceItems(itemTexts,
                                -1,
                                (d, w) -> {
                                    ApplicationExecutors.runOnBlockingExecutor(() -> wm.setMode(itemModes[w]));
                                    d.dismiss();
                                })
                        .show());
        });
    }

    private void showPaymentMenu(Activity context, Wallet wallet) {
        String[] availablePaymentMethods = new String[availablePaymentProtocols.size()];
        for (int choiceNum = 0;choiceNum < availablePaymentProtocols.size(); choiceNum++) {
            availablePaymentMethods[choiceNum] =  availablePaymentProtocols.get(choiceNum).toString();
        }
        runOnUiThread(() -> new AlertDialog.Builder(this)
                .setTitle("Payment Protocol")
                .setNegativeButton("Cancel", (d,w) -> {})
                .setPositiveButton("Ok", (d,w) -> {
                    int selected = ((AlertDialog)d).getListView().getCheckedItemPosition();
                    TransferCreatePaymentActivity.start(context, wallet, availablePaymentProtocols.get(selected));
                    d.dismiss();
                })
                .setSingleChoiceItems(availablePaymentMethods,0, null)
                .show());
    }

    private void showError(String message) {
        runOnUiThread(() -> new AlertDialog.Builder(this)
                .setTitle("Error")
                .setMessage(message)
                .setCancelable(false)
                .setNeutralButton("Ok", (d, w) -> {})
                .show());
    }

    private void copyReceiveAddress() {
        ApplicationExecutors.runOnUiExecutor(() -> {
            String value = wallet.getTarget().toString();
            clipboardManager.setPrimaryClip(ClipData.newPlainText("ReceiveAddress", value));

            String escapedValue = Html.escapeHtml(value);
            Spanned message = Html.fromHtml(String.format("Copied receive address <b>%s</b> to clipboard", escapedValue));

            runOnUiThread(() -> Toast.makeText(getApplicationContext(), message, Toast.LENGTH_SHORT).show());
        });
    }

    private interface OnItemClickListener<T> {
        void onItemClick(T item);
    }

    private static class TransferViewModel {

        static final ThreadLocal<DateFormat> DATE_FORMAT = new ThreadLocal<DateFormat>() {
            @Override protected DateFormat initialValue() {
                return DateFormat.getDateTimeInstance(DateFormat.LONG, DateFormat.LONG);
            }
        };

        static int compare(TransferViewModel vm1, TransferViewModel vm2) {
            if (vm1.transfer.equals(vm2.transfer)) return 0;

            TransferConfirmation vm1Conf = vm1.confirmation().orNull();
            TransferConfirmation vm2Conf = vm2.confirmation().orNull();

            if (vm1Conf != null && vm2Conf != null) {
                int blockCompare = vm2Conf.getBlockNumber().compareTo(vm1Conf.getBlockNumber());
                if (blockCompare != 0) return blockCompare;

                return vm2Conf.getTransactionIndex().compareTo(vm1Conf.getTransactionIndex());

            } else if (vm1Conf != null) {
                return 1;
            } else if (vm2Conf != null) {
                return -1;
            } else {
                return vm2.transferHashCode - vm1.transferHashCode;
            }
        }

        static boolean areContentsTheSame(TransferViewModel vm1, TransferViewModel vm2) {
            return vm1.dateText().equals(vm2.dateText()) &&
                    vm1.addressText().equals(vm2.addressText()) &&
                    vm1.amountText().equals(vm2.amountText()) &&
                    vm1.feeText().equals(vm2.feeText()) &&
                    vm1.stateText().equals(vm2.stateText());
        }

        static boolean areItemsTheSame(TransferViewModel vm1, TransferViewModel vm2) {
            return vm1.transfer.equals(vm2.transfer);
        }

        static List<TransferViewModel> create(List<? extends Transfer> transfers) {
            List<TransferViewModel> vms = new ArrayList<>(transfers.size());
            for (Transfer t: transfers) {
                vms.add(create(t));
            }
            return vms;
        }

        static TransferViewModel create(Transfer transfer) {
            return new TransferViewModel(transfer);
        }

        final Transfer transfer;
        final int transferHashCode;

        final Supplier<TransferState> state;
        final Supplier<TransferConfirmation> confirmation;

        final Supplier<String> amountText;
        final Supplier<String> feeText;
        final Supplier<String> stateText;
        final Supplier<String> dateText;
        final Supplier<String> addressText;

        TransferViewModel(Transfer transfer) {
            this.transfer = transfer;
            this.transferHashCode = transfer.hashCode();

            this.state = Suppliers.memoize(transfer::getState);
            this.confirmation = Suppliers.memoize(() -> state().getIncludedConfirmation().orNull());

            this.amountText = Suppliers.memoize(() -> transfer.getAmountDirected().toString());
            this.feeText = Suppliers.memoize(() -> String.format("Fee: %s", transfer.getFee()));
            this.stateText = Suppliers.memoize(() -> {
                String text = String.format("State: %s", state());
                return confirmation().transform((c) -> text +
                        (c.getSuccess() ? "" : String.format(" (%s)", c.getError().or("<err>"))))
                        .or(text);
            });
            this.dateText = Suppliers.memoize(() -> confirmation()
                    .transform(TransferConfirmation::getConfirmationTime)
                    .transform(t -> DATE_FORMAT.get().format(t))
                    .or("<pending>"));
            this.addressText = Suppliers.memoize(() ->
                    transfer.getHash()
                            .transform(h -> String.format("Hash: %s", h))
                            .or("<pending>"));
        }

        TransferState state() {
            return state.get();
        }

        Optional<TransferConfirmation> confirmation() {
            return Optional.fromNullable(confirmation.get());
        }

        String dateText() {
            return dateText.get();
        }

        String addressText() {
            return addressText.get();
        }

        String amountText() {
            return amountText.get();
        }

        String feeText() {
            return feeText.get();
        }

        String stateText() {
            return stateText.get();
        }
    }

    private static class TransferViewModelSortedListAdapterCallback extends SortedListAdapterCallback<TransferViewModel> {

        TransferViewModelSortedListAdapterCallback(Adapter adapter) {
            super(adapter);
        }

        @Override
        public int compare(TransferViewModel t1, TransferViewModel t2) {
            return TransferViewModel.compare(t1, t2);
        }

        @Override
        public boolean areContentsTheSame(TransferViewModel t1, TransferViewModel t2) {
            return TransferViewModel.areContentsTheSame(t1, t2);
        }

        @Override
        public boolean areItemsTheSame(TransferViewModel t1, TransferViewModel t2) {
            return TransferViewModel.areItemsTheSame(t1, t2);
        }
    }

    private static class Adapter extends RecyclerView.Adapter<ViewHolder> {

        final OnItemClickListener<Transfer> listener;
        final SortedList<TransferViewModel> viewModels;

        Adapter(OnItemClickListener<Transfer> listener) {
            this.listener = listener;
            this.viewModels = new SortedList<>(TransferViewModel.class, new TransferViewModelSortedListAdapterCallback(this));
        }

        @NonNull
        @Override
        public ViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
            return new ViewHolder(
                    LayoutInflater.from(
                            viewGroup.getContext()
                    ).inflate(
                            R.layout.layout_transfer_item,
                            viewGroup,
                            false
                    )
            );
        }

        @Override
        public void onBindViewHolder(@NonNull ViewHolder vh, int i) {
            TransferViewModel vm = viewModels.get(i);

            vh.itemView.setOnClickListener(v -> listener.onItemClick(vm.transfer));
            vh.dateView.setText(vm.dateText());
            vh.amountView.setText(vm.amountText());
            vh.addressView.setText(vm.addressText());
            vh.feeView.setText(vm.feeText());
            vh.stateView.setText(vm.stateText());
        }

        @Override
        public int getItemCount() {
            return viewModels.size();
        }

        void clear() {
            viewModels.clear();
        }

        void add(List<TransferViewModel> newTransfers) {
            viewModels.addAll(newTransfers);
        }

        void add(TransferViewModel transfer) {
            viewModels.add(transfer);
        }

        void remove(TransferViewModel transfer) {
            viewModels.add(transfer); // add it; will display with state of deledted
        }

        void update(TransferViewModel transfer) {
            viewModels.add(transfer);
        }
    }

    private static class ViewHolder extends RecyclerView.ViewHolder {

        TextView dateView;
        TextView amountView;
        TextView addressView;
        TextView feeView;
        TextView stateView;

        ViewHolder(@NonNull View view) {
            super(view);

            dateView = view.findViewById(R.id.item_date);
            amountView = view.findViewById(R.id.item_amount);
            addressView = view.findViewById(R.id.item_address);
            feeView = view.findViewById(R.id.item_fee);
            stateView = view.findViewById(R.id.item_state);
        }
    }
}
