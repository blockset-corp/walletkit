/*
 * Created by Bryan Goring <bryan.goring@breadwallet.com> on 07/05/21.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.demo;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import android.text.InputType;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;

import com.blockset.walletkit.Address;
import com.blockset.walletkit.Key;
import com.blockset.walletkit.ExportablePaperWallet;
import com.blockset.walletkit.Wallet;
import com.blockset.walletkit.errors.ExportablePaperWalletError;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.base.Optional;

import java.util.logging.Logger;

public class TransferCreateExportablePaperActivity extends AppCompatActivity {

    private static final Logger Log = Logger.getLogger(TransferCreateExportablePaperActivity.class.getName());
    private static final String EXTRA_WALLET_NAME = "com.blockset.walletkit.demo,TransferCreateExportablePaperActivity.EXTRA_WALLET_NAME";

    private Wallet      wallet;
    private EditText    address;
    private EditText    privateKey;
    private TextView    outcome;
    private ImageView   status;

    public static void start(Activity callerActivity, Wallet wallet) {
        Intent intent = new Intent(callerActivity, TransferCreateExportablePaperActivity.class);
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

    private void indicateFailure(String failMsg) {
        status.setVisibility(View.VISIBLE);
        outcome.setVisibility(View.VISIBLE);
        outcome.setText(failMsg);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_transfer_create_exportablepaper);

        DemoApplication.initialize(this);

        Intent intent = getIntent();

        wallet = getWallet(intent);
        if (null == wallet) {
            finish();
            return;
        }

        address = findViewById(R.id.account_address_view);
        address.setTextIsSelectable(true);
        address.setInputType(InputType.TYPE_NULL);
        privateKey = findViewById(R.id.account_pk_view);
        privateKey.setTextIsSelectable(true);
        privateKey.setInputType(InputType.TYPE_NULL);
        outcome = findViewById(R.id.export_outcome_view);
        outcome.setVisibility(View.GONE);
        status = findViewById(R.id.export_status_view);
        status.setVisibility(View.GONE);
        wallet.getWalletManager().createExportablePaperWallet(
                new CompletionHandler<ExportablePaperWallet, ExportablePaperWalletError>() {

                    @Override
                    public void handleData(ExportablePaperWallet data) {
                        boolean missing = false;
                        Optional<? extends Address> addr = data.getAddress();
                        Optional<? extends Key> key = data.getKey();
                        if (addr.isPresent())
                            address.setText(addr.get().toString());
                        else
                            missing = true;
                        if (key.isPresent())
                            privateKey.setText(new String(key.get().encodeAsPrivate()));
                        else
                            missing = true;

                        if (missing)
                            indicateFailure("Failed: missing field");
                    }

                    @Override
                    public void handleError(ExportablePaperWalletError error) {
                        indicateFailure("Failed: " + error.getMessage());
                    }
                }
        );

        Toolbar toolbar = findViewById(R.id.export_toolbar_view);
        toolbar.setTitle("Exportable Paper Wallet");
        setSupportActionBar(toolbar);
    }
}




