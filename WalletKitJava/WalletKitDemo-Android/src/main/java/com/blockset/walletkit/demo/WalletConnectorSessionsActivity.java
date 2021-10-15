/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.demo;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;

import androidx.appcompat.widget.Toolbar;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import androidx.recyclerview.widget.SortedList;
import androidx.recyclerview.widget.SortedListAdapterCallback;

import android.text.Editable;
import android.text.TextWatcher;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TableLayout;
import android.widget.TextView;

import com.blockset.walletkit.Address;
import com.blockset.walletkit.ExportablePaperWallet;
import com.blockset.walletkit.Key;
import com.blockset.walletkit.Wallet;
import com.blockset.walletkit.WalletConnector;
import com.blockset.walletkit.demo.walletconnect.WalletConnect;
import com.blockset.walletkit.demo.walletconnect.WalletConnectSessionDescription;
import com.blockset.walletkit.errors.ExportablePaperWalletError;
import com.blockset.walletkit.errors.WalletConnectorError;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.base.Optional;
import com.google.common.base.Supplier;
import com.google.common.base.Suppliers;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.logging.Logger;
import java.util.logging.Level;

public class WalletConnectorSessionsActivity extends AppCompatActivity {

    private static final String EXTRA_WALLET_NAME = "com.blockset.walletkit.demo.WalletConnectorSessionsActivity.EXTRA_WALLET_NAME";

    private static final Logger Log = Logger.getLogger(DemoApplication.class.getName());


    public static void start(Activity callerActivity, Wallet wallet) {
        Intent intent = new Intent(callerActivity, WalletConnectorSessionsActivity.class);
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

    // Constructs...
    private Wallet                              wallet;
    private WalletConnect                       walletConnect;

    // Widgets...
    private Button          connectToBridgeButton;
    private Button          disconnectFromBridgeButton;
    private EditText        dAppUriEditText;

    private TableLayout     containerOfSessionInfo;
    private TextView        connectedAppTextView;
    private TextView        topicTextView;
    private TextView        clientIdTextView;
    private TextView        peerIdTextView;

    private LinearLayout            containerOfPendingRequests;
    private PendingRequestsAdapter  pendingRequestsAdapter;

    private class UserSessionClient implements WalletConnect.DAppSessionClient {

        private final Context context;

        public UserSessionClient(Context context) {
            this.context = context;
        }

        @Override
        public void grantSession(
                String          bridgeUrl,
                String          dAppName,
                String          description,
                String[]        icons,
                WalletConnect.SessionApprover userApproval) {

            List<String> accounts = Collections.singletonList(wallet.getTarget().toString());
            List<String> favIcons = Collections.singletonList("https://brd.com/favicon.ico");
            runOnUiThread(() -> {
                new AlertDialog.Builder(this.context)
                        .setTitle("WalletConnect")
                        .setMessage(dAppName + " wants to connect with\n" + bridgeUrl)
                        .setNegativeButton("Reject", (d, w) -> {
                            userApproval.reject();
                        })
                        .setPositiveButton("Approve", (d, w) -> {
                            userApproval.accept(accounts,
                                                "https://brd.com/",
                                                "BRD Wallet",
                                                "",
                                                favIcons);
                        }).show();
                dAppUriEditText.setText("");
            });
        }

        @Override
        public void approveRequest(
                Number requestId,
                ApprovalType requestType,
                Map<String, String> requestData,
                WalletConnect.RequestApprover requestApproval) {

            PendingRequestItem whatToDo = new PendingRequestItem(requestId,
                                                                 requestType.toString(),
                                                                 requestData,
                                                                 requestApproval);
            runOnUiThread(() -> {
                PendingRequestViewModel vm = PendingRequestViewModel.create(whatToDo);
                pendingRequestsAdapter.add(vm);
            });
        }

        @Override
        public void sessionStarted(String dApp, String topic, String clientId, String peerId) {
            runOnUiThread(() -> {
                setUiConnected(dApp, topic, clientId, peerId);
            });
        }

        @Override
       public void sessionError(String reason) {

       //    if (err == WalletConnect.WalletConnectError.WALLET_CONNECT_CONNECTION_ERROR) {
               // Initial connection has failed
               AlertDialog dialog = new AlertDialog.Builder(this.context)
                   .setTitle("Failed")
                       .setMessage("Connection failed: " + reason)
                       .setPositiveButton("Cancel", (d, w) -> {}).show();
               dAppUriEditText.setText("");
      //     }
       }
    }

    private void setUiConnected(
            String dApp,
            String topic,
            String clientId,
            String peerId  ) {
        connectToBridgeButton.setEnabled(false);
        disconnectFromBridgeButton.setEnabled(true);
        containerOfSessionInfo.setVisibility(View.VISIBLE);
        containerOfPendingRequests.setVisibility(View.VISIBLE);
        connectedAppTextView.setText(dApp);
        topicTextView.setText(topic);
        clientIdTextView.setText(clientId);
        peerIdTextView.setText(peerId);
    }

    private void setUiDisconnected() {
        disconnectFromBridgeButton.setEnabled(false);
        connectToBridgeButton.setEnabled(false);
        containerOfSessionInfo.setVisibility(View.GONE);
        containerOfPendingRequests.setVisibility(View.GONE);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_wallet_connector_sessions);

        DemoApplication.initialize(this);

        wallet = getWallet(getIntent());

        wallet.getWalletManager().createWalletConnector(
                new CompletionHandler<WalletConnector, WalletConnectorError>() {

                    @Override
                    public void handleData(WalletConnector connector) {
                        walletConnect = new WalletConnect(wallet.getWalletManager().getNetwork().isMainnet(),
                                                          connector,
                                                          DemoApplication.getPaperKey());
                    }

                    @Override
                    public void handleError(WalletConnectorError error) {
                        runOnUiThread(() -> {
                            new AlertDialog.Builder(WalletConnectorSessionsActivity.this)
                                    .setTitle("Error")
                                    .setMessage("Failed to initialize WalletConnector: " + error.toString())
                                    .setPositiveButton("Ok", (d, w) -> {}).show();
                        });
                    }
                }
        );
        if (null == wallet) {
            finish();
            return;
        }

        containerOfSessionInfo = findViewById(R.id.sessionInfo_layout);
        topicTextView = findViewById(R.id.topicTextView);
        clientIdTextView = findViewById(R.id.clientIdTextView);
        peerIdTextView = findViewById(R.id.peerIdTextView);
        connectedAppTextView = findViewById(R.id.connectedAppTextView);
        containerOfPendingRequests = findViewById(R.id.pendingCallRequestsLayout);
        connectToBridgeButton = findViewById(R.id.connectToBridgeView);
        connectToBridgeButton.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                String uri = dAppUriEditText.getText().toString();
                WalletConnectSessionDescription sessionInfo = WalletConnectSessionDescription.createFromWalletConnectUri(uri);
                walletConnect.connect(sessionInfo,
                                      new UserSessionClient(WalletConnectorSessionsActivity.this));
            }
        });

        disconnectFromBridgeButton = findViewById(R.id.disconnectfrombridge_view);
        disconnectFromBridgeButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                walletConnect.disconnect();
                setUiDisconnected();
            }
        });

        dAppUriEditText = findViewById(R.id.dAppUri_view);
        dAppUriEditText.addTextChangedListener( new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {

                // Like: wc:6778992f-dbec-4fa2-a29a-f7da0bcd8268@1?bridge=https%3A%2F%2Fv.bridge.walletconnect.org&key=3fdc05843fc19940696295914cb39f0638ad94d23c0ac489d1e08c968d888978
                String uriCandidate = dAppUriEditText.getText().toString();
                if (WalletConnectSessionDescription.createFromWalletConnectUri(uriCandidate) != null) {
                    connectToBridgeButton.setEnabled(true);
                }
            }
        });

        // Pending requests
        RecyclerView requestsView = findViewById(R.id.pendingRequestsRecyclerView);
        requestsView.addItemDecoration(new DividerItemDecoration(getApplicationContext(), DividerItemDecoration.VERTICAL));
        RecyclerView.LayoutManager transferLayoutManager = new LinearLayoutManager(this, LinearLayoutManager.VERTICAL, false);
        requestsView.setLayoutManager(transferLayoutManager);

        pendingRequestsAdapter = new PendingRequestsAdapter((pendingRequest) -> {

            // Show a dialog for reject/approve/confirm
            Log.log(Level.FINE, "WC: Selected rpcid " + pendingRequest.requestId);
            StringBuilder builder = new StringBuilder();
            builder.append("Type: " + pendingRequest.requestType + "\n");
            builder.append("Params:\n");
            for (String key : pendingRequest.requestData.keySet()) {
                builder.append("  " + key + ": " + pendingRequest.requestData.get(key) + "\n");
            }
            Log.log(Level.FINE, "WC: User to bless:" + builder.toString());
            new AlertDialog.Builder(WalletConnectorSessionsActivity.this)
                    .setTitle("Approve Request " + pendingRequest.requestId)
                    .setMessage(builder.toString())
                    .setNegativeButton("Reject", (d, w) -> {
                        pendingRequest.callback.reject();
                        PendingRequestViewModel vm = PendingRequestViewModel.create(pendingRequest);
                        pendingRequestsAdapter.remove(vm);
                    })
                    .setPositiveButton("Approve", (d, w) -> {
                        pendingRequest.callback.approve();
                        PendingRequestViewModel vm = PendingRequestViewModel.create(pendingRequest);
                        pendingRequestsAdapter.remove(vm);
                    })
                    .setNeutralButton("Cancel", (d, w) -> { /* do nothing.. */ }).show();
        });

        requestsView.setAdapter(pendingRequestsAdapter);

        Toolbar toolbar = findViewById(R.id.wc_toolbar_view);
        toolbar.setTitle(String.format("Wallet Connect for %s", wallet.getName()));
        setSupportActionBar(toolbar);

        setUiDisconnected();
    }

    @Override
    protected void onResume() {
        super.onResume();

       // DemoApplication.getDispatchingSystemListener().addWalletListener(wallet, this);
       // loadTransfers();
    }

    @Override
    protected void onPause() {
      //  DemoApplication.getDispatchingSystemListener().removeWalletListener(wallet, this);

        super.onPause();
    }

    // Borrowing liberally from TransferListActivity. Hem!
    private static class PendingRequestItem {
        final Number requestId;
        final String requestType;
        final Map<String, String> requestData;

        // Not part of the request per-se
        // Maintained to facilitate the callback when the item is selected
        final WalletConnect.RequestApprover callback;

        PendingRequestItem(Number requestId,
                           String requestType,
                           Map<String, String> requestData,
                           WalletConnect.RequestApprover requestApproval) {
            this.requestId = requestId;
            this.requestType = requestType;
            this.requestData = requestData;
            this.callback = requestApproval;
        }

        int compare(PendingRequestItem other) {
            // No two items should have the same requestId...
            if (other.requestId.longValue() < requestId.longValue())
                return -1;
            if (other.requestId.longValue() > requestId.longValue())
                return +1;

            // Shouldn't be here... No two requests are the same
            return 0;
        }

        @Override
        public boolean equals(Object o) {
            if (!(o instanceof PendingRequestItem))
                return false;
            if (this == o)
                return true;

            PendingRequestItem pri = (PendingRequestItem)o;
            return requestId.equals(pri.requestId) &&
                   requestType.equals(pri.requestType) &&
                   requestData.equals(pri.requestData);
        }
    }

    private interface OnPendingRequestClickListener<T> {
        void onItemClick(T item);
    }

    private static class PendingRequestViewModel {
        final PendingRequestItem pendingItem;
        final Supplier<String> requestIdText;
        final Supplier<String> requestTypeText;

        static int compare(PendingRequestViewModel vm1, PendingRequestViewModel vm2) {
            return vm1.pendingItem.compare(vm2.pendingItem);
        }

        static boolean areContentsTheSame(PendingRequestViewModel vm1, PendingRequestViewModel vm2) {
            return vm1.requestIdText().equals(vm2.requestIdText()) &&
                   vm1.requestTypeText().equals(vm2.requestTypeText());
        }

        static boolean areItemsTheSame(PendingRequestViewModel vm1, PendingRequestViewModel vm2) {
            return vm1.pendingItem.equals(vm2.pendingItem);
        }

        static PendingRequestViewModel create(PendingRequestItem item) {
            return new PendingRequestViewModel(item);
        }

        PendingRequestViewModel(PendingRequestItem item) {
            this.pendingItem = item;
            this.requestIdText = Suppliers.memoize(() -> item.requestId.toString());
            this.requestTypeText = Suppliers.memoize(() -> item.requestType);
        }

        String requestIdText() { return requestIdText.get(); }
        String requestTypeText() { return requestTypeText.get(); }
    }

    private static class PendingRequestViewModelSortedListAdapterCallback extends SortedListAdapterCallback<PendingRequestViewModel> {

        PendingRequestViewModelSortedListAdapterCallback(PendingRequestsAdapter adapter) {
            super(adapter);
        }

        @Override
        public int compare(PendingRequestViewModel t1, PendingRequestViewModel t2) {
            return PendingRequestViewModel.compare(t1, t2);
        }

        @Override
        public boolean areContentsTheSame(PendingRequestViewModel t1, PendingRequestViewModel t2) {
            return PendingRequestViewModel.areContentsTheSame(t1, t2);
        }

        @Override
        public boolean areItemsTheSame(PendingRequestViewModel t1, PendingRequestViewModel t2) {
            return PendingRequestViewModel.areItemsTheSame(t1, t2);
        }
    }

    private static class PendingRequestsAdapter extends RecyclerView.Adapter<PendingRequestViewHolder> {

        final OnPendingRequestClickListener<PendingRequestItem> listener;
        final SortedList<PendingRequestViewModel> viewModels;

        PendingRequestsAdapter(OnPendingRequestClickListener<PendingRequestItem> listener) {
            this.listener = listener;
            this.viewModels = new SortedList<>(PendingRequestViewModel.class, new PendingRequestViewModelSortedListAdapterCallback(this));
        }

        @NonNull
        @Override
        public PendingRequestViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
            return new PendingRequestViewHolder(
                    LayoutInflater.from(
                            viewGroup.getContext()
                    ).inflate(
                            R.layout.layout_walletconnect_session,
                            viewGroup,
                            false
                    )
            );
        }

        @Override
        public void onBindViewHolder(@NonNull PendingRequestViewHolder vh, int i) {
            PendingRequestViewModel vm = viewModels.get(i);

            vh.itemView.setOnClickListener(v -> listener.onItemClick(vm.pendingItem));
            vh.requestIdView.setText(vm.requestIdText());
            vh.requestTypeView.setText(vm.requestTypeText());
        }

        @Override
        public int getItemCount() {
            return viewModels.size();
        }

        void add(PendingRequestViewModel pendingRequest) {
            viewModels.add(pendingRequest);
        }

        void remove(PendingRequestViewModel pendingRequest) {
           viewModels.remove(pendingRequest);
        }
    }

    private static class PendingRequestViewHolder extends RecyclerView.ViewHolder {

        TextView requestIdView;
        TextView requestTypeView;

        PendingRequestViewHolder(@NonNull View view) {
            super(view);
            requestIdView = view.findViewById(R.id.requestId_view);
            requestTypeView = view.findViewById(R.id.requestType_view);
        }
    }
}
